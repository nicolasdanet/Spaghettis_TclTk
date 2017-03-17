
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

/* Typsetting text in boxes. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BOX_MARGIN_LEFT         2
#define BOX_MARGIN_RIGHT        5
#define BOX_MARGIN_TOP          2
#define BOX_MARGIN_BOTTOM       2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOX_WIDTH_DEFAULT       60
#define BOX_WIDTH_EMPTY         3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _typesetproperties {
    int         p_x;
    int         p_y;
    t_fontsize  p_fontSize;
    int         p_selectionStart;
    int         p_selectionEnd;
    int         p_widthInPixels; 
    int         p_heightInPixels;
    int         p_bufferSize;
    char        *p_buffer;
    } t_typesetproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_typesetproperties *box_typesetAllocate (t_box *x, int a, int b, t_typesetproperties *p)
{
    int isCanvas = (pd_class (x->box_object) == canvas_class);
    
    /* Allocate engough space to add as many newline it could be required. */
    
    int size = PD_MAX (BOX_WIDTH_EMPTY, (2 * x->box_stringSizeInBytes)) + 1;
    
    p->p_x                  = a;
    p->p_y                  = b;
    p->p_fontSize           = canvas_getFontSize (isCanvas ? cast_glist (x->box_object) : x->box_glist);
    p->p_selectionStart     = 0;
    p->p_selectionEnd       = 0;
    p->p_widthInPixels      = 0; 
    p->p_heightInPixels     = 0;
    p->p_bufferSize         = size;
    p->p_buffer             = (char *)PD_MEMORY_GET (p->p_bufferSize);
    
    return p;
}

static void box_typesetFree (t_typesetproperties *p)
{
    PD_MEMORY_FREE (p->p_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Crop string to a given width in case of gatom. */
/* Assumed that the asterisk (0x2A) is a single octet long. */

static void box_typesetEllipsis (t_box *x)
{
    t_object *o = x->box_object;
    
    if (object_isAtom (o)) {
    //
    int width = object_getWidth (o);
    
    if ((width > 0) && (x->box_stringSizeInBytes > width)) {
    //
    int numberOfCharacters = u8_charnum (x->box_string, x->box_stringSizeInBytes);
    
    if (numberOfCharacters > width) {
    
        int t = u8_offset (x->box_string, width - 1) + 1;
        
        x->box_string = PD_MEMORY_RESIZE (x->box_string, x->box_stringSizeInBytes, t);
        x->box_stringSizeInBytes = t;
        x->box_string[x->box_stringSizeInBytes - 1] = '*';
    }
    //
    }
    //
    }
}

static int box_typesetHasResized (t_box *x, t_typesetproperties *p)
{
    if (p->p_widthInPixels != x->box_widthInPixels || p->p_heightInPixels != x->box_heightInPixels) {
    //
    x->box_widthInPixels  = p->p_widthInPixels;
    x->box_heightInPixels = p->p_heightInPixels;
        
    return 1;
    //
    }
    
    return 0;
}

static int box_typeset (t_box *x, t_typesetproperties *p)
{
    box_typesetEllipsis (x);
    
    {
    //
    int     bufferPosition          = 0;
    int     widthInCharacters       = object_getWidth (x->box_object);
    int     numberOfCharacters      = u8_charnum (x->box_string, x->box_stringSizeInBytes);
    double  fontWidth               = font_getHostFontWidth (p->p_fontSize);
    double  fontHeight              = font_getHostFontHeight (p->p_fontSize);
    int     lineLengthInCharacters  = (widthInCharacters ? widthInCharacters : BOX_WIDTH_DEFAULT);
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
    
    if (numberOfLines == (int)(p->p_y / fontHeight)) {
        int k = (int)((p->p_x / fontWidth) + 0.5);
        indexOfMouse = headInBytes + u8_offset (head, PD_CLAMP (k, 0, charactersUntilWrap));
    }
    
    /* Deplace selection points according new characters insertion. */
    
    if (x->box_selectionStart >= headInBytes) {         
        if (x->box_selectionStart <= headInBytes + bytesUntilWrap + eatCharacter) {
            p->p_selectionStart = accumulatedOffset + x->box_selectionStart;
        }
    }
            
    if (x->box_selectionEnd >= headInBytes) {
        if (x->box_selectionEnd <= headInBytes + bytesUntilWrap + eatCharacter) {
            p->p_selectionEnd = accumulatedOffset + x->box_selectionEnd;
        }
    }
    
    /* Append line to the buffer and continue next. */
    
    PD_ASSERT ((bufferPosition + bytesUntilWrap) < (p->p_bufferSize - 1));
    
    strncpy (p->p_buffer + bufferPosition, head, bytesUntilWrap);
        
    bufferPosition          += bytesUntilWrap;
    headInBytes             += bytesUntilWrap;
    headInBytes             += eatCharacter;
    charactersThatRemains   -= charactersUntilWrap;
    charactersThatRemains   -= eatCharacter;
    
    if (headInBytes < x->box_stringSizeInBytes) { p->p_buffer[bufferPosition++] = '\n'; }
    if (charactersUntilWrap > numberOfColumns)  { numberOfColumns = charactersUntilWrap; }
        
    numberOfLines++;
    //
    }
    
    if (indexOfMouse < 0)  { indexOfMouse = bufferPosition; }
    if (numberOfLines < 1) { numberOfLines = 1; }
    
    if (widthInCharacters) { numberOfColumns = widthInCharacters; } 
    else {
        while (numberOfColumns < BOX_WIDTH_EMPTY) {
            PD_ASSERT (bufferPosition < p->p_bufferSize);
            p->p_buffer[bufferPosition++] = ' ';
            numberOfColumns++;
        }
    }
    
    p->p_widthInPixels  = (int)((BOX_MARGIN_LEFT + BOX_MARGIN_RIGHT) + (numberOfColumns * fontWidth));
    p->p_heightInPixels = (int)((BOX_MARGIN_TOP + BOX_MARGIN_BOTTOM) + (numberOfLines * fontHeight));
    
    p->p_buffer[bufferPosition] = 0;
    
    return indexOfMouse;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void box_sendFirst (t_box *x, t_typesetproperties *p)
{
    t_glist *glist = canvas_getView (x->box_glist);
    int isSelected = canvas_isObjectSelected (x->box_glist, cast_gobj (x->box_object));
    
    sys_vGui ("::ui_box::newText .x%lx.c %s %d %d {%s} %d #%06x\n",     // --
                    glist,
                    x->box_tag,
                    (int)(object_getPixelX (x->box_object, x->box_glist) + BOX_MARGIN_LEFT), 
                    (int)(object_getPixelY (x->box_object, x->box_glist) + BOX_MARGIN_TOP),
                    p->p_buffer, 
                    font_getHostFontSize (p->p_fontSize),
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
}

static void box_sendUpdate (t_box *x, t_typesetproperties *p)
{
    t_glist *glist = canvas_getView (x->box_glist);
    
    sys_vGui ("::ui_box::setText .x%lx.c %s {%s}\n",    // --
                    glist,
                    x->box_tag,
                    p->p_buffer);
                
    if (x->box_isActivated) {
    
        if (p->p_selectionStart < p->p_selectionEnd) {
        
            sys_vGui (".x%lx.c select from %s %d\n",
                            glist, 
                            x->box_tag,
                            u8_charnum (x->box_string, p->p_selectionStart));
            sys_vGui (".x%lx.c select to %s %d\n",
                            glist, 
                            x->box_tag,
                            u8_charnum (x->box_string, p->p_selectionEnd) - 1);
            sys_vGui (".x%lx.c focus \"\"\n",
                            glist);
            
        } else {
        
            sys_vGui (".x%lx.c select clear\n",
                            glist);
            sys_vGui (".x%lx.c icursor %s %d\n",
                            glist,
                            x->box_tag,
                            u8_charnum (x->box_string, p->p_selectionStart));
            sys_vGui (".x%lx.c focus %s\n",
                            glist,
                            x->box_tag);        
        }
    }
}

int box_send (t_box *x, int action, int a, int b)
{
    t_typesetproperties p;
    
    int indexOfMouse = box_typeset (x, box_typesetAllocate (x, a, b, &p));
    int resized      = box_typesetHasResized (x, &p);
        
    if (action == BOX_CHECK)       { x->box_checked = 1;    }   /* Required only once at creation time. */
    else if (action == BOX_CREATE) { box_sendFirst (x, &p); }
    else if (action == BOX_UPDATE) {
    
        box_sendUpdate (x, &p); 
        
        if (resized) {
            canvas_drawBox (x->box_glist, x->box_object, x->box_tag, 0); 
        }
    }

    box_typesetFree (&p);
    
    return indexOfMouse;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
