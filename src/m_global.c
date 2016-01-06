
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *global_object;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_default (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    post_error (PD_TRANSLATE ("%s: unknown method '%s'"), class_getName (pd_class (x)), s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_initialize (void)
{
    global_object = class_new (gensym ("pd"), NULL, NULL, sizeof (t_pd), CLASS_DEFAULT, A_NULL);
        
    class_addMethod (global_object, (t_method)glob_initfromgui, gensym ("init"), A_GIMME, A_NULL);
    class_addMethod (global_object, (t_method)glob_menunew, gensym ("menunew"), A_SYMBOL, A_SYMBOL, A_NULL);
    
    class_addMethod(global_object, (t_method)glob_evalfile, gensym("open"),
        A_SYMBOL, A_SYMBOL, 0);
    class_addMethod(global_object, (t_method)glob_quit, gensym("quit"), 0);
    class_addMethod(global_object, (t_method)glob_verifyquit,
        gensym("verifyquit"), A_DEFFLOAT, 0);
    class_addMethod(global_object, (t_method)glob_dsp, gensym("dsp"), A_GIMME, 0);
    class_addMethod(global_object, (t_method)glob_meters, gensym("meters"),
        A_FLOAT, 0);
    class_addMethod(global_object, (t_method)glob_key, gensym("key"), A_GIMME, 0);
    class_addMethod(global_object, (t_method)glob_audiostatus,
        gensym("audiostatus"), 0);
    class_addMethod(global_object, (t_method)glob_audio_properties,
        gensym("audio-properties"), A_DEFFLOAT, 0);
    class_addMethod(global_object, (t_method)glob_audio_dialog,
        gensym("audio-dialog"), A_GIMME, 0);
    class_addMethod(global_object, (t_method)glob_audio_setapi,
        gensym("audio-setapi"), A_FLOAT, 0);
    class_addMethod(global_object, (t_method)glob_midi_setapi,
        gensym("midi-setapi"), A_FLOAT, 0);
    class_addMethod(global_object, (t_method)glob_midi_properties,
        gensym("midi-properties"), A_DEFFLOAT, 0);
    class_addMethod(global_object, (t_method)glob_midi_dialog,
        gensym("midi-dialog"), A_GIMME, 0);
    class_addMethod(global_object, (t_method)glob_start_path_dialog,
        gensym("start-path-dialog"), 0);
    class_addMethod(global_object, (t_method)glob_path_dialog,
        gensym("path-dialog"), A_GIMME, 0);
    class_addMethod(global_object, (t_method)glob_start_startup_dialog,
        gensym("start-startup-dialog"), 0);
    class_addMethod(global_object, (t_method)glob_startup_dialog,
        gensym("startup-dialog"), A_GIMME, 0);
    class_addMethod(global_object, (t_method)glob_ping, gensym("ping"), 0);
    class_addMethod(global_object, (t_method)glob_savepreferences,
        gensym("save-preferences"), 0);
#if defined(__linux__) || defined(__FreeBSD_kernel__)
    class_addMethod(global_object, (t_method)glob_watchdog,
        gensym("watchdog"), 0);
#endif
    class_addAnything(global_object, global_default);
    pd_bind(&global_object, gensym("pd"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
