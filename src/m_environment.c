
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol     *environment_fileName  = &s_;       /* Static. */
static t_symbol     *environment_directory = &s_;       /* Static. */
static t_atom       *environment_argv;                  /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          environment_argc;                   /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _environment {
    int         ce_dollarZeroValue;
    int         ce_argc;
    t_atom      *ce_argv;
    t_symbol    *ce_directory;
    t_symbol    *ce_fileName;
};

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

void environment_resetActiveArguments (void)
{
    environment_setActiveArguments (0, NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Caller is responsible to free the environment returned. */

t_environment *environment_fetchActiveIfAny (void)
{
    static int dollarZero = 1000;       /* Static. */
    
    if (environment_directory != &s_) {
    //
    t_environment *e = (t_environment *)PD_MEMORY_GET (sizeof (t_environment));

    e->ce_dollarZeroValue   = dollarZero++;
    e->ce_argc              = environment_argc;
    e->ce_argv              = environment_argv ? environment_argv : (t_atom *)PD_MEMORY_GET (0);
    e->ce_directory         = environment_directory;
    e->ce_fileName          = environment_fileName;

    PD_ASSERT (environment_argv != NULL || environment_argc == 0);
    
    environment_argc      = 0;
    environment_argv      = NULL;
    environment_fileName  = &s_;
    environment_directory = &s_;
    
    return e;
    //
    }
    
    return NULL;
}

void environment_free (t_environment *e)
{
    if (e) {
        PD_MEMORY_FREE (e->ce_argv);
        PD_MEMORY_FREE (e);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int environment_getDollarZero (t_environment *e)
{
    return e->ce_dollarZeroValue;
}

int environment_getNumberOfArguments (t_environment *e)
{
    return e->ce_argc;
}

t_atom *environment_getArguments (t_environment *e)
{
    return e->ce_argv;
}

t_symbol *environment_getDirectory (t_environment *e)
{
    return e->ce_directory;
}

char *environment_getDirectoryAsString (t_environment *e)
{
    return e->ce_directory->s_name;
}

t_symbol *environment_getFileName (t_environment *e)
{
    if (e) { return e->ce_fileName; }
    else {
        return sym_Patch;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void environment_release (void)
{
    if (environment_argv) { PD_MEMORY_FREE (environment_argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
