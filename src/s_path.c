
/* 
    Copyright (c) 1999 Guenter Geiger and others.
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

extern t_class *global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pathlist *path_search;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void path_slashToBackslashIfNecessary (char *dest, char *src)
{
    char c;
    while (c = *src++) {
        #if PD_WINDOWS
        if (c == '/') { c = '\\'; }
        #endif
        *dest++ = c;
    }
    *dest = 0;
}

void path_backslashToSlashIfNecessary (char *dest, char *src)
{
    char c;
    while (c = *src++) {
        #if PD_WINDOWS
        if (c == '\\') { c = '/'; }
        #endif
        *dest++ = c;
    }
    *dest = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if !PD_WINDOWS

int path_isFileExist (const char *filepath)
{
    struct stat t; return (stat (filepath, &t) == 0);
}

int path_isFileExistAsRegularFile (const char *filepath)
{
    struct stat t; return ((stat (filepath, &t) == 0) && S_ISREG (t.st_mode));
}

#else   
    /* < https://msdn.microsoft.com/en-us/library/aa364944%28v=vs.85%29.aspx > */
    #error
#endif

int path_isAbsoluteConsideringEnvironment (const char *f)
{
    #if PD_WINDOWS
    
    return (f[0] == '/' || f[0] == '~' || f[0] == '%' || (f[1] == ':' && f[2] == '/'));
    
    #else
    
    return (f[0] == '/' || f[0] == '~');
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error path_withDirectoryAndName (char *dest, 
                                    size_t size, 
                                    const char *directory, 
                                    const char *name, 
                                    int expandEnvironment)
{
    t_error err = PD_ERROR;
    
    if (*name) {
    
        err = PD_ERROR_NONE;
        
        if (expandEnvironment) { err |= path_expandEnvironment (dest, size, directory); } 
        else {
            err |= string_copy (dest, size, directory);
        }
        
        err |= string_add (dest, size, "/");
        err |= string_add (dest, size, name);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef PD_WINDOWS

t_error path_expandEnvironment (const char *dest, size_t size, char *src)
{
    return string_copy (dest, size, src);
}

#else

t_error path_expandEnvironment (char *dest, size_t size, const char *src)
{
    t_error err = PD_ERROR_NONE;

    if ((strlen (src) == 1 && src[0] == '~') || (strncmp (src, "~/", 2) == 0)) {
    
        const char *home = getenv ("HOME");
        
        if (!home) { *dest = 0; }
        else {
            err |= string_copy (dest, size, home);
            err |= string_add (dest, size, src + 1);
        }

    } else {
        err |= string_copy (dest, size, src);
    }

    return err;
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void path_setSearchPath (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    
    pathlist_free (path_search); 
    path_search = NULL;
    
    for (i = 0; i < argc; i++) {
        t_symbol *path = utils_decode (atom_getSymbolAtIndex (i, argc, argv));
        path_search = pathlist_newAppend (path_search, path->s_name);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
