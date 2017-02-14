
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Untyped attribute. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void field_setAsReset (t_fielddescriptor *fd)
{
    field_setAsFloatConstant (fd, (t_float)0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void field_setAsFloatConstant (t_fielddescriptor *fd, t_float f)
{
    fd->fd_type                 = DATA_FLOAT;
    fd->fd_isVariable           = 0;
    fd->fd_un.fd_float          = f;
}

void field_setAsFloatVariable (t_fielddescriptor *fd, t_symbol *s)
{
    fd->fd_type                 = DATA_FLOAT;
    fd->fd_isVariable           = 1;
    fd->fd_un.fd_variableName   = s;
}

void field_setAsFloat (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    field_setAsReset (fd);
        
    if (argc > 0) {
        if (IS_SYMBOL (argv)) { field_setAsFloatVariable (fd, GET_SYMBOL (argv)); }
        else {
            field_setAsFloatConstant (fd, atom_getFloatAtIndex (0, argc, argv));
        }
    }
}

void field_setAsArray (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    field_setAsReset (fd);
    
    if (argc > 0) {
        if (IS_SYMBOL (argv)) {
            fd->fd_type                 = DATA_ARRAY;
            fd->fd_isVariable           = 1;
            fd->fd_un.fd_variableName   = GET_SYMBOL (argv);
        } else { 
            field_setAsFloatConstant (fd, atom_getFloatAtIndex (0, argc, argv));
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int field_isFloat (t_fielddescriptor *fd)
{
    return (fd->fd_type == DATA_FLOAT);
}

int field_isFloatConstant (t_fielddescriptor *fd)
{
    return (field_isFloat (fd) && !field_isVariable (fd));
}

int field_isArray (t_fielddescriptor *fd)
{
    return (fd->fd_type == DATA_ARRAY);
}

int field_isVariable (t_fielddescriptor *fd)
{
    return (fd->fd_isVariable != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float field_getFloatConstant (t_fielddescriptor *fd)
{
    PD_ASSERT (field_isFloatConstant (fd));
    
    return fd->fd_un.fd_float;
}

t_symbol *field_getVariableName (t_fielddescriptor *fd)
{
    PD_ASSERT (field_isVariable (fd));
    
    return fd->fd_un.fd_variableName;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
