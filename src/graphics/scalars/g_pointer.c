
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

#define POINTER_NEXT        0
#define POINTER_DELETE      1
#define POINTER_DISABLE     2
#define POINTER_ENABLE      4
#define POINTER_SELECTED    8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *pointer_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct {
    t_symbol    *to_type;
    t_outlet    *to_outlet;
    } t_typedout;

typedef struct _pointer {
    t_object    x_obj;                  /* Must be the first. */
    t_gpointer  x_gpointer;
    int         x_outletTypedSize;
    t_typedout  *x_outletTyped;
    t_glist     *x_owner;
    t_outlet    *x_outletBeforeRight;
    t_outlet    *x_outletRight;
    } t_pointer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_clear (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int pointer_nextSkip (t_gobj *z, t_glist *glist, int wantSelected)
{
    if (!gobj_isScalar (z)) { return 1; }
    else if (wantSelected && !glist_objectIsSelected (glist, z)) { return 1; }
    else {
        return 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void pointer_bang (t_pointer *x)
{
    if (!gpointer_isValidOrNull (&x->x_gpointer)) { error_invalid (&s_pointer, &s_pointer); }
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
    gpointer_setByCopy (&x->x_gpointer, gp);
    
    pointer_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that functions below do not make sense for non scalars (i.g. element of arrays). */

static t_glist *pointer_fetch (t_pointer *x, t_symbol *s)
{
    if (s == &s_) { return x->x_owner; }
    else {
    //
    t_glist *glist = cast_glist (symbol_getThingByClass (symbol_makeBindIfNot (s), canvas_class));
    
    if (glist && !glist_isGraphicArray (glist)) { return glist; }
    
    return NULL;
    //
    }
}

static void pointer_traverse (t_pointer *x, t_symbol *s)
{
    t_glist *glist = pointer_fetch (x, s);
    
    if (glist) { gpointer_setAsNull (&x->x_gpointer, glist); }
    else {
        error_invalid (&s_pointer, sym_window);
    }
}

static void pointer_clear (t_pointer *x, t_symbol *s)
{
    t_glist *glist = pointer_fetch (x, s);
    
    if (glist) { canvas_clear (glist); gpointer_setAsNull (&x->x_gpointer, glist); }
    else {
        error_invalid (&s_pointer, sym_window);
    }
}

static void pointer_rewind (t_pointer *x)
{
    if (gpointer_isValidOrNull (&x->x_gpointer) && gpointer_isScalar (&x->x_gpointer)) {
        gpointer_setAsNull (&x->x_gpointer, gpointer_getOwner (&x->x_gpointer));
    } else {
        error_invalid (&s_pointer, &s_pointer);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void pointer_nextProceed (t_pointer *x, int flag)
{
    int deleteFirst  = (flag & POINTER_DELETE);
    int disableFirst = (flag & POINTER_DISABLE);
    int enableFirst  = (flag & POINTER_ENABLE);
    int wantSelected = (flag & POINTER_SELECTED);

    if (gpointer_isValidOrNull (&x->x_gpointer) && gpointer_isScalar (&x->x_gpointer)) {
    //
    t_glist *glist = gpointer_getOwner (&x->x_gpointer);

    if (!wantSelected || glist_isOnScreen (glist)) {

        t_gobj *z = cast_gobj (gpointer_getScalar (&x->x_gpointer));
        t_gobj *p = z;
        
        if (!z) { z = glist->gl_graphics; }
        else { 
            z = z->g_next;
        }
        
        while (z && pointer_nextSkip (z, glist, wantSelected)) { z = z->g_next; }
        
        if (p && deleteFirst)  { glist_objectRemove (glist, p); }
        if (p && enableFirst)  { scalar_enable (cast_scalar (p)); }
        if (p && disableFirst) { scalar_disable (cast_scalar (p)); }
        
        if (!z) { gpointer_unset (&x->x_gpointer); outlet_bang (x->x_outletRight); }
        else {
            gpointer_setAsScalar (&x->x_gpointer, cast_scalar (z));
            pointer_bang (x);
        }
        
        return;
    }
    //
    }
    
    error_invalid (&s_pointer, &s_pointer);
}

static void pointer_nextEnable (t_pointer *x)
{
    pointer_nextProceed (x, POINTER_NEXT | POINTER_ENABLE);
}

static void pointer_nextDisable (t_pointer *x)
{
    pointer_nextProceed (x, POINTER_NEXT | POINTER_DISABLE);
}

static void pointer_nextDelete (t_pointer *x)
{
    pointer_nextProceed (x, POINTER_NEXT | POINTER_DELETE);
}

static void pointer_next (t_pointer *x)
{
    pointer_nextProceed (x, POINTER_NEXT);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *pointer_new (t_symbol *s, int argc, t_atom *argv)
{
    t_pointer *x = (t_pointer *)pd_new (pointer_class);
    int i, n = argc;
    
    gpointer_init (&x->x_gpointer);
    
    x->x_outletTypedSize = n;
    x->x_outletTyped     = (t_typedout *)PD_MEMORY_GET (n * sizeof (t_typedout));
    
    for (i = 0; i < n; i++) {
        x->x_outletTyped[i].to_type   = template_makeIdentifierWithWildcard (atom_getSymbol (argv + i));
        x->x_outletTyped[i].to_outlet = outlet_newPointer (cast_object (x));
    }
    
    x->x_owner              = instance_contextGetCurrent();
    x->x_outletBeforeRight  = outlet_newPointer (cast_object (x));
    x->x_outletRight        = outlet_newBang (cast_object (x));
    
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
// MARK: -

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
    
    class_addBang (c, (t_method)pointer_bang); 
    class_addPointer (c, (t_method)pointer_pointer); 
        
    class_addMethod (c, (t_method)pointer_traverse,     sym_traverse,               A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)pointer_clear,        sym_clear,                  A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)pointer_rewind,       sym_rewind,                 A_NULL);
    class_addMethod (c, (t_method)pointer_next,         sym_next,                   A_NULL);
    class_addMethod (c, (t_method)pointer_nextDelete,   sym_delete,                 A_NULL);
    class_addMethod (c, (t_method)pointer_nextDisable,  sym_disable,                A_NULL);
    class_addMethod (c, (t_method)pointer_nextEnable,   sym_enable,                 A_NULL);

    pointer_class = c;
}

void pointer_destroy (void)
{
    class_free (pointer_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
