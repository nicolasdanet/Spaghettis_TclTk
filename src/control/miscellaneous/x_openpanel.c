
/* Copyright (c) 1997-2020 Miller Puckette and others. */

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
    int         x_directory;
    int         x_multiple;
    t_symbol    *x_last;
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
    openpanel_symbol (x, x->x_last);
}

static void openpanel_symbol (t_openpanel *x, t_symbol *s)
{
    if (x->x_directory) {
    
        gui_vAdd ("::ui_file::openPanelDirectory {%s} {%s}\n",      // --
                        proxy_getTagAsString (x->x_proxy),
                        s->s_name);
    
    } else {
    
        gui_vAdd ("::ui_file::openPanelFile {%s} {%s} %d\n",        // --
                        proxy_getTagAsString (x->x_proxy),
                        s->s_name,
                        x->x_multiple);
    }
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

static void openpanel_callback (t_openpanel *x, t_symbol *dummy, int argc, t_atom *argv)
{
    glist_redrawRequired (x->x_owner);      /* Avoid artefacts due to the modal window. */
    
    x->x_last = atom_getSymbolAtIndex (argc - 1, argc, argv);
    
    if (x->x_multiple == 0 && argc == 1) { outlet_symbol (x->x_outletLeft, x->x_last); }
    else {
        outlet_list (x->x_outletLeft, argc, argv);
    }
}

static void openpanel_cancel (t_openpanel *x)
{
    glist_redrawRequired (x->x_owner);      /* Ditto. */
    
    outlet_bang (x->x_outletRight);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *openpanel_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_openpanel *x = (t_openpanel *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendSymbol (b, x->x_last);
    
    return b;
    //
    }
    
    return NULL;
}

static void openpanel_restore (t_openpanel *x, t_symbol *s)
{
    t_openpanel *old = (t_openpanel *)instance_pendingFetch (cast_gobj (x));

    x->x_last = old ? old->x_last : s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Notice that this proxy is never signoff. */

static void *openpanel_new (t_symbol *s, int argc, t_atom *argv)
{
    t_openpanel *x = (t_openpanel *)pd_new (openpanel_class);

    while (argc) {
    
        t_symbol *t = atom_getSymbol (argv);
        
        if (t == sym___dash__directory)     { x->x_directory = 1; argc--; argv++; }
        else if (t == sym___dash__multiple) { x->x_multiple  = 1; argc--; argv++; }
        else {
            break;
        }
    }

    error__options (s, argc, argv);
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_owner       = instance_contextGetCurrent();
    x->x_proxy       = proxy_new (cast_pd (x));
    x->x_outletLeft  = outlet_newMixed (cast_object (x));
    x->x_outletRight = outlet_newBang (cast_object (x));
    
    if (x->x_directory) { x->x_last = environment_getFilePath (glist_getEnvironment (x->x_owner)); }
    else {
        x->x_last = environment_getDirectory (glist_getEnvironment (x->x_owner));
    }
    
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
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)openpanel_bang);
    class_addSymbol (c, (t_method)openpanel_symbol);
    class_addList (c, (t_method)openpanel_list);
    class_addAnything (c, (t_method)openpanel_anything);
    
    class_addMethod (c, (t_method)openpanel_callback,   sym__callback,  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)openpanel_cancel,     sym__cancel,    A_NULL);
    class_addMethod (c, (t_method)openpanel_restore,    sym__restore,   A_SYMBOL, A_NULL);
    
    class_setDataFunction (c, openpanel_functionData);
    class_requirePending (c);
    
    openpanel_class = c;
}

void openpanel_destroy (void)
{
    class_free (openpanel_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
