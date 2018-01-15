
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *mtof_class;         /* Shared. */
static t_class *ftom_class;         /* Shared. */
static t_class *powtodb_class;      /* Shared. */
static t_class *dbtopow_class;      /* Shared. */
static t_class *rmstodb_class;      /* Shared. */
static t_class *dbtorms_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _acoustic {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    t_outlet    *x_outlet;
    } t_acoustic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *mtof_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (mtof_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void mtof_bang (t_acoustic *x)
{
    outlet_float (x->x_outlet, math_midiToFrequency (x->x_f));
}

static void mtof_float (t_acoustic *x, t_float f)
{
    x->x_f = f; mtof_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *ftom_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (ftom_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void ftom_bang (t_acoustic *x)
{
    outlet_float (x->x_outlet, math_frequencyToMidi (x->x_f));
}

static void ftom_float (t_acoustic *x, t_float f)
{
    x->x_f = f; ftom_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *powtodb_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (powtodb_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void powtodb_bang (t_acoustic *x)
{
    outlet_float (x->x_outlet, math_powerToDecibel (x->x_f));
}

static void powtodb_float (t_acoustic *x, t_float f)
{
    x->x_f = f; powtodb_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dbtopow_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (dbtopow_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void dbtopow_bang (t_acoustic *x)
{
    outlet_float (x->x_outlet, math_decibelToPower (x->x_f));
}

static void dbtopow_float (t_acoustic *x, t_float f)
{
    x->x_f = f; dbtopow_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *rmstodb_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (rmstodb_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void rmstodb_bang (t_acoustic *x)
{
    outlet_float (x->x_outlet, math_rootMeanSquareToDecibel (x->x_f));
}

static void rmstodb_float (t_acoustic *x, t_float f)
{
    x->x_f = f; rmstodb_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *dbtorms_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (dbtorms_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void dbtorms_bang (t_acoustic *x)
{
    outlet_float (x->x_outlet, math_decibelToRootMeanSquare (x->x_f));
}

static void dbtorms_float (t_acoustic *x, t_float f)
{
    x->x_f = f; dbtorms_bang (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void acoustic_setup (void)
{
    mtof_class = class_new (sym_mtof, 
                        (t_newmethod)mtof_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    ftom_class = class_new (sym_ftom,
                        (t_newmethod)ftom_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    powtodb_class = class_new (sym_powtodb,
                        (t_newmethod)powtodb_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);
                        
    dbtopow_class = class_new (sym_dbtopow, 
                        (t_newmethod)dbtopow_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    rmstodb_class = class_new (sym_rmstodb,
                        (t_newmethod)rmstodb_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    dbtorms_class = class_new (sym_dbtorms,
                        (t_newmethod)dbtorms_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);
    
    class_addBang (mtof_class, (t_method)mtof_bang);
    class_addBang (ftom_class, (t_method)ftom_bang);
    class_addBang (powtodb_class, (t_method)powtodb_bang);
    class_addBang (dbtopow_class, (t_method)dbtopow_bang);
    class_addBang (rmstodb_class, (t_method)rmstodb_bang);
    class_addBang (dbtorms_class, (t_method)dbtorms_bang);
    
    class_addFloat (mtof_class, (t_method)mtof_float);
    class_addFloat (ftom_class, (t_method)ftom_float);
    class_addFloat (powtodb_class, (t_method)powtodb_float);
    class_addFloat (dbtopow_class, (t_method)dbtopow_float);
    class_addFloat (rmstodb_class, (t_method)rmstodb_float);
    class_addFloat (dbtorms_class, (t_method)dbtorms_float);
    
    class_setHelpName (mtof_class, sym_acoustic);
    class_setHelpName (ftom_class, sym_acoustic);
    class_setHelpName (powtodb_class, sym_acoustic);
    class_setHelpName (dbtopow_class, sym_acoustic);
    class_setHelpName (rmstodb_class, sym_acoustic);
    class_setHelpName (dbtorms_class, sym_acoustic);
    
    class_addCreator ((t_newmethod)dbtorms_new, sym_dbtoamp, A_NULL);
    class_addCreator ((t_newmethod)rmstodb_new, sym_amptodb, A_NULL);
}

void acoustic_destroy (void)
{
    class_free (mtof_class);
    class_free (ftom_class);
    class_free (powtodb_class);
    class_free (dbtopow_class);
    class_free (rmstodb_class);
    class_free (dbtorms_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

