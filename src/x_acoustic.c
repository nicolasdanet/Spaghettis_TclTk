
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
    t_outlet    *x_outlet;
    } t_acoustic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *mtof_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (mtof_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void mtof_float (t_acoustic *x, t_float f)
{
    outlet_float (x->x_outlet, math_midiToFrequency (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *ftom_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (ftom_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void ftom_float (t_acoustic *x, t_float f)
{
    outlet_float (x->x_outlet, math_frequencyToMidi (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *powtodb_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (powtodb_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void powtodb_float (t_acoustic *x, t_float f)
{
    outlet_float (x->x_outlet, math_powerToDecibel (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *dbtopow_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (dbtopow_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void dbtopow_float (t_acoustic *x, t_float f)
{
    outlet_float (x->x_outlet, math_decibelToPower (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *rmstodb_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (rmstodb_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void rmstodb_float (t_acoustic *x, t_float f)
{
    outlet_float (x->x_outlet, math_rootMeanSquareToDecibel (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *dbtorms_new (void)
{
    t_acoustic *x = (t_acoustic *)pd_new (dbtorms_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void dbtorms_float (t_acoustic *x, t_float f)
{
    outlet_float (x->x_outlet, math_decibelToRootMeanSquare (f));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void acoustic_setup (void)
{
    mtof_class = class_new (sym_mtof, 
                        mtof_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    ftom_class = class_new (sym_ftom,
                        ftom_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    powtodb_class = class_new (sym_powtodb,
                        powtodb_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);
                        
    dbtopow_class = class_new (sym_dbtopow, 
                        dbtopow_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    rmstodb_class = class_new (sym_rmstodb,
                        rmstodb_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);

    dbtorms_class = class_new (sym_dbtorms,
                        dbtorms_new,
                        NULL,
                        sizeof (t_acoustic),
                        CLASS_DEFAULT,
                        A_NULL);
    
    class_addFloat (mtof_class, (t_method)mtof_float);
    class_addFloat (ftom_class, (t_method)ftom_float);
    class_addFloat (powtodb_class, (t_method)powtodb_float);
    class_addFloat (dbtopow_class, (t_method)dbtopow_float);
    class_addFloat (rmstodb_class, (t_method)rmstodb_float);
    class_addFloat (dbtorms_class, (t_method)dbtorms_float);
    
    class_setHelpName (mtof_class, sym_mtof);
    class_setHelpName (ftom_class, sym_mtof);
    class_setHelpName (powtodb_class, sym_mtof);
    class_setHelpName (dbtopow_class, sym_mtof);
    class_setHelpName (rmstodb_class, sym_mtof);
    class_setHelpName (dbtorms_class, sym_mtof);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

