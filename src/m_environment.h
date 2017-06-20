
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_environment_h_
#define __m_environment_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _environment {
    int         env_dollarZeroValue;
    int         env_argc;
    t_atom      *env_argv;
    t_symbol    *env_directory;
    t_symbol    *env_fileName;
    } t_environment;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void environment_free           (t_environment *e);
void environment_setFileName    (t_environment *e, t_symbol *name);
void environment_setDirectory   (t_environment *e, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int environment_getDollarZero (t_environment *e)
{
    return e->env_dollarZeroValue;
}

static inline int environment_getNumberOfArguments (t_environment *e)
{
    return e->env_argc;
}

static inline t_atom *environment_getArguments (t_environment *e)
{
    return e->env_argv;
}

static inline t_symbol *environment_getDirectory (t_environment *e)
{
    return e->env_directory;
}

static inline char *environment_getDirectoryAsString (t_environment *e)
{
    return e->env_directory->s_name;
}

static inline t_symbol *environment_getFileName (t_environment *e)
{
    return (e ? e->env_fileName : sym_Patch);
}

static inline char *environment_getFileNameAsString (t_environment *e)
{
    return environment_getFileName (e)->s_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_environment_h_
