
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *scalar_class;
extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *pointer_class;           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct {
    t_symbol    *to_type;
    t_outlet    *to_outlet;
    } t_typedout;

typedef struct _ptrobj {
    t_object    x_obj;
    t_gpointer  x_gp;
    int         x_ntypedout;
    t_typedout  *x_typedout;
    t_outlet    *x_otherout;
    t_outlet    *x_bangout;
    } t_pointer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pointer_bang(t_pointer *x)
{
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    if (!gpointer_isValidOrHead(&x->x_gp))
    {
        post_error ("pointer_bang: empty pointer");
        return;
    }
    templatesym = gpointer_getTemplateIdentifier(&x->x_gp);
    for (n = x->x_ntypedout, to = x->x_typedout; n--; to++)
    {
        if (to->to_type == templatesym)
        {
            outlet_pointer(to->to_outlet, &x->x_gp);
            return;
        }
    }
    outlet_pointer(x->x_otherout, &x->x_gp);
}


static void pointer_pointer(t_pointer *x, t_gpointer *gp)
{
    //gpointer_unset(&x->x_gp);
    gpointer_setByCopy(gp, &x->x_gp);
    pointer_bang(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* send a message to the window containing the object pointed to */
static void pointer_sendwindow(t_pointer *x, t_symbol *s, int argc, t_atom *argv)
{
    t_scalar *sc;
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    t_glist *glist;
    t_pd *canvas;
    if (!gpointer_isValidOrHead (&x->x_gp))
    {
        post_error ("send-window: empty pointer");
        return;
    }
    
    glist = gpointer_getView (&x->x_gp);
    canvas = (t_pd *)canvas_getView(glist);
    if (argc && argv->a_type == A_SYMBOL)
        pd_message(canvas, argv->a_w.w_symbol, argc-1, argv+1);
    else { post_error ("send-window: no message?"); }
}


    /* send the pointer to the named object */
static void pointer_send(t_pointer *x, t_symbol *s)
{
    if (!s->s_thing)
        post_error ("%s: no such object", s->s_name);
    else if (!gpointer_isValidOrHead (&x->x_gp))
        post_error ("pointer_send: empty pointer");
    else pd_pointer(s->s_thing, &x->x_gp);
}

static void pointer_traverse(t_pointer *x, t_symbol *s)
{
    t_glist *glist = (t_glist *)pd_findByClass(s, canvas_class);
    if (glist) gpointer_setAsScalar(&x->x_gp, glist, 0);
    else { post_error (x, "pointer: list '%s' not found", s->s_name); }
}

static void pointer_rewind(t_pointer *x)
{
    t_scalar *sc;
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    t_glist *glist;
    t_pd *canvas;
    //t_gmaster *gs;
    if (!gpointer_isValidOrHead(&x->x_gp))
    {
        post_error ("pointer_rewind: empty pointer");
        return;
    }

    if (!gpointer_isScalar (&x->x_gp)) {
        post_error ("pointer_rewind: sorry, unavailable for arrays");
        return;
    }
    glist = gpointer_getParentGlist (&x->x_gp);
    gpointer_setAsScalar(&x->x_gp, glist, 0);
    pointer_bang(x);
}

static void pointer_vnext(t_pointer *x, t_float f)
{
    t_gobj *gobj;
    t_gpointer *gp = &x->x_gp;
    t_glist *glist;
    int wantselected = (f != 0);

    if (!gpointer_isSet (gp))
    {
        post_error ("pointer_next: no current pointer");
        return;
    }
    if (!gpointer_isScalar (gp)) {
        post_error ("pointer_next: lists only, not arrays");
        return;
    }
    
    glist = gpointer_getParentGlist (gp);
    if (glist->gl_uniqueIdentifier != gpointer_getUniqueIdentifier (gp))    /* isValid ? */
    {
        post_error ("pointer_next: stale pointer");
        return;
    }
    
    if (wantselected && !canvas_isMapped(glist))
    {
        post_error ("pointer_vnext: next-selected only works for a visible window");
        return;
    }
    
    t_scalar *scalar = gpointer_getScalar (gp);
    gobj = cast_gobj (scalar);
    
    if (!gobj) gobj = glist->gl_graphics;
    else gobj = gobj->g_next;
    while (gobj && ((pd_class(&gobj->g_pd) != scalar_class) ||
        (wantselected && !canvas_isObjectSelected(glist, gobj))))
            gobj = gobj->g_next;
    
    if (gobj)
    {
        t_typedout *to;
        int n;
        t_scalar *sc = (t_scalar *)gobj;
        t_symbol *templatesym = sc->sc_templateIdentifier;

        gpointer_setAsScalar (gp, glist, sc);
        // gp->gp_un.gp_scalar = sc; 
        for (n = x->x_ntypedout, to = x->x_typedout; n--; to++)
        {
            if (to->to_type == templatesym)
            {
                outlet_pointer(to->to_outlet, &x->x_gp);
                return;
            }
        }
        outlet_pointer(x->x_otherout, &x->x_gp);
    }
    else
    {
        gpointer_unset(gp);
        outlet_bang(x->x_bangout);
    }
}

static void pointer_next(t_pointer *x)
{
    pointer_vnext(x, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pointer_new (t_symbol *classname, int argc, t_atom *argv)
{
    t_pointer *x = (t_pointer *)pd_new(pointer_class);
    t_typedout *to;
    int n;
    gpointer_init(&x->x_gp);
    x->x_typedout = to = (t_typedout *)PD_MEMORY_GET(argc * sizeof (*to));
    x->x_ntypedout = n = argc;
    for (; n--; to++)
    {
        to->to_outlet = outlet_new(&x->x_obj, &s_pointer);
        to->to_type = template_makeIdentifierWithWildcard(atom_getSymbol(argv++));
    }
    x->x_otherout = outlet_new(&x->x_obj, &s_pointer);
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    inlet_newPointer(&x->x_obj, &x->x_gp);
    return (x);
}

static void pointer_free(t_pointer *x)
{
    PD_MEMORY_FREE(x->x_typedout);
    gpointer_unset(&x->x_gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pointer_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pointer,
            (t_newmethod)pointer_new,
            (t_method)pointer_free,
            sizeof (t_pointer),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, pointer_bang); 
    class_addPointer (c, pointer_pointer); 
        
    class_addMethod (c, (t_method)pointer_sendwindow,   sym_sendwindow,             A_GIMME, A_NULL); 
    class_addMethod (c, (t_method)pointer_send,         sym_send,                   A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)pointer_traverse,     sym_traverse,               A_SYMBOL, A_NULL); 
    class_addMethod (c, (t_method)pointer_rewind,       sym_rewind,                 A_NULL); 
    class_addMethod (c, (t_method)pointer_next,         sym_next,                   A_NULL);
    class_addMethod (c, (t_method)pointer_vnext,        sym_nextselected,           A_DEFFLOAT, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)pointer_vnext,        sym_vnext,                  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)pointer_sendwindow,   sym_send__dash__window,     A_GIMME, A_NULL); 
    
    #endif

    pointer_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
