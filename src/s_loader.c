
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://duartes.org/gustavo/blog/post/anatomy-of-a-program-in-memory/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_stub) (t_symbol *s);

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

static void loader_closeExternal (t_handle handle);

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void loader_initialize (void)
{
}

void loader_release (void)
{
    t_loadedlist *l = loader_alreadyLoaded;

    while (l) {
    //
    t_loadedlist *next = l->ll_next;
    loader_closeExternal (l->ll_handle);
    PD_MEMORY_FREE (l);
    l = next; 
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static t_handle loader_openExternalNative (char *filepath, char* stub, t_symbol *root)
{
    t_handle handle;
    
    path_slashToBackslashIfNecessary (filepath, filepath);

    handle = LoadLibrary (filepath);
    
    if (!handle) { post_error (PD_TRANSLATE ("loader: invalid %s %s"), filepath, dlerror()); }
    else {
        t_stub ctor = (t_stub)GetProcAddress (handle, stub);
    
        if (!ctor) { post_error (PD_TRANSLATE ("loader: stub not found")); }
        else {
            (*ctor) (root); return handle;
        }
    }
    
    if (handle) { loader_closeExternal (handle); }
    
    return NULL;
}

#else

static t_handle loader_openExternalNative (char *filepath, char* stub, t_symbol *root)
{
    t_handle handle = dlopen (filepath, RTLD_NOW | RTLD_GLOBAL);
    
    if (!handle) { post_error (PD_TRANSLATE ("loader: invalid %s %s"), filepath, dlerror()); }
    else {
        t_stub ctor = (t_stub)dlsym (handle, stub);
        
        if (!ctor) { post_error (PD_TRANSLATE ("loader: stub not found")); }
        else {
            (*ctor) (root); return handle;
        }
    }
    
    if (handle) { loader_closeExternal (handle); }
    
    return NULL;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error loader_openExternalGetStubName (char *dest, size_t size, char *name)
{
    t_error err = PD_ERROR_NONE;
    char *n = name;

    while (*n && !err) {
    //
    char c = *n;
    if (utils_isAlphanumericOrUnderscore (c)) { err |= string_append (dest, size, n, 1);  } 
    else if ((c == '~') && (*(n + 1) == 0))   { err |= string_add (dest, size, "_tilde"); }
    else {
        err |= string_add (dest, size, "_");
    }
    n++;
    //
    }
    
    err |= string_add (dest, size, "_setup");
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int loader_openExternal (t_glist *glist, char *name)
{
    t_handle handle = NULL;
    
    PD_ASSERT (strrchr (name, '/') == NULL); 
    
    if (loader_isAlreadyLoaded (name)) { return 1; }
    else {
    //
    char *nameResult = NULL;
    char directoryResult[PD_STRING] = { 0 };

    int f = canvas_openFile (glist, name, PD_PLUGIN, directoryResult, &nameResult, PD_STRING);
    
    if (f >= 0) {
        char filepath[PD_STRING] = { 0 };
        close (f);
        class_setDefaultExternalDirectory (gensym (directoryResult));
        if (!path_withDirectoryAndName (filepath, PD_STRING, directoryResult, nameResult, 0)) {
            char stub[PD_STRING] = { 0 };
            t_error err = loader_openExternalGetStubName (stub, PD_STRING, name);
            if (!err && (handle = loader_openExternalNative (filepath, stub, gensym (filepath)))) {
                loader_addLoaded (name, handle);
            }
        }
        class_setDefaultExternalDirectory (&s_);
    }
    //
    }
    
    return (handle != NULL);
}

static void loader_closeExternal (t_handle handle)
{
    #if PD_WINDOWS
        FreeLibrary (handle);
    #else
        dlclose (handle);
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int loader_loadExternal (t_glist *glist, char *name)
{
    int state = dsp_suspend();
    int done  = loader_openExternal (glist, name);
    dsp_resume (state);
    
    return done;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
