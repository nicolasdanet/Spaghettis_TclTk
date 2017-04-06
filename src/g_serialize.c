
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void glist_findTemplatesAppendProceed (t_symbol *templateIdentifier, int *n, t_symbol ***v)
{
    int t = *n;
    t_symbol **templates = *v;
    int alreadyExist = 0;
    int i;
    
    for (i = 0; i < t; i++) { if (templates[i] == templateIdentifier) { alreadyExist = 1; break; } }

    if (!alreadyExist) {
    //
    int oldSize = (int)(sizeof (t_symbol *) * (t));
    int newSize = (int)(sizeof (t_symbol *) * (t + 1));
        
    templates    = (t_symbol **)PD_MEMORY_RESIZE (templates, oldSize, newSize);
    templates[t] = templateIdentifier;
    
    *v = templates;
    *n = t + 1;
    //
    }
}

static void glist_findTemplatesAppendRecursive (t_template *tmpl, int *n, t_symbol ***v)
{
    int i;

    glist_findTemplatesAppendProceed (template_getTemplateIdentifier (tmpl), n, v);

    for (i = 0; i < template_getSize (tmpl); i++) {
        t_template *t = template_getTemplateIfArrayAtIndex (tmpl, i);
        if (t) {
            glist_findTemplatesAppendRecursive (t, n, v);
        }
    }
}

static void glist_findTemplatesRecursive (t_glist *glist, int *n, t_symbol ***v)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == scalar_class) {
            glist_findTemplatesAppendRecursive (scalar_getTemplate (cast_scalar (y)), n, v);
        }
        if (pd_class (y) == canvas_class) { 
            glist_findTemplatesRecursive (cast_glist (y), n, v);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void glist_serializeTemplates (t_glist *glist, t_buffer *b)
{
    t_symbol **v = PD_MEMORY_GET (0);
    int i, n = 0;
    
    glist_findTemplatesRecursive (glist, &n, &v);
    
    for (i = 0; i < n; i++) { template_serialize (template_findByIdentifier (v[i]), b); }
    
    PD_MEMORY_FREE (v);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void glist_serializeHeader (t_glist *glist, t_buffer *b)
{
    if (glist_isSubpatch (glist)) {
    
        /* Note that the name of a subpatch could be an A_DOLLARSYMBOL type. */
        
        t_buffer *z = buffer_new();
        t_symbol *s = NULL;
        buffer_serialize (z, object_getBuffer (cast_object (glist)));
        s = atom_getSymbolAtIndex (1, buffer_size (z), buffer_atoms (z));   /* Fetch unexpanded name. */
        buffer_free (z);
        
        buffer_vAppend (b, "ssiiiisi;", 
            sym___hash__N, 
            sym_canvas,
            rectangle_getTopLeftX (glist_getWindowGeometry (glist)),
            rectangle_getTopLeftY (glist_getWindowGeometry (glist)),
            rectangle_getWidth (glist_getWindowGeometry (glist)),
            rectangle_getHeight (glist_getWindowGeometry (glist)),
            (s != &s_ ? s : sym_Patch),
            glist_getMapped (glist));
            
    } else {
    
        glist_serializeTemplates (glist, b);
    
        buffer_vAppend (b, "ssiiiii;", 
            sym___hash__N,
            sym_canvas,
            rectangle_getTopLeftX (glist_getWindowGeometry (glist)),
            rectangle_getTopLeftY (glist_getWindowGeometry (glist)),
            rectangle_getWidth (glist_getWindowGeometry (glist)),
            rectangle_getHeight (glist_getWindowGeometry (glist)),
            (int)glist_getFontSize (glist));
    }
}

static void glist_serializeObjects (t_glist *glist, t_buffer *b)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { gobj_save (y, b); }
}

static void glist_serializeLines (t_glist *glist, t_buffer *b)
{
    t_outconnect *connection = NULL;
    t_traverser t;
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X,
            sym_connect,
            glist_objectGetIndexOf (glist, cast_gobj (traverser_getSource (&t))), 
            traverser_getIndexOfOutlet (&t), 
            glist_objectGetIndexOf (glist, cast_gobj (traverser_getDestination (&t))), 
            traverser_getIndexOfInlet (&t));
    }
}

/* For compatibility with legacy, top left coordinates must be serialized last. */

static void glist_serializeGraph (t_glist *glist, t_buffer *b)
{
    buffer_vAppend (b, "ssfffffffff;", 
            sym___hash__X, 
            sym_coords,
            bounds_getLeft (glist_getBounds (glist)),
            bounds_getTop (glist_getBounds (glist)),
            bounds_getRight (glist_getBounds (glist)),
            bounds_getBottom (glist_getBounds (glist)),
            (double)(rectangle_getWidth (glist_getGraphGeometry (glist))), 
            (double)(rectangle_getHeight (glist_getGraphGeometry (glist))),
            (double)(glist_isGraphOnParent (glist) ? 1 : 0),
            (double)(rectangle_getTopLeftX (glist_getGraphGeometry (glist))),
            (double)(rectangle_getTopLeftY (glist_getGraphGeometry (glist))));
}

static void glist_serializeFooter (t_glist *glist, t_buffer *b)
{
    if (glist_isSubpatch (glist)) {
    
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_restore,
            object_getX (cast_object (glist)),
            object_getY (cast_object (glist)));
        
        buffer_serialize (b, object_getBuffer (cast_object (glist)));
        buffer_appendSemicolon (b);
        object_saveWidth (cast_object (glist), b);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_serialize (t_glist *glist, t_buffer *b)
{
    glist_serializeHeader (glist, b);
    glist_serializeObjects (glist, b);
    glist_serializeLines (glist, b);
    glist_serializeGraph (glist, b);
    glist_serializeFooter (glist, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error glist_deserializeScalar (t_glist *glist, int argc, t_atom *argv)
{
    if (argc > 0 && IS_SYMBOL (argv)) {
    //
    t_symbol *templateIdentifier = utils_makeTemplateIdentifier (GET_SYMBOL (argv));
        
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *scalar = scalar_new (glist, templateIdentifier);
    
    PD_ASSERT (scalar);
    
    if (scalar) {
    
        glist_objectAdd (glist, cast_gobj (scalar));
        scalar_deserialize (scalar, glist, argc - 1, argv + 1);
        
        if (glist_isOnScreen (glist)) {
            gobj_visibilityChanged (cast_gobj (scalar), glist, 1);
        }
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
