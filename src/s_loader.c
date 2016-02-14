
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

static void loader_clearLoaded (void)
{
    t_loadedlist *l = loader_alreadyLoaded;
    t_loadedlist *next = NULL;
    
    while (l) { next = l->ll_next; PD_MEMORY_FREE (l); l = next; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void loader_initialize (void)
{

}

void loader_release (void)
{
    loader_clearLoaded();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error loader_openExternal (t_canvas *canvas, char *objectname)
{
    char symname[PD_STRING], filename[PD_STRING], dirbuf[PD_STRING],
        *classname, *nameptr, altsymname[PD_STRING];
    void *dlobj;
    t_xxx makeout = NULL;
    int i, hexmunge = 0, fd;
#ifdef _WIN32
    HINSTANCE ntdll;
#endif
    if (classname = strrchr(objectname, '/'))
        classname++;
    else classname = objectname;
    if (loader_isAlreadyLoaded(objectname))
    {
        post("%s: already loaded", objectname);
        return (1);
    }
    for (i = 0, nameptr = classname; i < PD_STRING-7 && *nameptr; nameptr++)
    {
        char c = *nameptr;
        if ((c>='0' && c<='9') || (c>='A' && c<='Z')||
           (c>='a' && c<='z' )|| c == '_')
        {
            symname[i] = c;
            i++;
        }
            /* trailing tilde becomes "_tilde" */
        else if (c == '~' && nameptr[1] == 0)
        {
            strcpy(symname+i, "_tilde");
            i += strlen(symname+i);
        }
        else /* anything you can't put in a C symbol is sprintf'ed in hex */
        {
            sprintf(symname+i, "0x%02x", c);
            i += strlen(symname+i);
            hexmunge = 1;
        }
    }
    symname[i] = 0;
    if (hexmunge)
    {
        memmove(symname+6, symname, strlen(symname)+1);
        strncpy(symname, "setup_", 6);
    }
    else strcat(symname, "_setup");
    
#if 0
    fprintf(stderr, "lib: %s\n", classname);
#endif

    if ((fd = canvas_open(canvas, objectname, PD_PLUGIN,
        dirbuf, &nameptr, PD_STRING, 1)) >= 0)
            goto gotone;
        /* next try (objectname)/(classname).(sys_dllextent) ... */
    strncpy(filename, objectname, PD_STRING);
    filename[PD_STRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, classname, PD_STRING-strlen(filename));
    filename[PD_STRING-1] = 0;
    if ((fd = canvas_open(canvas, filename, PD_PLUGIN,
        dirbuf, &nameptr, PD_STRING, 1)) >= 0)
            goto gotone;
#ifdef ANDROID
    /* Android libs always have a 'lib' prefix, '.so' suffix and don't allow ~ */
    char libname[PD_STRING] = "lib";
    strncat(libname, objectname, PD_STRING - 4);
    int len = strlen(libname);
    if (libname[len-1] == '~' && len < PD_STRING - 6) {
        strcpy(libname+len-1, "_tilde");
    }
    if ((fd = canvas_open(canvas, libname, ".so",
        dirbuf, &nameptr, PD_STRING, 1)) >= 0)
            goto gotone;
#endif
    return (0);
gotone:
    close(fd);
    class_setDefaultExternalDirectory(gensym(dirbuf));

        /* rebuild the absolute pathname */
    strncpy(filename, dirbuf, PD_STRING);
    filename[PD_STRING-2] = 0;
    strcat(filename, "/");
    strncat(filename, nameptr, PD_STRING-strlen(filename));
    filename[PD_STRING-1] = 0;

#ifdef _WIN32
    {
        char dirname[PD_STRING], *s, *basename;
        path_slashToBackslashIfNecessary(filename, filename);
        /* set the dirname as DllDirectory, meaning in the path for
           loading other DLLs so that dependent libraries can be included
           in the same folder as the external. SetDllDirectory() needs a
           minimum supported version of Windows XP SP1 for
           SetDllDirectory, so WINVER must be 0x0502 */
        strncpy(dirname, filename, PD_STRING);
        s = strrchr(dirname, '\\');
        basename = s;
        if (s && *s)
          *s = '\0';
        if (!SetDllDirectory(dirname))
           post_error ("Could not set '%s' as DllDirectory(), '%s' might not load.",
                 dirname, basename);
        /* now load the DLL for the external */
        ntdll = LoadLibrary(filename);
        if (!ntdll)
        {
            post("%s: couldn't load", filename);
            class_setDefaultExternalDirectory(&s_);
            return (0);
        }
        makeout = (t_xxx)GetProcAddress(ntdll, symname);  
        if (!makeout)
             makeout = (t_xxx)GetProcAddress(ntdll, "setup");
        SetDllDirectory(NULL); /* reset DLL dir to nothing */
    }
#else
    dlobj = dlopen(filename, RTLD_NOW | RTLD_GLOBAL);
    if (!dlobj)
    {
        post("%s: %s", filename, dlerror());
        class_setDefaultExternalDirectory(&s_);
        return (0);
    }
    makeout = (t_xxx)dlsym(dlobj,  symname);
    if(!makeout)
        makeout = (t_xxx)dlsym(dlobj,  "setup");
#endif

    if (!makeout)
    {
        post("load_object: Symbol \"%s\" not found", symname);
        class_setDefaultExternalDirectory(&s_);
        return 0;
    }
    (*makeout)();
    class_setDefaultExternalDirectory(&s_);
    loader_addLoaded(objectname);
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
