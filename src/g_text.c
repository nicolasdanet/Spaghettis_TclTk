
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* That file implement behaviors for comments. */
/* Note that they are used also by every text object (that means rather ALL objects). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *text_class;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void text_behaviorGetRectangle      (t_gobj *, t_glist *, t_rectangle *);
void text_behaviorDisplaced         (t_gobj *, t_glist *, int, int);
void text_behaviorSelected          (t_gobj *, t_glist *, int);
void text_behaviorActivated         (t_gobj *, t_glist *, int);
void text_behaviorDeleted           (t_gobj *, t_glist *);
void text_behaviorVisibilityChanged (t_gobj *, t_glist *, int);
int  text_behaviorMouse             (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_widgetbehavior text_widgetBehavior =              /* Shared. */
    {
        text_behaviorGetRectangle,
        text_behaviorDisplaced,
        text_behaviorSelected,
        text_behaviorActivated,
        text_behaviorDeleted,
        text_behaviorVisibilityChanged,
        text_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void text_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_object *x = cast_object (z);
    t_box *text = box_fetch (glist, x);
    int width   = box_getWidth (text);
    int height  = box_getHeight (text);
    int a       = glist_getPixelX (glist, x);
    int b       = glist_getPixelY (glist, x);
    
    rectangle_set (r, a, b, a + width, b + height);
}

void text_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_object *x = cast_object (z);
    
    object_setX (x, object_getX (x) + deltaX);
    object_setY (x, object_getY (x) + deltaY);
    
    if (glist_isOnScreen (glist)) {
    //
    t_box *text = box_fetch (glist, x);
    box_displace (text, deltaX, deltaY);
    glist_updateLinesForObject (glist, x);
    //
    }
}

void text_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_object *x = cast_object (z);

    if (glist_isOnScreen (glist)) {
    //
    t_box *text = box_fetch (glist, x);
    
    box_select (text, isSelected);
    
    if (gobj_isVisible (z, glist)) {
    //
    sys_vGui ("%s.c itemconfigure %sBORDER -fill #%06x\n",
                    glist_getTagAsString (glist), 
                    box_getTag (text),
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
    //
    }
    //
    }
}

void text_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    box_activate (box_fetch (glist, cast_object (z)), isActivated);
}

void text_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    glist_objectDeleteLines (glist, cast_object (z));
}

void text_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_object *x = cast_object (z);
    
    if (gobj_isVisible (z, glist)) {
    //
    t_box *text = box_fetch (glist, x);
    
    if (isVisible) { box_create (text); } 
    else {
        box_erase (text);
    }
    //
    }
}

int text_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_object *x = cast_object (z);
    
    int k = object_isAtom (x) || object_isMessage (x);
    
    if (k || (object_isObject (x) && class_hasMethod (pd_class (x), sym_click))) { 
        if (m->m_clicked) {
            pd_message (cast_pd (x), sym_click, mouse_argc (m), mouse_argv (m)); 
        }
        
        return 1;
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void text_anything (t_object *x, t_symbol *s, int argc, t_atom *argv)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void text_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text, 
            NULL,
            NULL, 
            sizeof (t_object),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);
        
    class_addAnything (c, (t_method)text_anything);
    
    text_class = c;
}

void text_destroy (void)
{
    CLASS_FREE (text_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
