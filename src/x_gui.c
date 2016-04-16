/* Copyright (c) 1997-2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* dialogs.  LATER, deal with the situation where the object goes 
away before the panel does... */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"
#include <stdio.h>
#include <string.h>

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
    sprintf(buf, "d%lx", (t_int)x);
    x->x_s = gensym(buf);
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
    openpanel_class = class_new(gensym("openpanel"),
        (t_newmethod)openpanel_new, (t_method)openpanel_free,
        sizeof(t_openpanel), 0, 0);
    class_addBang(openpanel_class, openpanel_bang);
    class_addSymbol(openpanel_class, openpanel_symbol);
    class_addMethod(openpanel_class, (t_method)openpanel_callback,
        gensym("callback"), A_SYMBOL, 0);
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
    sprintf(buf, "d%lx", (t_int)x);
    x->x_s = gensym(buf);
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
    savepanel_class = class_new(gensym("savepanel"),
        (t_newmethod)savepanel_new, (t_method)savepanel_free,
        sizeof(t_savepanel), 0, 0);
    class_addBang(savepanel_class, savepanel_bang);
    class_addSymbol(savepanel_class, savepanel_symbol);
    class_addMethod(savepanel_class, (t_method)savepanel_callback,
        gensym("callback"), A_SYMBOL, 0);
}

/* ---------------------- key and its relatives ------------------ */

static t_symbol *key_sym, *keyup_sym, *keyname_sym;
static t_class *key_class, *keyup_class, *keyname_class;

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

typedef struct _keyup
{
    t_object x_obj;
} t_keyup;

static void *keyup_new( void)
{
    t_keyup *x = (t_keyup *)pd_new(keyup_class);
    outlet_new(&x->x_obj, &s_float);
    pd_bind(&x->x_obj.te_g.g_pd, keyup_sym);
    return (x);
}

static void keyup_float(t_keyup *x, t_float f)
{
    outlet_float(x->x_obj.te_outlet, f);
}

static void keyup_free(t_keyup *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, keyup_sym);
}

typedef struct _keyname
{
    t_object x_obj;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
} t_keyname;

static void *keyname_new( void)
{
    t_keyname *x = (t_keyname *)pd_new(keyname_class);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_float);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_symbol);
    pd_bind(&x->x_obj.te_g.g_pd, keyname_sym);
    return (x);
}

static void keyname_list(t_keyname *x, t_symbol *s, int ac, t_atom *av)
{
    outlet_symbol(x->x_outlet2, atom_getSymbolAtIndex(1, ac, av));
    outlet_float(x->x_outlet1, atom_getFloatAtIndex(0, ac, av));
}

static void keyname_free(t_keyname *x)
{
    pd_unbind(&x->x_obj.te_g.g_pd, keyname_sym);
}

static void key_setup(void)
{
    key_class = class_new(gensym("key"),
        (t_newmethod)key_new, (t_method)key_free,
        sizeof(t_key), CLASS_NOINLET, 0);
    class_addFloat(key_class, key_float);
    key_sym = gensym("#key");

    keyup_class = class_new(gensym("keyup"),
        (t_newmethod)keyup_new, (t_method)keyup_free,
        sizeof(t_keyup), CLASS_NOINLET, 0);
    class_addFloat(keyup_class, keyup_float);
    keyup_sym = gensym("#keyup");
    class_setHelpName(keyup_class, gensym("key"));
    
    keyname_class = class_new(gensym("keyname"),
        (t_newmethod)keyname_new, (t_method)keyname_free,
        sizeof(t_keyname), CLASS_NOINLET, 0);
    class_addList(keyname_class, keyname_list);
    keyname_sym = gensym("#keyname");
    class_setHelpName(keyname_class, gensym("key"));
}

/* -------------------------- setup routine ------------------------------ */

void x_gui_setup(void)
{
    openpanel_setup();
    savepanel_setup();
    key_setup();
}
