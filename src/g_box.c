
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

void text_behaviorGetRectangle  (t_gobj *, t_glist *, t_rectangle *);
int box_send                    (t_box *x, int, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void box_drawObject (t_glist *view, t_object *o, char *tag, int create, t_rectangle *r)
{
    char *pattern = (pd_class (o) == text_class) ? "{6 4}" : "{}";  // --
    
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui ("%s.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -dash %s"
                        " -tags %sBORDER\n",
                        glist_getTagAsString (view),
                        a,
                        b,
                        c,
                        b, 
                        c, 
                        d, 
                        a, 
                        d,  
                        a, 
                        b,  
                        pattern,        /* Dashes for badly created boxes. */
                        tag);
                        
    } else {
    
        sys_vGui ("%s.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d\n",
                        glist_getTagAsString (view),
                        tag,
                        a,
                        b,
                        c,
                        b,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b);
                        
        sys_vGui ("%s.c itemconfigure %sBORDER -dash %s\n",
                        glist_getTagAsString (view),
                        tag,
                        pattern);
    }
}

static void box_drawMessage (t_glist *view, t_object *o, char *tag, int create, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui ("%s.c create line %d %d %d %d %d %d %d %d %d %d %d %d %d %d"
                        " -tags %sBORDER\n",
                        glist_getTagAsString (view),
                        a,
                        b,
                        c + 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d - 4,
                        c + 4,
                        d,
                        a,
                        d,
                        a,
                        b,
                        tag);
                    
    } else {
    
        sys_vGui ("%s.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        glist_getTagAsString (view),
                        tag,
                        a,
                        b,
                        c + 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d - 4,
                        c + 4,
                        d,
                        a,
                        d,
                        a,
                        b);
    }
}

static void box_drawAtom (t_glist *view, t_object *o, char *tag, int create, t_rectangle *r)
{
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui ("%s.c create line %d %d %d %d %d %d %d %d %d %d %d %d"
                        " -tags %sBORDER\n",
                        glist_getTagAsString (view),
                        a,
                        b,
                        c - 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b,
                        tag);
                        
    } else {
    
        sys_vGui ("%s.c coords %sBORDER %d %d %d %d %d %d %d %d %d %d %d %d\n",
                        glist_getTagAsString (view),
                        tag,
                        a,
                        b,
                        c - 4,
                        b,
                        c,
                        b + 4,
                        c,
                        d,
                        a,
                        d,
                        a,
                        b);
    }
}

static void box_drawComment (t_glist *view, t_object *o, char *tag, int create, t_rectangle *r)
{
    if (glist_hasEditMode (view)) {
    //
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    if (create) {
    
        sys_vGui ("%s.c create line %d %d %d %d"
                        " -tags [list %sBORDER COMMENTBAR]\n",      // --
                        glist_getTagAsString (view),
                        c,
                        b,
                        c,
                        d,
                        tag);
                        
    } else {
    
        sys_vGui ("%s.c coords %sBORDER %d %d %d %d\n",
                        glist_getTagAsString (view),
                        tag,
                        c,
                        b,
                        c,
                        d);
    }
    //
    }
}

static void box_drawInletsAndOutlets (t_glist *view, t_object *o, char *tag, int create, t_rectangle *r)
{
    int i;
    int m = object_getNumberOfInlets (o);
    int n = object_getNumberOfOutlets (o);
    int a = rectangle_getTopLeftX (r);
    int b = rectangle_getTopLeftY (r);
    int c = rectangle_getBottomRightX (r);
    int d = rectangle_getBottomRightY (r);
    
    for (i = 0; i < m; i++) {
    //
    int offset = a + inlet_offset ((c - a), i, m);
    
    if (create) {
    
        sys_vGui ("%s.c create rectangle %d %d %d %d -tags %sINLET%d\n",
                        glist_getTagAsString (view),
                        offset,
                        b,
                        offset + INLET_WIDTH,
                        b + INLET_HEIGHT,
                        tag,
                        i);
                        
    } else {
    
        sys_vGui ("%s.c coords %sINLET%d %d %d %d %d\n",
                        glist_getTagAsString (view),
                        tag,
                        i,
                        offset,
                        b,
                        offset + INLET_WIDTH,
                        b + INLET_HEIGHT);
    }
    //
    }
    
    for (i = 0; i < n; i++) {
    //
    int offset = a + inlet_offset ((c - a), i, n);
    
    if (create) {
    
        sys_vGui ("%s.c create rectangle %d %d %d %d -tags %sOUTLET%d\n",
                        glist_getTagAsString (view),
                        offset,
                        d - INLET_HEIGHT,
                        offset + INLET_WIDTH,
                        d,
                        tag,
                        i);
                        
    } else {
    
        sys_vGui ("%s.c coords %sOUTLET%d %d %d %d %d\n",
                        glist_getTagAsString (view),
                        tag,
                        i,
                        offset,
                        d - INLET_HEIGHT,
                        offset + INLET_WIDTH,
                        d);
    }
    //
    }
}

static void box_drawBox (t_glist *glist, t_object *o, char *tag, int create)
{
    t_glist *view = glist_getView (glist);
    
    t_rectangle r;
    
    text_behaviorGetRectangle (cast_gobj (o), glist, &r);

    if (object_isObject (o))            { box_drawObject (view, o, tag, create, &r);           }
    else if (object_isMessage (o))      { box_drawMessage (view, o, tag, create, &r);          }
    else if (object_isAtom (o))         { box_drawAtom (view, o, tag, create, &r);             }
    else if (object_isComment (o))      { box_drawComment (view, o, tag, create, &r);          }
    if (cast_objectIfConnectable (o))   { box_drawInletsAndOutlets (view, o, tag, create, &r); }
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

t_box *box_fetch (t_glist *glist, t_object *object)
{
    return editor_boxFetch (glist_getEditor (glist), object);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void box_retext (t_box *x)
{
    PD_MEMORY_FREE (x->box_string);
    
    buffer_toStringUnzeroed (object_getBuffer (x->box_object), &x->box_string, &x->box_stringSizeInBytes);
        
    if (glist_isOnScreen (x->box_owner)) { box_send (x, BOX_UPDATE, 0, 0); } 
}

/* Due to loadbang the content is set here (instead that in constructor). */

void box_create (t_box *x)
{
    PD_MEMORY_FREE (x->box_string);
    
    buffer_toStringUnzeroed (object_getBuffer (x->box_object), &x->box_string, &x->box_stringSizeInBytes);
    
    box_send (x, BOX_CREATE, 0, 0);
}

void box_draw (t_box *x)
{
    box_drawBox (x->box_owner, x->box_object, x->box_tag, 1);
}

void box_update (t_box *x)
{
    box_drawBox (x->box_owner, x->box_object, x->box_tag, 0);
}

void box_erase (t_box *x)
{
    sys_vGui ("%s.c delete %s\n", glist_getTagAsString (glist_getView (x->box_owner)), x->box_tag);
}

void box_displace (t_box *x, int deltaX, int deltaY)
{
    sys_vGui ("%s.c move %s %d %d\n", 
                    glist_getTagAsString (glist_getView (x->box_owner)), 
                    x->box_tag, 
                    deltaX, 
                    deltaY);
}

void box_select (t_box *x, int isSelected)
{
    sys_vGui ("%s.c itemconfigure %s -fill #%06x\n",
                    glist_getTagAsString (glist_getView (x->box_owner)), 
                    x->box_tag, 
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
}

void box_activate (t_box *x, int isActivated)
{
    PD_ASSERT (glist_hasWindow (x->box_owner));     /* Can't be activate in GOP. */
    
    if (isActivated) {

        sys_vGui ("::ui_box::setEditing %s %s 1\n", glist_getTagAsString (x->box_owner), x->box_tag);
                        
        editor_boxSelect (glist_getEditor (x->box_owner), x);
        
        x->box_draggedFrom      = 0;
        x->box_selectionStart   = 0;
        x->box_selectionEnd     = x->box_stringSizeInBytes;
        x->box_isActivated      = 1;

    } else {

        sys_vGui ("::ui_box::setEditing %s {} 0\n", glist_getTagAsString (x->box_owner));   // --
        
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
