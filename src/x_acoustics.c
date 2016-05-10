/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  utility functions for signals 
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include <math.h>
#define LOGTEN 2.302585092994

t_float mtof(t_float f)
{
    if (f <= -1500) return(0);
    else if (f > 1499) return(mtof(1499));
    else return (8.17579891564 * exp(.0577622650 * f));
}

t_float ftom(t_float f)
{
    return (f > 0 ? 17.3123405046 * log(.12231220585 * f) : -1500);
}

t_float powtodb(t_float f)
{
    if (f <= 0) return (0);
    else
    {
        t_float val = 100 + 10./LOGTEN * log(f);
        return (val < 0 ? 0 : val);
    }
}

t_float rmstodb(t_float f)
{
    if (f <= 0) return (0);
    else
    {
        t_float val = 100 + 20./LOGTEN * log(f);
        return (val < 0 ? 0 : val);
    }
}

t_float dbtopow(t_float f)
{
    if (f <= 0)
        return(0);
    else
    {
        if (f > 870)
            f = 870;
        return (exp((LOGTEN * 0.1) * (f-100.)));
    }
}

t_float dbtorms(t_float f)
{
    if (f <= 0)
        return(0);
    else
    {
        if (f > 485)
            f = 485;
    }
    return (exp((LOGTEN * 0.05) * (f-100.)));
}

/* ------------- corresponding objects ----------------------- */

static t_class *mtof_class;

static void *mtof_new(void)
{
    t_object *x = (t_object *)pd_new(mtof_class);
    outlet_new(x, &s_float);
    return (x);
}

static void mtof_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, mtof(f));
}


static t_class *ftom_class;

static void *ftom_new(void)
{
    t_object *x = (t_object *)pd_new(ftom_class);
    outlet_new(x, &s_float);
    return (x);
}

static void ftom_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, ftom(f));
}


static t_class *rmstodb_class;

static void *rmstodb_new(void)
{
    t_object *x = (t_object *)pd_new(rmstodb_class);
    outlet_new(x, &s_float);
    return (x);
}

static void rmstodb_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, rmstodb(f));
}


static t_class *powtodb_class;

static void *powtodb_new(void)
{
    t_object *x = (t_object *)pd_new(powtodb_class);
    outlet_new(x, &s_float);
    return (x);
}

static void powtodb_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, powtodb(f));
}


static t_class *dbtopow_class;

static void *dbtopow_new(void)
{
    t_object *x = (t_object *)pd_new(dbtopow_class);
    outlet_new(x, &s_float);
    return (x);
}

static void dbtopow_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, dbtopow(f));
}


static t_class *dbtorms_class;

static void *dbtorms_new(void)
{
    t_object *x = (t_object *)pd_new(dbtorms_class);
    outlet_new(x, &s_float);
    return (x);
}

static void dbtorms_float(t_object *x, t_float f)
{
    outlet_float(x->te_outlet, dbtorms(f));
}


void x_acoustics_setup(void)
{
    t_symbol *s = sym_mtof;
    mtof_class = class_new(sym_mtof, mtof_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(mtof_class, (t_method)mtof_float);
    class_setHelpName(mtof_class, s);
    
    ftom_class = class_new(sym_ftom, ftom_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(ftom_class, (t_method)ftom_float);
    class_setHelpName(ftom_class, s);

    powtodb_class = class_new (sym_powtodb, powtodb_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(powtodb_class, (t_method)powtodb_float);
    class_setHelpName(powtodb_class, s);

    rmstodb_class = class_new (sym_rmstodb, rmstodb_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(rmstodb_class, (t_method)rmstodb_float);
    class_setHelpName(rmstodb_class, s);

    dbtopow_class = class_new (sym_dbtopow, dbtopow_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(dbtopow_class, (t_method)dbtopow_float);
    class_setHelpName(dbtopow_class, s);

    dbtorms_class = class_new(sym_dbtorms, dbtorms_new, 0,
        sizeof(t_object), 0, 0);
    class_addFloat(dbtorms_class, (t_method)dbtorms_float);
    class_setHelpName(dbtorms_class, s);
}
   
