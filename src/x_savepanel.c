
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

static t_class *savepanel_class;

typedef struct _savepanel
{
    t_object x_obj;
    t_glist *x_canvas;
    t_symbol *x_s;
} t_savepanel;

static void *savepanel_new( void)
{
    char buf[50];
    t_savepanel *x = (t_savepanel *)pd_new(savepanel_class);
    sprintf(buf, "d%lx", x);
    x->x_s = gensym (buf);
    x->x_canvas = canvas_getCurrent();
    pd_bind(&x->x_obj.te_g.g_pd, x->x_s);
    outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

static void savepanel_symbol(t_savepanel *x, t_symbol *s)
{
    char *path = (s && s->s_name) ? s->s_name : "\"\"";
    sys_vGui("::ui_file::savePanel {%s} {%s}\n", x->x_s->s_name, path);
}

static void savepanel_bang(t_savepanel *x)
{
    savepanel_symbol(x, &s_);
}

static void savepanel_callback(t_savepanel *x, t_symbol *s)
{
    outlet_symbol(x->x_obj.te_outlet, s);
}

static void savepanel_free(t_savepanel *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, x->x_s);
}

void savepanel_setup(void)
{
    savepanel_class = class_new(sym_savepanel,
        (t_newmethod)savepanel_new, (t_method)savepanel_free,
        sizeof(t_savepanel), 0, 0);
    class_addBang(savepanel_class, savepanel_bang);
    class_addSymbol(savepanel_class, savepanel_symbol);
    class_addMethod(savepanel_class, (t_method)savepanel_callback,
        sym_callback, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
