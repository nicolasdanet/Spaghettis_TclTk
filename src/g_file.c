
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_serialize (t_glist *glist, t_buffer *b)
{
    t_gobj *y = NULL;
    t_outconnect *connection = NULL;
    t_linetraverser t;
    
    if (canvas_isSubpatch (glist)) {
    
        t_buffer *z = buffer_new();
        t_symbol *s = NULL;
        buffer_serialize (z, cast_object (glist)->te_buffer);
        s = atom_getSymbolAtIndex (1, buffer_size (z), buffer_atoms (z));
        buffer_free (z);
        
        buffer_vAppend (b, "ssiiiisi;", 
            sym___hash__N, 
            sym_canvas,
            glist->gl_windowTopLeftX,
            glist->gl_windowTopLeftY,
            glist->gl_windowBottomRightX - glist->gl_windowTopLeftX,
            glist->gl_windowBottomRightY - glist->gl_windowTopLeftY,
            (s != &s_ ? s : sym_Patch),
            glist->gl_isMapped);
            
    } else {
    
        buffer_vAppend (b, "ssiiiii;", 
            sym___hash__N,
            sym_canvas,
            glist->gl_windowTopLeftX,
            glist->gl_windowTopLeftY,
            glist->gl_windowBottomRightX - glist->gl_windowTopLeftX,
            glist->gl_windowBottomRightY - glist->gl_windowTopLeftY,
            (int)glist->gl_fontSize);
    }
    
    for (y = glist->gl_graphics; y; y = y->g_next) { gobj_save (y, b); }

    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
    
        buffer_vAppend (b, "ssiiii;", 
            sym___hash__X,
            sym_connect,
            canvas_getIndexOfObject (glist, cast_gobj (t.tr_srcObject)), 
            t.tr_srcIndexOfOutlet, 
            canvas_getIndexOfObject (glist, cast_gobj (t.tr_destObject)), 
            t.tr_destIndexOfInlet);
    }
    
    {
        buffer_vAppend (b, "ssfffffffff;", 
            sym___hash__X, 
            sym_coords,
            glist->gl_valueLeft,
            glist->gl_valueTop,
            glist->gl_valueRight,
            glist->gl_valueBottom,
            (double)glist->gl_graphWidth, 
            (double)glist->gl_graphHeight,
            (double)((glist->gl_hideText ? 2 : 0) | (glist->gl_isGraphOnParent ? 1 : 0)),
            (double)glist->gl_graphMarginLeft,
            (double)glist->gl_graphMarginTop);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_save (t_glist *glist, float destroy)
{
    t_glist *root = canvas_getRoot (glist);
    
    if (root->gl_name == &s_) { canvas_saveAs (root, destroy); }
    else {
    //
    canvas_saveToFile (root,
        root->gl_name,
        environment_getDirectory (canvas_getEnvironment (root)),
        destroy);
    //
    }
}

void canvas_saveAs (t_glist *glist, float destroy)
{
    t_glist *root = canvas_getRoot (glist);
    
    sys_vGui ("::ui_file::saveAs .x%lx {%s} {%s} %d\n",     // --
                    root,
                    root->gl_name->s_name,
                    environment_getDirectoryAsString (canvas_getEnvironment (root)), 
                    (int)destroy);
}

void canvas_saveToFile (t_glist *glist, t_symbol *name, t_symbol *directory, float destroy)
{
    t_buffer *b = buffer_new();
    
    canvas_serializeTemplates (glist, b);
    canvas_serialize (glist, b);
    
    if (buffer_write (b, name->s_name, directory->s_name)) { PD_BUG; }
    else {
        post (PD_TRANSLATE ("file: saved to %s/%s"), directory->s_name, name->s_name);  // --
        canvas_dirty (glist, 0);
        if (destroy != 0.0) {
            pd_vMessage (cast_pd (glist), sym_close, "f", (destroy == 2.0 ? 3.0 : 1.0)); 
        }
    }
    
    buffer_free (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
