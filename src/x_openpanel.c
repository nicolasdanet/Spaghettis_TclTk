
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

/* -------------------------- openpanel ------------------------------ */

static t_class *openpanel_class;

typedef struct _openpanel
{
    t_object x_obj;
    t_symbol *x_s;
} t_openpanel;

static void *openpanel_new( void)
{
    char buf[50];
    t_openpanel *x = (t_openpanel *)pd_new(openpanel_class);
    sprintf(buf, "d%lx", x);
    x->x_s = gensym (buf);
    pd_bind(&x->x_obj.te_g.g_pd, x->x_s);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

static void openpanel_symbol(t_openpanel *x, t_symbol *s)
{
    char *path = (s && s->s_name) ? s->s_name : "\"\"";
    sys_vGui("::ui_file::openPanel {%s} {%s}\n", x->x_s->s_name, path);
}

static void openpanel_bang(t_openpanel *x)
{
    openpanel_symbol(x, &s_);
}

static void openpanel_callback(t_openpanel *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.te_outlet, s);
}


static void openpanel_free(t_openpanel *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_s);
}

void openpanel_setup(void)
{
    openpanel_class = class_new(sym_openpanel,
        (t_newmethod)openpanel_new, (t_method)openpanel_free,
        sizeof(t_openpanel), 0, 0);
    class_addBang(openpanel_class, openpanel_bang);
    class_addSymbol(openpanel_class, openpanel_symbol);
    class_addMethod(openpanel_class, (t_method)openpanel_callback,
        sym_callback, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
