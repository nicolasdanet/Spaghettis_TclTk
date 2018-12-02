
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define RECENTFILES_MAXIMUM             10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_pathlist *recentfiles_list;    /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void recentfiles_removeOldPaths (void)
{
    int n = pathlist_getSize (recentfiles_list) - RECENTFILES_MAXIMUM;
    
    while (n > 0) { n--; recentfiles_list = pathlist_removeFirst (recentfiles_list); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void recentfiles_appendPath (const char *filepath)
{
    recentfiles_list = pathlist_newAppend (recentfiles_list, NULL, filepath);
}

void recentfiles_clear (void)
{
    pathlist_free (recentfiles_list); recentfiles_list = NULL;
}

void recentfiles_update (void)
{
    recentfiles_removeOldPaths();
    
    {
    //
    t_pathlist *l = recentfiles_get();

    gui_add ("set ::var(filesRecent) {}\n");                                    // --
    
    while (l) {
        gui_vAdd ("lappend ::var(filesRecent) {%s}\n", pathlist_getPath (l));   // --
        l = pathlist_getNext (l);
    }
    
    gui_add ("::ui_menu::updateRecent\n");                                      // --
    //
    }
    
    preferences_save();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void recentfiles_add (t_symbol *name, t_symbol *directory, int check)
{
    char filepath[PD_STRING] = { 0 };

    if (!path_withDirectoryAndName (filepath, PD_STRING, directory->s_name, name->s_name)) {
    //
    if (!check || path_isFileExistAsRegularFile (filepath)) {
    //
    recentfiles_appendPath (filepath); recentfiles_update();
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist *recentfiles_get (void)
{
    return recentfiles_list;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void recentfiles_release (void)
{
    pathlist_free (recentfiles_list);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
