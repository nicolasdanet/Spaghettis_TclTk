
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

void canvas_close (t_glist *, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* While quitting application a messy ping-pong is required. */
/* The dirty state of patch is checked sequentially. */
/* It avoids the executable to quit before user responses. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

enum {
    DESTROY  = 1,
    QUITTING = 2,
    CONTINUE = 3
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_saveProceed (t_glist *glist, t_symbol *name, t_symbol *directory, int destroy)
{
    t_buffer *b = buffer_new();
    
    glist_serialize (glist, b);
    
    if (buffer_write (b, name, directory)) { error_failsToWrite (name); }
    else {
        post (PD_TRANSLATE ("file: saved to %s/%s"), directory->s_name, name->s_name);  // --
        glist_setDirty (glist, 0);
        if (destroy) {
            canvas_close (glist, (t_float)(destroy == QUITTING ? CONTINUE : DESTROY)); 
        }
    }
    
    buffer_free (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_saveToFile (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 2) {
    
        t_symbol *name      = atom_getSymbol (argv + 0);
        t_symbol *directory = atom_getSymbol (argv + 1);
        
        canvas_saveProceed (glist, name, directory, (int)atom_getFloat (argv + 2));
    }
}

void canvas_saveAs (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getTop (glist);
    
    sys_vGui ("::ui_file::saveAs %s {%s} {%s} %d\n",     // --
                    glist_getTagAsString (root),
                    glist_getName (root)->s_name,
                    environment_getDirectoryAsString (glist_getEnvironment (root)), 
                    (int)destroy);
}

void canvas_save (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getTop (glist);
    
    if (glist_getName (root) == &s_) { canvas_saveAs (root, destroy); }
    else {
    
        t_symbol *name      = glist_getName (root);
        t_symbol *directory = environment_getDirectory (glist_getEnvironment (root));
        
        canvas_saveProceed (root, name, directory, destroy);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_quit (void)
{
    t_glist *glist = NULL;
    
    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) {
    //
    if (glist_isDirty (glist)) {
    //
    sys_vGui ("::ui_confirm::checkClose %s"
                    " { ::ui_interface::pdsend $top save %d  }"
                    " { ::ui_interface::pdsend $top close %d }"
                    " {}\n",    // --
                    glist_getTagAsString (glist), 
                    QUITTING, 
                    QUITTING);
    return;
    //
    }
    //
    }
    
    interface_quit();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_closeUnsetDirtyAndContinue (t_glist *glist)
{
    glist_setDirty (glist, 0); canvas_quit();   /* Note that patches not dirty are closed later. */
}

void canvas_closeHideSubpatchOrAbstraction (t_glist *glist)
{
    glist_visible (glist, 0);
}

void canvas_closeDestroyAlreadyChecked (t_glist *glist, int destroy)
{
    pd_free (cast_pd (glist)); if (destroy == CONTINUE) { canvas_quit(); } 
}

void canvas_closeDestroyOrCheckIfNecessary (t_glist *glist)
{
    if (glist_isDirty (glist)) {
            
        sys_vGui ("::ui_confirm::checkClose %s"
                        " { ::ui_interface::pdsend $top save %d  }"
                        " { ::ui_interface::pdsend $top close %d }"
                        " {}\n",    // --
                        glist_getTagAsString (glist), 
                        DESTROY,
                        DESTROY);

    } else {
        pd_free (cast_pd (glist));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_close (t_glist *glist, t_float f)
{
    int destroy = (int)f;
    
    if (glist_hasParent (glist))  { canvas_closeHideSubpatchOrAbstraction (glist); }
    else if (destroy == QUITTING) { canvas_closeUnsetDirtyAndContinue (glist); }
    else if (destroy)             { canvas_closeDestroyAlreadyChecked (glist, destroy); }
    else {
        canvas_closeDestroyOrCheckIfNecessary (glist);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
