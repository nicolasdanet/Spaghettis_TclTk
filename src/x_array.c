
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_class  *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *arraydefine_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void arraydefine_yrange(t_glist *x, t_float ylo, t_float yhi)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void arraydefine_ignore(t_glist *x,
    t_symbol *s, int argc, t_atom *argv)
{
}

    /* send a pointer to the scalar that owns this array to
    whomever is bound to the given symbol */
static void arraydefine_send(t_glist *x, t_symbol *s)
{
    t_glist *gl = (x->gl_graphics ? canvas_castToGlistChecked(&x->gl_graphics->g_pd) : 0);
    if (!s->s_thing)
        post_error ("arraydefine_send: %s: no such object", s->s_name);
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
static void arraydefine_anything(t_glist *x,
    t_symbol *s, int argc, t_atom *argv)
{
    t_glist *gl = (x->gl_graphics ? canvas_castToGlistChecked(&x->gl_graphics->g_pd) : 0);
    if (gl && gl->gl_graphics && pd_class(&gl->gl_graphics->g_pd) == garray_class)
        pd_message(&gl->gl_graphics->g_pd, s, argc, argv);
    else { PD_BUG; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraydefine_save(t_gobj *z, t_buffer *bb)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *arraydefine_new(t_symbol *s, int argc, t_atom *argv)
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
    x = (t_glist *)table_makeObject(arrayname, arraysize, keep);
    
        /* bash the class to "array define".  We don't do this earlier in
        part so that canvas_getCurrent() will work while the glist and
        garray are being created.  There may be other, unknown side effects. */
    x->gl_obj.te_g.g_pd = arraydefine_class;
    arraydefine_yrange(x, ylo, yhi);
    return (x);
}

static void *arraydefine_makeObject(t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = arraydefine_new(s, argc, argv);
    else
    {
        char *str = argv[0].a_w.w_symbol->s_name;
        if (!strcmp(str, "d") || !strcmp(str, "define"))
            pd_newest = arraydefine_new(s, argc-1, argv+1);
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraydefine_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__define,
            NULL,
            (t_method)canvas_free,
            sizeof (t_glist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)arraydefine_makeObject, sym_array, A_GIMME, A_NULL);
    
    class_addKey (c, canvas_key);
    class_addMouse (c, canvas_mouse);
    class_addMouseUp (c, canvas_mouseUp);
    class_addClick (c, canvas_click);
    class_addMotion (c, canvas_motion);
    
    class_addMethod (c, (t_method)canvas_dsp,           sym_dsp,        A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,       sym_restore,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_map,           sym__map,       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,         sym_close,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,          sym_save,       A_NULL); 
    class_addMethod (c, (t_method)canvas_saveAs,        sym_saveas,     A_NULL);
    
    class_addMethod (c, (t_method)canvas_window,
        sym_window,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)arraydefine_ignore,   sym_editmode,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)arraydefine_send,     sym_send,       A_SYMBOL, A_NULL);
    
    class_addAnything (c, arraydefine_anything);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_close,         sym_menuclose,  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,          sym_menusave,   A_NULL); 
    class_addMethod (c, (t_method)canvas_saveAs,        sym_menusaveas, A_NULL);
    
    class_addMethod (c, (t_method)canvas_window, 
        sym_setbounds,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    #endif
    
    class_setHelpName (c, sym_array);
    class_setSaveFunction (c, arraydefine_save);
    
    arraydefine_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
