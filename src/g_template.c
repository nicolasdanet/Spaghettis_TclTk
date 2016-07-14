
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

/*
This file contains text objects you would put in a canvas to define a
template.  Templates describe objects of type "array" (g_array.c) and
"scalar" (g_scalar.c).
*/

    /* the structure of a "struct" object (also the obsolete "gtemplate"
    you get when using the name "template" in a box.) */

struct _gtemplate
{
    t_object x_obj;
    t_template *x_template;
    t_glist *x_owner;
    t_symbol *x_sym;
    struct _gtemplate *x_next;
    int x_argc;
    t_atom *x_argv;
};

/* ---------------- forward definitions ---------------- */

static void template_conformarray(t_template *tfrom, t_template *tto,
    int *conformaction, t_array *a);
static void template_conformglist(t_template *tfrom, t_template *tto,
    t_glist *glist,  int *conformaction);

/* ---------------------- storage ------------------------- */

static t_class *gtemplate_class;
static t_class *template_class;

/* there's a pre-defined "float" template.  LATER should we bind this
to a symbol such as "pd-float" */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void array_getcoordinate (t_glist *glist,
    char *elem, int xonset, int yonset, int wonset, int indx,
    t_float basex, t_float basey, t_float xinc,
    t_fielddescriptor *xfielddesc, t_fielddescriptor *yfielddesc, t_fielddescriptor *wfielddesc,
    t_float *xp, t_float *yp, t_float *wp)
{
    t_float xval, yval, ypix, wpix;
    if (xonset >= 0)
        xval = *(t_float *)(elem + xonset);
    else xval = indx * xinc;
    if (yonset >= 0)
        yval = *(t_float *)(elem + yonset);
    else yval = 0;
    ypix = canvas_valueToPositionY(glist, basey +
        fielddesc_cvttocoord(yfielddesc, yval));
    if (wonset >= 0)
    {
            /* found "w" field which controls linewidth. */
        t_float wval = *(t_float *)(elem + wonset);
        wpix = canvas_valueToPositionY(glist, basey + 
            fielddesc_cvttocoord(yfielddesc, yval) +
                fielddesc_cvttocoord(wfielddesc, wval)) - ypix;
        if (wpix < 0)
            wpix = -wpix;
    }
    else wpix = 1;
    *xp = canvas_valueToPositionX(glist, basex +
        fielddesc_cvttocoord(xfielddesc, xval));
    *yp = ypix;
    *wp = wpix;
}

    /* return true if two dataslot definitions match */
static int dataslot_matches(t_dataslot *ds1, t_dataslot *ds2,
    int nametoo)
{
    return ((!nametoo || ds1->ds_name == ds2->ds_name) &&
        ds1->ds_type == ds2->ds_type &&
            (ds1->ds_type != DATA_ARRAY ||
                ds1->ds_templateIdentifier == ds2->ds_templateIdentifier));
}

/* -- templates, the active ingredient in gtemplates defined below. ------- */

t_template *template_new(t_symbol *templatesym, int argc, t_atom *argv)
{
    t_template *x = (t_template *)pd_new(template_class);
    x->tp_size = 0;
    x->tp_vector = (t_dataslot *)PD_MEMORY_GET(0);
    while (argc > 0)
    {
        int newtype, oldn, newn;
        t_symbol *newname, *newarraytemplate = &s_, *newtypesym;
        if (argc < 2 || argv[0].a_type != A_SYMBOL ||
            argv[1].a_type != A_SYMBOL)
                goto bad;
        newtypesym = argv[0].a_w.w_symbol;
        newname = argv[1].a_w.w_symbol;
        if (newtypesym == &s_float)
            newtype = DATA_FLOAT;
        else if (newtypesym == &s_symbol)
            newtype = DATA_SYMBOL;
                /* "list" is old name.. accepted here but never saved as such */
        else if (newtypesym == sym_text || newtypesym == &s_list)
            newtype = DATA_TEXT;
        else if (newtypesym == sym_array)
        {
            if (argc < 3 || argv[2].a_type != A_SYMBOL)
            {
                post_error ("array lacks element template or name");
                goto bad;
            }
            newarraytemplate = canvas_makeBindSymbol(argv[2].a_w.w_symbol);
            newtype = DATA_ARRAY;
            argc--;
            argv++;
        }
        else
        {
            post_error ("%s: no such type", newtypesym->s_name);
            goto bad;
        }
        newn = (oldn = x->tp_size) + 1;
        x->tp_vector = (t_dataslot *)PD_MEMORY_RESIZE(x->tp_vector,
            oldn * sizeof(*x->tp_vector), newn * sizeof(*x->tp_vector));
        x->tp_size = newn;
        x->tp_vector[oldn].ds_type = newtype;
        x->tp_vector[oldn].ds_name = newname;
        x->tp_vector[oldn].ds_templateIdentifier = newarraytemplate;
    bad: 
        argc -= 2; argv += 2;
    }
    if (*templatesym->s_name)
    {
        x->tp_symbol = templatesym;
        pd_bind(&x->tp_pd, x->tp_symbol);
    }
    else x->tp_symbol = templatesym;
    return (x);
}

int template_find_field(t_template *x, t_symbol *name, int *p_onset,
    int *p_type, t_symbol **p_arraytype)
{
    t_template *t;
    int i, n;
    if (!x)
    {
        PD_BUG;
        return (0);
    }
    n = x->tp_size;
    for (i = 0; i < n; i++)
        if (x->tp_vector[i].ds_name == name)
    {
        *p_onset = i * sizeof(t_word);
        *p_type = x->tp_vector[i].ds_type;
        *p_arraytype = x->tp_vector[i].ds_templateIdentifier;
        return (1);
    }
    return (0);
}

t_float template_getfloat(t_template *x, t_symbol *fieldname, t_word *wp)
{
    int onset, type;
    t_symbol *arraytype;
    t_float val = 0;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
    {
        if (type == DATA_FLOAT)
            val = *(t_float *)(((char *)wp) + onset);
        else if (0 /* loud */) post_error ("%s.%s: not a number",
            x->tp_symbol->s_name, fieldname->s_name);
    }
    else if (0 /* loud */) post_error ("%s.%s: no such field",
        x->tp_symbol->s_name, fieldname->s_name);
    return (val);
}

void template_setfloat(t_template *x, t_symbol *fieldname, t_word *wp, 
    t_float f, int loud)
{
    int onset, type;
    t_symbol *arraytype;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
     {
        if (type == DATA_FLOAT)
            *(t_float *)(((char *)wp) + onset) = f;
        else if (loud) post_error ("%s.%s: not a number",
            x->tp_symbol->s_name, fieldname->s_name);
    }
    else if (loud) post_error ("%s.%s: no such field",
        x->tp_symbol->s_name, fieldname->s_name);
}

t_symbol *template_getsymbol(t_template *x, t_symbol *fieldname, t_word *wp,
    int loud)
{
    int onset, type;
    t_symbol *arraytype;
    t_symbol *val = &s_;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
    {
        if (type == DATA_SYMBOL)
            val = *(t_symbol **)(((char *)wp) + onset);
        else if (loud) post_error ("%s.%s: not a symbol",
            x->tp_symbol->s_name, fieldname->s_name);
    }
    else if (loud) post_error ("%s.%s: no such field",
        x->tp_symbol->s_name, fieldname->s_name);
    return (val);
}

void template_setsymbol(t_template *x, t_symbol *fieldname, t_word *wp, 
    t_symbol *s, int loud)
{
    int onset, type;
    t_symbol *arraytype;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
     {
        if (type == DATA_SYMBOL)
            *(t_symbol **)(((char *)wp) + onset) = s;
        else if (loud) post_error ("%s.%s: not a symbol",
            x->tp_symbol->s_name, fieldname->s_name);
    }
    else if (loud) post_error ("%s.%s: no such field",
        x->tp_symbol->s_name, fieldname->s_name);
}

    /* stringent check to see if a "saved" template, x2, matches the current
        one (x1).  It's OK if x1 has additional scalar elements but not (yet)
        arrays.  This is used for reading in "data files". */
int template_match(t_template *x1, t_template *x2)
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
    if (scfrom->sc_templateIdentifier == tfrom->tp_symbol)
    {
            /* see scalar_new() for comment about the gpointer. */
        gpointer_init(&gp);
        x = (t_scalar *)PD_MEMORY_GET(sizeof(t_scalar) +
            (tto->tp_size - 1) * sizeof(*x->sc_vector));
        x->sc_g.g_pd = scalar_class;
        x->sc_templateIdentifier = tfrom->tp_symbol;
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
        scalartemplate = template_findbyname(x->sc_templateIdentifier);
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
    if (a->a_templateIdentifier == tfrom->tp_symbol)
    {
        /* the array elements must all be conformed */
        int oldelemsize = sizeof(t_word) * tfrom->tp_size,
            newelemsize = sizeof(t_word) * tto->tp_size;
        char *newarray = PD_MEMORY_GET(newelemsize * a->a_size);
        char *oldarray = a->a_vector;
        if (a->a_elementSize != oldelemsize) { PD_BUG; }
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
    else scalartemplate = template_findbyname(a->a_templateIdentifier);
        /* convert all arrays and sublist fields in each element of the array */
    for (i = 0; i < a->a_size; i++)
    {
        t_word *wp = (t_word *)(a->a_vector + sizeof(t_word) * a->a_size * i);
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

t_template *template_findbyname(t_symbol *s)
{
    return ((t_template *)pd_findByClass(s, template_class));
}

t_glist *template_findcanvas(t_template *template)
{
    t_gtemplate *gt;
    if (!template) { PD_BUG; }
    if (!(gt = template->tp_list))
        return (0);
    return (gt->x_owner);
    /* return ((t_glist *)pd_findByClass(template->tp_symbol, canvas_class)); */
}

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

    /* call this when reading a patch from a file to declare what templates
    we'll need.  If there's already a template, check if it matches.
    If it doesn't it's still OK as long as there are no "struct" (gtemplate)
    objects hanging from it; we just conform everyone to the new template.
    If there are still struct objects belonging to the other template, we're
    in trouble.  LATER we'll figure out how to conform the new patch's objects
    to the pre-existing struct. */
static void *template_usetemplate(void *dummy, t_symbol *s,
    int argc, t_atom *argv)
{
    t_template *x;
    t_symbol *templatesym =
        canvas_makeBindSymbol(atom_getSymbolAtIndex(0, argc, argv));
    if (!argc)
        return (0);
    argc--; argv++;
            /* check if there's already a template by this name. */
    if ((x = (t_template *)pd_findByClass(templatesym, template_class)))
    {
        t_template *y = template_new(&s_, argc, argv), *y2;
            /* If the new template is the same as the old one,
            there's nothing to do.  */
        if (!template_match(x, y))
        {
                /* Are there "struct" objects upholding this template? */
            if (x->tp_list)
            {
                    /* don't know what to do here! */
                post_error ("%s: template mismatch",
                    templatesym->s_name);
            }
            else
            {
                    /* conform everyone to the new template */
                template_conform(x, y);
                pd_free(&x->tp_pd);
                y2 = template_new(templatesym, argc, argv);
                y2->tp_list = 0;
            }
        }
        pd_free(&y->tp_pd);
    }
        /* otherwise, just make one. */
    else template_new(templatesym, argc, argv);
    return (0);
}

    /* here we assume someone has already cleaned up all instances of this. */
void template_free(t_template *x)
{
    if (*x->tp_symbol->s_name)
        pd_unbind(&x->tp_pd, x->tp_symbol);
    PD_MEMORY_FREE(x->tp_vector);
}

void template_setup(void)
{
    template_class = class_new(sym_template, 0, (t_method)template_free,
        sizeof(t_template), CLASS_NOBOX, 0);
    class_addMethod(pd_canvasMaker, (t_method)template_usetemplate,
        sym_struct, A_GIMME, 0);
        
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

/* ---------------  FIELD DESCRIPTORS ---------------------- */

/* a field descriptor can hold a constant or a variable; if a variable,
it's the name of a field in the template we belong to.  LATER, we might
want to cache the offset of the field so we don't have to search for it
every single time we draw the object.
*/

void fielddesc_setfloat_const(t_fielddescriptor *fd, t_float f)
{
    fd->fd_type = A_FLOAT;
    fd->fd_var = 0;
    fd->fd_un.fd_float = f;
    fd->fd_v1 = fd->fd_v2 = fd->fd_screen1 = fd->fd_screen2 =
        fd->fd_quantum = 0;
}

void fielddesc_setsymbol_const(t_fielddescriptor *fd, t_symbol *s)
{
    fd->fd_type = A_SYMBOL;
    fd->fd_var = 0;
    fd->fd_un.fd_symbol = s;
    fd->fd_v1 = fd->fd_v2 = fd->fd_screen1 = fd->fd_screen2 =
        fd->fd_quantum = 0;
}

void fielddesc_setfloat_var(t_fielddescriptor *fd, t_symbol *s)
{
    char *s1, *s2, *s3, strbuf[PD_STRING];
    int i;
    fd->fd_type = A_FLOAT;
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

#define CLOSED 1
#define BEZ 2
#define NOMOUSE 4

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
            fd->fd_type = A_SYMBOL;
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
            fd->fd_type = A_ARRAY;
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
    if (f->fd_type == A_FLOAT)
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
    if (f->fd_type == A_FLOAT)
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
    if (f->fd_type == A_SYMBOL)
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
    if (f->fd_type == A_FLOAT && f->fd_var)
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
#pragma mark -

/* ---------------- curves and polygons (joined segments) ---------------- */

/*
curves belong to templates and describe how the data in the template are to
be drawn.  The coordinates of the curve (and other display features) can
be attached to fields in the template.
*/

t_class *curve_class;

typedef struct _curve
{
    t_object x_obj;
    int x_flags;            /* CLOSED and/or BEZ and/or NOMOUSE */
    t_fielddescriptor x_fillcolor;
    t_fielddescriptor x_outlinecolor;
    t_fielddescriptor x_width;
    t_fielddescriptor x_vis;
    int x_npoints;
    t_fielddescriptor *x_vec;
    t_glist *x_canvas;
} t_curve;

static void *curve_new(t_symbol *classsym, int argc, t_atom *argv)
{
    t_curve *x = (t_curve *)pd_new(curve_class);
    char *classname = classsym->s_name;
    int flags = 0;
    int nxy, i;
    t_fielddescriptor *fd;
    x->x_canvas = canvas_getCurrent();
    if (classname[0] == 'f')
    {
        classname += 6;
        flags |= CLOSED;
    }
    else classname += 4;
    if (classname[0] == 'c') flags |= BEZ;
    fielddesc_setfloat_const(&x->x_vis, 1);
    while (1)
    {
        t_symbol *firstarg = atom_getSymbolAtIndex(0, argc, argv);
        if (!strcmp(firstarg->s_name, "-v") && argc > 1)
        {
            fielddesc_setfloatarg(&x->x_vis, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-x"))
        {
            flags |= NOMOUSE;
            argc -= 1; argv += 1;
        }
        else break;
    }
    x->x_flags = flags;
    if ((flags & CLOSED) && argc)
        fielddesc_setfloatarg(&x->x_fillcolor, argc--, argv++);
    else fielddesc_setfloat_const(&x->x_fillcolor, 0); 
    if (argc) fielddesc_setfloatarg(&x->x_outlinecolor, argc--, argv++);
    else fielddesc_setfloat_const(&x->x_outlinecolor, 0);
    if (argc) fielddesc_setfloatarg(&x->x_width, argc--, argv++);
    else fielddesc_setfloat_const(&x->x_width, 1);
    if (argc < 0) argc = 0;
    nxy =  (argc + (argc & 1));
    x->x_npoints = (nxy>>1);
    x->x_vec = (t_fielddescriptor *)PD_MEMORY_GET(nxy * sizeof(t_fielddescriptor));
    for (i = 0, fd = x->x_vec; i < argc; i++, fd++, argv++)
        fielddesc_setfloatarg(fd, 1, argv);
    if (argc & 1) fielddesc_setfloat_const(fd, 0);

    return (x);
}

void curve_float(t_curve *x, t_float f)
{
    int viswas;
    if (x->x_vis.fd_type != A_FLOAT || x->x_vis.fd_var)
    {
        post_error ("global vis/invis for a template with variable visibility");
        return;
    }
    viswas = (x->x_vis.fd_un.fd_float != 0);
    
    if ((f != 0 && viswas) || (f == 0 && !viswas))
        return;
    canvas_paintAllScalarsByView(x->x_canvas, SCALAR_ERASE);
    fielddesc_setfloat_const(&x->x_vis, (f != 0));
    canvas_paintAllScalarsByView(x->x_canvas, SCALAR_DRAW);
}

/* -------------------- widget behavior for curve ------------ */

static void curve_getrect(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_curve *x = (t_curve *)z;
    int i, n = x->x_npoints;
    t_fielddescriptor *f = x->x_vec;
    int x1 = PD_INT_MAX, x2 = -PD_INT_MAX, y1 = PD_INT_MAX, y2 = -PD_INT_MAX;
    if (!fielddesc_getfloat(&x->x_vis, template, data, 0) ||
        (x->x_flags & NOMOUSE))
    {
        *xp1 = *yp1 = PD_INT_MAX;
        *xp2 = *yp2 = -PD_INT_MAX;
        return;
    }
    for (i = 0, f = x->x_vec; i < n; i++, f += 2)
    {
        int xloc = canvas_valueToPositionX(glist,
            basex + fielddesc_getcoord(f, template, data, 0));
        int yloc = canvas_valueToPositionY(glist,
            basey + fielddesc_getcoord(f+1, template, data, 0));
        if (xloc < x1) x1 = xloc;
        if (xloc > x2) x2 = xloc;
        if (yloc < y1) y1 = yloc;
        if (yloc > y2) y2 = yloc;
    }
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2; 
}

static void curve_displace(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int dx, int dy)
{
    /* refuse */
}

static void curve_select(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* fill in later */
}

static void curve_activate(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* fill in later */
}

#if 0
static int rangecolor(int n)    /* 0 to 9 in 5 steps */
{
    int n2 = n/2;               /* 0 to 4 */
    int ret = (n2 << 6);        /* 0 to 256 in 5 steps */
    if (ret > 255) ret = 255;
    return (ret);
}
#endif

static int rangecolor(int n)    /* 0 to 9 in 5 steps */
{
    int n2 = (n == 9 ? 8 : n);               /* 0 to 8 */
    int ret = (n2 << 5);        /* 0 to 256 in 9 steps */
    if (ret > 255) ret = 255;
    return (ret);
}

void numbertocolor(int n, char *s)
{
    int red, blue, green;
    if (n < 0) n = 0;
    red = n / 100;
    blue = ((n / 10) % 10);
    green = n % 10;
    sprintf(s, "#%2.2x%2.2x%2.2x", rangecolor(red), rangecolor(blue),
        rangecolor(green));
}

static void curve_vis(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_float basex, t_float basey,
    int vis)
{
    t_curve *x = (t_curve *)z;
    int i, n = x->x_npoints;
    t_fielddescriptor *f = x->x_vec;
    
        /* see comment in plot_vis() */
    if (vis && !fielddesc_getfloat(&x->x_vis, template, data, 0))
        return;
    if (vis)
    {
        if (n > 1)
        {
            int flags = x->x_flags, closed = (flags & CLOSED);
            t_float width = fielddesc_getfloat(&x->x_width, template, data, 1);
            char outline[20], fill[20];
            int pix[200];
            if (n > 100)
                n = 100;
                /* calculate the pixel values before we start printing
                out the TK message so that "error" printout won't be
                interspersed with it.  Only show up to 100 points so we don't
                have to allocate memory here. */
            for (i = 0, f = x->x_vec; i < n; i++, f += 2)
            {
                pix[2*i] = canvas_valueToPositionX(glist,
                    basex + fielddesc_getcoord(f, template, data, 1));
                pix[2*i+1] = canvas_valueToPositionY(glist,
                    basey + fielddesc_getcoord(f+1, template, data, 1));
            }
            if (width < 1) width = 1;
            numbertocolor(
                fielddesc_getfloat(&x->x_outlinecolor, template, data, 1),
                outline);
            if (flags & CLOSED)
            {
                numbertocolor(
                    fielddesc_getfloat(&x->x_fillcolor, template, data, 1),
                    fill);
                sys_vGui(".x%lx.c create polygon\\\n",
                    canvas_getView(glist));
            }
            else sys_vGui(".x%lx.c create line\\\n", canvas_getView(glist));
            for (i = 0; i < n; i++)
                sys_vGui("%d %d\\\n", pix[2*i], pix[2*i+1]);
            sys_vGui("-width %f\\\n", width);
            if (flags & CLOSED) sys_vGui("-fill %s -outline %s\\\n",
                fill, outline);
            else sys_vGui("-fill %s\\\n", outline);
            if (flags & BEZ) sys_vGui("-smooth 1\\\n");
            sys_vGui("-tags curve%lx\n", data);
        }
        else post("warning: curves need at least two points to be graphed");
    }
    else
    {
        if (n > 1) sys_vGui(".x%lx.c delete curve%lx\n",
            canvas_getView(glist), data);      
    }
}

static int curve_motion_field;
static t_float curve_motion_xcumulative;
static t_float curve_motion_xbase;
static t_float curve_motion_xper;
static t_float curve_motion_ycumulative;
static t_float curve_motion_ybase;
static t_float curve_motion_yper;
static t_glist *curve_motion_glist;
static t_scalar *curve_motion_scalar;
static t_array *curve_motion_array;
static t_word *curve_motion_wp;
static t_template *curve_motion_template;
static t_gpointer curve_motion_gpointer;

    /* LATER protect against the template changing or the scalar disappearing
    probably by attaching a gpointer here ... */

static void curve_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    t_curve *x = (t_curve *)z;
    t_fielddescriptor *f = x->x_vec + curve_motion_field;
    t_atom at;
    if (!gpointer_isValid(&curve_motion_gpointer, 0))
    {
        post("curve_motion: scalar disappeared");
        return;
    }
    curve_motion_xcumulative += dx;
    curve_motion_ycumulative += dy;
    if (f->fd_var && (dx != 0))
    {
        fielddesc_setcoord(f, curve_motion_template, curve_motion_wp,
            curve_motion_xbase + curve_motion_xcumulative * curve_motion_xper,
                1); 
    }
    if ((f+1)->fd_var && (dy != 0))
    {
        fielddesc_setcoord(f+1, curve_motion_template, curve_motion_wp,
            curve_motion_ybase + curve_motion_ycumulative * curve_motion_yper,
                1); 
    }
        /* LATER figure out what to do to notify for an array? */
    if (curve_motion_scalar)
        template_notifyforscalar(curve_motion_template, curve_motion_glist, 
            curve_motion_scalar, sym_change, 1, &at);
    if (curve_motion_scalar)
        scalar_redraw(curve_motion_scalar, curve_motion_glist);
    if (curve_motion_array)
        array_redraw(curve_motion_array, curve_motion_glist);
}

static int curve_click(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_curve *x = (t_curve *)z;
    int i, n = x->x_npoints;
    int bestn = -1;
    int besterror = PD_INT_MAX;
    t_fielddescriptor *f;
    if (!fielddesc_getfloat(&x->x_vis, template, data, 0))
        return (0);
    for (i = 0, f = x->x_vec; i < n; i++, f += 2)
    {
        int xval = fielddesc_getcoord(f, template, data, 0),
            xloc = canvas_valueToPositionX(glist, basex + xval);
        int yval = fielddesc_getcoord(f+1, template, data, 0),
            yloc = canvas_valueToPositionY(glist, basey + yval);
        int xerr = xloc - xpix, yerr = yloc - ypix;
        if (!f->fd_var && !(f+1)->fd_var)
            continue;
        if (xerr < 0)
            xerr = -xerr;
        if (yerr < 0)
            yerr = -yerr;
        if (yerr > xerr)
            xerr = yerr;
        if (xerr < besterror)
        {
            curve_motion_xbase = xval;
            curve_motion_ybase = yval;
            besterror = xerr;
            bestn = i;
        }
    }
    if (besterror > 6)
        return (0);
    if (doit)
    {
        curve_motion_xper = canvas_positionToValueX(glist, 1)
            - canvas_positionToValueX(glist, 0);
        curve_motion_yper = canvas_positionToValueY(glist, 1)
            - canvas_positionToValueY(glist, 0);
        curve_motion_xcumulative = 0;
        curve_motion_ycumulative = 0;
        curve_motion_glist = glist;
        curve_motion_scalar = sc;
        curve_motion_array = ap;
        curve_motion_wp = data;
        curve_motion_field = 2*bestn;
        curve_motion_template = template;
        if (curve_motion_scalar)
            gpointer_setAsScalarType(&curve_motion_gpointer, curve_motion_glist,
                curve_motion_scalar);
        else gpointer_setAsWordType(&curve_motion_gpointer,
                curve_motion_array, curve_motion_wp);
        canvas_setMotionFunction(glist, z, (t_motionfn)curve_motion, xpix, ypix);
    }
    return (1);
}

t_parentwidgetbehavior curve_widgetbehavior =
{
    curve_getrect,
    curve_displace,
    curve_select,
    curve_activate,
    curve_vis,
    curve_click,
};

static void curve_free(t_curve *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

void curve_setup(void)
{
    curve_class = class_new(sym_drawpolygon, (t_newmethod)curve_new,
        (t_method)curve_free, sizeof(t_curve), 0, A_GIMME, 0);
    class_setDrawCommand(curve_class);
    class_addCreator((t_newmethod)curve_new, sym_drawcurve,
        A_GIMME, 0);
    class_addCreator((t_newmethod)curve_new, sym_filledpolygon,
        A_GIMME, 0);
    class_addCreator((t_newmethod)curve_new, sym_filledcurve,
        A_GIMME, 0);
    class_setParentWidgetBehavior(curve_class, &curve_widgetbehavior);
    class_addFloat(curve_class, curve_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
