
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
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error field_setAsFloatVariableParsed (t_fielddescriptor *fd,
    t_symbol *s, 
    char *firstOpeningParenthesis)
{
    char *t[PD_STRING] = { 0 };
    
    t_error err = string_append (t, PD_STRING, s->s_name, (int)(firstOpeningParenthesis - s->s_name));
    
    if (!err) {
    //
    double a, b, c, d, e;
    int k;
        
    fd->fd_un.fd_variableName = gensym (t);
    
    k = sscanf (firstOpeningParenthesis, "(%lf:%lf)(%lf:%lf)(%lf)", &a, &b, &c, &d, &e);
            
    fd->fd_v1       = 0.0;
    fd->fd_v2       = 0.0;
    fd->fd_screen1  = 0.0;
    fd->fd_screen2  = 0.0;
    fd->fd_quantum  = 0.0;
    
    if (k == 2) { fd->fd_v1 = a; fd->fd_v2 = b; fd->fd_screen1 = a; fd->fd_screen2 = b; }
    else if (k == 4 || k == 5) { 
        fd->fd_v1 = a; fd->fd_v2 = b; fd->fd_screen1 = c; fd->fd_screen2 = d;
        if (k == 5) {
            fd->fd_quantum = e;
        }
    }
    //
    }
    
    PD_ASSERT (!err);
    
    return err;
}

static void field_setAsReset (t_fielddescriptor *fd)
{
    field_setAsFloatConstant (fd, 0.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void field_setAsFloatConstant (t_fielddescriptor *fd, t_float f)
{
    fd->fd_type         = DATA_FLOAT;
    fd->fd_isVariable   = 0;
    fd->fd_un.fd_float  = f;
    fd->fd_v1           = 0.0;
    fd->fd_v2           = 0.0;
    fd->fd_screen1      = 0.0;
    fd->fd_screen2      = 0.0;
    fd->fd_quantum      = 0.0;
}

void field_setAsFloatVariable (t_fielddescriptor *fd, t_symbol *s)
{
    char *firstOpeningParenthesis = strchr (s->s_name, '(');
    char *firstClosingParenthesis = strchr (s->s_name, ')');

    int parse = 1;
    
    field_setAsReset (fd);
    
    parse &= (firstOpeningParenthesis != NULL);
    parse &= (firstClosingParenthesis != NULL);
    parse &= (firstClosingParenthesis > firstOpeningParenthesis);
    
    fd->fd_type         = DATA_FLOAT;
    fd->fd_isVariable   = 1;
    
    if (parse && !field_setAsFloatVariableParsed (fd, s, firstOpeningParenthesis)) { }
    else {
        fd->fd_un.fd_variableName   = s;
        fd->fd_v1                   = 0.0;
        fd->fd_v2                   = 0.0;
        fd->fd_screen1              = 0.0;
        fd->fd_screen2              = 0.0; 
        fd->fd_quantum              = 0.0;
    }
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
#pragma mark -

t_float field_convertValueToPosition (t_fielddescriptor *fd, t_float v)
{
    PD_ASSERT (field_isFloat (fd));
    PD_ASSERT (field_isVariable (fd));
    
    if (fd->fd_v2 == fd->fd_v1) { return v; }
    else {
        t_float m = PD_MIN (fd->fd_screen1, fd->fd_screen2);
        t_float n = PD_MAX (fd->fd_screen1, fd->fd_screen2);
        t_float d = (fd->fd_screen2 - fd->fd_screen1) / (fd->fd_v2 - fd->fd_v1);
        t_float k = (fd->fd_screen1 + ((v - fd->fd_v1) * d));
        
        return (PD_CLAMP (k, m, n));
    }
}

t_float field_convertPositionToValue (t_fielddescriptor *fd, t_float k)
{
    PD_ASSERT (field_isFloat (fd));
    PD_ASSERT (field_isVariable (fd));
    
    if (fd->fd_screen2 == fd->fd_screen1) { return k; }
    else {
        t_float m = PD_MIN (fd->fd_v1, fd->fd_v2);
        t_float n = PD_MAX (fd->fd_v1, fd->fd_v2);
        t_float d = (fd->fd_v2 - fd->fd_v1) / (fd->fd_screen2 - fd->fd_screen1);
        t_float v = (fd->fd_v1 + ((k - fd->fd_screen1) * d));
        
        if (fd->fd_quantum != 0.0) { v = ((int)((v / fd->fd_quantum) + 0.5)) * fd->fd_quantum; }
        
        return (PD_CLAMP (v, m, n));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
