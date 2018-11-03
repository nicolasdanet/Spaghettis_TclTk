
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *urn_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _urn {
    t_object    x_obj;                  /* Must be the first. */
    t_float     x_range;
    int         x_index;
    t_buffer    *x_buffer;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_urn;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define URN_LIMIT   4096                /* Arbitrary. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void urn_set (t_urn *x)
{
    int i, n = (int)x->x_range;
    
    x->x_index = 0;
    
    buffer_clear (x->x_buffer);
    buffer_reserve (x->x_buffer, n);
    
    for (i = 0; i < n; i++) { buffer_appendFloat (x->x_buffer, (t_float)i); }
    
    buffer_shuffle (x->x_buffer);
}

static int urn_next (t_urn *x)
{
    int n = buffer_getSize (x->x_buffer);
    
    if (x->x_index < n) {
        return (int)atom_getFloatAtIndex (x->x_index++, n, buffer_getAtoms (x->x_buffer));
    }
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void urn_bang (t_urn *x)
{
    int n = urn_next (x);
    
    if (n < 0) { outlet_bang (x->x_outletRight); } else { outlet_float (x->x_outletLeft, (t_float)n); }
}

static void urn_float (t_urn *x, t_float f)
{
    x->x_range = PD_CLAMP (1, f, URN_LIMIT); urn_set (x);
    
    // -- TODO: Use a best approach in case of a large range?
    
    /* < https://meta.stackoverflow.com/questions/334325 > */
    
    if (f > URN_LIMIT) { warning_invalid (sym_urn, sym_range); }
}

static void urn_clear (t_urn *x)
{
    urn_float (x, x->x_range);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *urn_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_urn *x = (t_urn *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__inlet2);
    buffer_appendFloat (b, x->x_range);
    
    return b;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *urn_new (t_float f)
{
    t_urn *x = (t_urn *)pd_new (urn_class);
    
    x->x_buffer      = buffer_new();
    x->x_outletLeft  = outlet_newFloat (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    inlet_new2 (x, &s_float);
    
    urn_float (x, f);
    
    return x;
}

static void urn_free (t_urn *x)
{
    buffer_free (x->x_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void urn_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_urn,
            (t_newmethod)urn_new,
            (t_method)urn_free,
            sizeof (t_urn),
            CLASS_DEFAULT,
            A_DEFFLOAT,
            A_NULL);
            
    class_addBang (c, (t_method)urn_bang);
    
    class_addMethod (c, (t_method)urn_clear, sym_clear,     A_NULL);
    class_addMethod (c, (t_method)urn_float, sym__inlet2,   A_FLOAT, A_NULL);

    class_setDataFunction (c, urn_functionData);
    
    urn_class = c;
}

void urn_destroy (void)
{
    class_free (urn_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
