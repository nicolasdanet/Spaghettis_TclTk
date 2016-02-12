
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
#include "s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pathlist *path_search;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

static int file_openRawPlatformSpecific (const char *filepath, int oflag)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t, t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, PD_STRING - 1);

    if (oflag & O_CREAT) { return _wopen (ucs2path, oflag | O_BINARY, _S_IREAD | _S_IWRITE); }
    else {
        return _wopen (ucs2path, oflag | O_BINARY);
    }
}

static FILE *file_openModePlatformSpecific (const char *filepath, const char *mode)
{
    char t[PD_STRING]           = { 0 };
    wchar_t ucs2path[PD_STRING] = { 0 };
    wchar_t ucs2mode[PD_STRING] = { 0 };
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t, t);
    u8_utf8toucs2 (ucs2path, PD_STRING, t, PD_STRING - 1);
    mbstowcs (ucs2mode, mode, PD_STRING);
    
    return _wfopen (ucs2path, ucs2mode);
}

#else

static int file_openRawPlatformSpecific (const char *filepath, int oflag)
{
    if (oflag & O_CREAT) { return open (filepath, oflag, 0666); }
    else {
        return open (filepath, oflag);
    } 
}

static FILE *file_openModePlatformSpecific (const char *filepath, const char *mode)
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
    
    return file_openRawPlatformSpecific (filepath, oflag);
}

FILE *file_openWrite (const char *filepath)
{
    return file_openModePlatformSpecific (filepath, "w");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int file_openWithDirectoryAndName (const char *directory, 
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
    
    if ((slash = strrchr (directoryResult, '/'))) { *slash = 0; *nameResult = slash + 1; }
    else {
        *nameResult = directoryResult; 
    }
    
    return f;  
    //
    }
    
    *directoryResult = 0;
    *nameResult = directoryResult;
    
    return -1;
}

int file_openConsideringSearchPath (const char *directory, 
                const char *name, 
                const char *extension,
                char *directoryResult, 
                char **nameResult, 
                size_t size)
{
    int f = file_openWithDirectoryAndName (directory, name, extension, directoryResult, nameResult, size);
    
    if (f < 0) {

        t_pathlist *l = NULL;
            
        for (l = path_search; l; l = pathlist_getNext (l)) {
            char *path = pathlist_getFile (l);
            f = file_openWithDirectoryAndName (path, name, extension, directoryResult, nameResult, size);
            if (f >= 0) { break; }
        }
    }

    return f;
}

void file_openHelp (const char *directory, const char *name)
{
    #if 0
    char realname[PD_STRING], dirbuf[PD_STRING], *basename;
        /* make up a silly "dir" if none is supplied */
    const char *usedir = (*directory ? directory : "./");
    int fd;

        /* 1. "objectname-help.pd" */
    strncpy(realname, name, PD_STRING-10);
    realname[PD_STRING-10] = 0;
    if (strlen(realname) > 3 && !strcmp(realname+strlen(realname)-3, PD_FILE))
        realname[strlen(realname)-3] = 0;
    strcat(realname, "-help.pd");
    if ((fd = file_openWithList(usedir, realname, "", dirbuf, &basename, 
        PD_STRING, path_help)) >= 0)
            goto gotone;

        /* 2. "help-objectname.pd" */
    strcpy(realname, "help-");
    strncat(realname, name, PD_STRING-10);
    realname[PD_STRING-1] = 0;
    if ((fd = file_openWithList(usedir, realname, "", dirbuf, &basename, 
        PD_STRING, path_help)) >= 0)
            goto gotone;

    post("sorry, couldn't find help patch for \"%s\"", name);
    return;
gotone:
    close (fd);
    buffer_openFile (0, gensym((char*)basename), gensym(dirbuf));
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
