
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

extern t_symbol *main_directorySupport;

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
    return (access (filepath, F_OK) == 0);
}

int path_isFileExistAsRegularFile (const char *filepath)
{
    struct stat t; return ((stat (filepath, &t) == 0) && S_ISREG (t.st_mode));
}

int path_isFileExistAsDirectory (const char *filepath)
{
    struct stat t; return ((stat (filepath, &t) == 0) && S_ISDIR (t.st_mode));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int path_containsHiddenDirectory (const char *filepath)
{
    return string_contains (filepath, "/.");
}

static int path_isInsideSupport (const char *filepath)
{
    return string_startWith (filepath, main_directorySupport->s_name);
}

int path_isValid (const char *filepath)
{
    if (path_containsHiddenDirectory (filepath) && !path_isInsideSupport (filepath)) { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that it fails in case of missing directory in path. */

t_error path_createDirectory (const char *filepath)
{
    return (mkdir (filepath, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) != 0);
}

t_error path_createDirectoryIfNeeded (const char *filepath)
{
    t_error err = PD_ERROR_NONE;
    
    if (!path_isFileExistAsDirectory (filepath)) { err = path_createDirectory (filepath); }
    
    return err;
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

t_symbol *path_withDirectoryAndNameAsSymbol (t_symbol *directory, t_symbol *name)
{
    char filepath[PD_STRING] = { 0 };

    if (!path_withDirectoryAndName (filepath, PD_STRING, directory->s_name, name->s_name)) {
    //
    return gensym (filepath);
    //
    }
    
    return &s_;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error path_toDirectoryAndName (char *dest, size_t size, const char *filepath, char **directory, char **name)
{
    t_error err = string_copy (dest, size, filepath);
    
    if (!err) {
    //
    int n = string_indexOfFirstOccurrenceFromEnd (dest, "/");
    
    err = (n < 0);
    
    if (!err) { dest[n] = 0; (*directory) = dest; (*name) = dest + n + 1; }
    //
    }
    
    if (err)  { dest[0] = 0; (*directory) = dest; (*name) = dest; }
    
    return err;
}

t_error path_toDirectoryAndNameAsSymbol (const char *filepath, t_symbol **directory, t_symbol **name)
{
    char t[PD_STRING] = { 0 };
    
    char *d = NULL;
    char *n = NULL;
    
    t_error err = path_toDirectoryAndName (t, PD_STRING, filepath, &d, &n);
    
    if (err) { (*directory) = &s_; (*name) = &s_; }
    else {
    //
    (*directory) = gensym (d);
    (*name)      = gensym (n);
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
