
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

t_error canvas_deserializeScalar (t_glist *glist, int argc, t_atom *argv)
{
    if (argc > 0 && IS_SYMBOL (argv)) {
    //
    t_symbol *templateIdentifier = utils_makeTemplateIdentifier (GET_SYMBOL (argv));
        
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *scalar = scalar_new (glist, templateIdentifier);
    
    PD_ASSERT (scalar);
    
    if (scalar) {
        canvas_addObject (glist, cast_gobj (scalar));
        scalar_deserialize (scalar, glist, argc - 1, argv + 1);
        if (canvas_isMapped (glist)) { gobj_visibilityChanged (cast_gobj (scalar), glist, 1); }
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
