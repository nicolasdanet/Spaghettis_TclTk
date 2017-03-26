
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

/* Everything related to the text contained in boxes. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int box_send (t_box *x, int, int, int);

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

t_box *box_fetch (t_glist *glist, t_object *object)
{
    return editor_boxFetch (glist_getEditor (glist), object);
}

void box_update (t_box *x)
{
    PD_ASSERT (x);

    PD_MEMORY_FREE (x->box_string);
    
    buffer_toStringUnzeroed (object_getBuffer (x->box_object), &x->box_string, &x->box_stringSizeInBytes);
        
    if (glist_isOnScreen (x->box_owner)) { box_send (x, BOX_UPDATE, 0, 0); } 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Due to loadbang the content is set here (instead that in constructor). */

void box_draw (t_box *x)
{
    PD_MEMORY_FREE (x->box_string);
    
    buffer_toStringUnzeroed (object_getBuffer (x->box_object), &x->box_string, &x->box_stringSizeInBytes);
    
    box_send (x, BOX_CREATE, 0, 0);
}

void box_erase (t_box *x)
{
    sys_vGui (".x%lx.c delete %s\n", 
                    glist_getView (x->box_owner), 
                    x->box_tag);
}

void box_displace (t_box *x, int deltaX, int deltaY)
{
    sys_vGui (".x%lx.c move %s %d %d\n", 
                    glist_getView (x->box_owner), 
                    x->box_tag, 
                    deltaX, 
                    deltaY);
}

void box_select (t_box *x, int isSelected)
{
    sys_vGui (".x%lx.c itemconfigure %s -fill #%06x\n",
                    glist_getView (x->box_owner), 
                    x->box_tag, 
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
}

void box_activate (t_box *x, int isActivated)
{
    if (isActivated) {

        sys_vGui ("::ui_box::setEditing .x%lx %s 1\n", x->box_owner, x->box_tag);
                        
        editor_boxSelect (glist_getEditor (x->box_owner), x);
        
        x->box_draggedFrom      = 0;
        x->box_selectionStart   = 0;
        x->box_selectionEnd     = x->box_stringSizeInBytes;
        x->box_isActivated      = 1;

    } else {

        sys_vGui ("::ui_box::setEditing .x%lx {} 0\n", x->box_owner);   // --
        
        editor_boxUnselect (glist_getEditor (x->box_owner), x);
        
        x->box_isActivated = 0;
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

static void box_keyDeleteProceed (t_box *x)
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
    editor_setSelectedBoxDirty (glist_getEditor (x->box_owner));
    //
    }
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
    
    if (s == sym_BackSpace || s == sym_Delete) { box_keyDeleteProceed (x); return 1; }
    else {
        return 0;
    }
}

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

static void box_keyASCII (t_box *x, t_keycode n)
{
    box_keyDeleteProceed (x);
    
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
    box_keyDeleteProceed (x);
    
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
            editor_setSelectedBoxDirty (glist_getEditor (x->box_owner));
        }
    }

    box_send (x, BOX_UPDATE, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
