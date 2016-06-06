
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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define BASE_DEFAULT_X          40
#define BASE_DEFAULT_Y          40
#define BASE_DEFAULT_WIDTH      200
#define BASE_DEFAULT_HEIGHT     140

#define BASE_DEFAULT_START      0.0
#define BASE_DEFAULT_UP         1.0
#define BASE_DEFAULT_END        100.0
#define BASE_DEFAULT_DOWN      -1.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_class              *text_class;
extern t_class              *canvas_class;
extern t_class              *vinlet_class;
extern t_class              *voutlet_class;
extern t_class              *array_define_class;
extern t_class              *scalar_define_class;
extern t_pd                 *pd_newest;
extern t_symbol             *canvas_fileName;
extern t_symbol             *canvas_directory;
extern t_atom               *canvas_argv;

extern t_pd                 pd_objectMaker;
extern t_widgetbehavior     text_widgetBehavior;
extern int                  canvas_argc;
extern int                  canvas_magic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_glist              *canvas_lastCanvas;             /* Shared. */

static int                  canvas_lastCanvasX;             /* Shared. */
static int                  canvas_lastCanvasY;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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
    canvas_pop (cast_glist (s__X.s_thing), 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_castToGlistChecked (t_pd *x)
{
    t_class *c = pd_class (x);
    
    if (c == canvas_class || c == array_define_class || c == scalar_define_class) {
        return cast_glist (x);
    } else {
        return NULL;
    }
}

int canvas_objectIsBox (t_object *x)
{
    return (pd_class (x)->c_behavior == &text_widgetBehavior)
        || (canvas_castToGlistChecked (cast_pd (x)) && !(cast_glist (x)->gl_isGraphOnParent));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_getCurrent (void)
{
    return (cast_glist (pd_findByClass (&s__X, canvas_class)));
}

t_glist *canvas_getRoot (t_glist *glist)
{
    if (canvas_isRoot (glist)) { return glist; }
    else {
        return (canvas_getRoot (glist->gl_parent));
    }
}

t_environment *canvas_getEnvironment (t_glist *glist)
{
    PD_ASSERT (glist);
    
    while (!glist->gl_environment) { glist = glist->gl_parent; PD_ASSERT (glist); }
    
    return glist->gl_environment;
}

t_glist *canvas_getView (t_glist *glist)
{
    while (glist->gl_parent && !canvas_canHaveWindow (glist)) { glist = glist->gl_parent; }
    
    return glist;
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

int canvas_isDrawnOnParent (t_glist *glist)
{
    return (!canvas_canHaveWindow (glist) && glist->gl_parent);
}

int canvas_canHaveWindow (t_glist *glist)
{
    return (glist->gl_haveWindow || !glist->gl_isGraphOnParent);
}

int canvas_hasEnvironment (t_glist *glist)
{
    return (glist->gl_environment != NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_addGraph (t_glist *glist,
    t_symbol *name,
    t_float valueStart,
    t_float valueUp,
    t_float valueEnd,
    t_float valueDown,
    t_float topLeftX,
    t_float topLeftY,
    t_float bottomRightX,
    t_float bottomRightY)
{
    static int graphCount = 0;      /* Shared. */

    int createdFromMenu = 0;
    t_glist *x = (t_glist *)pd_new (canvas_class);
    
    int fontSize = (canvas_getCurrent() ? canvas_getCurrent()->gl_fontSize : font_getDefaultFontSize());
    
    PD_ASSERT (name);
    
    if (name == &s_) {
        char n[PD_STRING] = { 0 };
        string_sprintf (n, PD_STRING, "graph%d", ++graphCount);
        name = gensym (n);
        createdFromMenu = 1;
        
    } else {
        char *s = name->s_name;
        int n;
        if (!strncmp (s, "graph", 5) && (n = atoi (s + 5)) > graphCount) { graphCount = n; }
    }

    if (valueStart >= valueEnd || valueUp == valueDown) {
    //
    valueStart  = BASE_DEFAULT_START;
    valueEnd    = BASE_DEFAULT_END;
    valueUp     = BASE_DEFAULT_UP;
    valueDown   = BASE_DEFAULT_DOWN;
    //
    }
    
    if (topLeftX >= bottomRightX || topLeftY >= bottomRightY) {
    //
    topLeftX     = BASE_DEFAULT_X;
    topLeftY     = BASE_DEFAULT_Y;
    bottomRightX = topLeftX + BASE_DEFAULT_WIDTH;
    bottomRightY = topLeftY + BASE_DEFAULT_HEIGHT;
    //
    }
    
    cast_object (x)->te_buffer          = buffer_new();
    cast_object (x)->te_xCoordinate     = topLeftX;
    cast_object (x)->te_yCoordinate     = topLeftY;
    cast_object (x)->te_type            = TYPE_OBJECT;
    x->gl_stub                          = gstub_new (x, NULL);
    x->gl_parent                        = glist;
    x->gl_name                          = name;
    x->gl_magic                         = ++canvas_magic;
    x->gl_width                         = bottomRightX - topLeftX;
    x->gl_height                        = bottomRightY - topLeftY;
    x->gl_valueStart                    = valueStart;
    x->gl_valueEnd                      = valueEnd;
    x->gl_valueUp                       = valueUp;
    x->gl_valueDown                     = valueDown;
    x->gl_windowTopLeftX                = 0;
    x->gl_windowTopLeftY                = WINDOW_HEADER;
    x->gl_windowBottomRightX            = WINDOW_WIDTH;
    x->gl_windowBottomRightY            = WINDOW_HEIGHT + WINDOW_HEADER;
    x->gl_fontSize                      = fontSize;
    x->gl_isGraphOnParent               = 1;
    x->gl_hasRectangle                  = 0;
    
    canvas_bind (x);
    buffer_vAppend (cast_object (x)->te_buffer, "s", sym_graph);
    if (!createdFromMenu) { stack_push (cast_pd (x)); }
    glist_add (glist, cast_gobj (x));
    
    return x;
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

    if (pd_newest) { x = canvas_castToObjectIfPatchable (pd_newest); }

    if (!x) {
    //
    x = (t_object *)pd_new (text_class);    /* Create a dummy box. */
    
    if (buffer_size (b)) {                  /* Avoid unnecessary moanings. */
    //
    post_atoms (buffer_size (b), buffer_atoms (b)); post_error (PD_TRANSLATE ("... couldn't create"));
    //
    }
    //
    }

    x->te_buffer        = b;
    x->te_xCoordinate   = positionX;
    x->te_yCoordinate   = positionY;
    x->te_width         = width;
    x->te_type          = TYPE_OBJECT;
    
    glist_add (glist, cast_gobj (x));
    
    if (isSelected) {
    //
    canvas_selectObject (glist, cast_gobj (x));
    gobj_activate (cast_gobj (x), glist, 1);
    //
    }
    
    if (pd_class (x) == vinlet_class)  { canvas_resortinlets (canvas_getView (glist)); }
    if (pd_class (x) == voutlet_class) { canvas_resortoutlets (canvas_getView (glist)); }
    //
    }
    
    stack_pop (cast_pd (glist));
}

void canvas_setAsGraphOnParent (t_glist *glist, int flags, int hasRectangle)
{
    int isGraphOnParent = (flags & 1) != 0;
    int hideText        = (flags & 2) != 0;
    int needToUpdate    = isGraphOnParent || (!isGraphOnParent && glist->gl_isGraphOnParent);
    
    glist->gl_hideText  = hideText;
    
    if (needToUpdate) {
        if (!glist->gl_isLoading && glist->gl_parent && canvas_isMapped (glist->gl_parent)) {
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
        }
    }
    
    if (!isGraphOnParent) { glist->gl_isGraphOnParent = 0; } 
    else {
        if (glist->gl_width <= 0)  { glist->gl_width  = BASE_DEFAULT_WIDTH;  }
        if (glist->gl_height <= 0) { glist->gl_height = BASE_DEFAULT_HEIGHT; }

        glist->gl_isGraphOnParent = 1;
        glist->gl_hasRectangle = hasRectangle;
        
        if (hasRectangle && canvas_isMapped (glist)) { glist_redraw (glist); }
    }
    
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
    t_symbol *t = s;
    
    if (strchr (s->s_name, '$')) {
    //
    t_environment *environment = canvas_getEnvironment (glist);
    stack_push (cast_pd (glist));
    t = dollar_expandDollarSymbol (s, environment->ce_argc, environment->ce_argv);
    stack_pop (cast_pd (glist));
    //
    }

    return t;
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

void canvas_setName (t_glist *glist, t_symbol *name, t_symbol *directory)
{
    canvas_unbind (glist);
    glist->gl_name = name;
    canvas_bind (glist);
    
    if (glist->gl_haveWindow) { canvas_updateTitle (glist); }
    if (directory && directory != &s_) {
        canvas_getEnvironment (glist)->ce_directory = directory; 
    }
}

void canvas_updateTitle (t_glist *glist)
{
    sys_vGui ("::ui_patch::setTitle .x%lx {%s} {%s} %d\n",  // --
                    glist,
                    canvas_getEnvironment (glist)->ce_directory->s_name,
                    glist->gl_name->s_name,
                    glist->gl_isDirty);
}

t_symbol *canvas_makeBindSymbol (t_symbol *s)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    PD_ASSERT (s);
    err = string_sprintf (t, PD_STRING, "pd-%s", s->s_name);
    PD_ASSERT (!err);
    return (gensym (t));
}

int canvas_hasGraphOnParentTitle (t_glist *glist)
{
    if (glist->gl_hideText) { return 0; }
    else {
    //
    int argc     = (cast_object (glist)->te_buffer ? buffer_size (cast_object (glist)->te_buffer)  : 0);
    t_atom *argv = (cast_object (glist)->te_buffer ? buffer_atoms (cast_object (glist)->te_buffer) : NULL);
    return !(argc && IS_SYMBOL (argv) && GET_SYMBOL (argv) == sym_graph);
    //
    }
}

int canvas_getFontSize (t_glist *glist)
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
            "crosshair",            // CURSOR_THICKEN
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
        *a = BASE_DEFAULT_X; *b = BASE_DEFAULT_Y;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_bind (t_glist *glist)
{
    if (strcmp (glist->gl_name->s_name, PD_NAME_SHORT)) {
        pd_bind (cast_pd (glist), canvas_makeBindSymbol (glist->gl_name));
    }
}

void canvas_unbind (t_glist *glist)
{
    if (strcmp (glist->gl_name->s_name, PD_NAME_SHORT)) {
        pd_unbind (cast_pd (glist), canvas_makeBindSymbol (glist->gl_name));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_getIndexOfObject (t_glist *glist, t_gobj *object)
{
    t_gobj *t = NULL;
    int n = 0;
    for (t = glist->gl_graphics; t && t != object; t = t->g_next) { n++; }
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
        if ((o = canvas_castToObjectIfPatchable (y))) { break; }     /* Only box objects are considered. */
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
