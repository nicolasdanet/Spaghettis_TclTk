
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *text_class;
extern t_class  *garray_class;
extern t_class  *canvas_class;
extern t_glist  *editor_pasteCurrentCanvas;

extern int      editor_pasteOffsetWhileConnectingObjects;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd                 pd_canvasMaker;
extern t_widgetbehavior     canvas_widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *canvas_class;                              /* Shared. */
t_symbol        *canvas_fileName  = &s_;                    /* Shared. */
t_symbol        *canvas_directory = &s_;                    /* Shared. */
t_atom          *canvas_argv;                               /* Shared. */

int             canvas_argc;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_loadbangAbstractions (t_glist *glist)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == canvas_class) {
            if (canvas_isAbstraction (cast_glist (y))) { canvas_loadbang (cast_glist (y)); }
            else {
                canvas_loadbangAbstractions (cast_glist (y));
            }
        }
    }
}

static void canvas_loadbangSubpatches (t_glist *glist)
{
    t_gobj   *y = NULL;
    t_symbol *s = sym_loadbang;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == canvas_class) {
            if (!canvas_isAbstraction (cast_glist (y))) { canvas_loadbangSubpatches (cast_glist (y)); }
        }
    }
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if ((pd_class (y) != canvas_class) && class_hasMethod (pd_class (y), s)) {
            pd_vMessage (cast_pd (y), s, "");
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *subpatch_new (t_symbol *s)
{
    t_atom a[6];
    t_glist *x = NULL;
    t_glist *z = canvas_getCurrent();
    
    if (!utils_isNameAllowedForWindow (s)) { warning_badName (sym_pd, s); }
    
    if (s == &s_) { s = sym_Patch; }
    
    SET_FLOAT  (a + 0, 0.0);
    SET_FLOAT  (a + 1, WINDOW_HEADER);
    SET_FLOAT  (a + 2, WINDOW_WIDTH);
    SET_FLOAT  (a + 3, WINDOW_HEIGHT);
    SET_SYMBOL (a + 4, s);
    SET_FLOAT  (a + 5, 1.0);
    
    x = canvas_new (NULL, NULL, 6, a);
    x->gl_parent = z;
    
    canvas_pop (x, 1);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_coords (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    glist->gl_valueLeft         = atom_getFloatAtIndex (0, argc, argv);
    glist->gl_valueTop          = atom_getFloatAtIndex (1, argc, argv);
    glist->gl_valueRight        = atom_getFloatAtIndex (2, argc, argv);
    glist->gl_valueBottom       = atom_getFloatAtIndex (3, argc, argv);
    glist->gl_graphWidth        = (int)atom_getFloatAtIndex (4, argc, argv);
    glist->gl_graphHeight       = (int)atom_getFloatAtIndex (5, argc, argv);
    glist->gl_graphMarginLeft   = (int)atom_getFloatAtIndex (7, argc, argv);
    glist->gl_graphMarginTop    = (int)atom_getFloatAtIndex (8, argc, argv);
    
    canvas_setAsGraphOnParent (glist, (int)atom_getFloatAtIndex (6, argc, argv));
}

void canvas_restore (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_pd *z = NULL;
    
    t_symbol *name = &s_;
    
    if (argc > 3) { name = dollar_expandGetIfSymbolByEnvironment (argv + 3, canvas_getCurrent()); }
    
    canvas_setName (glist, (name == &s_ ? sym_Patch : name));
    
    canvas_pop (glist, glist->gl_openedAtLoad);

    if (!(z = pd_getBoundX()) || (pd_class (z) != canvas_class)) { PD_BUG; }
    else {
        t_glist *parent  = cast_glist (z);
        t_object *object = cast_object (glist);
        
        glist->gl_parent        = parent;
        object->te_width        = 0;                           
        object->te_type         = TYPE_OBJECT;
        object->te_buffer       = buffer_new();
        object->te_xCoordinate  = atom_getFloatAtIndex (0, argc, argv);
        object->te_yCoordinate  = atom_getFloatAtIndex (1, argc, argv);
        
        if (argc > 2) { buffer_deserialize (object->te_buffer, argc - 2, argv + 2); }
        
        canvas_addObject (parent, cast_gobj (object));
    }
}

static void canvas_width (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    t_object *o = NULL;
    
    for (g1 = glist->gl_graphics; g2 = g1->g_next; g1 = g2) { }
    
    if (o = canvas_castToObjectIfPatchable (g1)) {
    //
    int w = atom_getFloatAtIndex (0, argc, argv);
    
    o->te_width = PD_MAX (1, w);
    
    if (canvas_isMapped (glist)) {
        gobj_visibilityChanged (g1, glist, 0);
        gobj_visibilityChanged (g1, glist, 1);
    }
    //
    }
    //
    }
}

void canvas_connect (t_glist *glist,
    t_float indexOfObjectOut,
    t_float indexOfOutlet,
    t_float indexOfObjectIn,
    t_float indexOfOInlet)
{
    int k = (editor_pasteCurrentCanvas == glist) ? editor_pasteOffsetWhileConnectingObjects : 0;
    
    t_gobj *src  = canvas_getObjectAtIndex (glist, k + (int)indexOfObjectOut);
    t_gobj *dest = canvas_getObjectAtIndex (glist, k + (int)indexOfObjectIn);
    t_object *srcObject  = canvas_castToObjectIfPatchable (src);
    t_object *destObject = canvas_castToObjectIfPatchable (dest);
    
    if (srcObject && destObject) {
    //
    int m = (int)indexOfOutlet;
    int n = (int)indexOfOInlet;
    t_outconnect *connection = NULL;
    
    /* Creates dummy outlets and inlets (failure at object creation). */
    
    if (pd_class (srcObject) == text_class && srcObject->te_type == TYPE_OBJECT) {
        while (m >= object_numberOfOutlets (srcObject)) {
            outlet_new (srcObject, NULL);
        }
    }
    
    if (pd_class (destObject) == text_class && destObject->te_type == TYPE_OBJECT) {
        while (n >= object_numberOfInlets (destObject)) {
            inlet_new (destObject, cast_pd (destObject), NULL, NULL);
        }
    }

    if ((connection = object_connect (srcObject, m, destObject, n))) {
    //
    if (canvas_isMapped (glist)) {
    
        sys_vGui (".x%lx.c create line %d %d %d %d -width %d -tags %lxLINE\n",
                        canvas_getView (glist),
                        0,
                        0,
                        0,
                        0,
                        (object_isSignalOutlet (srcObject, m) ? 2 : 1), connection);
                        
        canvas_updateLinesByObject (glist, srcObject);
    }
    
    return;
    //
    }
    //
    }

    error_failed (sym_connect);
}

void canvas_disconnect (t_glist *glist,
    t_float indexOfObjectOut, 
    t_float indexOfOutlet, 
    t_float indexOfObjectIn, 
    t_float indexOfInlet)
{
    t_outconnect *connection = NULL;
    t_linetraverser t;
        
    canvas_traverseLinesStart (&t, glist);
    
    while (connection = canvas_traverseLinesNext (&t)) {
    //
    if ((t.tr_srcIndexOfOutlet == (int)indexOfOutlet) && (t.tr_destIndexOfInlet == (int)indexOfInlet)) {
    
    int m = (canvas_getIndexOfObject (glist, cast_gobj (t.tr_srcObject)) == indexOfObjectOut);
    int n = (canvas_getIndexOfObject (glist, cast_gobj (t.tr_destObject)) == indexOfObjectIn);

    if (m && n) {
        sys_vGui (".x%lx.c delete %lxLINE\n", canvas_getView (glist), connection);
        object_disconnect (t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
        break;
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_requireArray (t_glist *glist)
{
    char t[PD_STRING] = { 0 };
    t_symbol *s = utils_getDefaultBindName (garray_class, sym_array);
    t_error err = string_sprintf (t, PD_STRING, "::ui_array::show %%s %s 100 3\n", s->s_name);
    
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (glist), (void *)glist, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Messy ping-pong required in order to check saving sequentially. */

void canvas_close (t_glist *glist, t_float f)
{
    int k = (int)f;
    
    if (k == 2) { canvas_dirty (glist, 0); global_shouldQuit (NULL); }      /* While quitting application. */
    else {
    //
    if (glist->gl_parent) { canvas_visible (glist, 0); }    /* Hide subpatches and abstractions. */
    else {
    //
    if (k == 1 || k == 3) {                                                 /* Has been saved right before. */
        pd_free (cast_pd (glist)); if (k == 3) { global_shouldQuit (NULL); }  
        
    } else {
        if (canvas_isDirty (glist)) {
            
            sys_vGui ("::ui_confirm::checkClose .x%lx"
                            " { ::ui_interface::pdsend $top save 1 }"
                            " { ::ui_interface::pdsend $top close 1 }"
                            " {}\n",
                            glist);
            return;
            
        } else {
            pd_free (cast_pd (glist));
        }
    }
    //
    }
    //
    }
}

static void canvas_open (t_glist *glist)
{
    /* Opening a graph on parent in its own window. */
    
    if (canvas_isMapped (glist) && !canvas_canHaveWindow (glist)) {
    //
    PD_ASSERT (glist->gl_parent);
    
    gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
    
    canvas_destroyEditorIfAny (glist);

    glist->gl_hasWindow = 1;    /* Note that it modifies how things are drawn below. */
    
    gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 1);
    //
    }
    
    canvas_visible (glist, 1);
}

void canvas_loadbang (t_glist *glist)
{
    canvas_loadbangAbstractions (glist);
    canvas_loadbangSubpatches (glist);
}

void canvas_dirty (t_glist *glist, t_float f)
{
    int isDirty = (f != 0.0);
        
    t_glist *y = canvas_getRoot (glist);
        
    if (y->gl_isDirty != isDirty) { 
        y->gl_isDirty = isDirty; if (y->gl_hasWindow) { canvas_updateTitle (y); }
    }
}

void canvas_visible (t_glist *glist, t_float f)
{
    int isVisible = (f != 0.0);
    
    if (isVisible) {

        if (glist->gl_editor && glist->gl_hasWindow) { sys_vGui ("::bringToFront .x%lx\n", glist); }
        else {
            canvas_createEditorIfNone (glist);
            
            sys_vGui ("::ui_patch::create .x%lx %d %d +%d+%d %d\n",
                            glist,
                            (int)(glist->gl_windowBottomRightX - glist->gl_windowTopLeftX),
                            (int)(glist->gl_windowBottomRightY - glist->gl_windowTopLeftY),
                            (int)(glist->gl_windowTopLeftX),
                            (int)(glist->gl_windowTopLeftY),
                            glist->gl_isEditMode);
                        
            canvas_updateTitle (glist);
            
            glist->gl_hasWindow = 1;
        }
        
    } else {

        if (!glist->gl_hasWindow) { canvas_destroyEditorIfAny (glist); }
        else {
            t_glist *t = NULL;
            
            canvas_deselectAll (glist);
            if (canvas_isMapped (glist)) { canvas_map (glist, 0); }
            canvas_destroyEditorIfAny (glist);
            sys_vGui ("destroy .x%lx\n", glist);
            
            if (glist->gl_isGraphOnParent && (t = glist->gl_parent) && (!t->gl_isDeleting)) {
                if (canvas_isMapped (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 0); }
                glist->gl_hasWindow = 0;
                if (canvas_isMapped (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 1); }
            } else {
                glist->gl_hasWindow = 0;
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_map (t_glist *glist, t_float f)
{
    int isMapped = (f != 0.0);

    if (isMapped != canvas_isMapped (glist)) {
    //
    if (!isMapped) { sys_vGui (".x%lx.c delete all\n", canvas_getView (glist)); glist->gl_isMapped = 0; }
    else {
    
        t_gobj *y = NULL;
        t_selection *selection = NULL;
        
        if (!glist->gl_hasWindow) { PD_BUG; canvas_visible (glist, 1); }
        for (y = glist->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, glist, 1); }
        for (selection = glist->gl_editor->e_selectedObjects; selection; selection = selection->sel_next) {
            gobj_selected (selection->sel_what, glist, 1);
        }

        glist->gl_isMapped = 1;
        
        canvas_drawLines (glist);
        canvas_drawGraphOnParentRectangle (glist);
    }
    //
    }
}

void canvas_pop (t_glist *glist, t_float f)
{
    if (f != 0.0) { canvas_visible (glist, 1); }
    
    stack_pop (cast_pd (glist));
    
    canvas_resortInlets (glist);
    canvas_resortOutlets (glist);
    
    glist->gl_isLoading = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_rename (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *name = &s_;
    
    if (argc) {
    //
    name = dollar_expandGetIfSymbolByEnvironment (argv, glist);
    if (name != &s_) { argc--; argv++; }
    if (argc) { warning_unusedArguments (class_getName (pd_class (glist)), argc, argv); }
    //
    }
    
    if (!utils_isNameAllowedForWindow (name)) { warning_badName (sym_pd, name); }
    
    canvas_setName (glist, (name == &s_ ? sym_Patch : name));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_functionSave (t_gobj *x, t_buffer *b)
{
    int needToSaveContents = 1;
    
    if (canvas_isAbstraction (cast_glist (x))) { needToSaveContents = 0; }
    else {                                                             
        if (utils_getFirstAtomOfObjectAsSymbol (cast_object (x)) == sym_table) { needToSaveContents = 0; }
    }

    if (needToSaveContents) { 
        canvas_serialize (cast_glist (x), b);
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_restore,
            cast_object (x)->te_xCoordinate,
            cast_object (x)->te_yCoordinate);
    } else {
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_obj,
            cast_object (x)->te_xCoordinate,
            cast_object (x)->te_yCoordinate);
    }
    
    buffer_serialize (b, cast_object (x)->te_buffer);
    buffer_appendSemicolon (b);
    object_saveWidth (cast_object (x), b);
}

static void canvas_functionProperties (t_gobj *x, t_glist *dummy)
{
    t_gobj *y = NULL;
    t_glist *g = cast_glist (x);
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    
    if (g->gl_isGraphOnParent) {
        err = string_sprintf (t, PD_STRING, "::ui_canvas::show %%s %g %g %d %g %g %g %g %d %d %d %d\n",
                                    0.,
                                    0.,
                                    g->gl_isGraphOnParent,
                                    g->gl_valueLeft,
                                    g->gl_valueTop,
                                    g->gl_valueRight,
                                    g->gl_valueBottom, 
                                    g->gl_graphWidth,
                                    g->gl_graphHeight,
                                    g->gl_graphMarginLeft,
                                    g->gl_graphMarginTop);
    } else {
        err = string_sprintf (t, PD_STRING, "::ui_canvas::show %%s %g %g %d %g %g %g %g %d %d %d %d\n",
                                    canvas_valueForOnePixelX (g),
                                    canvas_valueForOnePixelY (g),
                                    g->gl_isGraphOnParent,
                                    0.,
                                    1.,
                                    1.,
                                   -1., 
                                    g->gl_graphWidth, 
                                    g->gl_graphHeight,
                                    g->gl_graphMarginLeft,
                                    g->gl_graphMarginTop);
    }
    
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (g), (void *)g, t);

    for (y = g->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == garray_class) { garray_functionProperties ((t_garray *)y); }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_fromPopupDialog (t_glist *glist, t_float action, t_float positionX, t_float positionY)
{
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    int x1, y1, x2, y2;
    
    if (gobj_hit (y, glist, positionX, positionY, &x1, &y1, &x2, &y2)) {
    //
    if (action == 0) {                                                                  /* Properties. */
        if (class_hasPropertiesFunction (pd_class (y))) {
            (*class_getPropertiesFunction (pd_class (y))) (y, glist); return;
        }
    } 
    if (action == 1) {                                                                  /* Open. */
        if (class_hasMethod (pd_class (y), sym_open)) {
            pd_vMessage (cast_pd (y), sym_open, ""); return;
        }
    }
    if (action == 2) {                                                                  /* Help. */
    //
    char *directory = NULL;
    char name[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    if (pd_class (y) == canvas_class && canvas_isRoot (cast_glist (y))) {
        int argc = buffer_size (cast_object (y)->te_buffer);
        t_atom *argv = buffer_atoms (cast_object (y)->te_buffer);
        if (!(err = (argc < 1))) {
            atom_toString (argv, name, PD_STRING);
            directory = canvas_getEnvironment (cast_glist (y))->ce_directory->s_name;
        }
        
    } else {
        err = string_copy (name, PD_STRING, class_getHelpNameAsString (pd_class (y)));
        directory = class_getExternalDirectoryAsString (pd_class (y));
    }

    if (!err) { file_openHelp (directory, name); }
    
    return;
    //
    }
    //
    }
    //
    }
    
    if (action == 0) { canvas_functionProperties (cast_gobj (glist), NULL); }
}

static void canvas_fromDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    if (glist->gl_parent) {
        
        /* Activated text box triggering recreation. */
        
        err = canvas_deselectObjectIfSelected (glist->gl_parent, cast_gobj (glist));
    }
    
    PD_ASSERT (argc == 11);
        
    if (!err) {
    //
    t_float scaleX  = atom_getFloatAtIndex (0, argc, argv);
    t_float scaleY  = atom_getFloatAtIndex (1, argc, argv);
    t_float start   = atom_getFloatAtIndex (3, argc, argv);
    t_float up      = atom_getFloatAtIndex (4, argc, argv);
    t_float end     = atom_getFloatAtIndex (5, argc, argv);
    t_float down    = atom_getFloatAtIndex (6, argc, argv);
    int flags       = (int)atom_getFloatAtIndex (2, argc,  argv);
    int width       = (int)atom_getFloatAtIndex (7, argc,  argv);
    int height      = (int)atom_getFloatAtIndex (8, argc,  argv);
    int marginX     = (int)atom_getFloatAtIndex (9, argc,  argv);
    int marginY     = (int)atom_getFloatAtIndex (10, argc, argv);
    
    glist->gl_graphWidth        = width;
    glist->gl_graphHeight       = height;
    glist->gl_graphMarginLeft   = marginX;
    glist->gl_graphMarginTop    = marginY;

    if (scaleX == 0.0) { scaleX = 1.0; }
    if (scaleY == 0.0) { scaleY = 1.0; }

    if (flags & 1) {    /* Graph on parent. */
    
        PD_ASSERT (start != end);
        PD_ASSERT (up != down);
        
        glist->gl_valueLeft     = start;
        glist->gl_valueRight    = end; 
        glist->gl_valueTop      = up; 
        glist->gl_valueBottom   = down;
        
    } else {
    
        glist->gl_valueLeft     = 0.0;
        glist->gl_valueRight    = PD_ABS (scaleX);
        glist->gl_valueTop      = 0.0;
        glist->gl_valueBottom   = PD_ABS (scaleY);
    }
    
    canvas_setAsGraphOnParent (glist, flags);
    
    canvas_dirty (glist, 1);
    
    if (glist->gl_hasWindow) { canvas_redraw (glist); }
    else {
        if (canvas_isMapped (glist->gl_parent)) {
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
            gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 1);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Create a graph on parent fit for arrays. */

t_glist *canvas_newGraphOnParent (t_glist *glist,
    t_float valueStart,
    t_float valueUp,
    t_float valueEnd,
    t_float valueDown,
    t_float topLeftX,
    t_float topLeftY,
    t_float bottomRightX,
    t_float bottomRightY)
{
    t_glist *x = (t_glist *)pd_new (canvas_class);
    
    t_fontsize fontSize = canvas_getCurrent() ? canvas_getCurrent()->gl_fontSize : font_getDefaultFontSize();
    
    if (valueStart >= valueEnd || valueUp == valueDown) {
    //
    valueStart  = GRAPH_DEFAULT_START;
    valueEnd    = GRAPH_DEFAULT_END;
    valueUp     = GRAPH_DEFAULT_UP;
    valueDown   = GRAPH_DEFAULT_DOWN;
    //
    }
    
    if (topLeftX >= bottomRightX || topLeftY >= bottomRightY) {
    //
    topLeftX     = GRAPH_DEFAULT_X;
    topLeftY     = GRAPH_DEFAULT_Y;
    bottomRightX = topLeftX + GRAPH_DEFAULT_WIDTH;
    bottomRightY = topLeftY + GRAPH_DEFAULT_HEIGHT;
    //
    }
        
    cast_object (x)->te_buffer          = buffer_new();
    cast_object (x)->te_xCoordinate     = topLeftX;
    cast_object (x)->te_yCoordinate     = topLeftY;
    cast_object (x)->te_type            = TYPE_OBJECT;
    x->gl_master                        = gpointer_masterCreateWithGlist (x);
    x->gl_parent                        = glist;
    x->gl_name                          = utils_getDefaultBindName (canvas_class, sym__graph);
    x->gl_uniqueIdentifier              = utils_unique();
    x->gl_graphWidth                    = bottomRightX - topLeftX;
    x->gl_graphHeight                   = bottomRightY - topLeftY;
    x->gl_graphMarginLeft               = 0;
    x->gl_graphMarginTop                = 0;
    x->gl_valueLeft                     = valueStart;
    x->gl_valueRight                    = valueEnd;
    x->gl_valueTop                      = valueUp;
    x->gl_valueBottom                   = valueDown;
    x->gl_windowTopLeftX                = 0;
    x->gl_windowTopLeftY                = WINDOW_HEADER;
    x->gl_windowBottomRightX            = WINDOW_WIDTH;
    x->gl_windowBottomRightY            = WINDOW_HEIGHT + WINDOW_HEADER;
    x->gl_fontSize                      = fontSize;
    x->gl_isGraphOnParent               = 1;
    
    canvas_bind (x);
    
    buffer_vAppend (cast_object (x)->te_buffer, "s", sym_graph);
    
    canvas_addObject (glist, cast_gobj (x));
    
    return x;
}

t_glist *canvas_new (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    t_glist *x          = (t_glist *)pd_new (canvas_class);
    t_glist *owner      = canvas_getCurrent();
    t_symbol *name      = &s_;
    
    int visible         = 0;
    int width           = WINDOW_WIDTH;
    int height          = WINDOW_HEIGHT;
    int topLeftX        = 0;
    int topLeftY        = WINDOW_HEADER;
    t_fontsize fontSize = (owner ? owner->gl_fontSize : font_getDefaultFontSize());
    
    /* Top. */
    
    if (argc == 5) {
                                                  
        topLeftX    = (int)atom_getFloatAtIndex (0, argc, argv);
        topLeftY    = (int)atom_getFloatAtIndex (1, argc, argv);
        width       = (int)atom_getFloatAtIndex (2, argc, argv);
        height      = (int)atom_getFloatAtIndex (3, argc, argv);
        fontSize    = (int)atom_getFloatAtIndex (4, argc, argv);
    }
    
    /* Subpatch. */
    
    if (argc == 6)  {
    
        topLeftX    = (int)atom_getFloatAtIndex (0, argc, argv);
        topLeftY    = (int)atom_getFloatAtIndex (1, argc, argv);
        width       = (int)atom_getFloatAtIndex (2, argc, argv);
        height      = (int)atom_getFloatAtIndex (3, argc, argv);
        name        = atom_getSymbolAtIndex (4, argc, argv);
        visible     = (int)atom_getFloatAtIndex (5, argc, argv);
    }

    topLeftX = PD_MAX (topLeftX, 0);
    topLeftY = PD_MAX (topLeftY, WINDOW_HEADER);
    
    cast_object (x)->te_type = TYPE_OBJECT;
    
    x->gl_master = gpointer_masterCreateWithGlist (x);
    x->gl_parent = owner;
    x->gl_name   = (name != &s_ ? name : (canvas_fileName ? canvas_fileName : sym_Patch));
    
    x->gl_uniqueIdentifier = utils_unique();
    
    if (!owner) { instance_addToRoots (x); }
    
    if (canvas_directory == &s_) { x->gl_environment = NULL; }
    else {
    //
    static int dollarZero = 1000;       /* Shared. */
    
    x->gl_environment = (t_environment *)PD_MEMORY_GET (sizeof (t_environment));

    if (!canvas_argv) { 
        canvas_argv = PD_MEMORY_GET (0); 
    }
    
    x->gl_environment->ce_directory         = canvas_directory;
    x->gl_environment->ce_argc              = canvas_argc;
    x->gl_environment->ce_argv              = canvas_argv;
    x->gl_environment->ce_dollarZeroValue   = dollarZero++;

    canvas_directory    = &s_;
    canvas_argc         = 0;
    canvas_argv         = NULL;
    //
    }

    x->gl_valueLeft             = 0.0;
    x->gl_valueTop              = 0.0;
    x->gl_valueRight            = 1.0;
    x->gl_valueBottom           = 1.0;
    x->gl_windowTopLeftX        = topLeftX;
    x->gl_windowTopLeftY        = topLeftY;
    x->gl_windowBottomRightX    = topLeftX + width;
    x->gl_windowBottomRightY    = topLeftY + height;
    x->gl_fontSize              = font_getNearestValidFontSize (fontSize);
    x->gl_isLoading             = 1;
    x->gl_isEditMode            = 0;
    x->gl_openedAtLoad          = visible;
    
    canvas_bind (x);
    
    stack_push (cast_pd (x));
    
    return x;
}

void canvas_free (t_glist *glist)
{
    int dspstate = dsp_suspend();
    t_gobj *y = NULL;
        
    if (glist->gl_editor) { canvas_deselectAll (glist); }
    
    while (y = glist->gl_graphics) { canvas_removeObject (glist, y); }
    if (glist == canvas_getView (glist)) { canvas_visible (glist, 0); }
    
    canvas_destroyEditorIfAny (glist);
    canvas_unbind (glist);

    if (glist->gl_environment) {
        PD_MEMORY_FREE (glist->gl_environment->ce_argv);
        PD_MEMORY_FREE (glist->gl_environment);
    }
    
    dsp_resume (dspstate);
    
    gpointer_masterRelease (glist->gl_master);
    guistub_destroyWithKey ((void *)glist);
    
    if (!glist->gl_parent) { instance_removeFromRoots (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setup (void)
{
    t_class *c = NULL;
        
    /* Creator function by sending the "canvas" message to #N. */
            
    class_addMethod (pd_canvasMaker, (t_method)canvas_new, sym_canvas, A_GIMME, A_NULL);
    
    c = class_new (sym_canvas,
            NULL, 
            (t_method)canvas_free, 
            sizeof (t_glist), 
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);

    class_addCreator ((t_newmethod)subpatch_new, sym_pd, A_DEFSYMBOL, A_NULL);
    
    class_addKey (c, canvas_key);
    class_addClick (c, canvas_click);
    class_addMotion (c, canvas_motion);
    class_addMouse (c, canvas_mouse);
    class_addMouseUp (c, canvas_mouseUp);

    class_addMethod (c, (t_method)canvas_window,
        sym_window,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT, 
        A_FLOAT,
        A_NULL);
                                                
    class_addMethod (c, (t_method)canvas_coords,            sym_coords,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,           sym_restore,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_width,             sym_f,              A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_connect,
        sym_connect,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)canvas_disconnect,
        sym_disconnect,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
    
    class_addMethod (c, (t_method)canvas_makeObject,            sym_obj,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeMessage,           sym_msg,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeFloatAtom,         sym_floatatom,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSymbolAtom,        sym_symbolatom,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_comment,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeScalar,            sym_scalar,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeBang,              sym_bng,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_tgl,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVerticalSlider,    sym_vslider,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeHorizontalSlider,  sym_hslider,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeHorizontalRadio,   sym_hradio,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVerticalRadio,     sym_vradio,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vu,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_cnv,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_nbx,            A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_makeArray,
        sym_array,
        A_SYMBOL,
        A_FLOAT,
        A_SYMBOL,
        A_DEFFLOAT,
        A_NULL);
    
    class_addMethod (c, (t_method)canvas_makeGraphWithArray,
        sym__arraydialog,
        A_SYMBOL,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)canvas_requireArray,          sym__array,         A_NULL);

    class_addMethod (c, (t_method)canvas_editmode,              sym_editmode,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_close,          A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_open,                  sym_open,           A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,              sym_loadbang,       A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,                 sym_dirty,          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,               sym_visible,        A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_map,                   sym__map,           A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_pop,                   sym__pop,           A_DEFFLOAT, A_NULL);

    class_addMethod (c, (t_method)canvas_cut,                   sym__cut,           A_NULL);
    class_addMethod (c, (t_method)canvas_copy,                  sym__copy,          A_NULL);
    class_addMethod (c, (t_method)canvas_paste,                 sym__paste,         A_NULL);
    class_addMethod (c, (t_method)canvas_duplicate,             sym__duplicate,     A_NULL);
    class_addMethod (c, (t_method)canvas_selectAll,             sym__selectall,     A_NULL);
    
    class_addMethod (c, (t_method)canvas_clear,                 sym_clear,          A_NULL);
    class_addMethod (c, (t_method)canvas_dsp,                   sym_dsp,            A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_rename,                sym_rename,         A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_save,                  sym_save,           A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_saveas,         A_DEFFLOAT, A_NULL);

    class_addMethod (c, (t_method)canvas_saveToFile,
        sym_savetofile,
        A_SYMBOL,
        A_SYMBOL,
        A_DEFFLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)canvas_bounds,
        sym_bounds,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)canvas_fromPopupDialog,
        sym__popupdialog,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)canvas_fromDialog,
        sym__canvasdialog,
        A_GIMME,
        A_NULL);
   
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_open,          sym_menu__dash__open,  A_NULL);
    class_addMethod (c, (t_method)canvas_close,         sym_menuclose,         A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,   sym_text,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,    sym_toggle,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,        sym_vumeter,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,     sym_mycnv,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,      sym_numbox,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,       sym_vis,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,          sym_menusave,          A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,        sym_menusaveas,        A_DEFFLOAT, A_NULL);    
    class_addMethod (c, (t_method)canvas_requireArray,  sym_menuarray,         A_NULL);

    class_addCreator ((t_newmethod)subpatch_new, sym_page, A_DEFSYMBOL, A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &canvas_widgetbehavior);
    class_setSaveFunction (c, canvas_functionSave);
    class_setPropertiesFunction (c, canvas_functionProperties);

    canvas_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
