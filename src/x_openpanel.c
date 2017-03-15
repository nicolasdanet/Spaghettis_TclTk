
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

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

static t_class *openpanel_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _openpanel {
    t_object        x_obj;                  /* Must be the first. */
    t_proxy         *x_proxy;
    t_outlet        *x_outlet;
    } t_openpanel;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void openpanel_symbol (t_openpanel *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void openpanel_bang (t_openpanel *x)
{
    openpanel_symbol (x, &s_);
}

static void openpanel_symbol (t_openpanel *x, t_symbol *s)
{
    sys_vGui ("::ui_file::openPanel {%s} {%s}\n",   // --
                    proxy_getBound (x->x_proxy)->s_name,
                    s->s_name);
}

static void openpanel_callback (t_openpanel *x, t_symbol *s)
{
    outlet_symbol (x->x_outlet, s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

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
    
    class_addMethod (c, (t_method)openpanel_callback, sym_callback, A_SYMBOL, A_NULL);
    
    openpanel_class = c;
}

void openpanel_destroy (void)
{
    CLASS_FREE (openpanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
