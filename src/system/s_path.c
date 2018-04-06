
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

#if !PD_WINDOWS

int path_isFileExist (const char *filepath)
{
    struct stat t; return (stat (filepath, &t) == 0);
}

int path_isFileExistAsRegularFile (const char *filepath)
{
    struct stat t; return ((stat (filepath, &t) == 0) && S_ISREG (t.st_mode));
}

int path_isFileExistAsDirectory (const char *filepath)
{
    struct stat t; return ((stat (filepath, &t) == 0) && S_ISDIR (t.st_mode));
}

t_error path_createDirectory (const char *filepath)
{
    return (mkdir (filepath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0);
}

#else   
    /* < https://msdn.microsoft.com/en-us/library/aa364944%28v=vs.85%29.aspx > */
    #error
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error path_withDirectoryAndName (char *dest, size_t size, const char *directory, const char *name)
{
    t_error err = PD_ERROR;
    
    PD_ASSERT (directory);
    PD_ASSERT (name);
    
    if (*name) {
    //
    #if PD_WINDOWS 
        int absolute = (!(*directory) || (name[0] && name[1] == ':'));
    #else 
        int absolute = (!(*directory) || (name[0] == '/'));
    #endif
    
    err = PD_ERROR_NONE;
    
    if (!absolute) {
        err |= string_add (dest, size, directory);
        err |= string_add (dest, size, "/");
    }
    
    err |= string_add (dest, size, name);
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
