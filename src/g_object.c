
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *canvas_class;
extern t_class  *garray_class;
extern t_class  *scalar_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior     text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void gobj_getRectangle (t_gobj *x, t_glist *owner, int *a, int *b, int *c, int *d)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnGetRectangle) {
        (*(pd_class (x)->c_behavior->w_fnGetRectangle)) (x, owner, a, b, c, d);
    }
}

void gobj_displace (t_gobj *x, t_glist *owner, int deltaX, int deltaY)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDisplace) {
        (*(pd_class (x)->c_behavior->w_fnDisplace)) (x, owner, deltaX, deltaY);
    }
}

void gobj_select (t_gobj *x, t_glist *owner, int state)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnSelect) {
        (*(pd_class (x)->c_behavior->w_fnSelect)) (x, owner, state);
    }
}

void gobj_activate (t_gobj *x, t_glist *owner, int state)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnActivate) {
        (*(pd_class (x)->c_behavior->w_fnActivate)) (x, owner, state);
    }
}

void gobj_delete (t_gobj *x, t_glist *owner)
{
    if (pd_class (x)->c_behavior && pd_class (x)->c_behavior->w_fnDelete) {
        (*(pd_class (x)->c_behavior->w_fnDelete)) (x, owner);
    }
}

void gobj_save (t_gobj *x, t_buffer *buffer)
{
    if (pd_class (x)->c_fnSave) {
        (*(pd_class (x)->c_fnSave)) (x, buffer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int gobj_shouldvis(t_gobj *x, struct _glist *glist)
{
    t_object *ob;
            /* if our parent is a graph, and if that graph itself isn't
            visible, then we aren't either. */
    if (!glist->gl_haveWindow && glist->gl_isGraphOnParent && glist->gl_owner
        && !gobj_shouldvis(&glist->gl_obj.te_g, glist->gl_owner))
            return (0);
            /* if we're graphing-on-parent and the object falls outside the
            graph rectangle, don't draw it. */
    if (!glist->gl_haveWindow && glist->gl_isGraphOnParent && glist->gl_hasRectangle &&
        glist->gl_owner)
    {
        int x1, y1, x2, y2, gx1, gy1, gx2, gy2, m;
            /* for some reason the bounds check on arrays and scalars
            don't seem to apply here.  Perhaps this was in order to allow
            arrays to reach outside their containers?  I no longer understand
            this. */
        if (pd_class(&x->g_pd) == scalar_class
            || pd_class(&x->g_pd) == garray_class)
                return (1);
        gobj_getRectangle(&glist->gl_obj.te_g, glist->gl_owner, &x1, &y1, &x2, &y2);
        if (x1 > x2)
            m = x1, x1 = x2, x2 = m;
        if (y1 > y2)
            m = y1, y1 = y2, y2 = m;
        gobj_getRectangle(x, glist, &gx1, &gy1, &gx2, &gy2);
        /* post("graph %d %d %d %d, %s %d %d %d %d",
            x1, x2, y1, y2, class_getHelpName(x->g_pd), gx1, gx2, gy1, gy2); */
        if (gx1 < x1 || gx1 > x2 || gx2 < x1 || gx2 > x2 ||
            gy1 < y1 || gy1 > y2 || gy2 < y1 || gy2 > y2)
                return (0);
    }
    if (ob = canvas_castToObjectIfBox(&x->g_pd))
    {
        /* return true if the text box should be drawn.  We don't show text
        boxes inside graphs---except comments, if we're doing the new
        (goprect) style. */
        return (glist->gl_haveWindow ||
            (ob->te_g.g_pd != canvas_class &&
                ob->te_g.g_pd->c_behavior != &text_widgetBehavior) ||
            (ob->te_g.g_pd == canvas_class && (((t_glist *)ob)->gl_isGraphOnParent)) ||
            (glist->gl_hasRectangle && (ob->te_type == TYPE_TEXT)));
    }
    else return (1);
}

void gobj_vis(t_gobj *x, struct _glist *glist, int flag)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnVisible && gobj_shouldvis(x, glist))
        (*x->g_pd->c_behavior->w_fnVisible)(x, glist, flag);
}

int gobj_click(t_gobj *x, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if (x->g_pd->c_behavior && x->g_pd->c_behavior->w_fnClick)
        return ((*x->g_pd->c_behavior->w_fnClick)(x,
            glist, xpix, ypix, shift, alt, dbl, doit));
    else return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
