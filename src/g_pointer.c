
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
    t_object    x_obj;                  /* Must be the first. */
    t_gpointer  x_gpointer;
    int         x_outletTypedSize;
    t_typedout  *x_outletTyped;
    t_outlet    *x_outletBeforeRight;
    t_outlet    *x_outletRight;
    } t_pointer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int pointer_nextSkip (t_gobj *z, t_glist *glist, int wantSelected)
{
    if (pd_class (z) != scalar_class) { return 1; }
    else if (wantSelected && !canvas_isObjectSelected (glist, z)) { return 1; }
    else {
        return 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pointer_bang (t_pointer *x)
{
    if (!gpointer_isValidNullAllowed (&x->x_gpointer)) { error_invalid (&s_pointer, &s_pointer); }
    else {
    //
    int i;
    t_symbol *templateIdentifier = gpointer_getTemplateIdentifier (&x->x_gpointer);
        
    for (i = 0; i < x->x_outletTypedSize; i++) {
        if (x->x_outletTyped[i].to_type == templateIdentifier) {
            outlet_pointer (x->x_outletTyped[i].to_outlet, &x->x_gpointer);
            return;
        }
    }
    
    outlet_pointer (x->x_outletBeforeRight, &x->x_gpointer);
    //
    } 
}


static void pointer_pointer (t_pointer *x, t_gpointer *gp)
{
    gpointer_setByCopy (gp, &x->x_gpointer);
    
    pointer_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void pointer_sendwindow (t_pointer *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!gpointer_isValidNullAllowed (&x->x_gpointer)) { error_invalid (&s_pointer, &s_pointer); }
    else if (argc && IS_SYMBOL (argv)) {
        t_glist *view = canvas_getView (gpointer_getView (&x->x_gpointer));
        pd_message (cast_pd (view), GET_SYMBOL (argv), argc - 1, argv + 1);
    }
}

static void pointer_send (t_pointer *x, t_symbol *s)
{
    if (!gpointer_isValidNullAllowed (&x->x_gpointer)) { error_invalid (&s_pointer, &s_pointer); }
    else {
        if (pd_isThing (s)) { pd_pointer (s->s_thing, &x->x_gpointer); }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that functions below do not make sense for non scalars. */

static void pointer_traverse (t_pointer *x, t_symbol *s)
{
    t_glist *glist = (t_glist *)pd_findByClass (s, canvas_class);
    
    if (glist) { gpointer_setAsScalar (&x->x_gpointer, glist, NULL); }
    else { 
        error_invalid (&s_pointer, &s_pointer);
    }
}

static void pointer_rewind (t_pointer *x)
{
    if (gpointer_isValidNullAllowed (&x->x_gpointer) && gpointer_isScalar (&x->x_gpointer)) {
        gpointer_setAsScalar (&x->x_gpointer, gpointer_getParentGlist (&x->x_gpointer), NULL);
    } else {
        error_invalid (&s_pointer, &s_pointer);
    }
}

static void pointer_nextSelected (t_pointer *x, t_float f)
{
    int wantSelected = (f != 0.0);
    
    if (gpointer_isValidNullAllowed (&x->x_gpointer) && gpointer_isScalar (&x->x_gpointer)) {
    //
    t_glist *glist = gpointer_getParentGlist (&x->x_gpointer);

    if (!wantSelected || canvas_isMapped (glist)) {
    //
    t_gobj *z = cast_gobj (gpointer_getScalar (&x->x_gpointer));
    
    if (!z) { z = glist->gl_graphics; }
    else { 
        z = z->g_next;
    }
    
    while (z && pointer_nextSkip (z, glist, wantSelected)) { z = z->g_next; }
    
    if (!z) { gpointer_unset (&x->x_gpointer); outlet_bang (x->x_outletRight); }
    else {
        int i;
        t_symbol *templateIdentifier = scalar_getTemplateIdentifier (cast_scalar (z));
        
        gpointer_setAsScalar (&x->x_gpointer, glist, cast_scalar (z));

        for (i = 0; i < x->x_outletTypedSize; i++) {
            if (x->x_outletTyped[i].to_type == templateIdentifier) {
                outlet_pointer (x->x_outletTyped[i].to_outlet, &x->x_gpointer); return;
            }
        }
        
        outlet_pointer (x->x_outletBeforeRight, &x->x_gpointer);
    }
    
    return;
    }
    //
    }
    
    error_invalid (&s_pointer, &s_pointer);
}

static void pointer_next (t_pointer *x)
{
    pointer_nextSelected (x, 0.0);
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
        x->x_outletTyped[i].to_type   = template_makeIdentifierWithWildcard (atom_getSymbol (argv + i));
        x->x_outletTyped[i].to_outlet = outlet_new (cast_object (x), &s_pointer);
    }
    
    x->x_outletBeforeRight  = outlet_new (cast_object (x), &s_pointer);
    x->x_outletRight        = outlet_new (cast_object (x), &s_bang);
    
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
    
    c = class_new (&s_pointer,
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
    class_addMethod (c, (t_method)pointer_nextSelected, sym_nextselected,           A_DEFFLOAT, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)pointer_nextSelected, sym_vnext,                  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)pointer_sendwindow,   sym_send__dash__window,     A_GIMME, A_NULL); 
    
    #endif

    pointer_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
