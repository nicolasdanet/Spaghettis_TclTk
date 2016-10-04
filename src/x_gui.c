
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

static void openpanel_setup(void)
{
    openpanel_class = class_new(sym_openpanel,
        (t_newmethod)openpanel_new, (t_method)openpanel_free,
        sizeof(t_openpanel), 0, 0);
    class_addBang(openpanel_class, openpanel_bang);
    class_addSymbol(openpanel_class, openpanel_symbol);
    class_addMethod(openpanel_class, (t_method)openpanel_callback,
        sym_callback, A_SYMBOL, 0);
}

/* -------------------------- savepanel ------------------------------ */

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

static void savepanel_setup(void)
{
    savepanel_class = class_new(sym_savepanel,
        (t_newmethod)savepanel_new, (t_method)savepanel_free,
        sizeof(t_savepanel), 0, 0);
    class_addBang(savepanel_class, savepanel_bang);
    class_addSymbol(savepanel_class, savepanel_symbol);
    class_addMethod(savepanel_class, (t_method)savepanel_callback,
        sym_callback, A_SYMBOL, 0);
}

/* ---------------------- key and its relatives ------------------ */

static t_symbol *key_sym;
static t_class *key_class;

typedef struct _key
{
    t_object x_obj;
} t_key;

static void *key_new( void)
{
    t_key *x = (t_key *)pd_new(key_class);
    outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, key_sym);
    return (x);
}

static void key_float(t_key *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, f);
}

static void key_free(t_key *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, key_sym);
}

static void key_setup(void)
{
    key_class = class_new (sym_key,
        (t_newmethod)key_new, (t_method)key_free,
        sizeof(t_key), CLASS_NOINLET, 0);
    class_addFloat(key_class, key_float);
    key_sym = sym__key;
}

/* -------------------------- setup routine ------------------------------ */

void x_gui_setup(void)
{
    openpanel_setup();
    savepanel_setup();
    key_setup();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
