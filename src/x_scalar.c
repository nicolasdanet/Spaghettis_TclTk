/* Copyright (c) 1997-2013 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* The "scalar" object. */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

extern t_pd *pd_newest;
extern t_class *scalar_class;

t_class *scalar_define_class;

static void *scalar_define_new(t_symbol *s, int argc, t_atom *argv)
{
    t_atom a[9];
    t_glist *gl;
    t_glist *x, *z = canvas_getCurrent();
    t_symbol *templatesym = &s_float, *asym = sym___hash__A;
    t_template *template;
    t_scalar *sc;
    int keep = 0;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-k"))
            keep = 1;
        else
        {
            post_error ("scalar define: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc && argv->a_type == A_SYMBOL)
    {
        templatesym = argv->a_w.w_symbol;
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: scalar define ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    
        /* make a canvas... */
    SET_FLOAT(a, 0);
    SET_FLOAT(a+1, 50);
    SET_FLOAT(a+2, 600);
    SET_FLOAT(a+3, 400);
    SET_SYMBOL(a+4, s);
    SET_FLOAT(a+5, 0);
    x = canvas_new (NULL, NULL, 6, a);

    x->gl_parent = z;
    x->gl_saveScalar = 0;
        /* put a scalar in it */
    template = template_findByIdentifier(utils_makeBindSymbol(templatesym));
    if (!template)
    {
        post_error ("scalar define: couldn't find template %s",
            templatesym->s_name);
        goto noscalar;
    }
    sc = scalar_new(x, utils_makeBindSymbol(templatesym));
    if (!sc)
    {
        post_error ("%s: couldn't create scalar", templatesym->s_name);
        goto noscalar;
    }
    sc->sc_g.g_next = 0;
    x->gl_graphics = &sc->sc_g;
    x->gl_saveScalar = keep;
           /* bashily unbind #A -- this would create garbage if #A were
           multiply bound but we believe in this context it's at most
           bound to whichever text_define or array was created most recently */
    asym->s_thing = 0;
        /* and now bind #A to us to receive following messages in the
        saved file or copy buffer */
    pd_bind(&x->gl_obj.te_g.g_pd, asym); 
noscalar:
    pd_newest = &x->gl_obj.te_g.g_pd;     /* mimic action of canvas_pop() */
    stack_pop(&x->gl_obj.te_g.g_pd);
    x->gl_isLoading = 0;
    
        /* bash the class to "scalar define" -- see comment in x_array,c */
    x->gl_obj.te_g.g_pd = scalar_define_class;
    return (x);
}

    /* send a pointer to the scalar to whomever is bound to the symbol */
static void scalar_define_send(t_glist *x, t_symbol *s)
{
    if (!s->s_thing)
        post_error ("scalar_define_send: %s: no such object", s->s_name);
    else if (x->gl_graphics && pd_class(&x->gl_graphics->g_pd) == scalar_class)
    {
        t_gpointer gp = GPOINTER_INIT;
        gpointer_setAsScalarType(&gp, x, (t_scalar *)&x->gl_graphics->g_pd);
        pd_pointer(s->s_thing, &gp);
        gpointer_unset(&gp);
    }
    else { PD_BUG; }
}

    /* set to a list, used to restore from scalar_define_save()s below */
static void scalar_define_set(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->gl_graphics && pd_class(&x->gl_graphics->g_pd) == scalar_class)
    {
        t_buffer *b = buffer_new();
        int nextmsg = 0, natoms;
        t_atom *vec;
        canvas_clear(x);
        buffer_deserialize(b, argc, argv);
        natoms = buffer_size(b);
        vec = buffer_atoms(b);
        canvas_readscalar(x, natoms, vec, &nextmsg, 0);
        buffer_free(b);
    }
    else { PD_BUG; }
}

    /* save to a binbuf (for file save or copy) */
static void scalar_define_save(t_gobj *z, t_buffer *bb)
{
    t_glist *x = (t_glist *)z;
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)x->gl_obj.te_xCoordinate, (float)x->gl_obj.te_yCoordinate);
    buffer_serialize(bb, x->gl_obj.te_buffer);
    buffer_appendSemicolon(bb);
    if (x->gl_saveScalar && x->gl_graphics &&
        pd_class(&x->gl_graphics->g_pd) == scalar_class)
    {
        t_buffer *b2 = buffer_new();
        t_scalar *sc = (t_scalar *)(x->gl_graphics);
        buffer_vAppend(bb, "ss", sym___hash__A, sym_set);
        canvas_writescalar(sc->sc_templateIdentifier, sc->sc_vector, b2, 0);
        buffer_serialize(bb, b2);
        buffer_appendSemicolon(bb);
        buffer_free(b2);
    }
}

/* overall creator for "scalar" objects - dispatch to "scalar define" etc */
static void *scalarobj_new(t_symbol *s, int argc, t_atom *argv)
{
    if (!argc || argv[0].a_type != A_SYMBOL)
        pd_newest = scalar_define_new(s, argc, argv);
    else
    {
        char *str = argv[0].a_w.w_symbol->s_name;
        if (!strcmp(str, "d") || !strcmp(str, "define"))
            pd_newest = scalar_define_new(s, argc-1, argv+1);
        else 
        {
            post_error ("scalar %s: unknown function", str);
            pd_newest = 0;
        }
    }
    return (pd_newest);
}


/* ---------------- global setup function -------------------- */

void x_scalar_setup(void )
{
    scalar_define_class = class_new(sym_scalar__space__define, 0,
        (t_method)canvas_free, sizeof(t_glist), 0, 0);
        
    class_addMethod(scalar_define_class, (t_method)canvas_restore,
        sym_restore, A_GIMME, 0);
    class_addClick (scalar_define_class, canvas_click);
    /*class_addMethod(scalar_define_class, (t_method)canvas_click,
        sym_click, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0); */
    class_addMethod(scalar_define_class, (t_method)canvas_dsp,
        sym_dsp, A_CANT, 0);
    class_addMethod(scalar_define_class, (t_method)canvas_map,
        sym__map, A_FLOAT, A_NULL);
    class_addMethod(scalar_define_class, (t_method)canvas_window,
        sym_window, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(scalar_define_class, (t_method)canvas_window,    /* LEGACY !!! */
        sym_setbounds, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    
    class_addMouse (scalar_define_class, canvas_mouse);
    class_addMouseUp (scalar_define_class, canvas_mouseUp);
    
    /*class_addMethod(scalar_define_class, (t_method)canvas_mouse, sym_mouse,
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(scalar_define_class, (t_method)canvas_mouseUp, sym_mouseup,
        A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);*/
        
    class_addMethod(scalar_define_class, (t_method)canvas_key, sym_key,
        A_GIMME, A_NULL);
    class_addMethod(scalar_define_class, (t_method)canvas_motion, sym_motion,
        A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(scalar_define_class, (t_method)canvas_close,
        sym_close, A_DEFFLOAT, 0);
    class_addMethod(scalar_define_class, (t_method)canvas_close,
        sym_menuclose, A_DEFFLOAT, 0); /* LEGACY !!! */
    /*class_addMethod(scalar_define_class, (t_method)canvas_find_parent,
        gen_sym ("findparent"), A_NULL);*/
    class_addMethod(scalar_define_class, (t_method)canvas_save,
        sym_menusave, 0); /* LEGACY !!! */
    class_addMethod(scalar_define_class, (t_method)canvas_saveAs,
        sym_menusaveas, 0); /* LEGACY !!! */
    
    class_addMethod(scalar_define_class, (t_method)scalar_define_send,
        sym_send, A_SYMBOL, 0);
    class_addMethod(scalar_define_class, (t_method)scalar_define_set,
        sym_set, A_GIMME, 0);
    class_setHelpName(scalar_define_class, sym_scalar);
    class_setSaveFunction(scalar_define_class, scalar_define_save);

    class_addCreator((t_newmethod)scalarobj_new, sym_scalar, A_GIMME, 0);

}
