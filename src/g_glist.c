
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

/* A root has no parent and an environment. */
/* An abstraction has a parent and an environment. */
/* A subpatch has a parent also but no environment. */
/* A top patch is either a root or an abstraction. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isRoot (t_glist *glist)
{
    return (!glist_hasParent (glist));
}

int glist_isTop (t_glist *glist)
{
    int k = (glist_isRoot (glist) || glist_isAbstraction (glist));
    
    return k;
}

int glist_isAbstraction (t_glist *glist)
{
    return (glist_hasParent (glist) && (glist->gl_environment != NULL));
}

int glist_isSubpatch (t_glist *glist)
{
    return (!glist_isTop (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Array is a GOP patch that contains only a scalar. */
/* This scalar has an array of numbers as unique field. */
/* Dirty bit is always owned by the top patch. */
/* For GOP the view to draw on is owned by a parent. */
/* Note that a GOP can be opened in its own window on demand. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int glist_isArray (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

int glist_isDirty (t_glist *glist)
{
    return (glist_getTop (glist)->gl_isDirty != 0);
}

int glist_isOnScreen (t_glist *glist)
{
    return (!glist_isLoading (glist) && glist_getView (glist)->gl_isMapped);
}

int glist_isWindowable (t_glist *glist)
{
    return (!glist_isGraphOnParent (glist) || glist_hasWindow (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_bind (t_glist *glist)
{
    if (utils_isNameAllowedForWindow (glist_getName (glist))) {
        pd_bind (cast_pd (glist), utils_makeBindSymbol (glist_getName (glist)));
    }
}

void glist_unbind (t_glist *glist)
{
    if (utils_isNameAllowedForWindow (glist_getName (glist))) {
        pd_unbind (cast_pd (glist), utils_makeBindSymbol (glist_getName (glist)));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *glist_getTop (t_glist *glist)
{
    if (glist_isTop (glist)) { return glist; } else { return glist_getTop (glist_getParent (glist)); }
}

t_environment *glist_getEnvironment (t_glist *glist)
{
    return (glist_getTop (glist)->gl_environment);
}

t_glist *glist_getView (t_glist *glist)
{
    while (glist_hasParent (glist) && !glist_isWindowable (glist)) { glist = glist_getParent (glist); }
    
    return glist;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_setName (t_glist *glist, t_symbol *name)
{
    glist_unbind (glist);
    
    glist->gl_name = name;
    
    glist_bind (glist);
    
    if (glist_hasWindow (glist)) { canvas_updateTitle (glist); }
}

void glist_setDirty (t_glist *glist, int n)
{
    int isDirty = (n != 0);
        
    t_glist *y = glist_getTop (glist);
        
    if (y->gl_isDirty != isDirty) {
    //
    y->gl_isDirty = isDirty; 
    
    if (glist_hasWindow (y)) { canvas_updateTitle (y); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void glist_objectMake (t_glist *glist, int a, int b, int w, int isSelected, t_buffer *t)
{
    instance_setNewestObject (NULL);
    
    instance_stackPush (glist);
    
    {
    //
    t_environment *e = glist_getEnvironment (instance_contextGetCurrent());
    
    t_object *x = NULL;
    
    buffer_eval (t, 
        instance_getMakerObject(), 
        environment_getNumberOfArguments (e), 
        environment_getArguments (e));

    if (instance_getNewestObject()) { x = cast_objectIfConnectable (instance_getNewestObject()); }

    if (!x) {
        x = (t_object *)pd_new (text_class);    /* If failed create a dummy box. */
        if (buffer_size (t)) {
            error_canNotMake (buffer_size (t), buffer_atoms (t)); 
        }
    }

    object_setBuffer (x, t);
    object_setX (x, a);
    object_setY (x, b);
    object_setWidth (x, w);
    object_setType (x, TYPE_OBJECT);
    
    glist_objectAdd (glist, cast_gobj (x));
    
    if (isSelected) {
    //
    canvas_selectObject (glist, cast_gobj (x));
    gobj_activated (cast_gobj (x), glist, 1);
    //
    }
    
    if (pd_class (x) == vinlet_class)  { canvas_resortInlets (glist_getView (glist));  }
    if (pd_class (x) == voutlet_class) { canvas_resortOutlets (glist_getView (glist)); }
    //
    }
    
    instance_stackPop (glist);
}

void glist_objectAddProceed (t_glist *glist, t_gobj *first, t_gobj *next)
{
    next->g_next = NULL;
    
    if (first != NULL) { next->g_next = first->g_next; first->g_next = next; }
    else {
    //    
    if (!glist->gl_graphics) { glist->gl_graphics = next; }
    else {
        t_gobj *t = NULL; for (t = glist->gl_graphics; t->g_next; t = t->g_next) { } 
        t->g_next = next;
    }
    //
    }
}

void glist_objectAddNext (t_glist *glist, t_gobj *first, t_gobj *next)
{
    int needToRepaint = class_hasPainterWidgetBehavior (pd_class (next));
    
    if (needToRepaint) { paint_erase(); }
    
    glist_objectAddProceed (glist, first, next);
    
    if (cast_objectIfConnectable (next)) { editor_boxAdd (glist_getEditor (glist), cast_object (next)); }
    if (glist_isOnScreen (glist_getView (glist))) { gobj_visibilityChanged (next, glist, 1); }
    
    if (needToRepaint) { paint_draw(); }
}

void glist_objectAdd (t_glist *glist, t_gobj *y)
{
    glist_objectAddNext (glist, NULL, y);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_objectRemoveProceed (t_glist *glist, t_gobj *y)
{
    if (glist->gl_graphics == y) { glist->gl_graphics = y->g_next; }
    else {
        t_gobj *t = NULL;
        for (t = glist->gl_graphics; t; t = t->g_next) {
            if (t->g_next == y) { t->g_next = y->g_next; break; }
        }
    }
}

void glist_objectRemove (t_glist *glist, t_gobj *y)
{
    t_glist *view     = glist_getView (glist);
    int needToRebuild = class_hasDSP (pd_class (y));
    int needToRepaint = class_hasPainterWidgetBehavior (pd_class (y)) || (pd_class (y) == struct_class);
    
    glist_deleteBegin (view);
    
    editor_motionUnset (glist_getEditor (glist), y);
    
    if (canvas_isObjectSelected (glist, y)) { canvas_deselectObject (glist, y); }
    if (needToRepaint) { paint_erase(); }
    if (glist_isOnScreen (view)) { gobj_visibilityChanged (y, glist, 0); }
    
    {
        t_box *box = NULL;
        
        if (cast_objectIfConnectable (y)) { box = box_fetch (glist, cast_object (y)); }
        
        gobj_deleted (y, glist); 
        glist_objectRemoveProceed (glist, y); 
        pd_free (cast_pd (y));

        if (box) {
            editor_boxRemove (glist_getEditor (glist), box); 
        }
    }
    
    if (needToRebuild) { dsp_update(); }
    if (needToRepaint) { paint_draw(); }
    
    glist->gl_uniqueIdentifier = utils_unique();    /* Invalidate all pointers. */
    
    glist_deleteEnd (view);
}

/* If needed the DSP is suspended to avoid multiple rebuilds. */

void glist_objectRemoveAll (t_glist *glist)
{
    int dspState = 0;
    int dspSuspended = 0;
    t_gobj *y = NULL;
    
    while ((y = glist->gl_graphics)) {
    //
    if (!dspSuspended) { 
        if (class_hasDSP (pd_class (y))) { dspState = dsp_suspend(); dspSuspended = 1; }
    }

    glist_objectRemove (glist, y);
    //
    }
    
    if (dspSuspended) { dsp_resume (dspState); }
}

void glist_objectRemoveByTemplate (t_glist *glist, t_template *template)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
    
        if (pd_class (y) == scalar_class) {
            if (scalar_containsTemplate (cast_scalar (y), template_getTemplateIdentifier (template))) {
                glist_objectRemove (glist, y);
            }
        }
        
        if (pd_class (y) == canvas_class) {
            glist_objectRemoveByTemplate (cast_glist (y), template);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_gobj *glist_objectGetAtIndex (t_glist *glist, int n)
{
    t_gobj *t = NULL;
    int i = 0;
    
    for (t = glist->gl_graphics; t; t = t->g_next) {
        if (i == n) { return t; }
        i++;
    }
    
    return NULL;
}

int glist_objectGetIndexOf (t_glist *glist, t_gobj *y)
{
    t_gobj *t = NULL;
    int n = 0;
    
    for (t = glist->gl_graphics; t && t != y; t = t->g_next) { 
        n++; 
    }
    
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Files are searching in the directory of the patch first. */
/* Without success it tries to find it using the search path. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int glist_fileExist (t_glist *glist, char *name, char *extension, t_fileproperties *p)
{
    int f = glist_fileOpen (glist, name, extension, p);
    
    if (f >= 0) { close (f); return 1; }
    
    return 0;
}

/* Caller is responsible to close the file. */

int glist_fileOpen (t_glist *glist, char *name, char *extension, t_fileproperties *p)
{
    char *directory = glist ? environment_getDirectoryAsString (glist_getEnvironment (glist)) : ".";
    
    int f = file_openConsideringSearchPath (directory, name, extension, p);
        
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
