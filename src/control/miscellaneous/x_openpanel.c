
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *openpanel_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _openpanel {
    t_object    x_obj;                      /* Must be the first. */
    t_glist     *x_owner;
    t_proxy     *x_proxy;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
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
    glist_redrawRequired (x->x_owner);      /* Avoid artefacts due to the modal window. */

    outlet_symbol (x->x_outletLeft, s);
}

static void openpanel_cancel (t_openpanel *x)
{
    glist_redrawRequired (x->x_owner);      /* Ditto. */
    
    outlet_bang (x->x_outletRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that this proxy is never signoff. */

static void *openpanel_new (void)
{
    t_openpanel *x = (t_openpanel *)pd_new (openpanel_class);

    x->x_owner       = instance_contextGetCurrent();
    x->x_proxy       = proxy_new (cast_pd (x));
    x->x_outletLeft  = outlet_newSymbol (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
        
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
    
    class_addMethod (c, (t_method)openpanel_callback,   sym__callback,  A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)openpanel_cancel,     sym__cancel,    A_NULL);
    
    openpanel_class = c;
}

void openpanel_destroy (void)
{
    class_free (openpanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
