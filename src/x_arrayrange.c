
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

#define x_sym ar_arrayclient.ac_name
#define x_struct ar_arrayclient.ac_templateIdentifier
#define x_field ar_arrayclient.ac_fieldName
#define x_gp ar_arrayclient.ac_gpointer
#define x_outlet ar_arrayclient.ac_obj.te_outlet

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
    t_arrayrange *x = (t_arrayrange *)pd_new(class);
    x->x_sym = x->x_struct = x->x_field = 0;
    gpointer_init(&x->x_gp);
    x->ar_fieldName = sym_y; 
    x->ar_onset = 0;
    x->ar_size = -1;
    if (onsetin)
        inlet_newFloat(&x->ar_arrayclient.ac_obj, &x->ar_onset);
    if (nin)
        inlet_newFloat(&x->ar_arrayclient.ac_obj, &x->ar_size);
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
            x->ar_fieldName = argv[2].a_w.w_symbol;
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
        x->ar_onset = argv->a_w.w_float;
        argc--; argv++;
    }
    if (argc && argv->a_type == A_FLOAT)
    {
        x->ar_size = argv->a_w.w_float;
        argc--; argv++;
    }
    if (argc && warnextra)
    {
        post("warning: %s ignoring extra argument: ", class_getNameAsString(class));
        error__post (argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->ar_arrayclient.ac_obj, &x->x_gp);
    else inlet_newSymbol(&x->ar_arrayclient.ac_obj, &x->ar_arrayclient.ac_name);
    *argcp = argc;
    *argvp = argv;
    return (x);
}

int array_rangeop_getrange(t_arrayrange *x,
    char **firstitemp, int *nitemp, int *stridep, int *arrayonsetp)
{
    t_glist *glist;
    t_array *a = array_client_getbuf(&x->ar_arrayclient, &glist);
    char *elemp;
    int stride, fieldonset, arrayonset, nitem, i, type;
    t_symbol *arraytype;
    double sum;
    t_template *template;
    if (!a)
        return (0);
    template = array_getTemplate (a);
    if (!template_findField(template, x->ar_fieldName, &fieldonset, /* Remove template_findField ASAP !!! */
        &type, &arraytype) || type != DATA_FLOAT)
    {
        return (0);
    }
    stride = a->a_stride;   /* Encapsulate ASAP. */
    arrayonset = x->ar_onset;
    if (arrayonset < 0)
        arrayonset = 0;
    else if (arrayonset > array_getSize (a))
        arrayonset = array_getSize (a);
    if (x->ar_size < 0)
        nitem = array_getSize (a) - arrayonset;
    else
    {
        nitem = x->ar_size;
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
