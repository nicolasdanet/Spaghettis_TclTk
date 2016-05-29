
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
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"
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

struct _boxtext {
    struct _boxtext     *box_next;
    t_object            *box_object;
    t_glist             *box_glist;
    char                *box_string;                /* Unzeroed string UTF-8 formatted. */
    int                 box_stringSizeInBytes;
    int                 box_selectionStart; 
    int                 box_selectionEnd;
    int                 box_draggedFrom;
    int                 box_isActive;
    int                 box_widthInPixels;
    int                 box_heightInPixels;
    char                box_tag[BOX_TAG_SIZE];
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_MARGIN_LEFT         2
#define BOX_MARGIN_RIGHT        2
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

static void boxtext_sendTypesetEllipsis (t_boxtext *x)
{
    t_object *o = x->box_object;
    
    if (o->te_type == TYPE_ATOM && o->te_width && x->box_stringSizeInBytes > o->te_width) {
    //
    int i, n = PD_MIN (o->te_width, 3);
    PD_ASSERT (o->te_width > 0);
    x->box_string = PD_MEMORY_RESIZE (x->box_string, x->box_stringSizeInBytes, n);
    x->box_stringSizeInBytes = n;
    for (i = 0; i < x->box_stringSizeInBytes; i++) { x->box_string[i] = '.'; }
    //
    }
}

static int boxtext_sendTypeset (t_boxtext *x,
    int positionX, 
    int positionY,
    int fontSize,
    char *buffer,
    int bufferSize,
    int *selectionStart, 
    int *selectionEnd, 
    int *widthInPixels, 
    int *heightInPixels)
{
    boxtext_sendTypesetEllipsis (x);
    
    {
    //
    int bufferPosition          = 0;
    int widthInCharacters       = x->box_object->te_width;
    int numberOfCharacters      = u8_charnum (x->box_string, x->box_stringSizeInBytes);
    int fontWidth               = font_getHostFontWidth (fontSize);
    int fontHeight              = font_getHostFontHeight (fontSize);
    int lineLengthInCharacters  = (widthInCharacters ? widthInCharacters : BOX_DEFAULT_LINE);
    int indexOfMouse            = -1;
    int numberOfLines           = 0;
    int numberOfColumns         = 0;
    int headInBytes             = 0;
    int charactersThatRemains   = numberOfCharacters;
        
    while (charactersThatRemains > 0) { 
    //
    char *head                  = x->box_string + headInBytes;
    
    int charactersToConsider    = PD_MIN (lineLengthInCharacters, charactersThatRemains);
    int bytesToConsider         = u8_offset (head, charactersToConsider);
    int charactersUntilWrap     = 0;
    int bytesUntilWrap          = string_indexOfFirstOccurrenceUntil (head, '\n', bytesToConsider);
    int accumulatedOffset       = bufferPosition - headInBytes;
    
    int eatCharacter = 1;       /* Remove the character used to wrap (i.e. space and new line). */
    
    if (bytesUntilWrap >= 0) { charactersUntilWrap = u8_charnum (head, bytesUntilWrap); }
    else {
        if (charactersThatRemains > lineLengthInCharacters) {
        
            bytesUntilWrap = string_indexOfFirstOccurrenceFrom (head, ' ', bytesToConsider + 1);
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
    
    if (numberOfLines == (int)(positionY / fontHeight)) {
        int k = positionX / fontWidth;
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
    
    *widthInPixels  = (BOX_MARGIN_LEFT + BOX_MARGIN_RIGHT) + (numberOfColumns * fontWidth);
    *heightInPixels = (BOX_MARGIN_TOP + BOX_MARGIN_BOTTOM) + (numberOfLines * fontHeight);
    
    buffer[bufferPosition] = 0;
    
    return indexOfMouse;
    //
    }
}

static int boxtext_send (t_boxtext *x, int action, int positionX, int positionY)
{
    int isCanvas            = (pd_class (x->box_object) == canvas_class);
    int fontSize            = canvas_getFontSize (isCanvas ? cast_glist (x->box_object) : x->box_glist);
    int widthInPixels       = 0;
    int heightInPixels      = 0;
    int selectionStart      = 0;
    int selectionEnd        = 0;
    int bufferSize          = PD_MAX (BOX_DEFAULT_WIDTH, (2 * x->box_stringSizeInBytes)) + 1;
    char *buffer            = (char *)PD_MEMORY_GET (bufferSize);
        
    int indexOfMouse        = boxtext_sendTypeset (x,
                                    positionX, 
                                    positionY,
                                    fontSize,
                                    buffer,
                                    bufferSize,
                                    &selectionStart, 
                                    &selectionEnd, 
                                    &widthInPixels, 
                                    &heightInPixels);

    t_glist *view = canvas_getView (x->box_glist);
    int isSelected = canvas_isObjectSelected (x->box_glist, cast_gobj (x->box_object));
    
    if (action == BOX_FIRST) {
        sys_vGui ("::ui_object::newText .x%lx.c %s %f %f {%s} %d #%06x\n",
                        view,
                        x->box_tag,
                        (double)text_xpix (x->box_object, x->box_glist) + BOX_MARGIN_LEFT, 
                        (double)text_ypix (x->box_object, x->box_glist) + BOX_MARGIN_TOP,
                        buffer, 
                        font_getHostFontSize (fontSize),
                        (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
                
    } else if (action == BOX_UPDATE) {
        sys_vGui ("::ui_object::setText .x%lx.c %s {%s}\n",
                    view,
                    x->box_tag,
                    buffer);
                    
        if (widthInPixels != x->box_widthInPixels || heightInPixels != x->box_heightInPixels) {
            text_drawborder (x->box_object, x->box_glist, x->box_tag, widthInPixels, heightInPixels, 0);
        }
                
        if (x->box_isActive) {
        
            if (selectionStart < selectionEnd) {
                sys_vGui (".x%lx.c select from %s %d\n",
                            view, 
                            x->box_tag,
                            u8_charnum (x->box_string, selectionStart));
                sys_vGui (".x%lx.c select to %s %d\n",
                            view, 
                            x->box_tag,
                            u8_charnum (x->box_string, selectionEnd) - 1);
                sys_vGui (".x%lx.c focus \"\"\n",
                            view);
                
            } else {
                sys_vGui (".x%lx.c select clear\n",
                            view);
                sys_vGui (".x%lx.c icursor %s %d\n",
                            view,
                            x->box_tag,
                            u8_charnum (x->box_string, selectionStart));
                sys_vGui (".x%lx.c focus %s\n",
                            view,
                            x->box_tag);        
            }
        }
    }
    
    x->box_widthInPixels  = widthInPixels;
    x->box_heightInPixels = heightInPixels;
    
    PD_MEMORY_FREE (buffer);
    
    return indexOfMouse;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_boxtext *boxtext_new (t_glist *glist, t_object *object)
{
    t_boxtext *x  = (t_boxtext *)PD_MEMORY_GET (sizeof (t_boxtext));

    x->box_next   = glist->gl_editor->e_text;
    x->box_object = object;
    x->box_glist  = glist;

    buffer_toStringUnzeroed (object->te_buffer, &x->box_string, &x->box_stringSizeInBytes);
    
    { 
        t_glist *view = canvas_getView (glist);
        t_error err   = string_sprintf (x->box_tag, BOX_TAG_SIZE, ".x%lx.t%lx", (t_int)view, (t_int)x);
        PD_ASSERT (!err);
    }
    
    glist->gl_editor->e_text = x;
    
    return x;
}

void boxtext_free (t_boxtext *x)
{
    if (x->box_glist->gl_editor->e_selectedText == x) {
        x->box_glist->gl_editor->e_selectedText = NULL;
    }
    
    if (x->box_glist->gl_editor->e_text == x) { x->box_glist->gl_editor->e_text = x->box_next; }
    else {
        t_boxtext *t = NULL;
        for (t = x->box_glist->gl_editor->e_text; t; t = t->box_next) {
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
#pragma mark -

t_boxtext *boxtext_fetch (t_glist *glist, t_object *object)
{
    t_boxtext *x = NULL;
    
    canvas_createEditorIfNone (glist);
    
    for (x = glist->gl_editor->e_text; x && x->box_object != object; x = x->box_next) { }
    
    PD_ASSERT (x);
    
    return x;
}

void boxtext_update (t_boxtext *x)
{
    PD_MEMORY_FREE (x->box_string);
    
    buffer_toStringUnzeroed (x->box_object->te_buffer, &x->box_string, &x->box_stringSizeInBytes);
    
    boxtext_send (x, BOX_UPDATE, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *boxtext_getTag (t_boxtext *x)
{
    return x->box_tag;
}

int boxtext_getWidth (t_boxtext *x)
{
    boxtext_send (x, BOX_CHECK, 0, 0);
    return x->box_widthInPixels;
}

int boxtext_getHeight (t_boxtext *x)
{
    boxtext_send (x, BOX_CHECK, 0, 0);
    return x->box_heightInPixels;
}

void boxtext_getText (t_boxtext *x, char **p, int *size)
{
    *p    = x->box_string;
    *size = x->box_stringSizeInBytes;
}

void boxtext_getSelection (t_boxtext *x, char **p, int *size)
{
    *p    = x->box_string + x->box_selectionStart;
    *size = x->box_selectionEnd - x->box_selectionStart;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void boxtext_draw (t_boxtext *x)
{
    boxtext_send (x, BOX_FIRST, 0, 0);
}

void boxtext_erase (t_boxtext *x)
{
    sys_vGui (".x%lx.c delete %s\n", 
                    canvas_getView (x->box_glist), 
                    x->box_tag);
}

void boxtext_displace (t_boxtext *x, int deltaX, int deltaY)
{
    sys_vGui (".x%lx.c move %s %d %d\n", 
                    canvas_getView (x->box_glist), 
                    x->box_tag, 
                    deltaX, 
                    deltaY);
}

void boxtext_select (t_boxtext *x, int state)
{
    sys_vGui (".x%lx.c itemconfigure %s -fill #%06x\n",
                    canvas_getView (x->box_glist), 
                    x->box_tag, 
                    (state ? COLOR_SELECTED : COLOR_NORMAL));
}

void boxtext_activate (t_boxtext *x, int state)
{
    if (state) {
    //
    sys_vGui ("::ui_object::setEditing .x%lx %s 1\n", canvas_getView (x->box_glist), x->box_tag);
                    
    x->box_glist->gl_editor->e_selectedText = x;
    x->box_glist->gl_editor->e_isTextDirty  = 0;
    
    x->box_draggedFrom      = 0;
    x->box_selectionStart   = 0;
    x->box_selectionEnd     = x->box_stringSizeInBytes;
    x->box_isActive         = 1;
    //
    } else {
    //
    sys_vGui ("::ui_object::setEditing .x%lx {} 0\n", canvas_getView (x->box_glist));
                    
    if (x->box_glist->gl_editor->e_selectedText == x) { x->box_glist->gl_editor->e_selectedText = NULL; }
    
    x->box_isActive = 0;
    //
    }

    boxtext_send (x, BOX_UPDATE, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rtext_key(t_boxtext *x, int keynum, t_symbol *keysym)
{
    int i, newsize, ndel;
    char *s1, *s2;
    if (keynum)
    {
        int n = keynum;
        if (n == '\r') n = '\n';
        if (n == '\b')  /* backspace */
        {
                    /* LATER delete the box if all text is selected...
                    this causes reentrancy problems now. */
            /* if ((!x->box_selectionStart) && (x->box_selectionEnd == x->box_stringSizeInBytes))
            {
                ....
            } */
            if (x->box_selectionStart && (x->box_selectionStart == x->box_selectionEnd))
                u8_dec(x->box_string, &x->box_selectionStart);
        }
        else if (n == 127)      /* delete */
        {
            if (x->box_selectionEnd < x->box_stringSizeInBytes && (x->box_selectionStart == x->box_selectionEnd))
                u8_inc(x->box_string, &x->box_selectionEnd);
        }
        
        ndel = x->box_selectionEnd - x->box_selectionStart;
        for (i = x->box_selectionEnd; i < x->box_stringSizeInBytes; i++)
            x->box_string[i- ndel] = x->box_string[i];
        newsize = x->box_stringSizeInBytes - ndel;
        x->box_string = PD_MEMORY_RESIZE(x->box_string, x->box_stringSizeInBytes, newsize);
        x->box_stringSizeInBytes = newsize;

/* at Guenter's suggestion, use 'n>31' to test wither a character might
be printable in whatever 8-bit character set we find ourselves. */

/*-- moo:
  ... but test with "<" rather than "!=" in order to accomodate unicode
  codepoints for n (which we get since Tk is sending the "%A" substitution
  for bind <Key>), effectively reducing the coverage of this clause to 7
  bits.  Case n>127 is covered by the next clause.
*/
        if (n == '\n' || (n > 31 && n < 127))
        {
            newsize = x->box_stringSizeInBytes+1;
            x->box_string = PD_MEMORY_RESIZE(x->box_string, x->box_stringSizeInBytes, newsize);
            for (i = x->box_stringSizeInBytes; i > x->box_selectionStart; i--)
                x->box_string[i] = x->box_string[i-1];
            x->box_string[x->box_selectionStart] = n;
            x->box_stringSizeInBytes = newsize;
            x->box_selectionStart = x->box_selectionStart + 1;
        }
        /*--moo: check for unicode codepoints beyond 7-bit ASCII --*/
        else if (n > 127)
        {
            int ch_nbytes = u8_wc_nbytes(n);
            newsize = x->box_stringSizeInBytes + ch_nbytes;
            x->box_string = PD_MEMORY_RESIZE(x->box_string, x->box_stringSizeInBytes, newsize);
            for (i = newsize-1; i > x->box_selectionStart; i--)
                x->box_string[i] = x->box_string[i-ch_nbytes];
            x->box_stringSizeInBytes = newsize;
            /*-- moo: assume canvas_key() has encoded keysym as UTF-8 */
            strncpy(x->box_string+x->box_selectionStart, keysym->s_name, ch_nbytes);
            x->box_selectionStart = x->box_selectionStart + ch_nbytes;
        }
        x->box_selectionEnd = x->box_selectionStart;
        x->box_glist->gl_editor->e_isTextDirty = 1;
    }
    else if (!strcmp(keysym->s_name, "Right"))
    {
        if (x->box_selectionEnd == x->box_selectionStart && x->box_selectionStart < x->box_stringSizeInBytes)
        {
            u8_inc(x->box_string, &x->box_selectionStart);
            x->box_selectionEnd = x->box_selectionStart;
        }
        else
            x->box_selectionStart = x->box_selectionEnd;
    }
    else if (!strcmp(keysym->s_name, "Left"))
    {
        if (x->box_selectionEnd == x->box_selectionStart && x->box_selectionStart > 0)
        {
            u8_dec(x->box_string, &x->box_selectionStart);
            x->box_selectionEnd = x->box_selectionStart;
        }
        else
            x->box_selectionEnd = x->box_selectionStart;
    }
        /* this should be improved...  life's too short */
    else if (!strcmp(keysym->s_name, "Up"))
    {
        if (x->box_selectionStart)
            u8_dec(x->box_string, &x->box_selectionStart);
        while (x->box_selectionStart > 0 && x->box_string[x->box_selectionStart] != '\n')
            u8_dec(x->box_string, &x->box_selectionStart);
        x->box_selectionEnd = x->box_selectionStart;
    }
    else if (!strcmp(keysym->s_name, "Down"))
    {
        while (x->box_selectionEnd < x->box_stringSizeInBytes &&
            x->box_string[x->box_selectionEnd] != '\n')
            u8_inc(x->box_string, &x->box_selectionEnd);
        if (x->box_selectionEnd < x->box_stringSizeInBytes)
            u8_inc(x->box_string, &x->box_selectionEnd);
        x->box_selectionStart = x->box_selectionEnd;
    }

    boxtext_send(x, BOX_UPDATE, 0, 0);
}

void rtext_mouse(t_boxtext *x, int xval, int yval, int flag)
{
    int indx = boxtext_send (x, BOX_CHECK, xval, yval);
    
    if (flag == BOXTEXT_DOWN)
    {
        x->box_draggedFrom = x->box_selectionStart = x->box_selectionEnd = indx;
    }
    else if (flag == BOXTEXT_DOUBLE)
    {
        int whereseparator, newseparator;
        x->box_draggedFrom = -1;
        whereseparator = 0;
        if ((newseparator = string_indexOfFirstOccurrenceFrom(x->box_string, ' ', indx)) > whereseparator)
            whereseparator = newseparator+1;
        if ((newseparator = string_indexOfFirstOccurrenceFrom(x->box_string, '\n', indx)) > whereseparator)
            whereseparator = newseparator+1;
        if ((newseparator = string_indexOfFirstOccurrenceFrom(x->box_string, ';', indx)) > whereseparator)
            whereseparator = newseparator+1;
        if ((newseparator = string_indexOfFirstOccurrenceFrom(x->box_string, ',', indx)) > whereseparator)
            whereseparator = newseparator+1;
        x->box_selectionStart = whereseparator;
        
        whereseparator = x->box_stringSizeInBytes - indx;
        if ((newseparator =
            string_indexOfFirstOccurrenceUntil(x->box_string+indx, ' ', x->box_stringSizeInBytes - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        if ((newseparator =
            string_indexOfFirstOccurrenceUntil(x->box_string+indx, '\n', x->box_stringSizeInBytes - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        if ((newseparator =
            string_indexOfFirstOccurrenceUntil(x->box_string+indx, ';', x->box_stringSizeInBytes - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        if ((newseparator =
            string_indexOfFirstOccurrenceUntil(x->box_string+indx, ',', x->box_stringSizeInBytes - indx)) >= 0 &&
                newseparator < whereseparator)
                    whereseparator = newseparator;
        x->box_selectionEnd = indx + whereseparator;
    }
    else if (flag == BOXTEXT_SHIFT)
    {
        if (indx * 2 > x->box_selectionStart + x->box_selectionEnd)
            x->box_draggedFrom = x->box_selectionStart, x->box_selectionEnd = indx;
        else
            x->box_draggedFrom = x->box_selectionEnd, x->box_selectionStart = indx;
    }
    else if (flag == BOXTEXT_DRAG)
    {
        if (x->box_draggedFrom < 0)
            return;
        x->box_selectionStart = (x->box_draggedFrom < indx ? x->box_draggedFrom : indx);
        x->box_selectionEnd = (x->box_draggedFrom > indx ? x->box_draggedFrom : indx);
    }
    boxtext_send(x, BOX_UPDATE, xval, yval);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
