
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
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"

/* ------------- namecanvas (delete this later) --------------------- */
static t_class *namecanvas_class;

typedef struct _namecanvas
{
    t_object x_obj;
    t_symbol *x_sym;
    t_pd *x_owner;
} t_namecanvas;

static void *namecanvas_new(t_symbol *s)
{
    t_namecanvas *x = (t_namecanvas *)pd_new(namecanvas_class);
    x->x_owner = (t_pd *)canvas_getCurrent();
    x->x_sym = s;
    if (*s->s_name) pd_bind(x->x_owner, s);
    return (x);
}

static void namecanvas_free(t_namecanvas *x)
{
    if (*x->x_sym->s_name) pd_unbind(x->x_owner, x->x_sym);
}

void namecanvas_setup(void)
{
    namecanvas_class = class_new(sym_namecanvas,
        (t_newmethod)namecanvas_new, (t_method)namecanvas_free,
            sizeof(t_namecanvas), CLASS_NOINLET, A_DEFSYMBOL, 0);
}
