
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *savepanel_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _savepanel {
    t_object    x_obj;                  /* Must be the first. */
    t_proxy     *x_proxy;
    t_outlet    *x_outlet;
    } t_savepanel;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void savepanel_symbol (t_savepanel *x, t_symbol *s)
{
    sys_vGui ("::ui_file::savePanel {%s} {%s}\n",   // --
                    proxy_getTagAsString (x->x_proxy),
                    s->s_name);
}

static void savepanel_bang (t_savepanel *x)
{
    savepanel_symbol (x, &s_);
}

static void savepanel_callback (t_savepanel *x, t_symbol *s)
{
    outlet_symbol (x->x_outlet, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Notice that this proxy is never signoff. */

static void *savepanel_new (void)
{
    t_savepanel *x = (t_savepanel *)pd_new (savepanel_class);
    
    x->x_proxy  = proxy_new (cast_pd (x));
    x->x_outlet = outlet_new (cast_object (x), &s_symbol);
    
    return x;
}

static void savepanel_free (t_savepanel *x)
{
    proxy_release (x->x_proxy);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    class_addMethod (c, (t_method)savepanel_callback, sym_callback, A_SYMBOL, A_NULL);
    
    savepanel_class = c;
}

void savepanel_destroy (void)
{
    CLASS_FREE (savepanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
