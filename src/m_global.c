
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void dsp_state                      (int n);
void canvas_key                     (t_glist *, t_symbol *, int, t_atom *);
void interface_quit                 (void);
void font_withHostMeasured          (int, t_atom *);
void audio_requireDialog            (void);
void audio_fromDialog               (int, t_atom *);
void midi_requireDialog             (void);
void midi_fromDialog                (int, t_atom *);
void canvas_quit                    (void);
void preferences_save               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd global_object;                 /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void global_newPatch (void *x, t_symbol *name, t_symbol *directory)
{
    instance_makePatch (name, directory);
}

static void global_open (void *x, t_symbol *name, t_symbol *directory)
{
    buffer_fileOpen (name, directory);
}

static void global_dsp (void *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) { dsp_state ((int)atom_getFloatAtIndex (0, argc, argv)); }
}

static void global_key (void *x, t_symbol *s, int argc, t_atom *argv)
{
    canvas_key (NULL, s, argc, argv);
}

static void global_quit (void *x)
{
    interface_quit();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void global_font (void *x, t_symbol *s, int argc, t_atom *argv)
{
    font_withHostMeasured (argc, argv);
}

static void global_audioProperties (void *x)
{
    audio_requireDialog();
}

static void global_audioDialog (void *x, t_symbol *s, int argc, t_atom *argv)
{
    audio_fromDialog (argc, argv);
}

static void global_midiProperties (void *x)
{
    midi_requireDialog();
}

static void global_midiDialog (void *x, t_symbol *s, int argc, t_atom *argv)
{
    midi_fromDialog (argc, argv);
}

static void global_setSearchPath (void *x, t_symbol *s, int argc, t_atom *argv)
{
    instance_searchPathSetEncoded (argc, argv);
}

static void global_shouldQuit (void *x)
{
    canvas_quit();
}

static void global_savePreferences (void *x)
{
    preferences_save();
}

static void global_default (void *x, t_symbol *s, int argc, t_atom *argv)
{
    error_unknownMethod (class_getName (pd_class (x)), s);
}

static void global_dummy (void *x)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    class_addMethod (c, (t_method)global_dsp,                   sym_dsp,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_quit,                  sym_quit,   A_NULL);
    
    class_addMethod (c, (t_method)global_key,                   sym__key,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_font,                  sym__font,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_audioProperties,       sym__audioproperties,   A_NULL);
    class_addMethod (c, (t_method)global_audioDialog,           sym__audiodialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_midiProperties,        sym__midiproperties,    A_NULL);
    class_addMethod (c, (t_method)global_midiDialog,            sym__mididialog,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_setSearchPath,         sym__path,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,            sym__quit,              A_NULL);
    class_addMethod (c, (t_method)global_savePreferences,       sym__savepreferences,   A_NULL);
    class_addMethod (c, (t_method)global_dummy,                 sym__dummy,             A_NULL);
    
    #if PD_WATCHDOG
    
    class_addMethod (c, (t_method)interface_watchdog,           sym__watchdog,          A_NULL);
        
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
