
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

extern t_pd *pd_newest;

static t_class *bang_class;

typedef struct _bang
{
    t_object x_obj;
} t_bang;

/* Called by the t_bangmethod of the object maker class. */

static void *bang_new(t_pd *dummy)
{
    t_bang *x = (t_bang *)pd_new(bang_class);
    outlet_new(&x->x_obj, &s_bang);
    pd_newest = &x->x_obj.te_g.g_pd;
    return (x);
}

static void *bang_new2(t_bang f)
{
    return (bang_new(0));
}

static void bang_bang(t_bang *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

void bang_setup(void)
{
    bang_class = class_new (&s_bang, (t_newmethod)bang_new, 0,
        sizeof(t_bang), 0, 0);
    class_addCreator((t_newmethod)bang_new2, sym_b, 0);
    class_addBang(bang_class, bang_bang);
    class_addFloat(bang_class, bang_bang);
    class_addSymbol(bang_class, bang_bang);
    class_addList(bang_class, bang_bang);
    class_addAnything(bang_class, bang_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
