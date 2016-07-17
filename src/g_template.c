
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

extern t_pd             pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class          *template_class;            /* Shared. */

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
        if (x->tp_vector[i].ds_fieldName == name)
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
            x->tp_templateIdentifier->s_name, fieldname->s_name);
    }
    else if (0 /* loud */) post_error ("%s.%s: no such field",
        x->tp_templateIdentifier->s_name, fieldname->s_name);
    return (val);
}

void template_setfloat(t_template *x, t_symbol *fieldname, t_word *wp, t_float f)
{
    int onset, type;
    t_symbol *arraytype;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
     {
        if (type == DATA_FLOAT)
            *(t_float *)(((char *)wp) + onset) = f;
        else if (0 /* loud */) post_error ("%s.%s: not a number",
            x->tp_templateIdentifier->s_name, fieldname->s_name);
    }
    else if (0 /* loud */) post_error ("%s.%s: no such field",
        x->tp_templateIdentifier->s_name, fieldname->s_name);
}

t_symbol *template_getsymbol(t_template *x, t_symbol *fieldname, t_word *wp)
{
    int onset, type;
    t_symbol *arraytype;
    t_symbol *val = &s_;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
    {
        if (type == DATA_SYMBOL)
            val = *(t_symbol **)(((char *)wp) + onset);
        else if (0 /* loud */) post_error ("%s.%s: not a symbol",
            x->tp_templateIdentifier->s_name, fieldname->s_name);
    }
    else if (0 /* loud */) post_error ("%s.%s: no such field",
        x->tp_templateIdentifier->s_name, fieldname->s_name);
    return (val);
}

void template_setsymbol(t_template *x, t_symbol *fieldname, t_word *wp, t_symbol *s)
{
    int onset, type;
    t_symbol *arraytype;
    if (template_find_field(x, fieldname, &onset, &type, &arraytype))
     {
        if (type == DATA_SYMBOL)
            *(t_symbol **)(((char *)wp) + onset) = s;
        else if (0 /* loud */) post_error ("%s.%s: not a symbol",
            x->tp_templateIdentifier->s_name, fieldname->s_name);
    }
    else if (0 /* loud */) post_error ("%s.%s: no such field",
        x->tp_templateIdentifier->s_name, fieldname->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    if (a->a_templateIdentifier == tfrom->tp_templateIdentifier)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_template *template_findbyname(t_symbol *s)
{
    return ((t_template *)pd_findByClass(s, template_class));
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
        utils_makeBindSymbol(atom_getSymbolAtIndex(0, argc, argv));
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void template_newParse (t_template *x, int argc, t_atom *argv)
{
    while (argc > 0) {

        if ((argc >= 2) && IS_SYMBOL (argv + 0) && IS_SYMBOL (argv + 1)) {
        //
        t_symbol *type               = GET_SYMBOL (argv + 0);
        t_symbol *fieldName          = GET_SYMBOL (argv + 1);
        t_symbol *templateIdentifier = &s_;
        
        int k = -1;
        
        #if PD_WITH_LEGACY
        
        if (type == sym_list) { type = sym_text; }
            
        #endif
        
        if (type == sym_float)       { k = DATA_FLOAT;  }
        else if (type == sym_symbol) { k = DATA_SYMBOL; }
        else if (type == sym_text)   { k = DATA_TEXT;   }
        else if (type == sym_array)  {
            if (argc >= 3 && IS_SYMBOL (argv + 2)) {
                templateIdentifier = utils_makeBindSymbol (GET_SYMBOL (argv + 2));
                k = DATA_ARRAY;
                argc--;
                argv++;
            }
        }
        
        if (k < 0) { PD_BUG; }
        else {
        //
        int oldSize = x->tp_size;
        int newSize = x->tp_size + 1;
        size_t m = oldSize * sizeof (t_dataslot);
        size_t n = newSize * sizeof (t_dataslot);
        
        x->tp_vector = (t_dataslot *)PD_MEMORY_RESIZE (x->tp_vector, m, n);
        x->tp_size   = newSize;
        
        x->tp_vector[newSize - 1].ds_type               = k;
        x->tp_vector[newSize - 1].ds_fieldName          = fieldName;
        x->tp_vector[newSize - 1].ds_templateIdentifier = templateIdentifier;
        //
        }
        //
        }

    argc -= 2;
    argv += 2;
    //
    }
}
    
t_template *template_new (t_symbol *templateIdentifier, int argc, t_atom *argv)
{
    t_template *x = (t_template *)pd_new (template_class);
    
    PD_ASSERT (templateIdentifier);
    
    x->tp_size               = 0;
    x->tp_vector             = (t_dataslot *)PD_MEMORY_GET (0);
    x->tp_list               = NULL;
    x->tp_templateIdentifier = templateIdentifier;
    
    template_newParse (x, argc, argv);
    
    if (x->tp_templateIdentifier != &s_) { pd_bind (cast_pd (x), x->tp_templateIdentifier); }

    return x;
}

void template_free (t_template *x)
{
    if (x->tp_templateIdentifier != &s_) { pd_unbind (cast_pd (x), x->tp_templateIdentifier); }
    
    PD_MEMORY_FREE (x->tp_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void template_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_template,
        NULL, 
        (t_method)template_free,
        sizeof (t_template),
        CLASS_NOBOX,
        A_NULL);
    
    class_addMethod (pd_canvasMaker, (t_method)template_usetemplate, sym_struct, A_GIMME, A_NULL);
        
    template_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
