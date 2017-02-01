
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd global_object;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Messy ping-pong required in order to check saving sequentially. */
/* Furthermore it avoids the application to quit before responding. */
/* Note that patches not dirty are closed later. */

void global_shouldQuit (void *dummy)
{
    t_glist *glist = NULL;
    
    for (glist = pd_this->pd_roots; glist; glist = glist->gl_next) {
    //
    if (canvas_isDirty (glist)) {
    //
    sys_vGui ("::ui_confirm::checkClose .x%lx"
                    " { ::ui_interface::pdsend $top save 2 }"
                    " { ::ui_interface::pdsend $top close 2 }"
                    " {}\n",    // --
                    glist);
    return;
    //
    }
    //
    }
    
    interface_quit (NULL);
}

static void global_key (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    canvas_key (NULL, s, argc, argv);
}

static void global_dummy (void *dummy)
{
}

static void global_default (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    error_unknownMethod (class_getName (pd_class (x)), s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_pd, 
            NULL,
            NULL,
            0,
            CLASS_ABSTRACT,
            A_NULL);

    class_addMethod (c, (t_method)canvas_newPatch,          sym_new,    A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)buffer_fileOpen,          sym_open,   A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)dsp_state,                sym_dsp,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_key,               sym_key,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)interface_quit,           sym_quit,   A_NULL);
    
    class_addMethod (c, (t_method)font_withHostMeasured,    sym__font,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)audio_requireDialog,      sym__audioproperties,   A_NULL);
    class_addMethod (c, (t_method)audio_fromDialog,         sym__audiodialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)midi_requireDialog,       sym__midiproperties,    A_NULL);
    class_addMethod (c, (t_method)midi_fromDialog,          sym__mididialog,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)path_setSearchPath,       sym__path,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,        sym__quit,              A_NULL);
    class_addMethod (c, (t_method)preferences_save,         sym__savepreferences,   A_NULL);
    class_addMethod (c, (t_method)global_dummy,             sym__dummy,             A_NULL);
    
    #if PD_WATCHDOG
    
    class_addMethod (c, (t_method)interface_watchdog, sym__watchdog, A_NULL);
        
    #endif

    class_addAnything (c, (t_method)global_default);
    
    global_object = c;
        
    pd_bind (&global_object, sym_pd);
}

void global_destroy (void)
{
    pd_unbind (&global_object, sym_pd);
    
    CLASS_FREE (global_object);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
