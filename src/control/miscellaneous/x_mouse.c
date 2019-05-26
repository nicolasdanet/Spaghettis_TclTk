
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *mouse_class;                /* Shared. */
static t_class *mouseproxy_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _mouseproxy {
    t_object            x_obj;              /* Must be the first. */
    t_glist             *x_owner;
    t_symbol            *x_tag;
    struct _mouseobject *x_master;
    } t_mouseproxy;

typedef struct _mouseobject {
    t_object            x_obj;              /* Must be the first. */
    t_mouseproxy        *x_proxy;
    t_glist             *x_owner;
    t_symbol            *x_tag;
    t_outlet            *x_outletLeft;
    t_outlet            *x_outletMiddleLeft;
    t_outlet            *x_outletMiddleRight;
    t_outlet            *x_outletRight;
    } t_mouseobject;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Is there any sense to support GOP recursively nested? */
/* For now only one level is implemented. */

static void mouseproxy_anything (t_mouseproxy *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1 && (s == sym__motion || s == sym__mousedown || s == sym__mouseup)) {
    //
    t_glist *glist = x->x_master->x_owner;
    t_float a = atom_getFloat (argv + 0);
    t_float b = atom_getFloat (argv + 1);
    t_point p = point_make (a, b);
    
    if (!glist_convertPixelFromParent (glist, &p)) {
    //
    t_atom *t = NULL;
    
    PD_ATOMS_ALLOCA (t, argc);
    
    atom_copyAtoms (argv, argc, t, argc);
    
    SET_FLOAT (t + 0, point_getX (&p));
    SET_FLOAT (t + 1, point_getY (&p));
    
    pd_message (cast_pd (x->x_master), s, argc, t);
    
    PD_ATOMS_FREEA (t, argc);
    //
    }
    //
    }
}

static void *mouseproxy_new (t_mouseobject *master, t_glist *owner)
{
    t_mouseproxy *x = (t_mouseproxy *)pd_new (mouseproxy_class);
    
    x->x_owner  = owner;
    x->x_tag    = glist_getTag (x->x_owner);
    x->x_master = master;
    
    pd_bind (cast_pd (x), x->x_tag);
    
    return x;
}

static void mouseproxy_free (t_mouseproxy *x)
{
    pd_unbind (cast_pd (x), x->x_tag);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void mouse_motion (t_mouseobject *x, int argc, t_atom *argv)
{
    t_mouse m; mouse_parsed (&m, 0, argc, argv);
    
    outlet_list (x->x_outletLeft, 2, mouse_argv (&m));
}

static void mouse_down (t_mouseobject *x, int argc, t_atom *argv)
{
    t_mouse m; mouse_parsed (&m, 1, argc, argv);
        
    if (!m.m_clickedRight) {
    //
    outlet_float (x->x_outletRight, 1);
    
    if (m.m_dbl) { outlet_bang (x->x_outletMiddleRight); }
    
    outlet_list (x->x_outletLeft, 2, mouse_argv (&m));
    //
    }
}

static void mouse_up (t_mouseobject *x, int argc, t_atom *argv)
{
    t_mouse m; mouse_parsed (&m, 0, argc, argv);
    
    outlet_float (x->x_outletRight, 0); outlet_list (x->x_outletLeft, 2, mouse_argv (&m));
}

static void mouse_scroll (t_mouseobject *x, int argc, t_atom *argv)
{
    outlet_list (x->x_outletMiddleLeft, argc, argv);
}

static void mouse_cancel (t_mouseobject *x, int argc, t_atom *argv)
{
    if (atom_getSymbolAtIndex (0, argc, argv) == x->x_tag) { outlet_float (x->x_outletRight, 0); }
}

static void mouse_anything (t_mouseobject *x, t_symbol *s, int argc, t_atom *argv)
{
    if (s == sym__motion)           { mouse_motion (x, argc, argv); }
    else if (s == sym__mousedown)   { mouse_down (x,   argc, argv); }
    else if (s == sym__mouseup)     { mouse_up (x,     argc, argv); }
    else if (s == sym__scroll)      { mouse_scroll (x, argc, argv); }
    else if (s == sym__mousecancel) { mouse_cancel (x, argc, argv); }
}

static void *mouse_new (void)
{
    t_mouseobject *x = (t_mouseobject *)pd_new (mouse_class);
    
    x->x_owner              = instance_contextGetCurrent();
    x->x_tag                = glist_getTag (x->x_owner);
    x->x_outletLeft         = outlet_newList (cast_object (x));
    x->x_outletMiddleLeft   = outlet_newList (cast_object (x));
    x->x_outletMiddleRight  = outlet_newBang (cast_object (x));
    x->x_outletRight        = outlet_newFloat (cast_object (x));
    
    pd_bind (cast_pd (x), sym__mousecancel);
    pd_bind (cast_pd (x), x->x_tag);
    
    if (glist_hasParent (x->x_owner)) {
    //
    x->x_proxy = (t_mouseproxy *)mouseproxy_new (x, glist_getParent (x->x_owner));
    //
    }
    
    return x;
}

static void mouse_free (t_mouseobject *x)
{
    if (x->x_proxy) { pd_free (cast_pd (x->x_proxy)); }
    
    pd_unbind (cast_pd (x), x->x_tag);
    pd_unbind (cast_pd (x), sym__mousecancel);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void mouse_setup (void)
{
    mouse_class = class_new (sym_mouse,
                        (t_newmethod)mouse_new,
                        (t_method)mouse_free,
                        sizeof (t_mouseobject),
                        CLASS_DEFAULT | CLASS_NOINLET,
                        A_NULL);
    
    mouseproxy_class = class_new (sym_mouseproxy,
                        NULL,
                        (t_method)mouseproxy_free,
                        sizeof (t_mouseproxy),
                        CLASS_INVISIBLE,
                        A_NULL);
    
    class_addAnything (mouse_class,      (t_method)mouse_anything);
    class_addAnything (mouseproxy_class, (t_method)mouseproxy_anything);
}

void mouse_destroy (void)
{
    class_free (mouse_class);
    class_free (mouseproxy_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
