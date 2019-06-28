
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryTemplates;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define STARTUP_LEVELS  10
#define STARTUP_FDOPEN  15

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_pathlist *startup_templates;       /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if ! ( PD_WINDOWS )

static int startup_fetchAllTemplatesRecursive (const char *path,
    const struct stat *b,
    int flag,
    struct FTW *f)
{
    int abort = 0;
    
    if (flag == FTW_D) { abort = (f->level > STARTUP_LEVELS); }
    if (flag == FTW_F) {
    //
    if (string_endWith (path, PD_TEMPLATE)) {
        startup_templates = pathlist_newAppend (startup_templates, NULL, path);
    }
    //
    }
    
    if (abort) { return 1; } else { return scheduler_isExiting(); }
}

static t_pathlist *startup_fetchAllTemplates (const char *path)
{
    startup_templates = NULL;
    
    if (nftw (path, startup_fetchAllTemplatesRecursive, STARTUP_FDOPEN, FTW_MOUNT | FTW_PHYS) == 0) {
        return startup_templates;
    }
    
    PD_BUG; return NULL;
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void startup_openFilesTemplatesProceed (t_pathlist *l)
{
    t_symbol *filename  = NULL;
    t_symbol *directory = NULL;
    
    while (l) {
    //
    t_error err = path_toDirectoryAndNameAsSymbol (pathlist_getPath (l), &directory, &filename);
    
    if (!err && instance_patchOpen (filename, directory, 0) == PD_ERROR_NONE) {
        post_log ("Open %s", pathlist_getPath (l));
    }
    
    l = pathlist_getNext (l);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void startup_openFilesTemplates (void)
{
    t_pathlist *l = startup_fetchAllTemplates (main_directoryTemplates->s_name);
    
    startup_openFilesTemplatesProceed (l);
    
    pathlist_free (l);
}

void startup_openFilesPended (void)
{
    #if PD_APPLE
    
        gui_vAdd ("::openPendedFiles\n");
    
    #endif // PD_APPLE
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
