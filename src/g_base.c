
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_newPatch (void *dummy, t_symbol *name, t_symbol *directory)
{
    instance_environmentSetFile (name, directory);
    canvas_new (NULL, NULL, 0, NULL);
    instance_stackPopPatch (instance_contextGetCurrent(), 1);
    instance_environmentResetFile();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_makeTextObject (t_glist *glist, 
    int positionX,
    int positionY,
    int width,
    int isSelected,
    t_buffer *b)
{
    instance_setNewestObject (NULL);
    
    instance_stackPush (glist);
    
    {
    //
    t_environment *e = glist_getEnvironment (instance_contextGetCurrent());
    t_object *x = NULL;
    
    buffer_eval (b, 
        instance_getMakerObject(), 
        environment_getNumberOfArguments (e), 
        environment_getArguments (e));

    if (instance_getNewestObject()) { x = cast_objectIfConnectable (instance_getNewestObject()); }

    if (!x) {
        x = (t_object *)pd_new (text_class);    /* Create a dummy box. */
        if (buffer_size (b)) {
            error_canNotMake (buffer_size (b), buffer_atoms (b)); 
        }
    }

    object_setBuffer (x, b);
    object_setX (x, positionX);
    object_setY (x, positionY);
    object_setWidth (x, width);
    object_setType (x, TYPE_OBJECT);
    
    glist_addObject (glist, cast_gobj (x));
    
    if (isSelected) {
    //
    canvas_selectObject (glist, cast_gobj (x));
    gobj_activated (cast_gobj (x), glist, 1);
    //
    }
    
    if (pd_class (x) == vinlet_class)  { canvas_resortInlets (glist_getView (glist)); }
    if (pd_class (x) == voutlet_class) { canvas_resortOutlets (glist_getView (glist)); }
    //
    }
    
    instance_stackPop (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_fileExist (t_glist *glist, const char *name, const char *extension)
{
    char *p = NULL; char t[PD_STRING] = { 0 };
    
    int f = canvas_fileOpen (glist, name, extension, t, &p, PD_STRING);
    
    if (f >= 0) { close (f); return 1; }
    
    return 0;
}

int canvas_fileFind (t_glist *glist,
    const char *name,
    const char *extension,
    char *directoryResult,
    char **nameResult,
    size_t size)
{
    int f = canvas_fileOpen (glist, name, extension, directoryResult, nameResult, size);
    
    if (f >= 0) { close (f); return 1; }
    
    return 0;
}

/* Caller is responsible to close the file. */

int canvas_fileOpen (t_glist *glist,
    const char *name,
    const char *extension,
    char *directoryResult,
    char **nameResult,
    size_t size)
{
    const char *directory = glist ? environment_getDirectoryAsString (glist_getEnvironment (glist)) : ".";
    
    int f = file_openConsideringSearchPath (directory, 
                name,
                extension,
                directoryResult,
                nameResult, 
                size);
        
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *canvas_expandDollar (t_glist *glist, t_symbol *s)
{
    if (strchr (s->s_name, '$')) { return dollar_expandDollarSymbolByEnvironment (s, glist); }
    else {
        return s;
    }
}

t_error canvas_makeFilePath (t_glist *glist, char *name, char *dest, size_t size)
{
    t_error err = PD_ERROR_NONE;
    
    char *directory = environment_getDirectoryAsString (glist_getEnvironment (glist));
    
    if (name[0] == '/' || (name[0] && name[1] == ':') || !(*directory)) { 
        err |= string_copy (dest, size, name);
        
    } else {
        err |= string_copy (dest, size, directory);
        err |= string_addSprintf (dest, size, "/%s", name);
    }
    
    return err;
}

void canvas_updateTitle (t_glist *glist)
{
    sys_vGui ("::ui_patch::setTitle .x%lx {%s} {%s} %d\n",  // --
                    glist,
                    environment_getDirectoryAsString (glist_getEnvironment (glist)),
                    glist->gl_name->s_name,
                    glist_getDirty (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setCursorType (t_glist *glist, int type)
{
    static t_glist *lastGlist = NULL;           /* Static. */
    static int lastType = CURSOR_NOTHING;       /* Static. */
    static char *cursors[] =                    /* Static. */
        {
            "left_ptr",             // CURSOR_NOTHING
            "hand2",                // CURSOR_CLICK
            "sb_v_double_arrow",    // CURSOR_THICKEN
            "plus",                 // CURSOR_ADD
            "circle",               // CURSOR_CONNECT
            "sb_h_double_arrow"     // CURSOR_RESIZE
        };
    
    type = PD_CLAMP (type, CURSOR_NOTHING, CURSOR_RESIZE);
    
    if (lastGlist != glist || lastType != type) {
        sys_vGui (".x%lx configure -cursor %s\n", glist, cursors[type]);
        lastType  = type;
        lastGlist = glist;
    }
}

t_gobj *canvas_getHitObject (t_glist *glist, int positionX, int positionY, t_rectangle *r)
{
    t_gobj *y = NULL;
    t_gobj *object = NULL;
    
    t_rectangle t;
    
    rectangle_set (r, 0, 0, 0, 0);
    
    if (canvas_getNumberOfSelectedObjects (glist) > 1) {
    //
    t_selection *s = NULL;
    for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
    //
    if (gobj_hit (selection_getObject (s), glist, positionX, positionY, &t)) {
        rectangle_setCopy (r, &t);
        object = selection_getObject (s); 
    }
    //
    }
    //
    }
    
    if (!object) {
    //
    int k = -PD_INT_MAX;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_hit (y, glist, positionX, positionY, &t)) {
            if (rectangle_getTopLeftX (&t) > k) {
                rectangle_setCopy (r, &t);
                object = y; k = rectangle_getTopLeftX (&t);
            }
        }
    }
    //
    }

    return object;
}

int canvas_hasLine (t_glist *glist, t_object *objectOut, int m, t_object *objectIn, int n)
{
    t_outconnect *connection = NULL;
    
    t_traverser t;
    
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
        if (traverser_isLineBetween (&t, objectOut, m, objectIn, n)) { return 1; }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_bind (t_glist *glist)
{
    if (utils_isNameAllowedForWindow (glist->gl_name)) {
        pd_bind (cast_pd (glist), utils_makeBindSymbol (glist->gl_name));
    }
}

void canvas_unbind (t_glist *glist)
{
    if (utils_isNameAllowedForWindow (glist->gl_name)) {
        pd_unbind (cast_pd (glist), utils_makeBindSymbol (glist->gl_name));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_getIndexOfObject (t_glist *glist, t_gobj *y)
{
    t_gobj *t = NULL;
    int n = 0;
    for (t = glist->gl_graphics; t && t != y; t = t->g_next) { n++; }
    return n;
}

t_gobj *canvas_getObjectAtIndex (t_glist *glist, int n)
{
    t_gobj *t = NULL;
    int i = 0;
    
    for (t = glist->gl_graphics; t; t = t->g_next) {
        if (i == n) { return t; }
        i++;
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
