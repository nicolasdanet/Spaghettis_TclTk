
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

t_pathlist *path_help;          /* Shared. */
t_pathlist *path_search;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void path_slashToBackslashIfNecessary (char *src, char *dest)
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

void path_backslashToSlashIfNecessary (char *src, char *dest)
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

int path_isFileExist (const char *filepath)
{
    struct stat t; return (stat (filepath, &t) == 0);
}

int path_isAbsoluteWithEnvironment (const char *f)
{
    #if PD_WINDOWS
    
    return (f[0] == '/' || f[0] == '~' || f[0] == '%' || (f[1] == ':' && f[2] == '/'));
    
    #else
    
    return (f[0] == '/' || f[0] == '~');
    
    #endif
}

t_error path_withNameAndDirectory (char *dest, size_t size, const char *name, const char *directory)
{
    t_error err = PD_ERROR;
    
    if (*name) {
        err = PD_ERROR_NONE;
        err |= string_copy (dest, size, directory);
        err |= string_add (dest, size, "/");
        err |= string_add (dest, size, name);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef PD_WINDOWS

t_error path_expandEnvironment (const char *src, char *dest, size_t size)
{
    return PD_ERROR;
}

#else

t_error path_expandEnvironment (const char *src, char *dest, size_t size)
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

    /* try to open a file in the directory "dir", named "name""ext",
    for reading.  "Name" may have slashes.  The directory is copied to
    "dirresult" which must be at least "size" bytes.  "nameresult" is set
    to point to the filename (copied elsewhere into the same buffer). 
    The "bin" flag requests opening for binary (which only makes a difference
    on Windows). */

int sys_trytoopenone(const char *dir, const char *name, const char* ext,
    char *dirresult, char **nameresult, unsigned int size, int bin)
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
int sys_open_absolute(const char *name, const char* ext,
    char *dirresult, char **nameresult, unsigned int size, int bin, int *fdp)
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
        *fdp = sys_trytoopenone(dirbuf, name+(dirlen+1), ext,
            dirresult, nameresult, size, bin);
        return (1);
    }
    else return (0);
}

/* search for a file in a specified directory, then along the globally
defined search path, using ext as filename extension.  The
fd is returned, the directory ends up in the "dirresult" which must be at
least "size" bytes.  "nameresult" is set to point to the filename, which
ends up in the same buffer as dirresult.  Exception:
if the 'name' starts with a slash or a letter, colon, and slash in MSW,
there is no search and instead we just try to open the file literally.  */

/* see also canvas_open() which, in addition, searches down the
canvas-specific path. */

static int do_open_via_path(const char *dir, const char *name,
    const char *ext, char *dirresult, char **nameresult, unsigned int size,
    int bin, t_pathlist *searchpath)
{
    t_pathlist *nl;
    int fd = -1;

        /* first check if "name" is absolute (and if so, try to open) */
    if (sys_open_absolute(name, ext, dirresult, nameresult, size, bin, &fd))
        return (fd);
    
        /* otherwise "name" is relative; try the directory "dir" first. */
    if ((fd = sys_trytoopenone(dir, name, ext,
        dirresult, nameresult, size, bin)) >= 0)
            return (fd);

        /* next go through the search path */
    for (nl = searchpath; nl; nl = nl->pl_next)
        if ((fd = sys_trytoopenone(nl->pl_string, name, ext,
            dirresult, nameresult, size, bin)) >= 0)
                return (fd);

        /* next look in built-in paths like "extra" */
    /*if (0)
        for (nl = path_extra; nl; nl = nl->pl_next)
            if ((fd = sys_trytoopenone(nl->pl_string, name, ext,
                dirresult, nameresult, size, bin)) >= 0)
                    return (fd);*/

    *dirresult = 0;
    *nameresult = dirresult;
    return (-1);
}

    /* open via path, using the global search path. */
int open_via_path(const char *dir, const char *name, const char *ext,
    char *dirresult, char **nameresult, unsigned int size, int bin)
{
    return (do_open_via_path(dir, name, ext, dirresult, nameresult,
        size, bin, path_search));
}



    /* Open a help file using the help search path.  We expect the ".pd"
    suffix here, even though we have to tear it back off for one of the
    search attempts. */
void open_via_helppath(const char *name, const char *dir)
{
    char realname[PD_STRING], dirbuf[PD_STRING], *basename;
        /* make up a silly "dir" if none is supplied */
    const char *usedir = (*dir ? dir : "./");
    int fd;

        /* 1. "objectname-help.pd" */
    strncpy(realname, name, PD_STRING-10);
    realname[PD_STRING-10] = 0;
    if (strlen(realname) > 3 && !strcmp(realname+strlen(realname)-3, PD_FILE))
        realname[strlen(realname)-3] = 0;
    strcat(realname, "-help.pd");
    if ((fd = do_open_via_path(usedir, realname, "", dirbuf, &basename, 
        PD_STRING, 0, path_help)) >= 0)
            goto gotone;

        /* 2. "help-objectname.pd" */
    strcpy(realname, "help-");
    strncat(realname, name, PD_STRING-10);
    realname[PD_STRING-1] = 0;
    if ((fd = do_open_via_path(usedir, realname, "", dirbuf, &basename, 
        PD_STRING, 0, path_help)) >= 0)
            goto gotone;

    post("sorry, couldn't find help patch for \"%s\"", name);
    return;
gotone:
    close (fd);
    buffer_openFile(0, gensym((char*)basename), gensym(dirbuf));
}

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
