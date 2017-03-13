
/* 
    Copyright (c) 1999-2016 Guenter Geiger and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryHelp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static int file_openRawNative (const char *filepath, int oflag)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, -1);

    if (oflag & O_CREAT) { return _wopen (ucs2path, oflag | O_BINARY, _S_IREAD | _S_IWRITE); }
    else {
        return _wopen (ucs2path, oflag | O_BINARY);
    }
}

static FILE *file_openModeNative (const char *filepath, const char *mode)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    wchar_t ucs2mode[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, -1);
    
    mbstowcs (ucs2mode, mode, PD_STRING);
    
    return _wfopen (ucs2path, ucs2mode);
}

#else

static int file_openRawNative (const char *filepath, int oflag)
{
    if (oflag & O_CREAT) { return open (filepath, oflag, 0666); }
    else {
        return open (filepath, oflag);
    } 
}

static FILE *file_openModeNative (const char *filepath, const char *mode)
{
    return fopen (filepath, mode);
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int file_openRaw (const char *filepath, int oflag)
{
    if (!(oflag & O_CREAT)) { if (!(path_isFileExistAsRegularFile (filepath))) { return -1; } }
    
    return file_openRawNative (filepath, oflag);
}

FILE *file_openWrite (const char *filepath)
{
    return file_openModeNative (filepath, "w");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int file_openWithDirectoryAndName (const char *directory, 
    const char *name, 
    const char *extension,
    char *directoryResult, 
    char **nameResult, 
    size_t size)
{
    int f = -1;
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (directory);
    PD_ASSERT (name);
    
    err |= path_withDirectoryAndName (directoryResult, size, directory, name, 1);
    err |= string_add (directoryResult, size, extension);

    if (!err && (f = file_openRaw (directoryResult, O_RDONLY)) >= 0) {
    //
    char *slash = NULL;
    
    *nameResult = directoryResult;
    
    if ((slash = strrchr (directoryResult, '/'))) { *slash = 0; *nameResult = slash + 1; }
    
    return f;  
    //
    }
    
    *nameResult = directoryResult;
    *directoryResult = 0;
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int file_openConsideringSearchPath (const char *directory, 
    const char *name, 
    const char *extension,
    char *directoryResult, 
    char **nameResult, 
    size_t size)
{
    int f = file_openWithDirectoryAndName (directory, name, extension, directoryResult, nameResult, size);
    
    if (f < 0) {
        t_pathlist *l = path_getSearchPath();
        while (l) {
            char *path = pathlist_getPath (l);
            l = pathlist_getNext (l);
            f = file_openWithDirectoryAndName (path, name, extension, directoryResult, nameResult, size);
            if (f >= 0) { break; }
        }
    }

    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* First consider the sibling files of the object. */
/* Then look for in the application help folder. */
/* And last in the defined search path. */

void file_openHelp (const char *directory, const char *name)
{
    int f = -1;
    char *nameResult = NULL;
    char directoryResult[PD_STRING] = { 0 };
    
    if (*directory != 0) { 
        f = file_openWithDirectoryAndName (directory, name, PD_HELP, directoryResult, &nameResult, PD_STRING);
    }
    
    if (f < 0) {
        char *help = main_directoryHelp->s_name;
        f = file_openConsideringSearchPath (help, name, PD_HELP, directoryResult, &nameResult, PD_STRING);
    }
    
    if (f < 0) { error_canNotFind (gensym (name), sym_help); }
    else {
        close (f); buffer_fileOpen (NULL, gensym (nameResult), gensym (directoryResult));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
