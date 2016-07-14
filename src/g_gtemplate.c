
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
#include "s_system.h"    /* for font_getHostFontSize */
#include "g_graphics.h"

extern t_class *garray_class;
extern t_class *scalar_class;
extern t_pd pd_canvasMaker;
extern t_class *canvas_class;
extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_class *gtemplate_class;

/* ---------------- gtemplates.  One per canvas. ----------- */

/* "Struct": an object that searches for, and if necessary creates, 
a template (above).  Other objects in the canvas then can give drawing
instructions for the template.  The template doesn't go away when the
"struct" is deleted, so that you can replace it with
another one to add new fields, for example. */

static void *gtemplate_donew(t_symbol *sym, int argc, t_atom *argv)
{
    t_gtemplate *x = (t_gtemplate *)pd_new(gtemplate_class);
    t_template *t = template_findbyname(sym);
    int i;
    t_symbol *sx = sym_x;
    x->x_owner = canvas_getCurrent();
    x->x_next = 0;
    x->x_sym = sym;
    x->x_argc = argc;
    x->x_argv = (t_atom *)PD_MEMORY_GET(argc * sizeof(t_atom));
    for (i = 0; i < argc; i++)
        x->x_argv[i] = argv[i];

        /* already have a template by this name? */
    if (t)
    {
        x->x_template = t;
            /* if it's already got a "struct" object we
            just tack this one to the end of the list and leave it
            there. */
        if (t->tp_list)
        {
            t_gtemplate *x2, *x3;
            for (x2 = x->x_template->tp_list; x3 = x2->x_next; x2 = x3)
                ;
            x2->x_next = x;
            post("template %s: warning: already exists.", sym->s_name);
        }
        else
        {
                /* if there's none, we just replace the template with
                our own and conform it. */
            t_template *y = template_new(&s_, argc, argv);
            canvas_paintAllScalarsByTemplate(t, SCALAR_ERASE);
                /* Unless the new template is different from the old one,
                there's nothing to do.  */
            if (!template_match(t, y))
            {
                    /* conform everyone to the new template */
                template_conform(t, y);
                pd_free(&t->tp_pd);
                t = template_new(sym, argc, argv);
            }
            pd_free(&y->tp_pd);
            t->tp_list = x;
            canvas_paintAllScalarsByTemplate(t, SCALAR_DRAW);
        }
    }
    else
    {
            /* otherwise make a new one and we're the only struct on it. */
        x->x_template = t = template_new(sym, argc, argv);
        t->tp_list = x;
    }
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void *gtemplate_new(t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *sym = atom_getSymbolAtIndex(0, argc, argv);
    if (argc >= 1)
        argc--; argv++;
    if (sym->s_name[0] == '-')
        post("warning: struct '%s' initial '-' may confuse get/set, etc.",
            sym->s_name);  
    return (gtemplate_donew(canvas_makeBindSymbol(sym), argc, argv));
}

    /* old version (0.34) -- delete 2003 or so */
static void *gtemplate_new_old(t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *sym = canvas_makeBindSymbol(canvas_getCurrent()->gl_name);
    static int warned;
    if (!warned)
    {
        post("warning -- 'template' (%s) is obsolete; replace with 'struct'",
            sym->s_name);
        warned = 1;
    }
    return (gtemplate_donew(sym, argc, argv));
}

t_template *gtemplate_get(t_gtemplate *x)
{
    return (x->x_template);
}

static void gtemplate_free(t_gtemplate *x)
{
        /* get off the template's list */
    t_template *t = x->x_template;
    t_gtemplate *y;
    if (x == t->tp_list)
    {
        canvas_paintAllScalarsByTemplate(t, SCALAR_ERASE);
        if (x->x_next)
        {
                /* if we were first on the list, and there are others on
                the list, make a new template corresponding to the new
                first-on-list and replace the existing template with it. */
            t_template *z = template_new(&s_,
                x->x_next->x_argc, x->x_next->x_argv);
            template_conform(t, z);
            pd_free(&t->tp_pd);
            pd_free(&z->tp_pd);
            z = template_new(x->x_sym, x->x_next->x_argc, x->x_next->x_argv);
            z->tp_list = x->x_next;
            for (y = z->tp_list; y ; y = y->x_next)
                y->x_template = z;
        }
        else t->tp_list = 0;
        canvas_paintAllScalarsByTemplate(t, SCALAR_DRAW);
    }
    else
    {
        t_gtemplate *x2, *x3;
        for (x2 = t->tp_list; x3 = x2->x_next; x2 = x3)
        {
            if (x == x3)
            {
                x2->x_next = x3->x_next;
                break;
            }
        }
    }
    PD_MEMORY_FREE(x->x_argv);
}

void gtemplate_setup(void)
{
    gtemplate_class = class_new (sym_struct,
        (t_newmethod)gtemplate_new, (t_method)gtemplate_free,
        sizeof(t_gtemplate), CLASS_NOINLET, A_GIMME, 0);
    class_addCreator((t_newmethod)gtemplate_new_old, sym_template,
        A_GIMME, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
