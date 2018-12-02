
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *atan2_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _atan2 {
    t_object    x_obj;
    t_float     x_f1;
    t_float     x_f2;
    t_outlet    *x_outlet;
    } t_atan2;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *atan2_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_atan2 *x  = (t_atan2 *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendFloat (b, x->x_f2);
    
    return b;
    //
    }
    
    return NULL;
}

static void atan2_restore (t_atan2 *x, t_float f)
{
    x->x_f2 = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *atan2_new (void)
{
    t_atan2 *x = (t_atan2 *)pd_new (atan2_class);
    
    x->x_outlet = outlet_newFloat (cast_object (x));
        
    inlet_newFloat (cast_object (x), &x->x_f2);

    return x;
}

static void atan2_bang (t_atan2 *x)
{
    outlet_float (x->x_outlet, (x->x_f1 == 0.0 && x->x_f2 == 0.0 ? 0.0 : atan2 (x->x_f1, x->x_f2)));
}

static void atan2_float (t_atan2 *x, t_float f)
{
    x->x_f1 = f; atan2_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atan2_setup (void)
{
    t_class *c = NULL;

    c = class_new (sym_atan2,
            (t_newmethod)atan2_new,
            NULL,
            sizeof (t_atan2),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)atan2_bang);
    class_addFloat (c, (t_method)atan2_float);
    
    class_addMethod (c, (t_method)atan2_restore, sym__restore, A_FLOAT, A_NULL);

    class_setDataFunction (c, atan2_functionData);
    class_setHelpName (c, sym_math);
    
    atan2_class = c;
}

void atan2_destroy (void)
{
    class_free (atan2_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
