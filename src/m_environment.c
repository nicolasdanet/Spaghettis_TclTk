
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void instance_environmentSetFile (t_symbol *name, t_symbol *directory)
{
    instance_get()->pd_environment.env_directory = directory;
    instance_get()->pd_environment.env_fileName  = name;
}

void instance_environmentSetArguments (int argc, t_atom *argv)
{
    if (instance_get()->pd_environment.env_argv) { PD_MEMORY_FREE (instance_get()->pd_environment.env_argv); }
    
    PD_ASSERT (argc >= 0);

    instance_get()->pd_environment.env_argc = argc;
    instance_get()->pd_environment.env_argv = argc ? PD_MEMORY_GET (argc * sizeof (t_atom)) : NULL;
    
    if (argc) { atom_copyAtoms (argv, argc, instance_get()->pd_environment.env_argv, argc); }
}

void instance_environmentResetFile (void)
{
    instance_environmentSetFile (&s_, &s_);
}

void instance_environmentResetArguments (void)
{
    instance_environmentSetArguments (0, NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Caller is responsible to free the environment returned. */

t_environment *instance_environmentFetchIfAny (void)
{
    static int dollarZero = 1000;       /* Static. */
    
    if (instance_get()->pd_environment.env_directory != &s_) {
    //
    t_environment *e = (t_environment *)PD_MEMORY_GET (sizeof (t_environment));
    
    t_atom *argv = instance_get()->pd_environment.env_argv;
    
    e->env_dollarZeroValue  = dollarZero++;
    e->env_argc             = instance_get()->pd_environment.env_argc;
    e->env_argv             = argv ? argv : (t_atom *)PD_MEMORY_GET (0);
    e->env_directory        = instance_get()->pd_environment.env_directory;
    e->env_fileName         = instance_get()->pd_environment.env_fileName;

    instance_get()->pd_environment.env_argc      = 0;
    instance_get()->pd_environment.env_argv      = NULL;
    instance_get()->pd_environment.env_fileName  = &s_;
    instance_get()->pd_environment.env_directory = &s_;
    
    return e;
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void environment_free (t_environment *e)
{
    if (e) {
        PD_MEMORY_FREE (e->env_argv);
        PD_MEMORY_FREE (e);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void environment_setFileName (t_environment *e, t_symbol *name)
{
    if (string_endWith (name->s_name, PD_PATCH) || string_endWith (name->s_name, PD_HELP)) {
        e->env_fileName = name;
        
    } else {

        char t[PD_STRING] = { 0 };
        const char *extension = PD_PATCH;
        
        if (string_endWith (e->env_fileName->s_name, PD_HELP)) { extension = PD_HELP; }
        if (!string_sprintf (t, PD_STRING, "%s%s", name->s_name, extension)) {
            e->env_fileName = gensym (t);
        }
    }
}

void environment_setDirectory (t_environment *e, t_symbol *directory)
{
    e->env_directory = directory;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
