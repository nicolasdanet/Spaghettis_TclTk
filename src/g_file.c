
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

void interface_quit (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_close (t_glist *, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* While quitting application a messy ping-pong is required. */
/* The dirty state of patch is checked sequentially. */
/* It avoids the executable to quit before user responses. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    DESTROY  = 1,
    QUITTING = 2,
    CONTINUE = 3
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_saveProceed (t_glist *glist, t_symbol *name, t_symbol *directory, int destroy)
{
    t_buffer *b = buffer_new();
    
    glist_serialize (glist, b);
    
    if (buffer_fileWrite (b, name, directory)) { error_failsToWrite (name); }
    else {
        post (PD_TRANSLATE ("file: saved to %s/%s"), directory->s_name, name->s_name);  // --
        environment_setDirectory (glist_getEnvironment (glist), directory);
        glist_setDirty (glist, 0);
        if (destroy) {
            canvas_close (glist, (t_float)(destroy == QUITTING ? CONTINUE : DESTROY)); 
        }
    }
    
    buffer_free (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_saveToFile (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 2) {
    //
    t_glist *root       = glist_getTop (glist);
    t_symbol *fileName  = atom_getSymbol (argv + 0);
    t_symbol *directory = atom_getSymbol (argv + 1);
    
    glist_cancelEditingBox (glist);
    
    if (glist_isFrozen (glist)) { error_fileIsProtected (glist_getName (root)); }
    else {
    //
    if (fileName != glist_getName (root)) {
        glist_rename (root, 1, argv);
    }
    
    canvas_saveProceed (root, fileName, directory, (int)atom_getFloat (argv + 2));
    //
    }
    //
    }
}

void canvas_saveAs (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getTop (glist);
    
    gui_vAdd ("::ui_file::saveAs %s {%s} {%s} %d\n",     // --
                    glist_getTagAsString (root),
                    environment_getFileNameAsString (glist_getEnvironment (root)),
                    environment_getDirectoryAsString (glist_getEnvironment (root)), 
                    (int)destroy);
}

void canvas_save (t_glist *glist, t_float destroy)
{
    t_glist *root = glist_getTop (glist);
    t_symbol *fileName = environment_getFileName (glist_getEnvironment (root));
    
    if (glist_isFrozen (glist)) { error_fileIsProtected (fileName); }
    else {
    //
    if (fileName == &s_ || string_startWith (fileName->s_name, "Untitled")) { canvas_saveAs (root, destroy); }
    else {
        canvas_saveProceed (root, fileName, environment_getDirectory (glist_getEnvironment (root)), destroy);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_quit (void)
{
    t_glist *glist = NULL;
    
    for (glist = instance_getRoots(); glist; glist = glist_getNext (glist)) {
    //
    if (glist_isDirty (glist) && !glist_isFrozen (glist)) {
    //
    gui_vAdd ("::ui_confirm::checkClose %s"
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
// MARK: -

void canvas_closeUnsetDirtyAndContinue (t_glist *glist)
{
    glist_setDirty (glist, 0); canvas_quit();   /* Note that patches not dirty are closed later. */
}

void canvas_closeSubpatchOrAbstraction (t_glist *glist)
{
    if (glist_hasWindow (glist)) { glist_windowClose (glist); }
}

void canvas_closeDestroyAlreadyChecked (t_glist *glist, int destroy)
{
    pd_free (cast_pd (glist)); if (destroy == CONTINUE) { canvas_quit(); } 
}

void canvas_closeDestroyOrCheckIfNecessary (t_glist *glist)
{
    if (glist_isDirty (glist) && !glist_isFrozen (glist)) {
            
        gui_vAdd ("::ui_confirm::checkClose %s"
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
// MARK: -

void canvas_close (t_glist *glist, t_float f)
{
    int destroy = (int)f;
    
    if (glist_hasParent (glist))  { canvas_closeSubpatchOrAbstraction (glist); }
    else if (destroy == QUITTING) { canvas_closeUnsetDirtyAndContinue (glist); }
    else if (destroy)             { canvas_closeDestroyAlreadyChecked (glist, destroy); }
    else {
        canvas_closeDestroyOrCheckIfNecessary (glist);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
