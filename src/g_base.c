
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_graphics.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class              *text_class;
extern t_class              *canvas_class;
extern t_class              *vinlet_class;
extern t_class              *voutlet_class;
extern t_class              *arraydefine_class;
extern t_class              *scalardefine_class;
extern t_pd                 *pd_newest;
extern t_pdinstance         *pd_this;
extern t_symbol             *canvas_fileName;
extern t_symbol             *canvas_directory;
extern t_atom               *canvas_argv;

extern t_pd                 pd_objectMaker;
extern t_widgetbehavior     text_widgetBehavior;
extern int                  canvas_argc;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_glist              *canvas_lastCanvas;             /* Shared. */

static int                  canvas_lastCanvasX;             /* Shared. */
static int                  canvas_lastCanvasY;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_glist *cast_glistChecked (t_pd *x)
{
    t_class *c = pd_class (x);
    
    if (c == canvas_class || c == arraydefine_class || c == scalardefine_class) {
        return cast_glist (x);
    } else {
        return NULL;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setActiveFileNameAndDirectory (t_symbol *name, t_symbol *directory)
{
    canvas_fileName  = name;
    canvas_directory = directory;
}

void canvas_setActiveArguments (int argc, t_atom *argv)
{
    if (canvas_argv) { PD_MEMORY_FREE (canvas_argv); }
    
    canvas_argc = argc;
    canvas_argv = PD_MEMORY_GET_COPY (argv, argc * sizeof (t_atom));
}

void canvas_newPatch (void *dummy, t_symbol *name, t_symbol *directory)
{
    canvas_setActiveFileNameAndDirectory (name, directory);
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
    
    y->g_next = NULL;
    
    if (!glist->gl_graphics) { glist->gl_graphics = y; }
    else {
        t_gobj *t = NULL; for (t = glist->gl_graphics; t->g_next; t = t->g_next) { }
        t->g_next = y;
    }
    
    if (glist->gl_editor && (object = cast_objectIfPatchable (y))) { boxtext_new (glist, object); }
    if (canvas_isMapped (canvas_getView (glist))) { gobj_visibilityChanged (y, glist, 1); }
    
    if (needToPaintScalars) { paint_scalarsRedrawAll(); }
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
        
    if (glist->gl_editor) {
    //
    if (glist->gl_editor->e_grabbed == y)   { glist->gl_editor->e_grabbed = NULL; }
    if (canvas_isObjectSelected (glist, y)) { canvas_deselectObject (glist, y);   }
    //
    }
    
    if (needToPaintScalars) { paint_scalarsEraseAll(); }
    
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
    if (needToPaintScalars)   { paint_scalarsDrawAll(); }
    
    canvas->gl_isDeleting = deletingState;
    
    glist->gl_uniqueIdentifier = utils_unique();        /* Invalidate pointers. */
}

void canvas_clear (t_glist *glist)
{
    int dspState = 0;
    int dspSuspended = 0;
    t_gobj *y = NULL;
        
    while (y = glist->gl_graphics) {
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
    int argc = e->ce_argc;
    t_atom *argv = e->ce_argv;
    t_object *x = NULL;
    
    buffer_eval (b, &pd_objectMaker, argc, argv);

    if (pd_newest) { x = cast_objectIfPatchable (pd_newest); }

    if (!x) {
        x = (t_object *)pd_new (text_class);    /* Create a dummy box. */
        if (buffer_size (b)) {
            error_canNotMake (buffer_size (b), buffer_atoms (b)); 
        }
    }

    x->te_buffer        = b;
    x->te_xCoordinate   = positionX;
    x->te_yCoordinate   = positionY;
    x->te_width         = width;
    x->te_type          = TYPE_OBJECT;
    
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

void canvas_setAsGraphOnParent (t_glist *glist, int flags)
{
    int isGraphOnParent = (flags & 1) != 0;
    int hideText        = (flags & 2) != 0;
    int needToUpdate    = isGraphOnParent || (!isGraphOnParent && glist->gl_isGraphOnParent);
    
    if (needToUpdate) {
        if (!glist->gl_isLoading && glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
        }
    }
    
    glist->gl_hideText = hideText;
    
    if (!isGraphOnParent) { glist->gl_isGraphOnParent = 0; } 
    else {
        glist->gl_isGraphOnParent = 1;
    }
    
    if (glist->gl_graphWidth <= 0)  { glist->gl_graphWidth  = GRAPH_DEFAULT_WIDTH;  }
    if (glist->gl_graphHeight <= 0) { glist->gl_graphHeight = GRAPH_DEFAULT_HEIGHT; }
        
    if (needToUpdate) {
        if (!glist->gl_isLoading && glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 1);
            canvas_updateLinesByObject (glist->gl_parent, cast_object (glist));
        }
    }
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
    const char *directory = glist ? canvas_getEnvironment (glist)->ce_directory->s_name : ".";
    
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
    
    char *directory = canvas_getEnvironment (glist)->ce_directory->s_name;
    
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
    sys_vGui ("::ui_patch::setTitle .x%lx {%s} {%s} %d\n",
                    glist,
                    canvas_getEnvironment (glist)->ce_directory->s_name,
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
    static t_glist *lastGlist = NULL;           /* Shared. */
    static int lastType = CURSOR_NOTHING;       /* Shared. */
    static char *cursors[] =                    /* Shared. */
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

t_gobj *canvas_getHitObject (t_glist *glist, 
    int positionX,
    int positionY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    t_gobj *y = NULL;
    t_gobj *object = NULL;
    
    int x1, y1, x2, y2;
    
    *a = *b = *c = *d = 0;
    
    if (glist->gl_editor && canvas_getNumberOfSelectedObjects (glist) > 1) {
    //
    t_selection *selection = NULL;
    for (selection = glist->gl_editor->e_selectedObjects; selection; selection = selection->sel_next) {
    //
    if (gobj_hit (selection->sel_what, glist, positionX, positionY, &x1, &y1, &x2, &y2)) {
        *a = x1; *b = y1; *c = x2; *d = y2;
        object = selection->sel_what; 
    }
    //
    }
    //
    }
    
    if (!object) {
    //
    int t = -PD_INT_MAX;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (gobj_hit (y, glist, positionX, positionY, &x1, &y1, &x2, &y2)) {
            if (x1 > t) {
                *a = x1; *b = y1; *c = x2; *d = y2;
                object = y; t = x1;
            }
        }
    }
    //
    }

    return object;
}

int canvas_hasLine (t_glist *glist, t_object *objectOut, int m, t_object *objectIn, int n)
{
    t_linetraverser t;
    t_outconnect *connection = NULL;
    
    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
        if (t.tr_srcObject == objectOut && t.tr_destObject == objectIn) {
            if (t.tr_srcIndexOfOutlet == m && t.tr_destIndexOfInlet == n) {
                return 1;
            }
        }
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
        *a = GRAPH_DEFAULT_X; *b = GRAPH_DEFAULT_Y;
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
#pragma mark -

void canvas_traverseLinesStart (t_linetraverser *t, t_glist *glist)
{
    t->tr_owner             = glist;
    t->tr_connectionCached  = NULL;
    t->tr_srcObject         = NULL;
    
    t->tr_srcIndexOfNextOutlet = t->tr_srcNumberOfOutlets = 0;
}

/* Get the lines outlet per outlet, object per object. */
/* Coordinates are set at the same time. */

t_outconnect *canvas_traverseLinesNext (t_linetraverser *t)
{
    t_outconnect *connection = t->tr_connectionCached;
    
    while (!connection) {
    //
    int n = t->tr_srcIndexOfNextOutlet;
    
    while (n == t->tr_srcNumberOfOutlets) {
    //
    t_gobj   *y = NULL;
    t_object *o = NULL;
    
    if (!t->tr_srcObject) { y = t->tr_owner->gl_graphics; }
    else {
        y = cast_gobj (t->tr_srcObject)->g_next;
    }
    
    for (; y; y = y->g_next) {
        if ((o = cast_objectIfPatchable (y))) { break; }     /* Only box objects are considered. */
    }
    
    if (!o) { return NULL; }
    
    t->tr_srcObject          = o;
    t->tr_srcNumberOfOutlets = object_numberOfOutlets (o);
    n = 0;
    
    if (canvas_isMapped (t->tr_owner)) {
    
        gobj_getRectangle (y, t->tr_owner,
            &t->tr_srcTopLeftX,
            &t->tr_srcTopLeftY,
            &t->tr_srcBottomRightX,
            &t->tr_srcBottomRightY);
            
    } else {
        t->tr_srcTopLeftX = t->tr_srcTopLeftY = t->tr_srcBottomRightX = t->tr_srcBottomRightY = 0;
    }
    //
    }
    
    t->tr_srcIndexOfOutlet     = n;
    t->tr_srcIndexOfNextOutlet = n + 1;
    connection = object_traverseOutletStart (t->tr_srcObject, &t->tr_srcOutlet, n);
    //
    }
    
    t->tr_connectionCached = object_traverseOutletNext (connection,
        &t->tr_destObject,
        &t->tr_destInlet,
        &t->tr_destIndexOfInlet);
                                                            
    t->tr_destNumberOfInlets = object_numberOfInlets (t->tr_destObject);
    
    PD_ASSERT (t->tr_destNumberOfInlets);
    
    if (canvas_isMapped (t->tr_owner)) {

        gobj_getRectangle (cast_gobj (t->tr_destObject), t->tr_owner,
            &t->tr_destTopLeftX,
            &t->tr_destTopLeftY,
            &t->tr_destBottomRightX,
            &t->tr_destBottomRightY);
        
        {
            int w = t->tr_srcBottomRightX - t->tr_srcTopLeftX;
            int i = t->tr_srcIndexOfOutlet;
            int j = t->tr_srcNumberOfOutlets;
        
            t->tr_lineStartX = t->tr_srcTopLeftX + INLET_MIDDLE (w, i, j);
            t->tr_lineStartY = t->tr_srcBottomRightY;
        }
        {
            int w = t->tr_destBottomRightX - t->tr_destTopLeftX;
            int i = t->tr_destIndexOfInlet;
            int j = t->tr_destNumberOfInlets;
        
            t->tr_lineEndX = t->tr_destTopLeftX + INLET_MIDDLE (w, i, j);
            t->tr_lineEndY = t->tr_destTopLeftY;
        }
        
    } else {
        t->tr_lineStartX   = t->tr_lineStartY   = t->tr_lineEndX         = t->tr_lineEndY         = 0;
        t->tr_destTopLeftX = t->tr_destTopLeftY = t->tr_destBottomRightX = t->tr_destBottomRightY = 0;
    }
    
    return connection;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_initialize (void)
{
}

void canvas_release (void)      /* Close remaining (i.e. NOT dirty) and invisible patches. */
{    
    while (1) {
    //
    t_glist *glist = pd_this->pd_roots;
    
    if (glist == NULL) { break; }
    else {
        pd_free (cast_pd (glist));
        if (glist == pd_this->pd_roots) { PD_BUG; break; }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
