
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

static t_class *tabreceive_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _tabreceive {
    t_object    x_obj;                  /* Must be the first. */
    int         x_dismissed;
    t_id        x_tag;
    t_buffer    *x_previous;
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_tabreceive;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_dissmiss (t_tabreceive *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_output (t_tabreceive *x, t_garray *garray)
{
    int i;
    int k = 0;
    int n = 0;
    t_word *data = NULL;
    
    garray_getData (garray, &n, &data);
    
    if (n != buffer_getSize (x->x_previous)) { k = 1; buffer_resize (x->x_previous, n); }
    
    for (i = 0; i < n; i++) {
        t_atom *a = buffer_getAtomAtIndex (x->x_previous, i);
        t_float f = atom_getFloat (a);
        t_float v = w_getFloat (data + i);
        if (f != v) { k = 1; }
        SET_FLOAT (a, v);
    }
    
    if (k) { outlet_list (x->x_outlet, n, buffer_getAtoms (x->x_previous)); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_bang (t_tabreceive *x)
{
    t_garray *garray = (t_garray *)symbol_getThingByClass (x->x_name, garray_class);
    
    if (garray) { buffer_clear (x->x_previous); tabreceive_output (x, garray); }
}

static void tabreceive_set (t_tabreceive *x, t_symbol *s)
{
    x->x_name = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tabreceive_polling (t_tabreceive *x)
{
    t_garray *garray = (t_garray *)symbol_getThingByClass (x->x_name, garray_class);
    
    if (garray && (x->x_tag != garray_getChangeTag (garray))) {
        x->x_tag = garray_getChangeTag (garray);
        tabreceive_output (x, garray);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *tabreceive_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_tabreceive *x = (t_tabreceive *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym_set);
    buffer_appendSymbol (b, x->x_name);
    buffer_appendComma (b);
    buffer_appendSymbol (b, sym__restore);
    
    return b;
    //
    }
    
    return NULL;
}

static void tabreceive_functionDismiss (t_gobj *z)
{
    tabreceive_dissmiss ((t_tabreceive *)z);
}

static void tabreceive_restore (t_tabreceive *x)
{
    t_tabreceive *old = (t_tabreceive *)instance_pendingFetch (cast_gobj (x));

    if (old) { tabreceive_set (x, old->x_name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *tabreceive_new (t_symbol *s)
{
    t_tabreceive *x = (t_tabreceive *)pd_new (tabreceive_class);
    
    x->x_name     = s;
    x->x_previous = buffer_new();
    x->x_outlet   = outlet_newList (cast_object (x));
    
    instance_pollingRegister (cast_pd (x));
    
    return x;
}

static void tabreceive_dissmiss (t_tabreceive *x)
{
    instance_pollingUnregister (cast_pd (x)); x->x_dismissed = 1;
}

static void tabreceive_free (t_tabreceive *x)
{
    if (!x->x_dismissed) { tabreceive_dissmiss (x); }
    
    buffer_free (x->x_previous);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tabreceive_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_tabreceive,
            (t_newmethod)tabreceive_new,
            (t_method)tabreceive_free,
            sizeof (t_tabreceive),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
    
    class_addBang (c, (t_method)tabreceive_bang);
    class_addPolling (c, (t_method)tabreceive_polling);
    
    class_addMethod (c, (t_method)tabreceive_set,       sym_set,        A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)tabreceive_restore,   sym__restore,   A_NULL);
    
    class_setDataFunction (c, tabreceive_functionData);
    class_setDismissFunction (c, tabreceive_functionDismiss);
    class_requirePending (c);
    
    tabreceive_class = c;
}

void tabreceive_destroy (void)
{
    class_free (tabreceive_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
