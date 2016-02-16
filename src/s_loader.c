
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://duartes.org/gustavo/blog/post/anatomy-of-a-program-in-memory/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef t_error (*t_stub) (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS
    typedef HMODULE t_handle;
#else
    typedef void *t_handle;
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _loadedlist {
    struct _loadlist    *ll_next;
    t_symbol            *ll_name;
    t_handle            ll_handle;
    } t_loadedlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_loadedlist     *loader_alreadyLoaded;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int loader_isAlreadyLoaded (char *o)
{
    t_loadedlist *l = NULL;
    t_symbol *s = gensym (o);
    
    for (l = loader_alreadyLoaded; l; l = l->ll_next) { if (l->ll_name == s) { return 1; } }
    
    return 0;
}

static void loader_addLoaded (char *o, t_handle handle)
{
    t_loadedlist *l = (t_loadedlist *)PD_MEMORY_GET (sizeof (t_loadedlist));
    
    l->ll_next   = loader_alreadyLoaded;
    l->ll_name   = gensym (o);
    l->ll_handle = handle;
    
    loader_alreadyLoaded = l;
}

static void loader_releaseLoaded (void)
{
    t_loadedlist *l = loader_alreadyLoaded;

    while (l) { t_loadedlist *next = l->ll_next; PD_MEMORY_FREE (l); l = next; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void loader_initialize (void)
{
}

void loader_release (void)
{
    loader_releaseLoaded();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static t_handle loader_openExternalNative (char *filepath, t_symbol *root)
{
    t_handle handle;
    
    path_slashToBackslashIfNecessary (filepath, filepath);

    handle = LoadLibrary (filepath);
    
    if (!handle) { post_error (PD_TRANSLATE ("loader: invalid '%s'"), filepath); }      // --
    else {
    //
    t_stub ctor = (t_stub)GetProcAddress (handle, "initialize");
    
    if (!ctor)   { post_error (PD_TRANSLATE ("loader: stub not found")); }              // --
    else {
        if ((*ctor) (root) == PD_ERROR_NONE) { return handle; }
        else {
            return NULL;
        }
    }
    //
    }
}

#else

static t_handle loader_openExternalNative (char *filepath, t_symbol *root)
{
    t_handle handle = dlopen (filepath, RTLD_NOW | RTLD_GLOBAL);
    
    if (!handle) { post_error (PD_TRANSLATE ("loader: invalid '%s'"), filepath); }      // --
    else {
    //
    t_stub ctor = (t_stub)dlsym (handle, "initialize");
    
    if (!ctor)   { post_error (PD_TRANSLATE ("loader: stub not found")); }              // --
    else {
        if ((*ctor) (root) == PD_ERROR_NONE) { return handle; }
        else {
            return NULL;
        }
    }
    //
    }
    
    return NULL;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int loader_openExternal (t_canvas *canvas, char *name)
{
    t_handle handle = NULL;
    
    PD_ASSERT (strrchr (name, '/') == NULL); 
    
    if (loader_isAlreadyLoaded (name)) { return 1; }
    else {
    //
    char *nameResult = NULL;
    char directoryResult[PD_STRING] = { 0 };

    int f = canvas_open (canvas, name, PD_PLUGIN, directoryResult, &nameResult, PD_STRING, 1);
    
    if (f >= 0) {
        char filepath[PD_STRING] = { 0 };
        t_symbol *root = gensym (directoryResult);
        close (f);
        class_setDefaultExternalDirectory (root);
        if (!path_withDirectoryAndName (filepath, PD_STRING, directoryResult, nameResult, 0)) {
            if (handle = loader_openExternalNative (filepath, root)) {
                loader_addLoaded (name, handle);
            }
        }
        class_setDefaultExternalDirectory (&s_);
    }
    //
    }
    
    return (handle != NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int loader_loadExternal (t_canvas *canvas, char *name)
{
    int k = canvas_suspend_dsp();
    int done = loader_openExternal (canvas, name);
    canvas_resume_dsp (k);
    
    return done;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
