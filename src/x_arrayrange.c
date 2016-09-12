
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
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_control.h"

#define x_sym x_tc.tc_sym
#define x_struct x_tc.tc_struct
#define x_field x_tc.tc_field
#define x_gp x_tc.tc_gp
#define x_outlet x_tc.tc_obj.te_outlet

    /* generic creator for operations on ranges (array {get,set,sum,random,
        quantile,search,...}  "onsetin" and "nin" are true if we should make
        inlets for onset and n - if no inlet for 'n' we also won't allow
        it to be specified as an argument.  Everything can take an onset but
        sometimes we don't need an inlet because it's the inlet itself.  In
        any case we allow onset to be specified as an argument (even if it's
        the 'hot inlet') -- for the same reason as in the 'delay' object.
        Finally we can optionally warn if there are extra arguments; some
        specific arguments (e.g., search) allow them but most don't. */
void *array_rangeop_new(t_class *class,
    t_symbol *s, int *argcp, t_atom **argvp,
    int onsetin, int nin, int warnextra)
{
    int argc = *argcp;
    t_atom *argv = *argvp;
    t_array_rangeop *x = (t_array_rangeop *)pd_new(class);
    x->x_sym = x->x_struct = x->x_field = 0;
    gpointer_init(&x->x_gp);
    x->x_elemtemplate = &s_;
    x->x_elemfield = sym_y; 
    x->x_onset = 0;
    x->x_n = -1;
    if (onsetin)
        inlet_newFloat(&x->x_tc.tc_obj, &x->x_onset);
    if (nin)
        inlet_newFloat(&x->x_tc.tc_obj, &x->x_n);
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-s") &&
            argc >= 3 && argv[1].a_type == A_SYMBOL &&
                argv[2].a_type == A_SYMBOL)
        {
            x->x_struct = utils_makeTemplateIdentifier(argv[1].a_w.w_symbol);
            x->x_field = argv[2].a_w.w_symbol;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-f") &&
            argc >= 3 && argv[1].a_type == A_SYMBOL &&
                argv[2].a_type == A_SYMBOL)
        {
            x->x_elemtemplate = argv[1].a_w.w_symbol;
            x->x_elemfield = argv[2].a_w.w_symbol;
            argc -= 2; argv += 2;
        }
        else
        {
            post_error ("%s: unknown flag ...", class_getNameAsString(class));
            error__post (argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        if (x->x_struct)
        {
            post_error ("%s: extra names after -s..", class_getNameAsString(class));
            error__post (argc, argv);
        }
        else x->x_sym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc && argv->a_type == A_FLOAT)
    {
        x->x_onset = argv->a_w.w_float;
        argc--; argv++;
    }
    if (argc && argv->a_type == A_FLOAT)
    {
        x->x_n = argv->a_w.w_float;
        argc--; argv++;
    }
    if (argc && warnextra)
    {
        post("warning: %s ignoring extra argument: ", class_getNameAsString(class));
        error__post (argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_tc.tc_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_tc.tc_obj, &x->x_tc.tc_sym);
    *argcp = argc;
    *argvp = argv;
    return (x);
}

int array_rangeop_getrange(t_array_rangeop *x,
    char **firstitemp, int *nitemp, int *stridep, int *arrayonsetp)
{
    t_glist *glist;
    t_array *a = array_client_getbuf(&x->x_tc, &glist);
    char *elemp;
    int stride, fieldonset, arrayonset, nitem, i, type;
    t_symbol *arraytype;
    double sum;
    t_template *template;
    if (!a)
        return (0);
    template = array_getTemplate (a);
    if (!template_findField(template, x->x_elemfield, &fieldonset, /* Remove template_findField ASAP !!! */
        &type, &arraytype) || type != DATA_FLOAT)
    {
        return (0);
    }
    stride = a->a_stride;   /* Encapsulate ASAP. */
    arrayonset = x->x_onset;
    if (arrayonset < 0)
        arrayonset = 0;
    else if (arrayonset > array_getSize (a))
        arrayonset = array_getSize (a);
    if (x->x_n < 0)
        nitem = array_getSize (a) - arrayonset;
    else
    {
        nitem = x->x_n;
        if (nitem + arrayonset > array_getSize (a))
            nitem = array_getSize (a) - arrayonset;
    }
    *firstitemp = a->a_vector+(fieldonset+arrayonset*stride);   /* Encapsulate ASAP. */
    *nitemp = nitem;
    *stridep = stride;
    *arrayonsetp = arrayonset;
    return (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
