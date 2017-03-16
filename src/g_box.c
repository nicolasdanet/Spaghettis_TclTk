
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_TAG_SIZE    50

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _box {
    struct _box     *box_next;
    t_object        *box_object;
    t_glist         *box_glist;
    char            *box_string;                /* Unzeroed string UTF-8 formatted. */
    int             box_stringSizeInBytes;
    int             box_selectionStart; 
    int             box_selectionEnd;
    int             box_draggedFrom;
    int             box_isActivated;
    int             box_widthInPixels;
    int             box_heightInPixels;
    int             box_checked;
    char             box_tag[BOX_TAG_SIZE];
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_MARGIN_LEFT         2
#define BOX_MARGIN_RIGHT        5
#define BOX_MARGIN_TOP          2
#define BOX_MARGIN_BOTTOM       2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOX_CHECK               0
#define BOX_FIRST               1
#define BOX_UPDATE              2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOX_DEFAULT_LINE        60
#define BOX_DEFAULT_WIDTH       3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void box_ellipsis (t_box *x)
{
    t_object *o = x->box_object;
    
    if (object_isAtom (o)) {
        if ((object_getWidth (o) > 0) && (x->box_stringSizeInBytes > object_getWidth (o))) {
            x->box_string = PD_MEMORY_RESIZE (x->box_string, x->box_stringSizeInBytes, object_getWidth (o));
            x->box_stringSizeInBytes = object_getWidth (o);
            x->box_string[x->box_stringSizeInBytes - 1] = '*';
        }
    }
}

static int box_typeset (t_box *x,
    int a, 
    int b,
    int fontSize,
    char *buffer,
    int bufferSize,
    int *selectionStart, 
    int *selectionEnd, 
    int *widthInPixels, 
    int *heightInPixels)
{
    box_ellipsis (x);
    
    {
    //
    int     bufferPosition          = 0;
    int     widthInCharacters       = object_getWidth (x->box_object);
    int     numberOfCharacters      = u8_charnum (x->box_string, x->box_stringSizeInBytes);
    double  fontWidth               = font_getHostFontWidth (fontSize);
    double  fontHeight              = font_getHostFontHeight (fontSize);
    int     lineLengthInCharacters  = (widthInCharacters ? widthInCharacters : BOX_DEFAULT_LINE);
    int     indexOfMouse            = -1;
    int     numberOfLines           = 0;
    int     numberOfColumns         = 0;
    int     headInBytes             = 0;
    int     charactersThatRemains   = numberOfCharacters;
        
    while (charactersThatRemains > 0) { 
    //
    char *head                      = x->box_string + headInBytes;
    
    int charactersToConsider        = PD_MIN (lineLengthInCharacters, charactersThatRemains);
    int bytesToConsider             = u8_offset (head, charactersToConsider);
    int charactersUntilWrap         = 0;
    int bytesUntilWrap              = string_indexOfFirstOccurrenceUntil (head, "\n", bytesToConsider);
    int accumulatedOffset           = bufferPosition - headInBytes;
        
    int eatCharacter = 1;           /* Remove the character used to wrap (i.e. space and new line). */
    
    if (bytesUntilWrap >= 0) { charactersUntilWrap = u8_charnum (head, bytesUntilWrap); }
    else {
        if (charactersThatRemains > lineLengthInCharacters) {
        
            bytesUntilWrap = string_indexOfFirstOccurrenceFrom (head, " ", bytesToConsider + 1);
            if (bytesUntilWrap >= 0) { charactersUntilWrap = u8_charnum (head, bytesUntilWrap); }
            else {
                charactersUntilWrap = charactersToConsider;
                bytesUntilWrap = bytesToConsider;
                eatCharacter = 0;
            }
            
        } else {
            charactersUntilWrap = charactersThatRemains;
            bytesUntilWrap = x->box_stringSizeInBytes - headInBytes;
            eatCharacter = 0;
        }
    }

    /* Locate the insertion point. */
    
    if (numberOfLines == (int)(b / fontHeight)) {
        int k = (int)((a / fontWidth) + 0.5);
        indexOfMouse = headInBytes + u8_offset (head, PD_CLAMP (k, 0, charactersUntilWrap));
    }
    
    /* Deplace selection points according new characters insertion. */
    
    if (x->box_selectionStart >= headInBytes) {         
        if (x->box_selectionStart <= headInBytes + bytesUntilWrap + eatCharacter) {
            *selectionStart = accumulatedOffset + x->box_selectionStart;
        }
    }
            
    if (x->box_selectionEnd >= headInBytes) {
        if (x->box_selectionEnd <= headInBytes + bytesUntilWrap + eatCharacter) {
            *selectionEnd   = accumulatedOffset + x->box_selectionEnd;
        }
    }
    
    /* Append line to the buffer and continue next. */
    
    PD_ASSERT ((bufferPosition + bytesUntilWrap) < (bufferSize - 1));
    
    strncpy (buffer + bufferPosition, head, bytesUntilWrap);
        
    bufferPosition          += bytesUntilWrap;
    headInBytes             += bytesUntilWrap;
    headInBytes             += eatCharacter;
    charactersThatRemains   -= charactersUntilWrap;
    charactersThatRemains   -= eatCharacter;
    
    if (headInBytes < x->box_stringSizeInBytes) { buffer[bufferPosition++] = '\n'; }
    if (charactersUntilWrap > numberOfColumns)  { numberOfColumns = charactersUntilWrap; }
        
    numberOfLines++;
    //
    }
    
    if (indexOfMouse < 0)  { indexOfMouse = bufferPosition; }
    if (numberOfLines < 1) { numberOfLines = 1; }
    
    if (widthInCharacters) { numberOfColumns = widthInCharacters; } 
    else {
        while (numberOfColumns < BOX_DEFAULT_WIDTH) {
            PD_ASSERT (bufferPosition < bufferSize);
            buffer[bufferPosition++] = ' ';
            numberOfColumns++;
        }
    }
    
    *widthInPixels  = (int)((BOX_MARGIN_LEFT + BOX_MARGIN_RIGHT) + (numberOfColumns * fontWidth));
    *heightInPixels = (int)((BOX_MARGIN_TOP + BOX_MARGIN_BOTTOM) + (numberOfLines * fontHeight));
    
    buffer[bufferPosition] = 0;
    
    return indexOfMouse;
    //
    }
}

static int box_send (t_box *x, int action, int a, int b)
{
    int isCanvas            = (pd_class (x->box_object) == canvas_class);
    t_fontsize fontSize     = canvas_getFontSize (isCanvas ? cast_glist (x->box_object) : x->box_glist);
    int widthInPixels       = 0;
    int heightInPixels      = 0;
    int selectionStart      = 0;
    int selectionEnd        = 0;
    int bufferSize          = PD_MAX (BOX_DEFAULT_WIDTH, (2 * x->box_stringSizeInBytes)) + 1;
    char *buffer            = (char *)PD_MEMORY_GET (bufferSize);
        
    int indexOfMouse        = box_typeset (x,
                                    a, 
                                    b,
                                    fontSize,
                                    buffer,
                                    bufferSize,
                                    &selectionStart, 
                                    &selectionEnd, 
                                    &widthInPixels, 
                                    &heightInPixels);

    t_glist *canvas = canvas_getView (x->box_glist);
    int isSelected  = canvas_isObjectSelected (x->box_glist, cast_gobj (x->box_object));
        
    int resized = 0;
    
    if (widthInPixels != x->box_widthInPixels || heightInPixels != x->box_heightInPixels) {
        resized = 1;
        x->box_widthInPixels  = widthInPixels;
        x->box_heightInPixels = heightInPixels;
    }
        
    if (action == BOX_CHECK) {
        x->box_checked = 1;                     /* Used once at creation time. */
        
    } else if (action == BOX_FIRST) {
        sys_vGui ("::ui_box::newText .x%lx.c %s %d %d {%s} %d #%06x\n",     // --
                        canvas,
                        x->box_tag,
                        (int)(object_getPixelX (x->box_object, x->box_glist) + BOX_MARGIN_LEFT), 
                        (int)(object_getPixelY (x->box_object, x->box_glist) + BOX_MARGIN_TOP),
                        buffer, 
                        font_getHostFontSize (fontSize),
                        (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
                                
    } else if (action == BOX_UPDATE) {
        sys_vGui ("::ui_box::setText .x%lx.c %s {%s}\n",    // --
                        canvas,
                        x->box_tag,
                        buffer);
                    
        if (resized) { canvas_drawBox (x->box_glist, x->box_object, x->box_tag, 0); }
                
        if (x->box_isActivated) {
        
            if (selectionStart < selectionEnd) {
                sys_vGui (".x%lx.c select from %s %d\n",
                                canvas, 
                                x->box_tag,
                                u8_charnum (x->box_string, selectionStart));
                sys_vGui (".x%lx.c select to %s %d\n",
                                canvas, 
                                x->box_tag,
                                u8_charnum (x->box_string, selectionEnd) - 1);
                sys_vGui (".x%lx.c focus \"\"\n",
                                canvas);
                
            } else {
                sys_vGui (".x%lx.c select clear\n",
                                canvas);
                sys_vGui (".x%lx.c icursor %s %d\n",
                                canvas,
                                x->box_tag,
                                u8_charnum (x->box_string, selectionStart));
                sys_vGui (".x%lx.c focus %s\n",
                                canvas,
                                x->box_tag);        
            }
        }
    }

    PD_MEMORY_FREE (buffer);
    
    return indexOfMouse;
}

static void box_restore (t_box *x)
{
    PD_MEMORY_FREE (x->box_string);
    
    buffer_toStringUnzeroed (object_getBuffer (x->box_object), &x->box_string, &x->box_stringSizeInBytes);
}

static void box_delete (t_box *x)
{
    int toDelete = (x->box_selectionEnd - x->box_selectionStart);

    if (toDelete > 0) {
    //
    int oldSize = x->box_stringSizeInBytes;
    int newSize = x->box_stringSizeInBytes - toDelete;
    int i;
    for (i = x->box_selectionEnd; i < oldSize; i++) { x->box_string[i - toDelete] = x->box_string[i]; }
    x->box_string = PD_MEMORY_RESIZE (x->box_string, oldSize, newSize);
    x->box_stringSizeInBytes = newSize;
    x->box_selectionEnd = x->box_selectionStart;
    x->box_glist->gl_editor->e_isTextDirty = 1;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_box *box_fetch (t_glist *glist, t_object *object)
{
    t_box *x = NULL;
    
    canvas_createEditorIfNone (glist);
    
    for (x = glist->gl_editor->e_boxes; x && x->box_object != object; x = x->box_next) { }
    
    PD_ASSERT (x);
    
    return x;
}

void box_retext (t_glist *glist, t_object *object)
{
    t_box *text = box_fetch (glist, object);
    
    PD_ASSERT (text);
    
    if (text) { box_restore (text); if (canvas_isMapped (glist)) { box_update (text); } }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *box_getTag (t_box *x)
{
    return x->box_tag;
}

int box_getWidth (t_box *x)
{
    if (!x->box_checked) { box_send (x, BOX_CHECK, 0, 0); }
    return x->box_widthInPixels;
}

int box_getHeight (t_box *x)
{
    if (!x->box_checked) { box_send (x, BOX_CHECK, 0, 0); }
    return x->box_heightInPixels;
}

void box_getText (t_box *x, char **p, int *size)
{
    *p    = x->box_string;
    *size = x->box_stringSizeInBytes;
}

void box_getSelection (t_box *x, char **p, int *size)
{
    *p    = x->box_string + x->box_selectionStart;
    *size = x->box_selectionEnd - x->box_selectionStart;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void box_draw (t_box *x)
{
    if (object_isAtom (x->box_object)) { box_restore (x); }
    
    box_send (x, BOX_FIRST, 0, 0);
}

void box_update (t_box *x)
{
    box_send (x, BOX_UPDATE, 0, 0);
}

void box_erase (t_box *x)
{
    sys_vGui (".x%lx.c delete %s\n", 
                    canvas_getView (x->box_glist), 
                    x->box_tag);
}

void box_displace (t_box *x, int deltaX, int deltaY)
{
    sys_vGui (".x%lx.c move %s %d %d\n", 
                    canvas_getView (x->box_glist), 
                    x->box_tag, 
                    deltaX, 
                    deltaY);
}

void box_select (t_box *x, int isSelected)
{
    sys_vGui (".x%lx.c itemconfigure %s -fill #%06x\n",
                    canvas_getView (x->box_glist), 
                    x->box_tag, 
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
}

void box_activate (t_box *x, int state)
{
    if (state) {
    //
    sys_vGui ("::ui_box::setEditing .x%lx %s 1\n", x->box_glist, x->box_tag);
                    
    x->box_glist->gl_editor->e_selectedText = x;
    x->box_glist->gl_editor->e_isTextDirty  = 0;
    
    x->box_draggedFrom      = 0;
    x->box_selectionStart   = 0;
    x->box_selectionEnd     = x->box_stringSizeInBytes;
    x->box_isActivated      = 1;
    //
    } else {
    //
    sys_vGui ("::ui_box::setEditing .x%lx {} 0\n", x->box_glist);   // --
                    
    if (x->box_glist->gl_editor->e_selectedText == x) { x->box_glist->gl_editor->e_selectedText = NULL; }
    
    x->box_isActivated = 0;
    //
    }

    box_send (x, BOX_UPDATE, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that mouse position is relative to the origin of the object. */

void box_mouse (t_box *x, int a, int b, int flag)
{
    int i = box_send (x, BOX_CHECK, a, b);
    
    if (flag == BOX_DOWN) { 

        x->box_draggedFrom    = i;
        x->box_selectionStart = i;
        x->box_selectionEnd   = i;

    } else if (flag == BOX_DOUBLE) {

        int k = x->box_stringSizeInBytes - i;
        int m = string_indexOfFirstOccurrenceFrom (x->box_string, " ;,\n", i);          // --
        int n = string_indexOfFirstOccurrenceUntil (x->box_string + i, " ;,\n", k);     // --
        
        x->box_draggedFrom    = -1;
        x->box_selectionStart = (m == -1) ? 0 : m + 1;
        x->box_selectionEnd   = i + ((n == -1) ? k : n);

    } else if (flag == BOX_SHIFT) {

        if (i > (x->box_selectionStart + x->box_selectionEnd) / 2) {
            x->box_draggedFrom    = x->box_selectionStart;
            x->box_selectionEnd   = i;
        } else {
            x->box_draggedFrom    = x->box_selectionEnd;
            x->box_selectionStart = i;
        }
        
    } else if (flag == BOX_DRAG) {

        if (x->box_draggedFrom >= 0) {
            x->box_selectionStart = (x->box_draggedFrom < i ? x->box_draggedFrom : i);
            x->box_selectionEnd   = (x->box_draggedFrom > i ? x->box_draggedFrom : i);
        }
    }
    
    box_send (x, BOX_UPDATE, a, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int box_keyArrows (t_box *x, t_symbol *s)
{
    if (s == sym_Right) {
    
        if (x->box_selectionEnd == x->box_selectionStart) {
            if (x->box_selectionStart < x->box_stringSizeInBytes) {
                u8_inc (x->box_string, &x->box_selectionStart);
            }
        }  
        
        x->box_selectionEnd = x->box_selectionStart; 
        
    } else if (s == sym_Left) {
    
        if (x->box_selectionEnd == x->box_selectionStart) {
            if (x->box_selectionStart > 0) {
                u8_dec (x->box_string, &x->box_selectionStart);
            }
        }
        
         x->box_selectionEnd = x->box_selectionStart;
    }
    
    return (s == sym_Right || s == sym_Left);
}

static int box_keyDelete (t_box *x, t_symbol *s)
{
    if (s == sym_BackSpace) {                                           /* Backward. */
        if (x->box_selectionStart == x->box_selectionEnd) {
            if (x->box_selectionStart > 0) { 
                u8_dec (x->box_string, &x->box_selectionStart); 
            }
        }
        
    } else if (s == sym_Delete) {                                       /* Forward. */
        if (x->box_selectionStart == x->box_selectionEnd) {
            if (x->box_selectionEnd < x->box_stringSizeInBytes) {
                u8_inc (x->box_string, &x->box_selectionEnd);
            }
        }
    }
    
    if (s == sym_BackSpace || s == sym_Delete) { box_delete (x); return 1; }
    else {
        return 0;
    }
}

static void box_keyASCII (t_box *x, t_keycode n)
{
    box_delete (x);
    
    {
    //
    int i;
    int oldSize = x->box_stringSizeInBytes;
    int newSize = x->box_stringSizeInBytes + 1;
    x->box_string = PD_MEMORY_RESIZE (x->box_string, oldSize, newSize);
    for (i = oldSize; i > x->box_selectionStart; i--) { x->box_string[i] = x->box_string[i - 1]; }
    x->box_string[x->box_selectionStart] = (char)n;
    x->box_stringSizeInBytes = newSize;
    x->box_selectionStart = x->box_selectionStart + 1;
    //
    }
}

static void box_keyCodePoint (t_box *x, t_keycode n, t_symbol *s)
{
    box_delete (x);
    
    {
    //
    int i, k = u8_wc_nbytes (n);
    int oldSize = x->box_stringSizeInBytes;
    int newSize = x->box_stringSizeInBytes + k;
    x->box_string = PD_MEMORY_RESIZE (x->box_string, oldSize, newSize);
    for (i = newSize - 1; i > x->box_selectionStart; i--) { x->box_string[i] = x->box_string[i - k]; }
    x->box_stringSizeInBytes = newSize;
    strncpy (x->box_string + x->box_selectionStart, s->s_name, k);
    x->box_selectionStart = x->box_selectionStart + k;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void box_key (t_box *x, t_keycode n, t_symbol *s)
{
    PD_ASSERT (s);
    
    if (box_keyArrows (x, s)) { }
    else if (box_keyDelete (x, s)) { }
    else {
        if (s == sym_Return || s == sym_Enter)  { box_keyASCII (x, '\n');     }
        else if (n > 31 && n < 127)             { box_keyASCII (x, n);        }
        else if (n > 127)                       { box_keyCodePoint (x, n, s); }

        if (n) {
            x->box_selectionEnd = x->box_selectionStart;
            x->box_glist->gl_editor->e_isTextDirty = 1;
        }
    }

    box_send (x, BOX_UPDATE, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_box *box_new (t_glist *glist, t_object *object)
{
    t_box *x  = (t_box *)PD_MEMORY_GET (sizeof (t_box));

    x->box_next   = glist->gl_editor->e_boxes;
    x->box_object = object;
    x->box_glist  = glist;

    buffer_toStringUnzeroed (object_getBuffer (object), &x->box_string, &x->box_stringSizeInBytes);
    
    {
    //
    t_glist *canvas = canvas_getView (glist);
    t_error err = string_sprintf (x->box_tag, BOX_TAG_SIZE, ".x%lx.%lxBOX", canvas, x);
    PD_ASSERT (!err);
    //
    }
    
    glist->gl_editor->e_boxes = x;
    
    return x;
}

void box_free (t_box *x)
{
    if (x->box_glist->gl_editor->e_selectedText == x) {
        x->box_glist->gl_editor->e_selectedText = NULL;
    }
    
    if (x->box_glist->gl_editor->e_boxes == x) { x->box_glist->gl_editor->e_boxes = x->box_next; }
    else {
        t_box *t = NULL;
        for (t = x->box_glist->gl_editor->e_boxes; t; t = t->box_next) {
            if (t->box_next == x) { 
                t->box_next = x->box_next; break; 
            }
        }
    }

    PD_MEMORY_FREE (x->box_string);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
