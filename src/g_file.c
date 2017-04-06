
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

void canvas_save (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getTop (glist);
    
    if (glist_getName (root) == &s_) { canvas_saveAs (root, destroy); }
    else {
    //
    t_atom t[3];
    
    SET_SYMBOL (t + 0, glist_getName (root));
    SET_SYMBOL (t + 1, environment_getDirectory (glist_getEnvironment (root)));
    SET_FLOAT  (t + 2, destroy);
    
    canvas_saveToFile (root, NULL, 3, t);
    //
    }
}

void canvas_saveAs (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getTop (glist);
    
    sys_vGui ("::ui_file::saveAs .x%lx {%s} {%s} %d\n",     // --
                    root,
                    glist_getName (root)->s_name,
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
    
    glist_serialize (glist, b);
    
    if (buffer_write (b, name, directory)) { PD_BUG; }
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
