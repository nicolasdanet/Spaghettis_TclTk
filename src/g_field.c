
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void fielddesc_setfloat_const(t_fielddescriptor *fd, t_float f)
{
    fd->fd_type = DATA_FLOAT;
    fd->fd_var = 0;
    fd->fd_un.fd_float = f;
    fd->fd_v1 = fd->fd_v2 = fd->fd_screen1 = fd->fd_screen2 =
        fd->fd_quantum = 0;
}

void fielddesc_setsymbol_const(t_fielddescriptor *fd, t_symbol *s)
{
    fd->fd_type = DATA_SYMBOL;
    fd->fd_var = 0;
    fd->fd_un.fd_symbol = s;
    fd->fd_v1 = fd->fd_v2 = fd->fd_screen1 = fd->fd_screen2 =
        fd->fd_quantum = 0;
}

void fielddesc_setfloat_var(t_fielddescriptor *fd, t_symbol *s)
{
    char *s1, *s2, *s3, strbuf[PD_STRING];
    int i;
    fd->fd_type = DATA_FLOAT;
    fd->fd_var = 1;
    if (!(s1 = strchr(s->s_name, '(')) || !(s2 = strchr(s->s_name, ')'))
        || (s1 > s2))
    {
        fd->fd_un.fd_varsym = s;
        fd->fd_v1 = fd->fd_v2 = fd->fd_screen1 = fd->fd_screen2 =
            fd->fd_quantum = 0;
    }
    else
    {
        int cpy = s1 - s->s_name, got;
        double v1, v2, screen1, screen2, quantum;
        if (cpy > PD_STRING-5)
            cpy = PD_STRING-5;
        strncpy(strbuf, s->s_name, cpy);
        strbuf[cpy] = 0;
        fd->fd_un.fd_varsym = gensym (strbuf);
        got = sscanf(s1, "(%lf:%lf)(%lf:%lf)(%lf)",
            &v1, &v2, &screen1, &screen2,
                &quantum);
        fd->fd_v1=v1;
        fd->fd_v2=v2;
        fd->fd_screen1=screen1;
        fd->fd_screen2=screen2;
        fd->fd_quantum=quantum;
        if (got < 2)
            goto fail;
        if (got == 3 || (got < 4 && strchr(s2, '(')))
            goto fail;
        if (got < 5 && (s3 = strchr(s2, '(')) && strchr(s3+1, '('))
            goto fail;
        if (got == 4)
            fd->fd_quantum = 0;
        else if (got == 2)
        {
            fd->fd_quantum = 0;
            fd->fd_screen1 = fd->fd_v1;
            fd->fd_screen2 = fd->fd_v2;
        }
        return;
    fail:
        post("parse error: %s", s->s_name);
        fd->fd_v1 = fd->fd_screen1 = fd->fd_v2 = fd->fd_screen2 =
            fd->fd_quantum = 0;
    }
}

void fielddesc_setfloatarg(t_fielddescriptor *fd, int argc, t_atom *argv)
{
        if (argc <= 0) fielddesc_setfloat_const(fd, 0);
        else if (argv->a_type == A_SYMBOL)
            fielddesc_setfloat_var(fd, argv->a_w.w_symbol);
        else fielddesc_setfloat_const(fd, argv->a_w.w_float);
}

void fielddesc_setsymbolarg(t_fielddescriptor *fd, int argc, t_atom *argv)
{
        if (argc <= 0) fielddesc_setsymbol_const(fd, &s_);
        else if (argv->a_type == A_SYMBOL)
        {
            fd->fd_type = DATA_SYMBOL;
            fd->fd_var = 1;
            fd->fd_un.fd_varsym = argv->a_w.w_symbol;
            fd->fd_v1 = fd->fd_v2 = fd->fd_screen1 = fd->fd_screen2 =
                fd->fd_quantum = 0;
        }
        else fielddesc_setsymbol_const(fd, &s_);
}

void fielddesc_setarrayarg(t_fielddescriptor *fd, int argc, t_atom *argv)
{
        if (argc <= 0) fielddesc_setfloat_const(fd, 0);
        else if (argv->a_type == A_SYMBOL)
        {
            fd->fd_type = DATA_ARRAY;
            fd->fd_var = 1;
            fd->fd_un.fd_varsym = argv->a_w.w_symbol;
        }
        else fielddesc_setfloat_const(fd, argv->a_w.w_float);
}

    /* getting and setting values via fielddescs -- note confusing names;
    the above are setting up the fielddesc itself. */
t_float fielddesc_getfloat(t_fielddescriptor *f, t_template *template,
    t_word *wp, int loud)
{
    if (f->fd_type == DATA_FLOAT)
    {
        if (f->fd_var)
            return (template_getfloat(template, f->fd_un.fd_varsym, wp));
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
        if (f->fd_var)
        {
            t_float val = template_getfloat(template,
                f->fd_un.fd_varsym, wp);
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

static t_symbol *fielddesc_getsymbol(t_fielddescriptor *f, t_template *template,
    t_word *wp, int loud)
{
    if (f->fd_type == DATA_SYMBOL)
    {
        if (f->fd_var)
            return(template_getsymbol(template, f->fd_un.fd_varsym, wp, loud));
        else return (f->fd_un.fd_symbol);
    }
    else
    {
        if (loud)
            post_error ("numeric data field used as symbol");
        return (&s_);
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
    if (f->fd_type == DATA_FLOAT && f->fd_var)
    {
        t_float val = fielddesc_cvtfromcoord(f, coord);
        template_setfloat(template,
                f->fd_un.fd_varsym, wp, val, loud);
    }
    else
    {
        if (loud)
            post_error ("attempt to set constant or symbolic data field to a number");
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
