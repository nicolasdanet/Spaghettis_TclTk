
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_behaviorGetRectangle             (t_gobj *, t_glist *, t_rectangle *);
void glist_behaviorDisplaced                (t_gobj *, t_glist *, int, int);
void glist_behaviorSelected                 (t_gobj *, t_glist *, int);
void glist_behaviorActivated                (t_gobj *, t_glist *, int);
void glist_behaviorDeleted                  (t_gobj *, t_glist *);
void glist_behaviorVisibilityChanged        (t_gobj *, t_glist *, int);
int  glist_behaviorMouse                    (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_widgetbehavior glist_widgetbehavior =     /* Shared. */
    {
        glist_behaviorGetRectangle,
        glist_behaviorDisplaced,
        glist_behaviorSelected,
        glist_behaviorActivated,
        glist_behaviorDeleted,
        glist_behaviorVisibilityChanged,
        glist_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void glist_behaviorGetRectangleProceed (t_glist *x, t_glist *owner, t_rectangle *r)
{
    glist_getRectangleOnParent (x, r);
}

static void glist_behaviorDisplacedProceed (t_glist *x, t_glist *owner, int deltaX, int deltaY)
{
    if (glist_isParentOnScreen (x)) { glist_behaviorVisibilityChanged (cast_gobj (x), owner, 0); }
    
    object_setX (cast_object (x), object_getX (cast_object (x)) + deltaX);
    object_setY (cast_object (x), object_getY (cast_object (x)) + deltaY);
    
    if (glist_isParentOnScreen (x)) { glist_behaviorVisibilityChanged (cast_gobj (x), owner, 1); }
    
    glist_updateLinesForObject (owner, cast_object (x));
}

static void glist_behaviorSelectedProceed (t_glist *x, t_glist *owner, int isSelected)
{
    glist_updateGraph (x);
}

static int glist_behaviorMouseProceed (t_glist *x, t_glist *owner, t_mouse *m)
{
    if (glist_hasWindow (x)) { return 0; }
    else {
        int k = 0;
        int a = m->m_x;
        int b = m->m_y;
        
        t_gobj *y = NULL;
            
        for (y = x->gl_graphics; y; y = y->g_next) {
            t_rectangle t;
            if (gobj_hit (y, x, a, b, &t)) {
                if ((k = gobj_mouse (y, x, m))) {
                    break;
                }
            }
        }

        return k; 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnGetRectangle (z, glist, r); }
    else {
        glist_behaviorGetRectangleProceed (x, glist, r);
    }
}

void glist_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnDisplaced (z, glist, deltaX, deltaY); }
    else {
        glist_behaviorDisplacedProceed (x, glist, deltaX, deltaY);
    }
}

void glist_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_glist *x = cast_glist (z);
    
    glist_setSelected (x, (isSelected != 0));
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnSelected (z, glist, isSelected); }
    else {
        glist_behaviorSelectedProceed (x, glist, isSelected);
    }
}

void glist_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
    t_glist *x = cast_glist (z);
    
    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnActivated (z, glist, isActivated); }
}

void glist_behaviorDeleted (t_gobj *z, t_glist *glist)
{
    t_glist *x = cast_glist (z);

    glist_objectRemoveAll (x);
    
    text_widgetBehavior.w_fnDeleted (z, glist);
}

void glist_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_glist *x = cast_glist (z);

    if (!glist_isGraphOnParent (x)) { text_widgetBehavior.w_fnVisibilityChanged (z, glist, isVisible); }
    else {
    //
    char tag[PD_STRING] = { 0 }; t_error err = string_sprintf (tag, PD_STRING, "%lxGRAPH", x);
    
    t_rectangle r;
    
    glist_behaviorGetRectangle (z, glist, &r);
    
    if (!err) {
    //
    int a = rectangle_getTopLeftX (&r);
    int b = rectangle_getTopLeftY (&r);
    int c = rectangle_getBottomRightX (&r);
    int d = rectangle_getBottomRightY (&r);
        
    if (glist_hasWindow (x)) {
    //
    if (isVisible) {
        
        sys_vGui (".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %s\n",
                        glist_getView (glist_getParent (x)),
                        a,
                        b,
                        a,
                        d,
                        c,
                        d,
                        c,
                        b,
                        a,
                        b,
                        COLOR_MASKED,
                        tag);
    } else {
                
        sys_vGui (".x%lx.c delete %s\n",
                        glist_getView (glist_getParent (x)),
                        tag);
    }
    //
    } else {
    //
    t_gobj *y = NULL;
                
    if (isVisible) {
        
        int i = 0;

        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -fill #%06x"
                        " -tags %s\n",
                        glist_getView (glist_getParent (x)),
                        a,
                        b,
                        a,
                        d,
                        c,
                        d,
                        c,
                        b,
                        a,
                        b,
                        (glist_isSelected (x) ? COLOR_SELECTED : COLOR_NORMAL),
                        tag);
        
        for (y = x->gl_graphics; y; y = y->g_next) {
        //
        if (pd_class (y) == garray_class) {
        //
        sys_vGui (".x%lx.c create text %d %d -text {%s}"    // --
                        " -anchor nw"
                        " -font [::getFont %d]"             // --
                        " -fill #%06x"
                        " -tags %s\n",
                        glist_getView (glist_getParent (x)),
                        a,
                        b - (++i) * (int)font_getHostFontHeight (glist_getFontSize (x)),
                        garray_getName ((t_garray *)y)->s_name,
                        font_getHostFontSize (glist_getFontSize (x)),
                        (glist_isSelected (x) ? COLOR_SELECTED : COLOR_NORMAL),
                        tag);
        //
        }
        //
        }
        
        for (y = x->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, x, 1); }
            
    } else {
    
        sys_vGui (".x%lx.c delete %s\n",
                    glist_getView (glist_getParent (x)),
                    tag);
                    
        for (y = x->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, x, 0); }
    }
    //
    }
    //
    }
    //
    }
}

int glist_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_glist *x = cast_glist (z);

    if (!glist_isGraphOnParent (x)) { return text_widgetBehavior.w_fnMouse (z, glist, m); }
    else {
        return glist_behaviorMouseProceed (x, glist, m);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
