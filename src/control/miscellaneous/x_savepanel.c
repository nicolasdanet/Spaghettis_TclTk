
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
    t_symbol    *x_last;
    t_glist     *x_owner;
    t_proxy     *x_proxy;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_savepanel;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void savepanel_symbol (t_savepanel *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void savepanel_bang (t_savepanel *x)
{
    savepanel_symbol (x, x->x_last);
}

static void savepanel_symbol (t_savepanel *x, t_symbol *s)
{
    gui_vAdd ("::ui_file::savePanel {%s} {%s}\n",   // --
                    proxy_getTagAsString (x->x_proxy),
                    s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void savepanel_callback (t_savepanel *x, t_symbol *s)
{
    glist_redrawRequired (x->x_owner);      /* Avoid artefacts due to the modal window. */

    x->x_last = s;
    
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

static t_buffer *savepanel_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_savepanel *x = (t_savepanel *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendSymbol (b, x->x_last);
    
    return b;
    //
    }
    
    return NULL;
}

static void savepanel_restore (t_savepanel *x, t_symbol *s)
{
    t_savepanel *old = (t_savepanel *)instance_pendingFetch (cast_gobj (x));

    x->x_last = old ? old->x_last : s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that this proxy is never signoff. */

static void *savepanel_new (void)
{
    t_savepanel *x = (t_savepanel *)pd_new (savepanel_class);
    
    x->x_owner       = instance_contextGetCurrent();
    x->x_last        = environment_getDirectory (glist_getEnvironment (x->x_owner));
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
    class_addMethod (c, (t_method)savepanel_restore,    sym__restore,   A_SYMBOL, A_NULL);
    
    class_setDataFunction (c, savepanel_functionData);
    class_requirePending (c);
    
    savepanel_class = c;
}

void savepanel_destroy (void)
{
    class_free (savepanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
