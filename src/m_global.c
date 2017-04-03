
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_key                     (t_glist *, t_symbol *, int, t_atom *);
void interface_quit                 (void *);
void audio_requireDialog            (void *);
void midi_requireDialog             (void *);
void preferences_save               (void *);
void dsp_state                      (void *, t_symbol *, int, t_atom *);
void font_withHostMeasured          (void *, t_symbol *, int, t_atom *);
void audio_fromDialog               (void *, t_symbol *, int, t_atom *);
void midi_fromDialog                (void *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd global_object;                 /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_newPatch (void *dummy, t_symbol *name, t_symbol *directory)
{
    instance_makePatch (name, directory);
}

static void global_open (void *dummy, t_symbol *name, t_symbol *directory)
{
    buffer_fileOpen (name, directory);
}

static void global_key (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    canvas_key (NULL, s, argc, argv);
}

static void global_default (void *x, t_symbol *s, int argc, t_atom *argv)
{
    error_unknownMethod (class_getName (pd_class (x)), s);
}

static void global_setSearchPath (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    instance_searchPathSetEncoded (argc, argv);
}

/* Messy ping-pong required in order to check saving sequentially. */
/* Furthermore it avoids the application to quit before responding. */
/* Note that patches not dirty are closed later. */

void global_shouldQuit (void *dummy)
{
    t_glist *glist = NULL;
    
    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) {
    //
    if (glist_isDirty (glist)) {
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

static void global_dummy (void *dummy)
{
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

    class_addMethod (c, (t_method)global_newPatch,              sym_new,    A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_open,                  sym_open,   A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)dsp_state,                    sym_dsp,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_key,                   sym_key,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)interface_quit,               sym_quit,   A_NULL);
    
    class_addMethod (c, (t_method)font_withHostMeasured,        sym__font,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)audio_requireDialog,          sym__audioproperties,   A_NULL);
    class_addMethod (c, (t_method)audio_fromDialog,             sym__audiodialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)midi_requireDialog,           sym__midiproperties,    A_NULL);
    class_addMethod (c, (t_method)midi_fromDialog,              sym__mididialog,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_setSearchPath,         sym__path,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,            sym__quit,              A_NULL);
    class_addMethod (c, (t_method)preferences_save,             sym__savepreferences,   A_NULL);
    class_addMethod (c, (t_method)global_dummy,                 sym__dummy,             A_NULL);
    
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
