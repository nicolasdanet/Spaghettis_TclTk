
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

static t_class *makefilename_class;

typedef struct _makefilename
{
    t_object x_obj;
    t_symbol *x_format;
    t_atomtype x_accept;
    int x_intconvert;
} t_makefilename;

static void makefilename_scanformat(t_makefilename *x)
{
    int num=0, infmt=0;
    char *str,*chr;
    if (!x->x_format) return;
    x->x_accept = A_NULL;
    for (str=x->x_format->s_name; *str; str++) {
        if (!infmt && *str=='%') {
            infmt=1;
            continue;
        }
        if (infmt) {
            if (strchr("-.#0123456789",*str)!=0)
                continue;
            if (*str=='s') {
                x->x_accept = A_SYMBOL;
                x->x_intconvert = 0;
                break;
            }
            if (strchr("fgGeE",*str)!=0) {
                x->x_accept = A_FLOAT;
                x->x_intconvert = 0;
                break;
            }
            if (strchr("xXdiouc",*str)!=0) {
                x->x_accept = A_FLOAT;
                x->x_intconvert = 1;
                break;
            }
            infmt=0;
        }
    }
}

static void *makefilename_new(t_symbol *s)
{
    t_makefilename *x = (t_makefilename *)pd_new(makefilename_class);
    if (!s || !*s->s_name)
        s = sym_file__dot____percent__d;
    outlet_new(&x->x_obj, &s_symbol);
    x->x_format = s;
    x->x_accept = A_NULL;
    x->x_intconvert = 0;
    makefilename_scanformat(x);
    return (x);
}

static void makefilename_float(t_makefilename *x, t_float f)
{
    char buf[PD_STRING];
    if (x->x_accept == A_FLOAT) {
        if (x->x_intconvert)
            sprintf(buf, x->x_format->s_name, (int)f);
        else sprintf(buf, x->x_format->s_name, f);
    }
    else
    {
        char buf2[PD_STRING];
        sprintf(buf2, "%g", f);
        sprintf(buf, x->x_format->s_name, buf2);
    }
    if (buf[0]!=0)
    outlet_symbol(x->x_obj.te_outlet, gensym (buf));
}

static void makefilename_symbol(t_makefilename *x, t_symbol *s)
{
    char buf[PD_STRING];
    if (x->x_accept == A_SYMBOL)
    sprintf(buf, x->x_format->s_name, s->s_name);
    else
        sprintf(buf, x->x_format->s_name, 0);
    if (buf[0]!=0)
    outlet_symbol(x->x_obj.te_outlet, gensym (buf));
}

static void makefilename_set(t_makefilename *x, t_symbol *s)
{
    x->x_format = s;
    makefilename_scanformat(x);
}

void makefilename_setup(void)
{
    makefilename_class = class_new(sym_makefilename,
    (t_newmethod)makefilename_new, 0,
        sizeof(t_makefilename), 0, A_DEFSYMBOL, 0);
    class_addFloat(makefilename_class, makefilename_float);
    class_addSymbol(makefilename_class, makefilename_symbol);
    class_addMethod(makefilename_class, (t_method)makefilename_set,
        sym_set, A_SYMBOL, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
