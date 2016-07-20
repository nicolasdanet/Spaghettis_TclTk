
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

extern t_class          *garray_class;
extern t_class          *scalar_class;
extern t_class          *canvas_class;
extern t_pdinstance     *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class          *gtemplate_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _gtemplate
{
    t_object            x_obj;                      /* MUST be the first. */
    t_template          *x_template;
    t_glist             *x_owner;
    t_symbol            *x_sym;
    struct _gtemplate   *x_next;
    int                 x_argc;
    t_atom              *x_argv;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void template_conformarray (t_template *, t_template *, int *, t_array *);
static void template_conformglist (t_template *, t_template *, t_glist *, int *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* return true if two dataslot definitions match */
static int dataslot_matches(t_dataslot *ds1, t_dataslot *ds2,
    int nametoo)
{
    return ((!nametoo || ds1->ds_fieldName == ds2->ds_fieldName) &&
        ds1->ds_type == ds2->ds_type &&
            (ds1->ds_type != DATA_ARRAY ||
                ds1->ds_templateIdentifier == ds2->ds_templateIdentifier));
}

    /* stringent check to see if a "saved" template, x2, matches the current
        one (x1).  It's OK if x1 has additional scalar elements but not (yet)
        arrays.  This is used for reading in "data files". */
int template_equals(t_template *x1, t_template *x2)
{
    int i;
    if (x1->tp_size < x2->tp_size)
        return (0);
    for (i = x2->tp_size; i < x1->tp_size; i++)
    {
        if (x1->tp_vector[i].ds_type == DATA_ARRAY)
                return (0);
    }
    if (x2->tp_size > x1->tp_size)
        post("add elements...");
    for (i = 0; i < x2->tp_size; i++)
        if (!dataslot_matches(&x1->tp_vector[i], &x2->tp_vector[i], 1))
            return (0);
    return (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* --------------- CONFORMING TO CHANGES IN A TEMPLATE ------------ */

/* the following routines handle updating scalars to agree with changes
in their template.  The old template is assumed to be the "installed" one
so we can delete old items; but making new ones we have to avoid scalar_new
which would make an old one whereas we will want a new one (but whose array
elements might still be old ones.)
    LATER deal with graphics updates too... */

    /* conform the word vector of a scalar to the new template */    
static void template_conformwords(t_template *tfrom, t_template *tto,
    int *conformaction, t_word *wfrom, t_word *wto)
{
    int nfrom = tfrom->tp_size, nto = tto->tp_size, i;
    for (i = 0; i < nto; i++)
    {
        if (conformaction[i] >= 0)
        {
                /* we swap the two, in case it's an array or list, so that
                when "wfrom" is deleted the old one gets cleaned up. */
            t_word wwas = wto[i];
            wto[i] = wfrom[conformaction[i]];
            wfrom[conformaction[i]] = wwas;
        }
    }
}

    /* conform a scalar, recursively conforming arrays  */
static t_scalar *template_conformscalar(t_template *tfrom, t_template *tto,
    int *conformaction, t_glist *glist, t_scalar *scfrom)
{
    t_scalar *x;
    t_gpointer gp = GPOINTER_INIT;
    int nto = tto->tp_size, nfrom = tfrom->tp_size, i;
    t_template *scalartemplate;
    /* post("conform scalar"); */
        /* possibly replace the scalar */
    if (scfrom->sc_templateIdentifier == tfrom->tp_templateIdentifier)
    {
            /* see scalar_new() for comment about the gpointer. */
        gpointer_init(&gp);
        x = (t_scalar *)PD_MEMORY_GET(sizeof(t_scalar) +
            (tto->tp_size - 1) * sizeof(*x->sc_vector));
        x->sc_g.g_pd = scalar_class;
        x->sc_templateIdentifier = tfrom->tp_templateIdentifier;
        gpointer_setAsScalarType(&gp, glist, x);
            /* Here we initialize to the new template, but array and list
            elements will still belong to old template. */
        word_init(x->sc_vector, tto, &gp);

        // gpointer_unset?
        template_conformwords(tfrom, tto, conformaction,
            scfrom->sc_vector, x->sc_vector);
            
            /* replace the old one with the new one in the list */
        if (glist->gl_graphics == &scfrom->sc_g)
        {
            glist->gl_graphics = &x->sc_g;
            x->sc_g.g_next = scfrom->sc_g.g_next;
        }
        else
        {
            t_gobj *y, *y2;
            for (y = glist->gl_graphics; y2 = y->g_next; y = y2)
                if (y2 == &scfrom->sc_g)
            {
                x->sc_g.g_next = y2->g_next;
                y->g_next = &x->sc_g;
                goto nobug;
            }
            PD_BUG;
        nobug: ;
        }
            /* burn the old one */
        pd_free(&scfrom->sc_g.g_pd);
        scalartemplate = tto;
    }
    else
    {
        x = scfrom;
        scalartemplate = template_findByIdentifier(x->sc_templateIdentifier);
    }
        /* convert all array elements */
    for (i = 0; i < scalartemplate->tp_size; i++)
    {
        t_dataslot *ds = scalartemplate->tp_vector + i;
        if (ds->ds_type == DATA_ARRAY)
        {
            template_conformarray(tfrom, tto, conformaction, 
                x->sc_vector[i].w_array);
        }
    }
    return (x);
}

    /* conform an array, recursively conforming sublists and arrays  */
static void template_conformarray(t_template *tfrom, t_template *tto,
    int *conformaction, t_array *a)
{
    int i, j;
    t_template *scalartemplate = 0;
    if (a->a_templateIdentifier == tfrom->tp_templateIdentifier)
    {
        /* the array elements must all be conformed */
        int oldelemsize = ARRAY_WORD * tfrom->tp_size,
            newelemsize = ARRAY_WORD * tto->tp_size;
        char *newarray = PD_MEMORY_GET(newelemsize * a->a_size);
        char *oldarray = a->a_vector;
        if (a->a_stride != oldelemsize) { PD_BUG; }
        for (i = 0; i < a->a_size; i++)
        {
            t_word *wp = (t_word *)(newarray + newelemsize * i);
            word_init(wp, tto, &a->a_parent);
            template_conformwords(tfrom, tto, conformaction,
                (t_word *)(oldarray + oldelemsize * i), wp);
            word_free((t_word *)(oldarray + oldelemsize * i), tfrom);
        }
        scalartemplate = tto;
        a->a_vector = newarray;
        PD_MEMORY_FREE(oldarray);
    }
    else scalartemplate = template_findByIdentifier(a->a_templateIdentifier);
        /* convert all arrays and sublist fields in each element of the array */
    for (i = 0; i < a->a_size; i++)
    {
        t_word *wp = (t_word *)(a->a_vector + ARRAY_WORD * a->a_size * i);
        for (j = 0; j < scalartemplate->tp_size; j++)
        {
            t_dataslot *ds = scalartemplate->tp_vector + j;
            if (ds->ds_type == DATA_ARRAY)
            {
                template_conformarray(tfrom, tto, conformaction, 
                    wp[j].w_array);
            }
        }
    }
}

    /* this routine searches for every scalar in the glist that belongs
    to the "from" template and makes it belong to the "to" template.  Descend
    glists recursively.
    We don't handle redrawing here; this is to be filled in LATER... */

static void template_conformglist(t_template *tfrom, t_template *tto,
    t_glist *glist,  int *conformaction)
{
    t_gobj *g;
    /* post("conform glist %s", glist->gl_name->s_name); */
    for (g = glist->gl_graphics; g; g = g->g_next)
    {
        if (pd_class(&g->g_pd) == scalar_class)
            g = &template_conformscalar(tfrom, tto, conformaction,
                glist, (t_scalar *)g)->sc_g;
        else if (pd_class(&g->g_pd) == canvas_class)
            template_conformglist(tfrom, tto, (t_glist *)g, conformaction);
        else if (pd_class(&g->g_pd) == garray_class)
            template_conformarray(tfrom, tto, conformaction,
                garray_getArray((t_garray *)g));
    }
}

    /* globally conform all scalars from one template to another */ 
void template_conform(t_template *tfrom, t_template *tto)
{
    int nto = tto->tp_size, nfrom = tfrom->tp_size, i, j,
        *conformaction = (int *)PD_MEMORY_GET(sizeof(int) * nto),
        *conformedfrom = (int *)PD_MEMORY_GET(sizeof(int) * nfrom), doit = 0;
    for (i = 0; i < nto; i++)
        conformaction[i] = -1;
    for (i = 0; i < nfrom; i++)
        conformedfrom[i] = 0;
    for (i = 0; i < nto; i++)
    {
        t_dataslot *dataslot = &tto->tp_vector[i];
        for (j = 0; j < nfrom; j++)
        {
            t_dataslot *dataslot2 = &tfrom->tp_vector[j];
            if (dataslot_matches(dataslot, dataslot2, 1))
            {
                conformaction[i] = j;
                conformedfrom[j] = 1;
            }
        }
    }
    for (i = 0; i < nto; i++)
        if (conformaction[i] < 0)
    {
        t_dataslot *dataslot = &tto->tp_vector[i];
        for (j = 0; j < nfrom; j++)
            if (!conformedfrom[j] &&
                dataslot_matches(dataslot, &tfrom->tp_vector[j], 0))
        {
            conformaction[i] = j;
            conformedfrom[j] = 1;
        }
    }
    if (nto != nfrom)
        doit = 1;
    else for (i = 0; i < nto; i++)
        if (conformaction[i] != i)
            doit = 1;

    if (doit)
    {
        t_glist *gl;
        for (gl = pd_this->pd_roots; gl; gl = gl->gl_next)
            template_conformglist(tfrom, tto, gl, conformaction);
    }
    PD_MEMORY_FREE(conformaction);
    PD_MEMORY_FREE(conformedfrom);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *template_findcanvas(t_template *template)
{
    t_gtemplate *gt;
    if (!template) { PD_BUG; }
    if (!(gt = template->tp_list))
        return (0);
    return (gt->x_owner);
    /* return ((t_glist *)pd_findByClass(template->tp_templateIdentifier, canvas_class)); */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void template_notify(t_template *template, t_symbol *s, int argc, t_atom *argv)
{
    if (template->tp_list)
        outlet_anything(template->tp_list->x_obj.te_outlet, s, argc, argv);
}

    /* bash the first of (argv) with a pointer to a scalar, and send on
    to template as a notification message */
void template_notifyforscalar(t_template *template, t_glist *owner, t_scalar *sc, t_symbol *s, int argc, t_atom *argv)
{
    t_gpointer gp = GPOINTER_INIT;
    gpointer_setAsScalarType(&gp, owner, sc);
    SET_POINTER(argv, &gp);
    template_notify(template, s, argc, argv);
    gpointer_unset(&gp);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *gtemplate_create (t_symbol *templateIdentifier, int argc, t_atom *argv)
{
    t_template *template = template_findByIdentifier (templateIdentifier);
    
    t_gtemplate *x = (t_gtemplate *)pd_new (gtemplate_class);
        
    int i;
    t_symbol *sx = sym_x;
    x->x_owner = canvas_getCurrent();
    x->x_next = 0;
    x->x_sym = templateIdentifier;
    x->x_argc = argc;
    x->x_argv = (t_atom *)PD_MEMORY_GET(argc * sizeof(t_atom));
    for (i = 0; i < argc; i++)
        x->x_argv[i] = argv[i];

        /* already have a template by this name? */
    if (template)
    {
        x->x_template = template;
            /* if it's already got a "struct" object we
            just tack this one to the end of the list and leave it
            there. */
        if (template->tp_list)
        {
            t_gtemplate *x2, *x3;
            for (x2 = x->x_template->tp_list; x3 = x2->x_next; x2 = x3)
                ;
            x2->x_next = x;
            post("template %s: warning: already exists.", templateIdentifier->s_name);
        }
        else
        {
                /* if there's none, we just replace the template with
                our own and conform it. */
            t_template *y = template_new(&s_, argc, argv);
            canvas_paintAllScalarsByTemplate(template, SCALAR_ERASE);
                /* Unless the new template is different from the old one,
                there's nothing to do.  */
            if (!template_equals(template, y))
            {
                    /* conform everyone to the new template */
                template_conform(template, y);
                pd_free(&template->tp_pd);
                template = template_new(templateIdentifier, argc, argv);
            }
            pd_free(&y->tp_pd);
            template->tp_list = x;
            canvas_paintAllScalarsByTemplate(template, SCALAR_DRAW);
        }
    }
    else
    {
            /* otherwise make a new one and we're the only struct on it. */
        x->x_template = template = template_new(templateIdentifier, argc, argv);
        template->tp_list = x;
    }
    outlet_new(&x->x_obj, 0);
    return (x);
}

static void *gtemplate_new (t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *templateIdentifier = utils_makeBindSymbol (atom_getSymbolAtIndex (0, argc, argv));
    
    if (argc >= 1) { argc--; argv++; }
            
    return (gtemplate_create (templateIdentifier, argc, argv));
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gtemplate_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_struct,
            (t_newmethod)gtemplate_new,
            (t_method)gtemplate_free,
            sizeof (t_gtemplate),
            CLASS_DEFAULT | CLASS_NOINLET,
            A_GIMME,
            A_NULL);
                
    gtemplate_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
