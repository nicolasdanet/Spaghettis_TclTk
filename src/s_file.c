
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WINDOWS

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

static FILE *file_openModeNative (const char *filepath, const char *mode)
{
    return fopen (filepath, mode);
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

FILE *file_fopenWrite (const char *filepath)
{
    return file_openModeNative (filepath, "w");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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

#else

static int file_openRawNative (const char *filepath, int oflag)
{
    if (oflag & O_CREAT) { return open (filepath, oflag, 0666); }
    else {
        return open (filepath, oflag);
    } 
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int file_openWrite (const char *filepath)
{
    return file_openRawNative (filepath, O_CREAT | O_TRUNC | O_WRONLY);
}

int file_openRead (const char *filepath)
{
    if (!(path_isFileExistAsRegularFile (filepath))) { return -1; }
    
    return file_openRawNative (filepath, O_RDONLY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int file_openReadWithDirectoryAndName (const char *directory,
    const char *name, 
    const char *extension,
    t_fileproperties *p)
{
    int f = -1;
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (directory);
    PD_ASSERT (name);
    
    p->f_directory[0] = 0; p->f_name = p->f_directory;
    
    err |= path_withDirectoryAndName (p->f_directory, PD_STRING, directory, name);
    err |= string_add (p->f_directory, PD_STRING, extension);

    if (!err && (f = file_openRead (p->f_directory)) >= 0) {
    //
    char *slash = NULL;

    if ((slash = strrchr (p->f_directory, '/'))) { *slash = 0; p->f_name = slash + 1; }
    
    return f;  
    //
    }
    
    p->f_directory[0] = 0; p->f_name = p->f_directory;
    
    return -1;
}

int file_openReadConsideringSearchPath (const char *directory, 
    const char *name, 
    const char *extension,
    t_fileproperties *p)
{
    int f = file_openReadWithDirectoryAndName (directory, name, extension, p);
    
    if (f < 0) {
        t_pathlist *l = instance_getSearchPath();
        while (l) {
            char *path = pathlist_getPath (l);
            l = pathlist_getNext (l);
            f = file_openReadWithDirectoryAndName (path, name, extension, p);
            if (f >= 0) { break; }
        }
    }

    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
