
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
    t_gpointer  x_gpointer;
    int         x_outletTypedSize;
    t_typedout  *x_outletTyped;
    t_outlet    *x_outletOther;
    t_outlet    *x_outletBang;
    } t_pointer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pointer_bang(t_pointer *x)
{
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    if (!gpointer_isValidOrHead(&x->x_gpointer))
    {
        post_error ("pointer_bang: empty pointer");
        return;
    }
    templatesym = gpointer_getTemplateIdentifier(&x->x_gpointer);
    for (n = x->x_outletTypedSize, to = x->x_outletTyped; n--; to++)
    {
        if (to->to_type == templatesym)
        {
            outlet_pointer(to->to_outlet, &x->x_gpointer);
            return;
        }
    }
    outlet_pointer(x->x_outletOther, &x->x_gpointer);
}


static void pointer_pointer (t_pointer *x, t_gpointer *gp)
{
    //gpointer_unset(&x->x_gpointer);
    gpointer_setByCopy(gp, &x->x_gpointer);
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
    if (!gpointer_isValidOrHead (&x->x_gpointer))
    {
        post_error ("send-window: empty pointer");
        return;
    }
    
    glist = gpointer_getView (&x->x_gpointer);
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
    else if (!gpointer_isValidOrHead (&x->x_gpointer))
        post_error ("pointer_send: empty pointer");
    else pd_pointer(s->s_thing, &x->x_gpointer);
}

static void pointer_traverse(t_pointer *x, t_symbol *s)
{
    t_glist *glist = (t_glist *)pd_findByClass(s, canvas_class);
    if (glist) gpointer_setAsScalar(&x->x_gpointer, glist, 0);
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
    if (!gpointer_isValidOrHead(&x->x_gpointer))
    {
        post_error ("pointer_rewind: empty pointer");
        return;
    }

    if (!gpointer_isScalar (&x->x_gpointer)) {
        post_error ("pointer_rewind: sorry, unavailable for arrays");
        return;
    }
    glist = gpointer_getParentGlist (&x->x_gpointer);
    gpointer_setAsScalar(&x->x_gpointer, glist, 0);
    pointer_bang(x);
}

static void pointer_vnext(t_pointer *x, t_float f)
{
    t_gobj *gobj;
    t_gpointer *gp = &x->x_gpointer;
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
        for (n = x->x_outletTypedSize, to = x->x_outletTyped; n--; to++)
        {
            if (to->to_type == templatesym)
            {
                outlet_pointer(to->to_outlet, &x->x_gpointer);
                return;
            }
        }
        outlet_pointer(x->x_outletOther, &x->x_gpointer);
    }
    else
    {
        gpointer_unset(gp);
        outlet_bang(x->x_outletBang);
    }
}

static void pointer_next(t_pointer *x)
{
    pointer_vnext(x, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *pointer_new (t_symbol *s, int argc, t_atom *argv)
{
    t_pointer *x = (t_pointer *)pd_new (pointer_class);
    int i, n = argc;
    
    gpointer_init (&x->x_gpointer);
    
    x->x_outletTypedSize = n;
    x->x_outletTyped     = (t_typedout *)PD_MEMORY_GET (n * sizeof (t_typedout));
    
    for (i = 0; i < n; i++) {
        x->x_outletTyped[i].to_type   = template_makeBindSymbolWithWildcard (atom_getSymbol (argv + i));
        x->x_outletTyped[i].to_outlet = outlet_new (cast_object (x), &s_pointer);
    }
    
    x->x_outletOther = outlet_new (cast_object (x), &s_pointer);
    x->x_outletBang  = outlet_new (cast_object (x), &s_bang);
    
    inlet_newPointer (cast_object (x), &x->x_gpointer);
    
    return x;
}

static void pointer_free (t_pointer *x)
{
    PD_MEMORY_FREE (x->x_outletTyped);
    
    gpointer_unset (&x->x_gpointer);
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
