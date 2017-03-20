
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

extern t_class  *text_class;
extern t_class  *garray_class;
extern t_class  *canvas_class;

extern t_paste  editor_paste;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior     canvas_widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *canvas_class;      /* Shared. */

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
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == canvas_class) {
            if (!canvas_isAbstraction (cast_glist (y))) { canvas_loadbangSubpatches (cast_glist (y)); }
        }
    }
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
        if ((pd_class (y) != canvas_class) && class_hasMethod (pd_class (y), sym_loadbang)) {
            pd_message (cast_pd (y), sym_loadbang, 0, NULL);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_setAsGraphOnParent (t_glist *glist, int flags)
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
        
    #if PD_WITH_LEGACY
    
    if (!isGraphOnParent) {
    
        t_float scaleX = canvas_valueForOnePixelX (glist);
        t_float scaleY = canvas_valueForOnePixelY (glist);
        
        bounds_set (&glist->gl_bounds, (t_float)0.0, (t_float)0.0, PD_ABS (scaleX), PD_ABS (scaleY));
    }
    
    #endif
    
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

static void *subpatch_new (t_symbol *s)
{
    t_atom a[6];
    t_glist *x = NULL;
    t_glist *z = canvas_getCurrent();
    
    if (!utils_isNameAllowedForWindow (s)) { warning_badName (sym_pd, s); }
    
    if (s == &s_) { s = sym_Patch; }
    
    SET_FLOAT  (a + 0, (t_float)0.0);
    SET_FLOAT  (a + 1, WINDOW_HEADER);
    SET_FLOAT  (a + 2, WINDOW_WIDTH);
    SET_FLOAT  (a + 3, WINDOW_HEIGHT);
    SET_SYMBOL (a + 4, s);
    SET_FLOAT  (a + 5, (t_float)1.0);
    
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
    int flags   = (int)atom_getFloatAtIndex (6, argc, argv);
    int a       = (int)atom_getFloatAtIndex (7, argc, argv);
    int b       = (int)atom_getFloatAtIndex (8, argc, argv);
    int width   = (int)atom_getFloatAtIndex (4, argc, argv);
    int height  = (int)atom_getFloatAtIndex (5, argc, argv);
    
    bounds_setByAtoms (&glist->gl_bounds, argc, argv);
    
    rectangle_setByWidthAndHeight (&glist->gl_geometryGraph, a, b, width, height);
    
    canvas_setAsGraphOnParent (glist, flags);
}

void canvas_restore (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *name = &s_;
    
    if (argc > 3) { name = dollar_expandGetIfSymbolByEnvironment (argv + 3, canvas_getCurrent()); }
    
    canvas_setName (glist, (name == &s_ ? sym_Patch : name));
    
    canvas_pop (glist, glist->gl_openedAtLoad);

    {
        t_glist *parent = instance_contextGetCurrent();
    
        PD_ASSERT (parent);
        PD_ABORT (!parent);     /* Hot to manage corrupted files? */
        
        glist->gl_parent = parent;
        
        object_setBuffer (cast_object (glist), buffer_new());
        object_setX (cast_object (glist), atom_getFloatAtIndex (0, argc, argv));
        object_setY (cast_object (glist), atom_getFloatAtIndex (1, argc, argv));
        object_setWidth (cast_object (glist), 0);
        object_setType (cast_object (glist), TYPE_OBJECT);
        
        if (argc > 2) { buffer_deserialize (object_getBuffer (cast_object (glist)), argc - 2, argv + 2); }
        
        canvas_addObject (parent, cast_gobj (glist));
    }
}

static void canvas_width (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    t_object *o = NULL;
    
    for ((g1 = glist->gl_graphics); (g2 = g1->g_next); (g1 = g2)) { }
    
    if ((o = cast_objectIfConnectable (g1))) {
    //
    int w = atom_getFloatAtIndex (0, argc, argv);
    
    object_setWidth (o, PD_MAX (1, w));
    
    if (canvas_isMapped (glist)) {
        gobj_visibilityChanged (g1, glist, 0);
        gobj_visibilityChanged (g1, glist, 1);
    }
    //
    }
    //
    }
}

void canvas_connect (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    int indexOfObjectOut = (int)atom_getFloat (argv + 0);
    int indexOfOutlet    = (int)atom_getFloat (argv + 1);
    int indexOfObjectIn  = (int)atom_getFloat (argv + 2);
    int indexOfOInlet    = (int)atom_getFloat (argv + 3);
    
    int k = (editor_paste.e_current == glist) ? editor_paste.e_offset : 0;
    
    t_gobj *src  = canvas_getObjectAtIndex (glist, k + indexOfObjectOut);
    t_gobj *dest = canvas_getObjectAtIndex (glist, k + indexOfObjectIn);
    t_object *srcObject  = cast_objectIfConnectable (src);
    t_object *destObject = cast_objectIfConnectable (dest);
    
    if (srcObject && destObject) {
    //
    int m = indexOfOutlet;
    int n = indexOfOInlet;
    t_outconnect *connection = NULL;
    
    /* Creates dummy outlets and inlets (failure at object creation). */
    
    if (pd_class (srcObject) == text_class && object_isObject (srcObject)) {
        while (m >= object_getNumberOfOutlets (srcObject)) {
            outlet_new (srcObject, NULL);
        }
    }
    
    if (pd_class (destObject) == text_class && object_isObject (destObject)) {
        while (n >= object_getNumberOfInlets (destObject)) {
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
    //
    }
    
    error_failed (sym_connect);
}

void canvas_disconnect (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    int indexOfObjectOut = (int)atom_getFloat (argv + 0);
    int indexOfOutlet    = (int)atom_getFloat (argv + 1);
    int indexOfObjectIn  = (int)atom_getFloat (argv + 2);
    int indexOfInlet     = (int)atom_getFloat (argv + 3);
    
    t_outconnect *connection = NULL;
    t_traverser t;
        
    traverser_start (&t, glist);
    
    while ((connection = traverser_next (&t))) {
    //
    if ((traverser_getIndexOfOutlet (&t) == indexOfOutlet)) {
        if ((traverser_getIndexOfInlet (&t) == indexOfInlet)) {

            int m = canvas_getIndexOfObject (glist, cast_gobj (traverser_getSource (&t)));
            int n = canvas_getIndexOfObject (glist, cast_gobj (traverser_getDestination (&t)));

            if (m == indexOfObjectOut && n == indexOfObjectIn) {
                sys_vGui (".x%lx.c delete %lxLINE\n", canvas_getView (glist), connection);
                traverser_disconnect (&t);
                break;
            }
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_requireArrayDialog (t_glist *glist)
{
    char t[PD_STRING] = { 0 };
    t_symbol *s = utils_getDefaultBindName (garray_class, sym_array);
    t_error err = string_sprintf (t, PD_STRING, "::ui_array::show %%s %s 100 3\n", s->s_name);
    
    PD_ASSERT (!err);
    
    stub_new (cast_pd (glist), (void *)glist, t);
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
                            " {}\n",    // --
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
            
            sys_vGui ("::ui_patch::create .x%lx %d %d +%d+%d %d\n",     // --
                            glist,
                            rectangle_getWidth (&glist->gl_geometryWindow),
                            rectangle_getHeight (&glist->gl_geometryWindow),
                            rectangle_getTopLeftX (&glist->gl_geometryWindow),
                            rectangle_getTopLeftY (&glist->gl_geometryWindow),
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
        t_selection *s = NULL;
        
        if (!glist->gl_hasWindow) { PD_BUG; canvas_visible (glist, 1); }
        for (y = glist->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, glist, 1); }
        for (s = glist->gl_editor->e_selectedObjects; s; s = selection_getNext (s)) {
            gobj_selected (selection_getObject (s), glist, 1);
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
    
    instance_stackPop (glist);
    
    canvas_resortInlets (glist);
    canvas_resortOutlets (glist);
    
    glist->gl_isLoading = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_remove (t_glist *glist, t_symbol *s)
{
    t_symbol *t = utils_makeTemplateIdentifier (s);
    
    if (!template_isPrivate (t)) {
    //
    t_template *template = template_findByIdentifier (t);
    
    if (template) {
        canvas_removeScalarsRecursive (glist, template); 
    }
    //
    }
}

void canvas_rename (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
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

    if (needToSaveContents) { 
        canvas_serialize (cast_glist (x), b);
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_restore,
            object_getX (cast_object (x)),
            object_getY (cast_object (x)));
    } else {
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_obj,
            object_getX (cast_object (x)),
            object_getY (cast_object (x)));
    }
    
    buffer_serialize (b, object_getBuffer (cast_object (x)));
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
                                    0.0,
                                    0.0,
                                    g->gl_isGraphOnParent,
                                    bounds_getLeft (&g->gl_bounds),
                                    bounds_getTop (&g->gl_bounds),
                                    bounds_getRight (&g->gl_bounds),
                                    bounds_getBottom (&g->gl_bounds), 
                                    rectangle_getWidth (&g->gl_geometryGraph),
                                    rectangle_getHeight (&g->gl_geometryGraph),
                                    rectangle_getTopLeftX (&g->gl_geometryGraph),
                                    rectangle_getTopLeftY (&g->gl_geometryGraph));
    } else {
        err = string_sprintf (t, PD_STRING, "::ui_canvas::show %%s %g %g %d %g %g %g %g %d %d %d %d\n",
                                    canvas_valueForOnePixelX (g),
                                    canvas_valueForOnePixelY (g),
                                    g->gl_isGraphOnParent,
                                    0.0,
                                    1.0,
                                    1.0,
                                   -1.0, 
                                    rectangle_getWidth (&g->gl_geometryGraph),
                                    rectangle_getHeight (&g->gl_geometryGraph),
                                    rectangle_getTopLeftX (&g->gl_geometryGraph),
                                    rectangle_getTopLeftY (&g->gl_geometryGraph));
    }
    
    PD_ASSERT (!err);
    
    stub_new (cast_pd (g), (void *)g, t);

    for (y = g->gl_graphics; y; y = y->g_next) {
        if (pd_class (y) == garray_class) { garray_functionProperties ((t_garray *)y); }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_fromPopupDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 3) {
    //
    int k = (int)atom_getFloat (argv + 0);
    int a = (int)atom_getFloat (argv + 1);
    int b = (int)atom_getFloat (argv + 2);
    
    t_gobj *y = NULL;
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_rectangle t;
    
    if (gobj_hit (y, glist, a, b, &t)) {
    //
    if (k == 0) {                                                                  /* Properties. */
        if (class_hasPropertiesFunction (pd_class (y))) {
            (*class_getPropertiesFunction (pd_class (y))) (y, glist); return;
        }
    } 
    if (k == 1) {                                                                  /* Open. */
        if (class_hasMethod (pd_class (y), sym_open)) {
            pd_message (cast_pd (y), sym_open, 0, NULL); return;
        }
    }
    if (k == 2) {                                                                  /* Help. */
    //
    char *directory = NULL;
    char name[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    if (pd_class (y) == canvas_class && canvas_isRoot (cast_glist (y))) {
        int ac = buffer_size (object_getBuffer (cast_object (y)));
        t_atom *av = buffer_atoms (object_getBuffer (cast_object (y)));
        if (!(err = (ac < 1))) {
            atom_toString (av, name, PD_STRING);
            directory = environment_getDirectoryAsString (canvas_getEnvironment (cast_glist (y)));
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
    
    if (k == 0) { canvas_functionProperties (cast_gobj (glist), NULL); }
    //
    }
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
    
    rectangle_setByWidthAndHeight (&glist->gl_geometryGraph, marginX, marginY, width, height);

    if (scaleX == 0.0) { scaleX = (t_float)1.0; }
    if (scaleY == 0.0) { scaleY = (t_float)1.0; }

    if (flags & 1) { bounds_set (&glist->gl_bounds, start, up, end, down); }
    else {
        bounds_set (&glist->gl_bounds, (t_float)0.0, (t_float)0.0, PD_ABS (scaleX), PD_ABS (scaleY));
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
    
    object_setType (cast_object (x), TYPE_OBJECT);
    
    x->gl_holder = gmaster_createWithGlist (x);
    x->gl_parent = owner;
    
    x->gl_uniqueIdentifier = utils_unique();
    
    if (!owner) { instance_rootsAdd (x); }
    
    x->gl_environment           = instance_environmentFetchIfAny();
    x->gl_name                  = (name != &s_ ? name : environment_getFileName (x->gl_environment));
    x->gl_fontSize              = font_getNearestValidFontSize (fontSize);
    x->gl_isLoading             = 1;
    x->gl_isEditMode            = 0;
    x->gl_openedAtLoad          = visible;
    
    bounds_set (&x->gl_bounds, (t_float)0.0, (t_float)0.0, (t_float)1.0, (t_float)1.0);
    rectangle_set (&x->gl_geometryWindow, topLeftX, topLeftY, topLeftX + width, topLeftY + height);
    
    canvas_bind (x);
    
    instance_stackPush (x);
    
    return x;
}

void canvas_free (t_glist *glist)
{
    int dspstate = dsp_suspend();
    t_gobj *y = NULL;
        
    if (glist->gl_editor) { canvas_deselectAll (glist); }
    
    while ((y = glist->gl_graphics)) { canvas_removeObject (glist, y); }
    if (glist == canvas_getView (glist)) { canvas_visible (glist, 0); }
    
    canvas_destroyEditorIfAny (glist);
    canvas_unbind (glist);

    environment_free (glist->gl_environment);
    
    dsp_resume (dspstate);
    
    gmaster_reset (glist->gl_holder);
    stub_destroyWithKey ((void *)glist);
    
    if (!glist->gl_parent) { instance_rootsRemove (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setup (void)
{
    t_class *c = NULL;
        
    c = class_new (sym_canvas,
            NULL, 
            (t_method)canvas_free, 
            sizeof (t_glist), 
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);

    class_addCreator ((t_newmethod)subpatch_new, sym_pd, A_DEFSYMBOL, A_NULL);
    
    class_addDSP (c, (t_method)canvas_dsp);
    class_addClick (c, (t_method)canvas_click);
    
    class_addMethod (c, (t_method)canvas_key,                   sym_key,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_motion,                sym_motion,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouse,                 sym_mouse,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseUp,               sym_mouseup,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_window,                sym_window,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_coords,                sym_coords,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,               sym_restore,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_width,                 sym_f,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_connect,               sym_connect,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_disconnect,            sym_disconnect,     A_GIMME, A_NULL);
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
    class_addMethod (c, (t_method)canvas_makeArray,             sym_array,          A_GIMME, A_NULL);
            
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym__array,         A_NULL);

    class_addMethod (c, (t_method)canvas_editmode,              sym_editmode,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_close,          A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_open,                  sym_open,           A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,              sym_loadbang,       A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,                 sym_dirty,          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,               sym_visible,        A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_map,                   sym__map,           A_FLOAT, A_NULL);

    class_addMethod (c, (t_method)canvas_cut,                   sym__cut,           A_NULL);
    class_addMethod (c, (t_method)canvas_copy,                  sym__copy,          A_NULL);
    class_addMethod (c, (t_method)canvas_paste,                 sym__paste,         A_NULL);
    class_addMethod (c, (t_method)canvas_duplicate,             sym__duplicate,     A_NULL);
    class_addMethod (c, (t_method)canvas_selectAll,             sym__selectall,     A_NULL);
    
    class_addMethod (c, (t_method)canvas_remove,                sym_destroy,        A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)canvas_clear,                 sym_clear,          A_NULL);
    class_addMethod (c, (t_method)canvas_rename,                sym_rename,         A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_save,                  sym_save,           A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_saveas,         A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveToFile,            sym_savetofile,     A_GIMME, A_NULL);
        
    class_addMethod (c, (t_method)canvas_bounds,                sym_bounds,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromArrayDialog,       sym__arraydialog,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromPopupDialog,       sym__popupdialog,   A_GIMME, A_NULL);
        
    class_addMethod (c, (t_method)canvas_fromDialog,
        sym__canvasdialog,
        A_GIMME,
        A_NULL);
   
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_open,                  sym_menu__dash__open,   A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_menuclose,          A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_text,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_toggle,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vumeter,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_mycnv,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_numbox,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,               sym_vis,                A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,                  sym_menusave,           A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_menusaveas,         A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym_menuarray,          A_NULL);

    class_addCreator ((t_newmethod)subpatch_new, sym_page, A_DEFSYMBOL, A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &canvas_widgetbehavior);
    class_setSaveFunction (c, canvas_functionSave);
    class_setPropertiesFunction (c, canvas_functionProperties);

    canvas_class = c;
}

void canvas_destroy (void)
{
    CLASS_FREE (canvas_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
