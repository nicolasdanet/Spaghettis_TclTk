
/* Copyright (c) 1999-2017 Guenter Geiger and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "s_utf8.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryHelp;

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
// MARK: -

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
// MARK: -

static int file_openWithDirectoryAndName (const char *directory, 
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

    if (!err && (f = file_openRaw (p->f_directory, O_RDONLY)) >= 0) {
    //
    char *slash = NULL;

    if ((slash = strrchr (p->f_directory, '/'))) { *slash = 0; p->f_name = slash + 1; }
    
    return f;  
    //
    }
    
    p->f_directory[0] = 0; p->f_name = p->f_directory;
    
    return -1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int file_openConsideringSearchPath (const char *directory, 
    const char *name, 
    const char *extension,
    t_fileproperties *p)
{
    int f = file_openWithDirectoryAndName (directory, name, extension, p);
    
    if (f < 0) {
        t_pathlist *l = instance_getSearchPath();
        while (l) {
            char *path = pathlist_getPath (l);
            l = pathlist_getNext (l);
            f = file_openWithDirectoryAndName (path, name, extension, p);
            if (f >= 0) { break; }
        }
    }

    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void file_openHelp (const char *directory, const char *name)
{
    t_fileproperties p;
    int f = -1;
    
    if (*directory != 0) { f = file_openWithDirectoryAndName (directory, name, PD_HELP, &p); }
    
    if (f < 0) { 
        f = file_openConsideringSearchPath (main_directoryHelp->s_name, name, PD_HELP, &p); 
    }
    
    if (f < 0) { error_canNotFind (gensym (name), sym_help); }
    else {
        t_symbol *s1 = gensym (fileproperties_getName (&p));
        t_symbol *s2 = gensym (fileproperties_getDirectory (&p));
        close (f); 
        buffer_fileOpen (s1, s2);
    }
}

/* First consider the sibling files of the object (or abstraction). */
/* Then look for in the application "help" folder. */
/* Then look for in the application "extras" folder. */
/* And last in the defined search path. */

void file_openHelpPatch (t_gobj *y)
{
    char *directory = NULL;
    char name[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    if (pd_class (y) == canvas_class && glist_isAbstraction (cast_glist (y))) {
        if (!(err = (buffer_size (object_getBuffer (cast_object (y))) < 1))) {
            atom_toString (buffer_atoms (object_getBuffer (cast_object (y))), name, PD_STRING);
            directory = environment_getDirectoryAsString (glist_getEnvironment (cast_glist (y)));
        }
    
    } else if (pd_class (y) == canvas_class && glist_isArray (cast_glist (y))) {
        err = string_copy (name, PD_STRING, sym_garray->s_name);
        directory = "";
        
    } else {
        err = string_copy (name, PD_STRING, class_getHelpNameAsString (pd_class (y)));
        directory = class_getExternalDirectoryAsString (pd_class (y));
    }
    
    if (!err) { file_openHelp (directory, name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
