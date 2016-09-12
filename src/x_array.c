
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

extern t_pd *pd_newest;    /* OK - this should go into a .h file now :) */
extern t_class *garray_class;

/* -- "table" - classic "array define" object by Guenter Geiger --*/

static int tabcount = 0;

static void *table_donew(t_symbol *s, int size, int flags,
    int xpix, int ypix)
{
    t_atom a[9];
    t_glist *gl;
    t_glist *x, *z = canvas_getCurrent();
    if (s == &s_)
    {
         char  tabname[255];
         t_symbol *t = sym_table; 
         sprintf(tabname, "%s%d", t->s_name, tabcount++);
         s = gensym (tabname); 
    }
    if (size < 1)
        size = 100;
    SET_FLOAT(a, 0);
    SET_FLOAT(a+1, 50);
    SET_FLOAT(a+2, xpix + 100);
    SET_FLOAT(a+3, ypix + 100);
    SET_SYMBOL(a+4, s);
    SET_FLOAT(a+5, 0);
    x = canvas_new (NULL, NULL, 6, a);

    x->gl_parent = z;

        /* create a graph for the table */
    gl = canvas_newGraph((t_glist*)x, 0, -1, (size > 1 ? size-1 : 1), 1,
        50, ypix+50, xpix+50, 50);

    garray_makeObject(gl, s, &s_float, size, flags);

    pd_newest = &x->gl_obj.te_g.g_pd;     /* mimic action of canvas_pop() */
    stack_pop(&x->gl_obj.te_g.g_pd);
    x->gl_isLoading = 0;

    return (x);
}

static void *table_new(t_symbol *s, t_float f)
{
    return (table_donew(s, f, 0, 500, 300));
}

t_class *array_define_class;

static void array_define_yrange(t_glist *x, t_float ylo, t_float yhi)
{
    t_glist *gl = (x->gl_graphics ? canvas_castToGlistChecked(&x->gl_graphics->g_pd) : 0);
    if (gl && gl->gl_graphics && pd_class(&gl->gl_graphics->g_pd) == garray_class)
    {
        int n = array_getSize (garray_getArray ((t_garray *)gl->gl_graphics));
        pd_vMessage(&x->gl_graphics->g_pd, sym_bounds,
            "ffff", 0., yhi, (double)(n == 1 ? n : n-1), ylo);
    }
    else { PD_BUG; }
}

static void *array_define_new(t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *arrayname = &s_;
    float arraysize = 100;
    t_glist *x;
    int keep = 0;
    float ylo = -1, yhi = 1;
    float xpix = 500, ypix = 300;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-k"))
            keep = 1;
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-yrange") &&
            argc >= 3 && argv[1].a_type == A_FLOAT &&
                argv[2].a_type == A_FLOAT)
        {
            ylo = atom_getFloatAtIndex(1, argc, argv);
            yhi = atom_getFloatAtIndex(2, argc, argv);
            if (ylo == yhi)
                ylo = -1, yhi = 1;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-pix") &&
            argc >= 3 && argv[1].a_type == A_FLOAT &&
                argv[2].a_type == A_FLOAT)
        {
            if ((xpix = atom_getFloatAtIndex(1, argc, argv)) < 10)
                xpix = 10;
            if ((ypix = atom_getFloatAtIndex(2, argc, argv)) < 10)
                ypix = 10;
            argc -= 2; argv += 2;
        }
        else
        {
            post_error ("array define: unknown flag ...");
            error__post (argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        arrayname = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc && argv->a_type == A_FLOAT)
    {
        arraysize = argv->a_w.w_float;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: array define ignoring extra argument: ");
        error__post (argc, argv);
    }
    x = (t_glist *)table_donew(arrayname, arraysize, keep, xpix, ypix);
    
        /* bash the class to "array define".  We don't do this earlier in
        part so that canvas_getCurrent() will work while the glist and
        garray are being created.  There may be other, unknown side effects. */
    x->gl_obj.te_g.g_pd = array_define_class;
    array_define_yrange(x, ylo, yhi);
    return (x);
}

void array_define_save(t_gobj *z, t_buffer *bb)
{
    t_glist *x = (t_glist *)z;
    t_glist *gl = (x->gl_graphics ? canvas_castToGlistChecked(&x->gl_graphics->g_pd) : 0);
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)x->gl_obj.te_xCoordinate, (float)x->gl_obj.te_yCoordinate);
    buffer_serialize(bb, x->gl_obj.te_buffer);
    buffer_appendSemicolon(bb);

    garray_saveContentsToBuffer ((t_garray *)gl->gl_graphics, bb);
    object_saveWidth(&x->gl_obj, bb);
}



    /* send a pointer to the scalar that owns this array to
    whomever is bound to the given symbol */
static void array_define_send(t_glist *x, t_symbol *s)
{
    t_glist *gl = (x->gl_graphics ? canvas_castToGlistChecked(&x->gl_graphics->g_pd) : 0);
    if (!s->s_thing)
        post_error ("array_define_send: %s: no such object", s->s_name);
    else if (gl && gl->gl_graphics && pd_class(&gl->gl_graphics->g_pd) == garray_class)
    {
        t_gpointer gp = GPOINTER_INIT;
        gpointer_setAsScalar(&gp, gl,
            garray_getScalar((t_garray *)gl->gl_graphics));
        pd_pointer(s->s_thing, &gp);
        gpointer_unset(&gp);
    }
    else { PD_BUG; }
}

    /* just forward any messages to the garray */
static void array_define_anything(t_glist *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_glist *gl = (x->gl_graphics ? canvas_castToGlistChecked(&x->gl_graphics->g_pd) : 0);
    if (gl && gl->gl_graphics && pd_class(&gl->gl_graphics->g_pd) == garray_class)
        pd_message(&gl->gl_graphics->g_pd, s, argc, argv);
    else { PD_BUG; }
}

    /* ignore messages like "editmode" */
static void array_define_ignore(t_glist *x,
    t_symbol *s, int argc, t_atom *argv)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ---  array_client - common code for objects that refer to arrays -- */

#define x_sym x_tc.tc_sym
#define x_struct x_tc.tc_struct
#define x_field x_tc.tc_field
#define x_gp x_tc.tc_gp
#define x_outlet x_tc.tc_obj.te_outlet

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----------  array size : get or set size of an array ---------------- */
static t_class *array_size_class;

typedef struct _array_size
{
    t_array_client x_tc;
} t_array_size;

static void *array_size_new(t_symbol *s, int argc, t_atom *argv)
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
        inlet_newPointer(&x->x_tc.tc_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_tc.tc_obj, &x->x_tc.tc_sym);
    outlet_new(&x->x_tc.tc_obj, &s_float);
    return (x);
}

static void array_size_bang(t_array_size *x)
{
    t_glist *glist;
    t_array *a = array_client_getbuf(&x->x_tc, &glist);
    if (a)
        outlet_float(x->x_outlet, array_getSize (a));
}

static void array_size_float(t_array_size *x, t_float f)
{
    t_glist *glist;
    t_array *a = array_client_getbuf(&x->x_tc, &glist);
    if (a)
    {
              /* if it's a named array object we have to go back and find the
              garray (repeating work done in array_client_getbuf()) because
              the garray might want to adjust.  Maybe array_client_getbuf
              should have a return slot for the garray if any?  */
        if (x->x_tc.tc_sym)
        {
            t_garray *y = (t_garray *)pd_findByClass(x->x_tc.tc_sym, garray_class);
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

/* ----------------  array sum -- add them up ------------------- */
static t_class *array_sum_class;

#define t_array_sum t_array_rangeop

static void *array_sum_new(t_symbol *s, int argc, t_atom *argv)
{
    t_array_sum *x = array_rangeop_new(array_sum_class, s, &argc, &argv,
        0, 1, 1);
    outlet_new(&x->x_tc.tc_obj, &s_float);
    return (x);
}

static void array_sum_bang(t_array_rangeop *x)
{
    char *itemp, *firstitem;
    int stride, nitem, arrayonset, i;
    double sum;
    if (!array_rangeop_getrange(x, &firstitem, &nitem, &stride, &arrayonset))
        return;
    for (i = 0, sum = 0, itemp = firstitem; i < nitem; i++, itemp += stride)
        sum += *(t_float *)itemp;
    outlet_float(x->x_outlet, sum);
}

static void array_sum_float(t_array_rangeop *x, t_float f)
{
    x->x_onset = f;
    array_sum_bang(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* overall creator for "array" objects - dispatch to "array define" etc */
static void *arrayobj_new(t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = array_define_new(s, argc, argv);
    else
    {
        char *str = argv[0].a_w.w_symbol->s_name;
        if (!strcmp(str, "d") || !strcmp(str, "define"))
            pd_newest = array_define_new(s, argc-1, argv+1);
        else if (!strcmp(str, "size"))
            pd_newest = array_size_new(s, argc-1, argv+1);
        else if (!strcmp(str, "sum"))
            pd_newest = array_sum_new(s, argc-1, argv+1);
        else if (!strcmp(str, "get"))
            pd_newest = array_get_new(s, argc-1, argv+1);
        else if (!strcmp(str, "set"))
            pd_newest = array_set_new(s, argc-1, argv+1);
        else if (!strcmp(str, "quantile"))
            pd_newest = array_quantile_new(s, argc-1, argv+1);
        else if (!strcmp(str, "random"))
            pd_newest = array_random_new(s, argc-1, argv+1);
        else if (!strcmp(str, "max"))
            pd_newest = array_max_new(s, argc-1, argv+1);
        else if (!strcmp(str, "min"))
            pd_newest = array_min_new(s, argc-1, argv+1);
        else 
        {
            post_error ("array %s: unknown function", str);
            pd_newest = 0;
        }
    }
    return (pd_newest);
}



/* ---------------- global setup function -------------------- */

void x_array_setup(void)
{
    array_define_class = class_new(sym_array__space__define, 0,
        (t_method)canvas_free, sizeof(t_glist), 0, 0);
    
    class_addMethod(array_define_class, (t_method)canvas_restore,
        sym_restore, A_GIMME, 0);
    
    class_addClick (array_define_class, canvas_click);
    /*class_addMethod(array_define_class, (t_method)canvas_click,
        sym_click, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0); */
    class_addMethod(array_define_class, (t_method)canvas_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(array_define_class, (t_method)canvas_map,
        sym__map, A_FLOAT, A_NULL);
    class_addMethod(array_define_class, (t_method)canvas_window,
        sym_window, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(array_define_class, (t_method)canvas_window,
        sym_setbounds, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); /* LEGACY !!! */
    
    class_addMouse (array_define_class, canvas_mouse);
    class_addMouseUp (array_define_class, canvas_mouseUp);
    
    /*class_addMethod(array_define_class, (t_method)canvas_mouse, sym_mouse,
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(array_define_class, (t_method)canvas_mouseUp, sym_mouseup,
        A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); */
    class_addMethod(array_define_class, (t_method)canvas_key, sym_key,
        A_GIMME, A_NULL);
    class_addMethod(array_define_class, (t_method)canvas_motion, sym_motion,
        A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(array_define_class, (t_method)canvas_close,
        sym_close, A_DEFFLOAT, 0);
    class_addMethod(array_define_class, (t_method)canvas_close,
        sym_menuclose, A_DEFFLOAT, 0); /* LEGACY !!! */
    /*class_addMethod(array_define_class, (t_method)canvas_find_parent,
        gen_sym ("findparent"), A_NULL)*/
    class_addMethod(array_define_class, (t_method)canvas_save,
        sym_menusave, 0); /* LEGACY !!! */
    class_addMethod(array_define_class, (t_method)canvas_saveAs,
        sym_menusaveas, 0); /* LEGACY !!! */
    
    class_addMethod(array_define_class, (t_method)array_define_send,
        sym_send, A_SYMBOL, 0);
    class_addAnything(array_define_class, array_define_anything);
    class_setHelpName(array_define_class, sym_array);
    class_setSaveFunction(array_define_class, array_define_save);

    class_addMethod(array_define_class, (t_method)array_define_ignore,
        sym_editmode, A_GIMME, 0);

    class_addCreator((t_newmethod)arrayobj_new, sym_array, A_GIMME, 0);

    class_addCreator((t_newmethod)table_new, sym_table,
        A_DEFSYMBOL, A_DEFFLOAT, 0);

    array_size_class = class_new(sym_array__space__size,
        (t_newmethod)array_size_new, (t_method)array_client_free,
            sizeof(t_array_size), 0, A_GIMME, 0);
    class_addBang(array_size_class, array_size_bang);
    class_addFloat(array_size_class, array_size_float);
    class_setHelpName(array_size_class, sym_array);

    array_sum_class = class_new(sym_array__space__sum,
        (t_newmethod)array_sum_new, (t_method)array_client_free,
            sizeof(t_array_sum), 0, A_GIMME, 0);
    class_addBang(array_sum_class, array_sum_bang);
    class_addFloat(array_sum_class, array_sum_float);
    class_setHelpName(array_sum_class, sym_array);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
