
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
#pragma mark -

typedef void (*t_xxx)(void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _loadedlist {
    struct _loadlist    *ll_next;
    t_symbol            *ll_name;
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

static void loader_addLoaded (char *o)
{
    t_loadedlist *l = (t_loadedlist *)PD_MEMORY_GET (sizeof (t_loadedlist));
    
    l->ll_name = gensym (o);
    l->ll_next = loader_alreadyLoaded;
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

    while (l) { t_loadedlist *next = l->ll_next; PD_MEMORY_FREE (l); l = next; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error loader_openExternal (t_canvas *canvas, char *name)
{
    char stub[PD_STRING]            = { 0 };
    char filepath[PD_STRING]        = { 0 };
    char directoryResult[PD_STRING] = { 0 };
    char *nameResult = NULL;
    char *c = NULL;
    void *object = NULL;
    t_xxx constructor = NULL;
    
    int i;
    int f;
    
#ifdef _WIN32
    HINSTANCE ntdll;
#endif

    if (c = strrchr(name, '/'))
        c++;
    else c = name;
    
    if (loader_isAlreadyLoaded(name))
    {
        post("%s: already loaded", name);
        return (1);
    }
    for (i = 0, nameResult = c; i < PD_STRING-7 && *nameResult; nameResult++)
    {
        char c = *nameResult;
        if ((c>='0' && c<='9') || (c>='A' && c<='Z')||
           (c>='a' && c<='z' )|| c == '_')
        {
            stub[i] = c;
            i++;
        }
            /* trailing tilde becomes "_tilde" */
        else if (c == '~' && nameResult[1] == 0)
        {
            strcpy(stub+i, "_tilde");
            i += strlen(stub+i);
        }
        else
        {
            PD_BUG;
        }
    }
    stub[i] = 0;
    strcat(stub, "_setup");
    
#if 0
    fprintf(stderr, "lib: %s\n", c);
#endif

    if ((f = canvas_open(canvas, name, PD_PLUGIN,
        directoryResult, &nameResult, PD_STRING, 1)) >= 0)
            goto gotone;

#ifdef ANDROID
    /* Android libs always have a 'lib' prefix, '.so' suffix and don't allow ~ */
    char libname[PD_STRING] = "lib";
    strncat(libname, name, PD_STRING - 4);
    int len = strlen(libname);
    if (libname[len-1] == '~' && len < PD_STRING - 6) {
        strcpy(libname+len-1, "_tilde");
    }
    if ((f = canvas_open(canvas, libname, ".so",
        directoryResult, &nameResult, PD_STRING, 1)) >= 0)
            goto gotone;
#endif
    return (0);
gotone:
    close(f);
    class_setDefaultExternalDirectory(gensym(directoryResult));

        /* rebuild the absolute pathname */
    strncpy(filepath, directoryResult, PD_STRING);
    filepath[PD_STRING-2] = 0;
    strcat(filepath, "/");
    strncat(filepath, nameResult, PD_STRING-strlen(filepath));
    filepath[PD_STRING-1] = 0;

#ifdef _WIN32
    {
        char dirname[PD_STRING], *s, *basename;
        path_slashToBackslashIfNecessary(filepath, filepath);
        /* set the dirname as DllDirectory, meaning in the path for
           loading other DLLs so that dependent libraries can be included
           in the same folder as the external. SetDllDirectory() needs a
           minimum supported version of Windows XP SP1 for
           SetDllDirectory, so WINVER must be 0x0502 */
        strncpy(dirname, filepath, PD_STRING);
        s = strrchr(dirname, '\\');
        basename = s;
        if (s && *s)
          *s = '\0';
        if (!SetDllDirectory(dirname))
           post_error ("Could not set '%s' as DllDirectory(), '%s' might not load.",
                 dirname, basename);
        /* now load the DLL for the external */
        ntdll = LoadLibrary(filepath);
        if (!ntdll)
        {
            post("%s: couldn't load", filepath);
            class_setDefaultExternalDirectory(&s_);
            return (0);
        }
        constructor = (t_xxx)GetProcAddress(ntdll, stub);  
        if (!constructor)
             constructor = (t_xxx)GetProcAddress(ntdll, "setup");
        SetDllDirectory(NULL); /* reset DLL dir to nothing */
    }
#else
    object = dlopen(filepath, RTLD_NOW | RTLD_GLOBAL);
    if (!object)
    {
        post("%s: %s", filepath, dlerror());
        class_setDefaultExternalDirectory(&s_);
        return (0);
    }
    constructor = (t_xxx)dlsym(object,  stub);
    if(!constructor)
        constructor = (t_xxx)dlsym(object,  "setup");
#endif

    if (!constructor)
    {
        post("load_object: Symbol \"%s\" not found", stub);
        class_setDefaultExternalDirectory(&s_);
        return 0;
    }
    (*constructor)();
    class_setDefaultExternalDirectory(&s_);
    loader_addLoaded(name);
    return (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error loader_loadExternal (t_canvas *canvas, char *name)
{
    int dsp = canvas_suspend_dsp();
    t_error err = loader_openExternal (canvas, name);
    canvas_resume_dsp (dsp);
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
