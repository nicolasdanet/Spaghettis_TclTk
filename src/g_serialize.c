
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void glist_serializeHeader (t_glist *glist, t_buffer *b)
{
    buffer_appendSymbol (b, sym___hash__N);
    buffer_appendSymbol (b, sym_canvas);
    buffer_appendFloat (b,  rectangle_getTopLeftX (glist_getWindowGeometry (glist)));
    buffer_appendFloat (b,  rectangle_getTopLeftY (glist_getWindowGeometry (glist)));
    buffer_appendFloat (b,  rectangle_getWidth (glist_getWindowGeometry (glist)));
    buffer_appendFloat (b,  rectangle_getHeight (glist_getWindowGeometry (glist)));
    
    if (!glist_isSubpatch (glist)) { buffer_appendFloat (b,  glist_getFontSize (glist)); }
    else {
    //
    t_symbol *s = &s_;
    
    /* Note that the name of a subpatch or an array could be expanded. */
    /* Code below is required to fetch the unexpanded form. */
    
    if (glist_isArray (glist)) { s = garray_getUnexpandedName (glist_getArray (glist)); }
    else {
        t_buffer *z = buffer_new();
        buffer_serialize (z, object_getBuffer (cast_object (glist)));
        s = atom_getSymbolAtIndex (1, buffer_getSize (z), buffer_getAtoms (z));
        buffer_free (z);
    }
    
    buffer_appendSymbol (b, (s != &s_ ? s : sym_Patch));
    buffer_appendFloat (b,  glist_getMapped (glist));
    //
    }
    
    buffer_appendSemicolon (b);
}

static void glist_serializeObjects (t_glist *glist, t_buffer *b)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) { if (!gobj_isScalar (y)) { gobj_save (y, b); } }
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
        
        if (!glist_isArray (glist)) { buffer_serialize (b, object_getBuffer (cast_object (glist))); }
        else {
            buffer_appendSymbol (b, sym_graph);
            buffer_appendSymbol (b, garray_getUnexpandedName (glist_getArray (glist)));
        }
        
        buffer_appendSemicolon (b);
        object_serializeWidth (cast_object (glist), b);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
