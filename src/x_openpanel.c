
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *openpanel_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _openpanel {
    t_object    x_obj;                      /* Must be the first. */
    t_proxy     *x_proxy;
    t_outlet    *x_outlet;
    } t_openpanel;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void openpanel_symbol (t_openpanel *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void openpanel_bang (t_openpanel *x)
{
    openpanel_symbol (x, &s_);
}

static void openpanel_symbol (t_openpanel *x, t_symbol *s)
{
    gui_vAdd ("::ui_file::openPanel {%s} {%s}\n",   // --
                    proxy_getTagAsString (x->x_proxy),
                    s->s_name);
}

static void openpanel_list (t_openpanel *x, t_symbol *s, int argc, t_atom *argv)
{
    openpanel_symbol (x, symbol_withAtoms (argc, argv));
}

static void openpanel_anything (t_openpanel *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)openpanel_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void openpanel_callback (t_openpanel *x, t_symbol *s)
{
    outlet_symbol (x->x_outlet, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that this proxy is never signoff. */

static void *openpanel_new (void)
{
    t_openpanel *x = (t_openpanel *)pd_new (openpanel_class);

    x->x_proxy  = proxy_new (cast_pd (x));
    x->x_outlet = outlet_new (cast_object (x), &s_symbol);
        
    return x;
}

static void openpanel_free (t_openpanel *x)
{
    proxy_release (x->x_proxy);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void openpanel_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_openpanel,
            (t_newmethod)openpanel_new,
            (t_method)openpanel_free,
            sizeof (t_openpanel),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, (t_method)openpanel_bang);
    class_addSymbol (c, (t_method)openpanel_symbol);
    class_addList (c, (t_method)openpanel_list);
    class_addAnything (c, (t_method)openpanel_anything);
    
    class_addMethod (c, (t_method)openpanel_callback, sym_callback, A_SYMBOL, A_NULL);
    
    openpanel_class = c;
}

void openpanel_destroy (void)
{
    class_free (openpanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
