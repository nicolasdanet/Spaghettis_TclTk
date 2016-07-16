
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
        
    fd->fd_un.fd_varname = gensym (t);
    
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
    
    parse &= (firstOpeningParenthesis != NULL);
    parse &= (firstClosingParenthesis != NULL);
    parse &= (firstClosingParenthesis > firstOpeningParenthesis);
    
    fd->fd_type         = DATA_FLOAT;
    fd->fd_isVariable   = 1;
    
    if (parse && !field_setAsFloatVariableParsed (fd, s, firstOpeningParenthesis)) { }
    else {
        fd->fd_un.fd_varname    = s;
        fd->fd_v1               = 0.0;
        fd->fd_v2               = 0.0;
        fd->fd_screen1          = 0.0;
        fd->fd_screen2          = 0.0; 
        fd->fd_quantum          = 0.0;
    }
}

void field_setAsFloat (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    if (argc <= 0) { field_setAsFloatConstant (fd, 0.0); }
    else {
        if (IS_SYMBOL (argv)) { field_setAsFloatVariable (fd, GET_SYMBOL (argv)); }
        else {
            field_setAsFloatConstant (fd, atom_getFloatAtIndex (0, argc, argv));
        }
    }
}

void field_setAsArray (t_fielddescriptor *fd, int argc, t_atom *argv)
{
    if (argc <= 0) { field_setAsFloatConstant (fd, 0.0); }
    else {
        if (IS_SYMBOL (argv)) {
            fd->fd_type             = DATA_ARRAY;
            fd->fd_isVariable       = 1;
            fd->fd_un.fd_varname    = GET_SYMBOL (argv);
        } else { 
            field_setAsFloatConstant (fd, atom_getFloatAtIndex (0, argc, argv));
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float fielddesc_getfloat(t_fielddescriptor *f, t_template *template, t_word *wp, int loud)
{
    if (f->fd_type == DATA_FLOAT)
    {
        if (f->fd_isVariable)
            return (template_getfloat(template, f->fd_un.fd_varname, wp));
        else return (f->fd_un.fd_float);
    }
    else
    {
        if (loud)
            post_error ("symbolic data field used as number");
        return (0);
    }
}

    /* convert a variable's value to a screen coordinate via its fielddesc */
t_float fielddesc_cvttocoord(t_fielddescriptor *f, t_float val)
{
    t_float coord, pix, extreme, div;
    if (f->fd_v2 == f->fd_v1)
        return (val);
    div = (f->fd_screen2 - f->fd_screen1)/(f->fd_v2 - f->fd_v1);
    coord = f->fd_screen1 + (val - f->fd_v1) * div;
    extreme = (f->fd_screen1 < f->fd_screen2 ?
        f->fd_screen1 : f->fd_screen2);
    if (coord < extreme)
        coord = extreme;
    extreme = (f->fd_screen1 > f->fd_screen2 ? 
        f->fd_screen1 : f->fd_screen2);
    if (coord > extreme)
        coord = extreme;
    return (coord);
}

    /* read a variable via fielddesc and convert to screen coordinate */
t_float fielddesc_getcoord(t_fielddescriptor *f, t_template *template,
    t_word *wp, int loud)
{
    if (f->fd_type == DATA_FLOAT)
    {
        if (f->fd_isVariable)
        {
            t_float val = template_getfloat(template,
                f->fd_un.fd_varname, wp);
            return (fielddesc_cvttocoord(f, val));
        }
        else return (f->fd_un.fd_float);
    }
    else
    {
        if (loud)
            post_error ("symbolic data field used as number");
        return (0);
    }
}

    /* convert from a screen coordinate to a variable value */
t_float fielddesc_cvtfromcoord(t_fielddescriptor *f, t_float coord)
{
    t_float val;
    if (f->fd_screen2 == f->fd_screen1)
        val = coord;
    else
    {
        t_float div = (f->fd_v2 - f->fd_v1)/(f->fd_screen2 - f->fd_screen1);
        t_float extreme;
        val = f->fd_v1 + (coord - f->fd_screen1) * div;
        if (f->fd_quantum != 0)
            val = ((int)((val/f->fd_quantum) + 0.5)) *  f->fd_quantum;
        extreme = (f->fd_v1 < f->fd_v2 ?
            f->fd_v1 : f->fd_v2);
        if (val < extreme) val = extreme;
        extreme = (f->fd_v1 > f->fd_v2 ?
            f->fd_v1 : f->fd_v2);
        if (val > extreme) val = extreme;
    }
    return (val);
 }

void fielddesc_setcoord(t_fielddescriptor *f, t_template *template,
    t_word *wp, t_float coord, int loud)
{
    if (f->fd_type == DATA_FLOAT && f->fd_isVariable)
    {
        t_float val = fielddesc_cvtfromcoord(f, coord);
        template_setfloat(template,
                f->fd_un.fd_varname, wp, val, loud);
    }
    else
    {
        if (loud)
            post_error ("attempt to set constant or symbolic data field to a number");
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
