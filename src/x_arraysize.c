
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_class *garray_class;

/* ---  array_client - common code for objects that refer to arrays -- */

#define x_sym ar_arrayclient.ac_name
#define x_struct ar_arrayclient.ac_templateIdentifier
#define x_field ar_arrayclient.ac_fieldName
#define x_gp ar_arrayclient.ac_gpointer
#define x_outlet ar_arrayclient.ac_obj.te_outlet

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------  array size : get or set size of an array ---------------- */
static t_class *array_size_class;

typedef struct _array_size
{
    t_arrayclient ar_arrayclient;
} t_array_size;

void *array_size_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_size *x = (t_array_size *)pd_new(array_size_class);
    x->x_sym = x->x_struct = x->x_field = 0;
    gpointer_init(&x->x_gp);
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
        else
        {
            post_error ("array setline: unknown flag ...");
            error__post (argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        if (x->x_struct)
        {
            post_error ("array setline: extra names after -s..");
            error__post (argc, argv);
        }
        else x->x_sym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: array setline ignoring extra argument: ");
        error__post (argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->ar_arrayclient.ac_obj, &x->x_gp);
    else inlet_newSymbol(&x->ar_arrayclient.ac_obj, &x->ar_arrayclient.ac_name);
    outlet_new(&x->ar_arrayclient.ac_obj, &s_float);
    return (x);
}

static void array_size_bang(t_array_size *x)
{
    t_glist *glist;
    t_array *a = arrayclient_fetchArray(&x->ar_arrayclient, &glist);
    if (a)
        outlet_float(x->x_outlet, array_getSize (a));
}

static void array_size_float(t_array_size *x, t_float f)
{
    t_glist *glist;
    t_array *a = arrayclient_fetchArray(&x->ar_arrayclient, &glist);
    if (a)
    {
              /* if it's a named array object we have to go back and find the
              garray (repeating work done in arrayclient_fetchArray()) because
              the garray might want to adjust.  Maybe arrayclient_fetchArray
              should have a return slot for the garray if any?  */
        if (x->ar_arrayclient.ac_name)
        {
            t_garray *y = (t_garray *)pd_findByClass(x->ar_arrayclient.ac_name, garray_class);
            garray_resizeWithInteger (y, (int)f);
        }
        else
        {
            int n = f;
            if (n < 1)
                n = 1;
             array_resizeAndRedraw(a, glist, n);
        }
    }
}

/* ---------------- global setup function -------------------- */

void arraysize_setup(void)
{
    array_size_class = class_new(sym_array__space__size,
        (t_newmethod)array_size_new, (t_method)arrayclient_free,
            sizeof(t_array_size), 0, A_GIMME, 0);
    class_addBang(array_size_class, array_size_bang);
    class_addFloat(array_size_class, array_size_float);
    class_setHelpName(array_size_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
