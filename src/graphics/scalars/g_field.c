
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Untyped attribute. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void field_setAsFloatConstant (t_fielddescriptor *fd, t_float f)
{
    fd->fd_type               = DATA_FLOAT;
    fd->fd_offset             = 0.0;
    fd->fd_isVariable         = 0;
    fd->fd_isVariableOpposite = 0;
    fd->fd_un.fd_float        = f;
}

void field_setAsFloatVariable (t_fielddescriptor *fd, t_symbol *s)
{
    fd->fd_type               = DATA_FLOAT;
    fd->fd_offset             = 0.0;
    fd->fd_isVariable         = 1;
    fd->fd_isVariableOpposite = 0;
    fd->fd_un.fd_variableName = s;
}

void field_setAsFloat (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    field_setAsFloatConstant (fd, 0.0);
        
    if (argc > 0) {
        if (IS_SYMBOL (argv)) { field_setAsFloatVariable (fd, GET_SYMBOL (argv)); }
        else {
            field_setAsFloatConstant (fd, atom_getFloatAtIndex (0, argc, argv));
        }
    }
}

void field_setAsArray (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    field_setAsFloatConstant (fd, 0.0);
    
    if (argc > 0 && IS_SYMBOL (argv)) {
    //
    fd->fd_type               = DATA_ARRAY;
    fd->fd_offset             = 0.0;
    fd->fd_isVariable         = 1;
    fd->fd_isVariableOpposite = 0;
    fd->fd_un.fd_variableName = GET_SYMBOL (argv);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void field_setAsFloatExtendedParse (t_fielddescriptor *fd, t_symbol *s)
{
    char t[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, s->s_name) == PD_ERROR_NONE) {
    //
    size_t end     = strlen (t);
    size_t start   = 0;
    t_float offset = 0.0;
    int isOpposite = 0;
    
    PD_ASSERT (end > 1);
    
    end--; start++; t[end] = 0;     /* Remove parentheses. */
    
    if (t[start] == '+') { start++; }
    if (t[start] == '-') { start++; isOpposite = 1; }
    
    while (string_containsOccurrence (t, "+-")) {
    //
    int m = string_indexOfFirstOccurrenceFromEnd (t, "+-");
    char *p = t + m + 1; t_atom a; t_error err = atom_withStringUnzeroed (&a, p, (int)strlen (p));
    if (!err && IS_FLOAT (&a)) {
        t_float f = GET_FLOAT (&a); offset += (t[m] == '-') ? -f : f;
        t[m] = 0;
            
    } else {
        break;
    }
    //
    }
        
    fd->fd_offset = offset;
    fd->fd_isVariableOpposite = isOpposite;
    fd->fd_un.fd_variableName = gensym (t + start);
    //
    }
}

void field_setAsFloatExtended (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    field_setAsFloatConstant (fd, 0.0);
    
    if (argc > 0) {
    //
    if (IS_SYMBOL (argv)) {
    
        t_symbol *s = GET_SYMBOL (argv);
    
        field_setAsFloatVariable (fd, s);
        
        if (string_startWith (s->s_name, "(") && string_endWith (s->s_name, ")")) {
            field_setAsFloatExtendedParse (fd, s);
        }
        
    } else {
        field_setAsFloatConstant (fd, atom_getFloatAtIndex (0, argc, argv));
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
