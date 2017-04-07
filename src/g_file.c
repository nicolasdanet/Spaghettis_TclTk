
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

void interface_quit (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Messy ping-pong required in order to check saving sequentially. */

/* Messy ping-pong required in order to check saving sequentially. */
/* Furthermore it avoids the application to quit before responding. */
/* Note that patches not dirty are closed later. */

void canvas_quit (void)
{
    t_glist *glist = NULL;
    
    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) {
    //
    if (glist_isDirty (glist)) {
    //
    sys_vGui ("::ui_confirm::checkClose %s"
                    " { ::ui_interface::pdsend $top save 2 }"
                    " { ::ui_interface::pdsend $top close 2 }"
                    " {}\n",    // --
                    glist_getTagAsString (glist));
    return;
    //
    }
    //
    }
    
    interface_quit();
}

void canvas_close (t_glist *glist, t_float f)
{
    int k = (int)f;
    
    if (k == 2) { glist_setDirty (glist, 0); canvas_quit(); }  /* While quitting application. */
    else {
    //
    if (glist_hasParent (glist)) { canvas_visible (glist, 0); }     /* Hide subpatches and abstractions. */
    else {
    //
    if (k == 1 || k == 3) {                                         /* Has been saved right before. */

        pd_free (cast_pd (glist)); if (k == 3) { canvas_quit(); }  
        
    } else {
        if (glist_isDirty (glist)) {
            
            sys_vGui ("::ui_confirm::checkClose .x%lx"
                            " { ::ui_interface::pdsend $top save 1 }"
                            " { ::ui_interface::pdsend $top close 1 }"
                            " {}\n",    // --
                            glist);
            return;
            
        } else {
            pd_free (cast_pd (glist));
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_saveToFile (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc >= 2) {
    //
    t_symbol *name      = atom_getSymbol (argv + 0);
    t_symbol *directory = atom_getSymbol (argv + 1);
    
    int destroy = (int)atom_getFloatAtIndex (2, argc, argv);
    
    t_buffer *b = buffer_new();
    
    glist_serialize (glist, b);
    
    if (!buffer_write (b, name, directory)) {
        post (PD_TRANSLATE ("file: saved to %s/%s"), directory->s_name, name->s_name);  // --
        glist_setDirty (glist, 0);
        if (destroy) { canvas_close (glist, (t_float)(destroy == 2 ? 3 : 1)); }
    }
    
    buffer_free (b);
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
