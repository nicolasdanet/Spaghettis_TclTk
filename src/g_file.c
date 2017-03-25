
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

void canvas_serialize (t_glist *glist, t_buffer *b)
{
    t_gobj *y = NULL;
    t_outconnect *connection = NULL;
    t_traverser t;
    
    if (glist_isSubpatch (glist)) {
    
        t_buffer *z = buffer_new();
        t_symbol *s = NULL;
        buffer_serialize (z, object_getBuffer (cast_object (glist)));
        s = atom_getSymbolAtIndex (1, buffer_size (z), buffer_atoms (z));
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
    
        buffer_vAppend (b, "ssiiiii;", 
            sym___hash__N,
            sym_canvas,
            rectangle_getTopLeftX (glist_getWindowGeometry (glist)),
            rectangle_getTopLeftY (glist_getWindowGeometry (glist)),
            rectangle_getWidth (glist_getWindowGeometry (glist)),
            rectangle_getHeight (glist_getWindowGeometry (glist)),
            (int)glist_getFontSize (glist));
    }
    
    for (y = glist->gl_graphics; y; y = y->g_next) { gobj_save (y, b); }

    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X,
            sym_connect,
            canvas_getIndexOfObject (glist, cast_gobj (traverser_getSource (&t))), 
            traverser_getIndexOfOutlet (&t), 
            canvas_getIndexOfObject (glist, cast_gobj (traverser_getDestination (&t))), 
            traverser_getIndexOfInlet (&t));
    }
    
    {
        buffer_vAppend (b, "ssfffffffff;", 
            sym___hash__X, 
            sym_coords,
            bounds_getLeft (glist_getBounds (glist)),
            bounds_getTop (glist_getBounds (glist)),
            bounds_getRight (glist_getBounds (glist)),
            bounds_getBottom (glist_getBounds (glist)),
            (double)rectangle_getWidth (glist_getGraphGeometry (glist)), 
            (double)rectangle_getHeight (glist_getGraphGeometry (glist)),
            (double)((glist->gl_hideText ? 2 : 0) | (glist_isGraphOnParent (glist) ? 1 : 0)),
            (double)rectangle_getTopLeftX (glist_getGraphGeometry (glist)),
            (double)rectangle_getTopLeftY (glist_getGraphGeometry (glist)));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_save (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getRoot (glist);
    
    if (root->gl_name == &s_) { canvas_saveAs (root, destroy); }
    else {
    //
    t_atom t[3];
    
    SET_SYMBOL (t + 0, root->gl_name);
    SET_SYMBOL (t + 1, environment_getDirectory (glist_getEnvironment (root)));
    SET_FLOAT  (t + 2, destroy);
    
    canvas_saveToFile (root, NULL, 3, t);
    //
    }
}

void canvas_saveAs (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getRoot (glist);
    
    sys_vGui ("::ui_file::saveAs .x%lx {%s} {%s} %d\n",     // --
                    root,
                    root->gl_name->s_name,
                    environment_getDirectoryAsString (glist_getEnvironment (root)), 
                    (int)destroy);
}

void canvas_saveToFile (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 3) {
    //
    t_symbol *name      = atom_getSymbol (argv + 0);
    t_symbol *directory = atom_getSymbol (argv + 1);
    t_float destroy     = atom_getFloat (argv + 2);
    
    t_buffer *b = buffer_new();
    
    canvas_serializeTemplates (glist, b);
    canvas_serialize (glist, b);
    
    if (buffer_write (b, name->s_name, directory->s_name)) { PD_BUG; }
    else {
        post (PD_TRANSLATE ("file: saved to %s/%s"), directory->s_name, name->s_name);  // --
        glist_setDirty (glist, 0);
        if (destroy != 0.0) {
            t_atom t;
            SET_FLOAT (&t, (t_float)(destroy == 2.0 ? 3.0 : 1.0));
            pd_message (cast_pd (glist), sym_close, 1, &t); 
        }
    }
    
    buffer_free (b);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
