
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
    post_error (PD_TRANSLATE ("%s: unknown method '%s'"), class_getName (pd_class (x)), s->s_name); // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_initialize (void)
{
    t_class *c = class_new (gensym ("pd"), NULL, NULL, sizeof (t_pd), CLASS_DEFAULT, A_NULL);

    class_addMethod (c, (t_method)global_new,               gensym ("new"),  A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)buffer_openFile,          gensym ("open"), A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)global_dsp,               gensym ("dsp"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_key,               gensym ("key"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_quit,              gensym ("quit"), A_NULL);
    class_addMethod (c, (t_method)global_gui,               gensym ("_gui"), A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)global_audioProperties,   gensym ("_audioProperties"), A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)global_midiProperties,    gensym ("_midiProperties"),  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)global_audioDialog,       gensym ("_audioDialog"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_midiDialog,        gensym ("_midiDialog"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_audioAPI,          gensym ("_audioAPI"),        A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)global_midiAPI,           gensym ("_midiAPI"),         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)global_setPath,           gensym ("_path"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)global_pathDialog,        gensym ("_pathDialog"),      A_NULL);
    class_addMethod (c, (t_method)global_shouldQuit,        gensym ("_quit"),            A_NULL);
    class_addMethod (c, (t_method)global_ping,              gensym ("_ping"),            A_NULL);
    class_addMethod (c, (t_method)global_savePreferences,   gensym ("_savePreferences"), A_NULL);
    
    #if (PD_LINUX || PD_BSD)
    
    class_addMethod (c, (t_method)global_watchdog, gensym ("_watchdog"), A_NULL);
        
    #endif

    class_addAnything (c, global_default);
    
    global_object = c;
        
    pd_bind (&global_object, gensym ("pd"));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
