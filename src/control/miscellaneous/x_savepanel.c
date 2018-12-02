
/* Copyright (c) 1997-2019 Miller Puckette and others. */

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

static t_class *savepanel_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _savepanel {
    t_object    x_obj;                  /* Must be the first. */
    t_glist     *x_owner;
    t_proxy     *x_proxy;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_savepanel;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void savepanel_symbol (t_savepanel *x, t_symbol *s)
{
    gui_vAdd ("::ui_file::savePanel {%s} {%s}\n",   // --
                    proxy_getTagAsString (x->x_proxy),
                    s->s_name);
}

static void savepanel_bang (t_savepanel *x)
{
    savepanel_symbol (x, &s_);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void savepanel_callback (t_savepanel *x, t_symbol *s)
{
    glist_redrawRequired (x->x_owner);      /* Avoid artefacts due to the modal window. */

    outlet_symbol (x->x_outletLeft, s);
}

static void savepanel_cancel (t_savepanel *x)
{
    glist_redrawRequired (x->x_owner);      /* Ditto. */
    
    outlet_bang (x->x_outletRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that this proxy is never signoff. */

static void *savepanel_new (void)
{
    t_savepanel *x = (t_savepanel *)pd_new (savepanel_class);
    
    x->x_owner       = instance_contextGetCurrent();
    x->x_proxy       = proxy_new (cast_pd (x));
    x->x_outletLeft  = outlet_newSymbol (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    return x;
}

static void savepanel_free (t_savepanel *x)
{
    proxy_release (x->x_proxy);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void savepanel_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_savepanel,
            (t_newmethod)savepanel_new,
            (t_method)savepanel_free,
            sizeof (t_savepanel),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, (t_method)savepanel_bang);
    class_addSymbol (c, (t_method)savepanel_symbol);
    
    class_addMethod (c, (t_method)savepanel_callback,   sym__callback,  A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)savepanel_cancel,     sym__cancel,    A_NULL);
    
    savepanel_class = c;
}

void savepanel_destroy (void)
{
    class_free (savepanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
