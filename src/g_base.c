
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class              *text_class;
extern t_class              *scalar_class;
extern t_class              *struct_class;
extern t_class              *canvas_class;
extern t_class              *vinlet_class;
extern t_class              *voutlet_class;
extern t_pd                 *pd_newest;

extern t_pd                 pd_objectMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_glist              *canvas_lastCanvas;             /* Static. */

static int                  canvas_lastCanvasX;             /* Static. */
static int                  canvas_lastCanvasY;             /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist *cast_glistChecked (t_pd *x)
{
    t_class *c = pd_class (x);
    
    if (c == canvas_class) { return cast_glist (x); }
    else {
        return NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_newPatch (void *dummy, t_symbol *name, t_symbol *directory)
{
    environment_setActiveFile (name, directory);
    canvas_new (NULL, NULL, 0, NULL);
    canvas_pop (cast_glist (pd_getBoundX()), 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_getCurrent (void)
{
    return cast_glist (pd_getBoundX());
}

t_glist *canvas_getRoot (t_glist *glist)
{
    if (canvas_isRoot (glist)) { return glist; }
    else {
        return canvas_getRoot (glist->gl_parent);
    }
}

t_environment *canvas_getEnvironment (t_glist *glist)
{
    PD_ASSERT (glist);
    
    while (!glist->gl_environment) { glist = glist->gl_parent; PD_ASSERT (glist); }
    
    PD_ASSERT (canvas_isRoot (glist));
    
    return glist->gl_environment;
}

t_glist *canvas_getView (t_glist *glist)
{
    while (glist->gl_parent && !canvas_canHaveWindow (glist)) { glist = glist->gl_parent; }
    
    return glist;
}

t_symbol *canvas_getName (t_glist *glist)
{
    return glist->gl_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_isMapped (t_glist *glist)
{
    return (!glist->gl_isLoading && canvas_getView (glist)->gl_isMapped);
}

int canvas_isRoot (t_glist *glist)
{
    int k = (!glist->gl_parent || canvas_isAbstraction (glist));
    
    if (k) { PD_ASSERT (glist->gl_environment != NULL); }
    
    return k;
}

int canvas_isAbstraction (t_glist *glist)
{
    return (glist->gl_parent && glist->gl_environment != NULL);
}

int canvas_isSubpatch (t_glist *glist)
{
    return !canvas_isRoot (glist);
}

int canvas_isDirty (t_glist *glist)
{
    return (canvas_getRoot (glist)->gl_isDirty != 0);
}

/* A graph-on-parent that contains an array of numbers. */

int canvas_isGraph (t_glist *glist)
{
    return (utils_getFirstAtomOfObjectAsSymbol (cast_object (glist)) == sym_graph);
}

/* Either a top window a subpacth or a graph-on-parent forced. */

int canvas_canHaveWindow (t_glist *glist)
{
    return (glist->gl_hasWindow || !glist->gl_isGraphOnParent);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_addScalarNext (t_glist *glist, t_scalar *first, t_scalar *next)
{
    if (first != NULL) {
        cast_gobj (next)->g_next  = cast_gobj (first)->g_next;
        cast_gobj (first)->g_next = cast_gobj (next);
        
    } else {
        cast_gobj (next)->g_next  = glist->gl_graphics;
        glist->gl_graphics        = cast_gobj (next);
    }

    if (canvas_isMapped (canvas_getView (glist))) { gobj_visibilityChanged (cast_gobj (next), glist, 1); }
}

void canvas_addObject (t_glist *glist, t_gobj *y)
{
    t_object *object = NULL;
    
    int needToPaintScalars = class_hasDrawCommand (pd_class (y));
    
    if (needToPaintScalars) { paint_erase(); }
    
    y->g_next = NULL;
    
    if (!glist->gl_graphics) { glist->gl_graphics = y; }
    else {
        t_gobj *t = NULL; for (t = glist->gl_graphics; t->g_next; t = t->g_next) { }
        t->g_next = y;
    }
    
    if (glist->gl_editor && (object = cast_objectIfPatchable (y))) { boxtext_new (glist, object); }
    if (canvas_isMapped (canvas_getView (glist))) { gobj_visibilityChanged (y, glist, 1); }
    
    if (needToPaintScalars) { paint_draw(); }
}

void canvas_removeObject (t_glist *glist, t_gobj *y)
{
    t_boxtext *text  = NULL;
    t_object *object = NULL;
    t_glist *canvas  = canvas_getView (glist);
        
    int needToUpdateDSPChain = class_hasDSP (pd_class (y));
    int needToPaintScalars   = class_hasDrawCommand (pd_class (y));
    int deletingState        = canvas->gl_isDeleting;
    
    canvas->gl_isDeleting = 1;
    
    needToPaintScalars |= (pd_class (y) == struct_class);
    
    if (glist->gl_editor) {
    //
    if (glist->gl_editor->e_grabbed == y)   { glist->gl_editor->e_grabbed = NULL; }
    if (canvas_isObjectSelected (glist, y)) { canvas_deselectObject (glist, y);   }
    //
    }
    
    if (needToPaintScalars) { paint_erase(); }
    
    if (canvas_isMapped (canvas)) { gobj_visibilityChanged (y, glist, 0); }
    
    gobj_deleted (y, glist);
    
    if (glist->gl_graphics == y)  { glist->gl_graphics = y->g_next; }
    else {
        t_gobj *t = NULL;
        for (t = glist->gl_graphics; t; t = t->g_next) {
            if (t->g_next == y) { t->g_next = y->g_next; break; }
        }
    }
    
    if (glist->gl_editor && (object = cast_objectIfPatchable (y))) {
        text = boxtext_fetch (glist, object);
    }
    
    pd_free (cast_pd (y));

    if (text) { boxtext_free (text); }
    
    if (needToUpdateDSPChain) { dsp_update(); }
    if (needToPaintScalars)   { paint_draw(); }
    
    canvas->gl_isDeleting = deletingState;
    
    glist->gl_uniqueIdentifier = utils_unique();        /* Invalidate all pointers. */
}

void canvas_removeScalarsRecursive (t_glist *glist, t_template *template)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
    
        if (pd_class (y) == scalar_class) {
            if (scalar_containsTemplate (cast_scalar (y), template_getTemplateIdentifier (template))) {
                canvas_removeObject (glist, y);
            }
        }
        
        if (pd_class (y) == canvas_class) {
            canvas_removeScalarsRecursive (cast_glist (y), template);
        }
    }
}

void canvas_clear (t_glist *glist)
{
    int dspState = 0;
    int dspSuspended = 0;
    t_gobj *y = NULL;
        
    while ((y = glist->gl_graphics)) {
    //
    if (!dspSuspended) {
        if (cast_objectIfPatchable (y) && class_hasDSP (pd_class (y))) {
            dspState = dsp_suspend();
            dspSuspended = 1;
        }
    }

    canvas_removeObject (glist, y);
    //
    }
    
    if (dspSuspended) { dsp_resume (dspState); }
}

void canvas_makeTextObject (t_glist *glist, 
    int positionX,
    int positionY,
    int width,
    int isSelected,
    t_buffer *b)
{
    pd_newest = NULL;
    
    stack_push (cast_pd (glist));
    
    {
    //
    t_environment *e = canvas_getEnvironment (canvas_getCurrent());
    t_object *x = NULL;
    
    buffer_eval (b, &pd_objectMaker, environment_getNumberOfArguments (e), environment_getArguments (e));

    if (pd_newest) { x = cast_objectIfPatchable (pd_newest); }

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
    
    canvas_addObject (glist, cast_gobj (x));
    
    if (isSelected) {
    //
    canvas_selectObject (glist, cast_gobj (x));
    gobj_activated (cast_gobj (x), glist, 1);
    //
    }
    
    if (pd_class (x) == vinlet_class)  { canvas_resortInlets (canvas_getView (glist)); }
    if (pd_class (x) == voutlet_class) { canvas_resortOutlets (canvas_getView (glist)); }
    //
    }
    
    stack_pop (cast_pd (glist));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_openFileExist (t_glist *glist, const char *name, const char *extension)
{
    char *p = NULL; char t[PD_STRING] = { 0 };
    
    return ((canvas_openFile (glist, name, extension, t, &p, PD_STRING)) >= 0);
}
                                                            
/* Caller is responsible to close the file. */

int canvas_openFile (t_glist *glist,
    const char *name,
    const char *extension,
    char *directoryResult,
    char **nameResult,
    size_t size)
{
    const char *directory = glist ? environment_getDirectoryAsString (canvas_getEnvironment (glist)) : ".";
    
    int f = file_openConsideringSearchPath (directory, 
                name,
                extension,
                directoryResult,
                nameResult, 
                size);
        
    return f;
}

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
    
    char *directory = environment_getDirectoryAsString (canvas_getEnvironment (glist));
    
    if (name[0] == '/' || (name[0] && name[1] == ':') || !(*directory)) { 
        err |= string_copy (dest, size, name);
        
    } else {
        err |= string_copy (dest, size, directory);
        err |= string_addSprintf (dest, size, "/%s", name);
    }
    
    return err;
}

void canvas_setName (t_glist *glist, t_symbol *name)
{
    canvas_unbind (glist);
    glist->gl_name = name;
    canvas_bind (glist);
    
    if (glist->gl_hasWindow) { canvas_updateTitle (glist); }
}

void canvas_updateTitle (t_glist *glist)
{
    sys_vGui ("::ui_patch::setTitle .x%lx {%s} {%s} %d\n",  // --
                    glist,
                    environment_getDirectoryAsString (canvas_getEnvironment (glist)),
                    glist->gl_name->s_name,
                    glist->gl_isDirty);
}

t_fontsize canvas_getFontSize (t_glist *glist)
{
    while (!glist->gl_environment) { if (!(glist = glist->gl_parent)) { PD_BUG; } }
    
    return glist->gl_fontSize;
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
    
    if (glist->gl_editor && canvas_getNumberOfSelectedObjects (glist) > 1) {
    //
    t_selection *selection = NULL;
    for (selection = glist->gl_editor->e_selectedObjects; selection; selection = selection->sel_next) {
    //
    if (gobj_hit (selection->sel_what, glist, positionX, positionY, &t)) {
        rectangle_setCopy (r, &t);
        object = selection->sel_what; 
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
    
    t_linetraverser t;
    
    linetraverser_start (&t, glist);
    
    while ((connection = linetraverser_next (&t))) {
        if (linetraverser_isLineBetween (&t, objectOut, m, objectIn, n)) { return 1; }
    }
    
    return 0;
}

void canvas_setLastMotionCoordinates (t_glist *glist, int a, int b)
{
    canvas_lastCanvas   = glist;
    canvas_lastCanvasX  = a;
    canvas_lastCanvasY  = b;
}

void canvas_getLastMotionCoordinates (t_glist *glist, int *a, int *b)
{
    if (canvas_lastCanvas == glist) { *a = canvas_lastCanvasX; *b = canvas_lastCanvasY; } 
    else {
    //
    const int x = 40;
    const int y = 40;
        
    *a = x;
    *b = y;
    //
    }
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
