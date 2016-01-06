
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
    t_class *c = class_new (gensym ("pd"), NULL, NULL, sizeof (t_pd), CLASS_DEFAULT, A_NULL);
    
    class_addMethod (c, (t_method)global_new,           gensym ("new"),  A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_open,          gensym ("open"), A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_dsp,           gensym ("dsp"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_key,           gensym ("key"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_quit,          gensym ("quit"), A_NULL);
    

    class_addMethod (c, (t_method)global_gui,               gensym ("_gui"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_audioProperties,   gensym ("_audioProperties"), A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,        gensym ("_quit"),            A_NULL);
    
    class_addMethod (c, (t_method)glob_audio_dialog,
        gensym("audio-dialog"), A_GIMME, 0);
    class_addMethod(c, (t_method)glob_audio_setapi,
        gensym("audio-setapi"), A_FLOAT, 0);
    class_addMethod(c, (t_method)glob_midi_setapi,
        gensym("midi-setapi"), A_FLOAT, 0);
    class_addMethod(c, (t_method)glob_midi_properties,
        gensym("midi-properties"), A_DEFFLOAT, 0);
    class_addMethod(c, (t_method)glob_midi_dialog,
        gensym("midi-dialog"), A_GIMME, 0);
    class_addMethod(c, (t_method)glob_start_path_dialog,
        gensym("start-path-dialog"), 0);
    class_addMethod(c, (t_method)glob_path_dialog,
        gensym("path-dialog"), A_GIMME, 0);
    class_addMethod(c, (t_method)glob_start_startup_dialog,
        gensym("start-startup-dialog"), 0);
    class_addMethod(c, (t_method)glob_startup_dialog,
        gensym("startup-dialog"), A_GIMME, 0);
    class_addMethod(c, (t_method)glob_ping, gensym("ping"), 0);
    class_addMethod(c, (t_method)glob_savepreferences,
        gensym("save-preferences"), 0);
#if defined(__linux__) || defined(__FreeBSD_kernel__)
    class_addMethod(c, (t_method)glob_watchdog,
        gensym("watchdog"), 0);
#endif
    class_addAnything(c, global_default);
    
    global_object = c;
        
    pd_bind(&global_object, gensym("pd"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
