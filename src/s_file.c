
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

extern t_pathlist *path_help;
extern t_pathlist *path_search;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

FILE *file_openWrite (const char *filepath)
{
    return file_openMode (filepath, "w");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WINDOWS

int file_openRaw (const char *filepath, int oflag)
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

FILE *file_openMode (const char *filepath, const char *mode)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#else

int file_openRaw (const char *filepath, int oflag)
{
    if (oflag & O_CREAT) { return open (filepath, oflag, 0666); }
    else {
        return open (filepath, oflag);
    } 
}

FILE *file_openMode (const char *filepath, const char *mode)
{
    return fopen (filepath, mode);
}

#endif // PD_WINDOWS

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* search for a file in a specified directory, then along the globally
defined search path, using ext as filename extension.  The
fd is returned, the directory ends up in the "dirresult" which must be at
least "size" bytes.  "nameresult" is set to point to the filename, which
ends up in the same buffer as dirresult.  Exception:
if the 'name' starts with a slash or a letter, colon, and slash in MSW,
there is no search and instead we just try to open the file literally.  */

/* see also canvas_open() which, in addition, searches down the
canvas-specific path. */

static int file_openWithPathList (const char *dir, const char *name,
    const char *ext, char *dirresult, char **nameresult, size_t size,
    int bin, t_pathlist *searchpath)
{
    t_pathlist *nl;
    int fd = -1;

        /* first check if "name" is absolute (and if so, try to open) */
    if (file_openWithAbsolutePath(name, ext, dirresult, nameresult, size, bin, &fd))
        return (fd);
    
        /* otherwise "name" is relative; try the directory "dir" first. */
    if ((fd = file_openWithDirectoryAndName(dir, name, ext,
        dirresult, nameresult, size, bin)) >= 0)
            return (fd);

        /* next go through the search path */
    for (nl = searchpath; nl; nl = nl->pl_next)
        if ((fd = file_openWithDirectoryAndName(nl->pl_string, name, ext,
            dirresult, nameresult, size, bin)) >= 0)
                return (fd);

        /* next look in built-in paths like "extra" */
    /*if (0)
        for (nl = path_extra; nl; nl = nl->pl_next)
            if ((fd = file_openWithDirectoryAndName(nl->pl_string, name, ext,
                dirresult, nameresult, size, bin)) >= 0)
                    return (fd);*/

    *dirresult = 0;
    *nameresult = dirresult;
    return (-1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* try to open a file in the directory "dir", named "name""ext",
    for reading.  "Name" may have slashes.  The directory is copied to
    "dirresult" which must be at least "size" bytes.  "nameresult" is set
    to point to the filename (copied elsewhere into the same buffer). 
    The "bin" flag requests opening for binary (which only makes a difference
    on Windows). */

int file_openWithDirectoryAndName (const char *dir, const char *name, const char* ext,
    char *dirresult, char **nameresult, size_t size, int bin)
{
    int fd;
    char buf[PD_STRING];
    if (strlen(dir) + strlen(name) + strlen(ext) + 4 > size)
        return (-1);
    path_expandEnvironment(dir, buf, PD_STRING);
    strcpy(dirresult, buf);
    if (*dirresult && dirresult[strlen(dirresult)-1] != '/')
        strcat(dirresult, "/");
    strcat(dirresult, name);
    strcat(dirresult, ext);

        /* see if we can open the file for reading */
    if ((fd=file_openRaw(dirresult, O_RDONLY)) >= 0)
    {
            /* in unix, further check that it's not a directory */
#ifndef _WIN32
        struct stat statbuf;
        int ok =  ((fstat(fd, &statbuf) >= 0) &&
            !S_ISDIR(statbuf.st_mode));
        if (!ok)
        {
            if (0) post("tried %s; stat failed or directory",
                dirresult);
            close (fd);
            fd = -1;
        }
        else
#endif
        {
            char *slash;
            if (0) post("tried %s and succeeded", dirresult);
            path_backslashToSlashIfNecessary(dirresult, dirresult);
            slash = strrchr(dirresult, '/');
            if (slash)
            {
                *slash = 0;
                *nameresult = slash + 1;
            }
            else *nameresult = dirresult;

            return (fd);  
        }
    }
    else
    {
        if (0) post("tried %s and failed", dirresult);
    }
    return (-1);
}

    /* check if we were given an absolute pathname, if so try to open it
    and return 1 to signal the caller to cancel any path searches */
int file_openWithAbsolutePath(const char *name, const char* ext,
    char *dirresult, char **nameresult, size_t size, int bin, int *fdp)
{
    if (path_isAbsoluteWithEnvironment(name))
    {
        char dirbuf[PD_STRING], *z = strrchr(name, '/');
        int dirlen;
        if (!z)
            return (0);
        dirlen = z - name;
        if (dirlen > PD_STRING-1) 
            dirlen = PD_STRING-1;
        strncpy(dirbuf, name, dirlen);
        dirbuf[dirlen] = 0;
        *fdp = file_openWithDirectoryAndName(dirbuf, name+(dirlen+1), ext,
            dirresult, nameresult, size, bin);
        return (1);
    }
    else return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int file_openBySearchPath (const char *directory, 
                                    const char *name, 
                                    const char *extension,
                                    char *directoryResult, 
                                    char **nameResult, 
                                    size_t size, 
                                    int isBinary)
{   
    return (file_openWithPathList (directory, name, extension, directoryResult, nameResult, size, isBinary, path_search));
}


void file_openHelp (const char *directory, const char *name)
{
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
    if ((fd = file_openWithPathList(usedir, realname, "", dirbuf, &basename, 
        PD_STRING, 0, path_help)) >= 0)
            goto gotone;

        /* 2. "help-objectname.pd" */
    strcpy(realname, "help-");
    strncat(realname, name, PD_STRING-10);
    realname[PD_STRING-1] = 0;
    if ((fd = file_openWithPathList(usedir, realname, "", dirbuf, &basename, 
        PD_STRING, 0, path_help)) >= 0)
            goto gotone;

    post("sorry, couldn't find help patch for \"%s\"", name);
    return;
gotone:
    close (fd);
    buffer_openFile (0, gensym((char*)basename), gensym(dirbuf));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
