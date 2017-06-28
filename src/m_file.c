
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *main_directoryHelp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_fromFile (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;
    
    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory, name))) {
    //
    int f = file_openRaw (filepath, O_RDONLY);
    
    err = (f < 0);
    
    if (err) { PD_BUG; }
    else {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (err) { PD_BUG; }
    else {
        char *t = (char *)PD_MEMORY_GET ((size_t)length);
        err = (read (f, t, (size_t)length) != length);
        if (err) { PD_BUG; } else { buffer_withStringUnzeroed (x, t, (int)length); }
        PD_MEMORY_FREE (t);
    }
    
    close (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error buffer_fileRead (t_buffer *x, t_symbol *name, t_glist *glist)
{
    t_error err = PD_ERROR;
    
    t_fileproperties p;
    
    if (glist_fileExist (glist, name->s_name, "", &p)) {
        err = buffer_fromFile (x, fileproperties_getName (&p), fileproperties_getDirectory (&p));
    }
    
    if (err) { error_canNotOpen (name); }
    
    return err;
}

t_error buffer_fileWrite (t_buffer *x, t_symbol *name, t_symbol *directory)
{
    t_error err = PD_ERROR;

    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory->s_name, name->s_name))) {
    //
    FILE *f = 0;

    err = !(f = file_openWrite (filepath));
    
    if (!err) {
    //
    char *s = NULL;
    int size = 0;
    
    buffer_toStringUnzeroed (x, &s, &size);

    err |= (fwrite (s, size, 1, f) < 1);
    err |= (fflush (f) != 0);

    PD_ASSERT (!err);
    PD_MEMORY_FREE (s);
        
    fclose (f);
    //
    }
    //
    }
    
    return err;
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
        instance_patchOpen (s1, s2);
    }
}

/* First consider the sibling files of an abstraction. */
/* For an external search in its help directory if provided. */
/* Then look for in the application "help" folder. */
/* And last in the user search path. */

void file_openHelpPatch (t_gobj *y)
{
    char *directory = NULL;
    char name[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    if (pd_class (y) == canvas_class && glist_isAbstraction (cast_glist (y))) {
        if (!(err = (buffer_getSize (object_getBuffer (cast_object (y))) < 1))) {
            atom_toString (buffer_getAtoms (object_getBuffer (cast_object (y))), name, PD_STRING);
            directory = environment_getDirectoryAsString (glist_getEnvironment (cast_glist (y)));
        }
    
    } else if (pd_class (y) == canvas_class && glist_isArray (cast_glist (y))) {
        err = string_copy (name, PD_STRING, sym_garray->s_name);
        directory = "";
        
    } else {
        err = string_copy (name, PD_STRING, class_getHelpNameAsString (pd_class (y)));
        directory = class_getHelpDirectoryAsString (pd_class (y));
    }
    
    if (!err) { file_openHelp (directory, name); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
