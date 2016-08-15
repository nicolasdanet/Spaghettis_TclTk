
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
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *scalar_class;
extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_findTemplatesAppendPerform (t_symbol *templateIdentifier, int *n, t_symbol ***v)
{
    int t = *n;
    t_symbol **templates = *v;
    int alreadyExist = 0;
    int i;
    
    for (i = 0; i < t; i++) { if (templates[i] == templateIdentifier) { alreadyExist = 1; break; } }

    if (!alreadyExist) {
    //
    int oldSize = sizeof (t_symbol *) * (t);
    int newSize = sizeof (t_symbol *) * (t + 1);
        
    templates    = (t_symbol **)PD_MEMORY_RESIZE (templates, oldSize, newSize);
    templates[t] = templateIdentifier;
    
    *v = templates;
    *n = t + 1;
    //
    }
}

static void canvas_findTemplatesAppendRecursive (t_template *tmpl, int *n, t_symbol ***v)
{
    int i;

    canvas_findTemplatesAppendPerform (template_getTemplateIdentifier (tmpl), n, v);

    for (i = 0; i < template_getSize (tmpl); i++) {
        t_template *t = template_getTemplateIfArrayAtIndex (tmpl, i);
        if (t) {
            canvas_findTemplatesAppendRecursive (t, n, v);
        }
    }
}

static void canvas_findTemplatesRecursive (t_glist *glist, int *n, t_symbol ***v)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == scalar_class) {
            canvas_findTemplatesAppendRecursive (scalar_getTemplate (cast_scalar (y)), n, v);
        }
        if (pd_class (y) == canvas_class) { 
            canvas_findTemplatesRecursive (cast_glist (y), n, v);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_serializeTemplates (t_glist *glist, t_buffer *b)
{
    t_symbol **v = PD_MEMORY_GET (0);
    int i, n = 0;
    
    canvas_findTemplatesRecursive (glist, &n, &v);
    
    for (i = 0; i < n; i++) { template_serialize (template_findByIdentifier (v[i]), b); }
    
    PD_MEMORY_FREE (v);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int canvas_fetchNext (int argc, t_atom *argv, int *current, int *next)
{
    int k = *next;
    int n = 0;
    
    *current = k;
    
    if (k < argc) {
    //
    int i;
    for (i = k; i < argc && !IS_SEMICOLON (argv + i); i++) { }
    *next = PD_MIN (i + 1, argc);
    n = i - k;
    //
    }
    
    // post_atoms (n, argv + k);
    
    return n;
}

static void canvas_readElements (t_glist *x,
    int natoms,
    t_atom *vec,
    int *p_nextmsg,
    t_symbol *templatesym,
    t_word *w,
    int argc,
    t_atom *argv)
{
    int message, nline, n, i;

    t_template *template = template_findByIdentifier(templatesym);
    if (!template)
    {
        post_error ("%s: no such template", templatesym->s_name);
        *p_nextmsg = natoms;
        return;
    }
    word_restore(w, template, argc, argv);
    n = template->tp_size;
    for (i = 0; i < n; i++)
    {
        if (template->tp_vector[i].ds_type == DATA_ARRAY)
        {
            int j;
            t_array *a = w[i].w_array;
            int elemsize = a->a_stride, nitems = 0;
            t_symbol *arraytemplatesym = template->tp_vector[i].ds_templateIdentifier;
            t_template *arraytemplate =
                template_findByIdentifier(arraytemplatesym);
            if (!arraytemplate)
            {
                post_error ("%s: no such template", arraytemplatesym->s_name);
            }
            else while (1)
            {
                t_word *element;
                int nline = canvas_fetchNext(natoms, vec, &message, p_nextmsg);
                    /* empty line terminates array */
                if (!nline)
                    break;
                array_resize(a, nitems + 1);
                element = (t_word *)(((char *)a->a_vector) +
                    nitems * elemsize);
                canvas_readElements(x, natoms, vec, p_nextmsg, arraytemplatesym,
                    element, nline, vec + message);
                nitems++;
            }
        }
        else if (template->tp_vector[i].ds_type == DATA_TEXT)
        {
            t_buffer *z = buffer_new();
            int first = *p_nextmsg, last;
            for (last = first; last < natoms && vec[last].a_type != A_SEMICOLON;
                last++);
            buffer_deserialize(z, last-first, vec+first);
            buffer_append(w[i].w_buffer, buffer_size(z), buffer_atoms(z));
            buffer_free(z);
            last++;
            if (last > natoms) last = natoms;
            *p_nextmsg = last;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error canvas_deserializeScalar (t_glist *glist, int argc, t_atom *argv)
{
    if (argc > 0 && IS_SYMBOL (argv)) {
    //
    t_symbol *templateIdentifier = utils_makeBindSymbol (GET_SYMBOL (argv));
        
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *scalar = scalar_new (glist, templateIdentifier);
    
    PD_ASSERT (scalar);
    
    if (scalar) {
    //
    int next = 1;       /* Start after the template name. */
    int i;
    int n;
    
    canvas_addObject (glist, cast_gobj (scalar));
    
    n = canvas_fetchNext (argc, argv, &i, &next);
    
    canvas_readElements (glist, argc, argv, &next, templateIdentifier, scalar->sc_vector, n, argv + i);

    if (canvas_isMapped (glist)) { gobj_visibilityChanged (cast_gobj (scalar), glist, 1); }
    //
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
