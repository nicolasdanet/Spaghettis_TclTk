
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol    *environment_fileName  = &s_;        /* Shared. */
static t_symbol    *environment_directory = &s_;        /* Shared. */
static t_atom      *environment_argv;                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          environment_argc;                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void environment_setActiveFile (t_symbol *name, t_symbol *directory)
{
    environment_fileName  = name;
    environment_directory = directory;
}

void environment_setActiveArguments (int argc, t_atom *argv)
{
    if (environment_argv) { PD_MEMORY_FREE (environment_argv); }
    
    PD_ASSERT (argc >= 0);
    
    environment_argc = argc;
    environment_argv = PD_MEMORY_GET (argc * sizeof (t_atom));
    
    if (argc) { atom_copyAtomsUnchecked (argc, argv, environment_argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Caller is responsible to free the environment returned. */

t_environment *environment_fetchActiveIfAny (void)
{
    static int dollarZero = 1000;       /* Shared. */
    
    if (environment_directory != &s_) {
    //
    t_environment *e = (t_environment *)PD_MEMORY_GET (sizeof (t_environment));

    e->ce_dollarZeroValue   = dollarZero++;
    e->ce_argc              = environment_argc;
    e->ce_argv              = environment_argv ? environment_argv : (t_atom *)PD_MEMORY_GET (0);
    e->ce_directory         = environment_directory;
    e->ce_fileName          = environment_fileName;

    PD_ASSERT (environment_argv != NULL || environment_argc == 0);
    
    environment_argc = 0;
    environment_argv = NULL;
    
    environment_setActiveFile (&s_, &s_);
    
    return e;
    //
    }
    
    return NULL;
}

void environment_free (t_environment *environment)
{
    if (environment) {
        PD_MEMORY_FREE (environment->ce_argv);
        PD_MEMORY_FREE (environment);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *environment_getFileName (t_environment *environment)
{
    if (environment) { return environment->ce_fileName; }
    else {
        return sym_Patch;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
