
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://duartes.org/gustavo/blog/post/anatomy-of-a-program-in-memory/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_ctor) (t_symbol *s);
typedef void (*t_dtor) (void);

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
    struct _loadedlist  *ll_next;
    t_symbol            *ll_name;
    t_handle            ll_handle;
    } t_loadedlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_loadedlist     *loader_alreadyLoaded;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void loader_closeExternal (t_handle handle, t_symbol *name);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int loader_isAlreadyLoaded (t_symbol *name)
{
    t_loadedlist *l = NULL;
    
    for (l = loader_alreadyLoaded; l; l = l->ll_next) { if (l->ll_name == name) { return 1; } }
    
    return 0;
}

static void loader_addLoaded (t_symbol *name, t_handle handle)
{
    t_loadedlist *l = (t_loadedlist *)PD_MEMORY_GET (sizeof (t_loadedlist));
    
    l->ll_next   = loader_alreadyLoaded;
    l->ll_name   = name;
    l->ll_handle = handle;
    
    loader_alreadyLoaded = l;
}

static t_error loader_makeStubName (char *dest, size_t size, t_symbol *name, const char *suffix)
{
    t_error err = PD_ERROR_NONE;
    char *n = name->s_name;

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
    
    err |= string_add (dest, size, suffix);
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static t_handle loader_openExternalNative (char *filepath, char *stub, t_symbol *root)
{
    t_handle handle;
    
    path_slashToBackslashIfNecessary (filepath);

    handle = LoadLibrary (filepath);
    
    if (!handle) { error__error2 (filepath, dlerror()); }
    else {
        t_ctor ctor = (t_ctor)GetProcAddress (handle, stub);
    
        if (!ctor) { error_stubNotFound(); }
        else {
            (*ctor) (root); return handle;
        }
    }
    
    if (handle) { loader_closeExternal (handle, NULL); }
    
    return NULL;
}

#else

static t_handle loader_openExternalNative (char *filepath, char *stub, t_symbol *root)
{
    t_handle handle = dlopen (filepath, RTLD_NOW | RTLD_GLOBAL);
    
    if (!handle) { error__error2 (filepath, dlerror()); }
    else {
        t_ctor ctor = (t_ctor)dlsym (handle, stub);
        
        if (!ctor) { error_stubNotFound(); }
        else {
            (*ctor) (root); return handle;
        }
    }
    
    if (handle) { loader_closeExternal (handle, NULL); }
    
    return NULL;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS

static void loader_closeExternalNative (t_handle handle, char *stub)
{
    t_dtor dtor = (t_dtor)GetProcAddress (handle, stub);
    
    if (dtor) { (*dtor)(); }
}

#else

static void loader_closeExternalNative (t_handle handle, char *stub)
{
    t_dtor dtor = (t_dtor)dlsym (handle, stub);
    
    if (dtor) { (*dtor)(); }
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int loader_openExternal (t_glist *glist, t_symbol *name)
{
    t_handle handle = NULL;
    
    PD_ASSERT (strrchr (name->s_name, '/') == NULL); 
    
    if (loader_isAlreadyLoaded (name)) { return 1; }
    else {
    //
    char directoryResult[PD_STRING] = { 0 }; 
    char *nameResult = NULL;

    if (glist_fileFind (glist, name->s_name, PD_PLUGIN, directoryResult, &nameResult, PD_STRING)) {
    //
    char filepath[PD_STRING] = { 0 };
    
    class_setCurrentExternalDirectory (gensym (directoryResult));
    
    if (!path_withDirectoryAndName (filepath, PD_STRING, directoryResult, nameResult, 0)) {
        char stub[PD_STRING] = { 0 };
        t_error err = loader_makeStubName (stub, PD_STRING, name, "_setup");
        if (!err && (handle = loader_openExternalNative (filepath, stub, gensym (filepath)))) {
            loader_addLoaded (name, handle);
        }
    }
    
    class_setCurrentExternalDirectory (&s_);
    //
    }
    //
    }
    
    return (handle != NULL);
}

static void loader_closeExternal (t_handle handle, t_symbol *name)
{
    if (name) {
    //
    char stub[PD_STRING] = { 0 };
            
    if (!(loader_makeStubName (stub, PD_STRING, name, "_destroy"))) { 
        loader_closeExternalNative (handle, stub); 
    }
    //
    }
    
    #if PD_WINDOWS
        FreeLibrary (handle);
    #else
        dlclose (handle);
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int loader_load (t_glist *glist, t_symbol *name)
{
    int state = dsp_suspend();
    int done  = loader_openExternal (glist, name);
    dsp_resume (state);
    
    return done;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void loader_release (void)
{
    t_loadedlist *l = loader_alreadyLoaded;

    while (l) {
    //
    t_loadedlist *next = l->ll_next;
    loader_closeExternal (l->ll_handle, l->ll_name);
    PD_MEMORY_FREE (l);
    l = next; 
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
