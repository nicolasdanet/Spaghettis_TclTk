
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

static t_class *clip_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _clip
{
    t_object x_ob;
    t_float x_f1;
    t_float x_f2;
    t_float x_f3;
} t_clip;

static void *clip_new(t_float f1, t_float f2)
{
    t_clip *x = (t_clip *)pd_new(clip_class);
    inlet_newFloat(&x->x_ob, &x->x_f2);
    inlet_newFloat(&x->x_ob, &x->x_f3);
    outlet_new(&x->x_ob, &s_float);
    x->x_f2 = f1;
    x->x_f3 = f2;
    return (x);
}

static void clip_bang(t_clip *x)
{
        outlet_float(x->x_ob.te_outlet, (x->x_f1 < x->x_f2 ? x->x_f2 : (
        x->x_f1 > x->x_f3 ? x->x_f3 : x->x_f1)));
}

static void clip_float(t_clip *x, t_float f)
{
        x->x_f1 = f;
        outlet_float(x->x_ob.te_outlet, (x->x_f1 < x->x_f2 ? x->x_f2 : (
        x->x_f1 > x->x_f3 ? x->x_f3 : x->x_f1)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clip_setup (void)
{
    clip_class = class_new (sym_clip, (t_newmethod)clip_new, 0,
        sizeof(t_clip), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addFloat(clip_class, clip_float);
    class_addBang(clip_class, clip_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
