
/* 
    Copyright (c) 1999-2016 Guenter Geiger and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static t_error path_expandHomeDirectory (const char *dest, size_t size, char *src)
{
    return string_copy (dest, size, src);
}

#else

static t_error path_expandHomeDirectory (char *dest, size_t size, const char *src)
{
    t_error err = PD_ERROR_NONE;

    if ((strlen (src) == 1 && src[0] == '~') || string_startWith (src, "~/")) {
    
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

void path_slashToBackslashIfNecessary (char *s)
{
    #if PD_WINDOWS
    
    string_replaceCharacter (s, '/', '\\');
    
    #endif
}

void path_backslashToSlashIfNecessary (char *s)
{
    #if PD_WINDOWS
    
    string_replaceCharacter (s, '\\', '/');
    
    #endif
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error path_withDirectoryAndName (char *dest, 
    size_t size, 
    const char *directory, 
    const char *name, 
    int expandHome)
{
    t_error err = PD_ERROR;
    
    if (*name) {
    
        err = PD_ERROR_NONE;
        
        if (expandHome) { err |= path_expandHomeDirectory (dest, size, directory); } 
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
