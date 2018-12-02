
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"
#include "../../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior     glist_widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *canvas_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_dsp                     (t_glist *, t_signal **);

void canvas_makeObject              (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeMessage             (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeArray               (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeArrayFromDialog     (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeFloatAtom           (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeSymbolAtom          (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeComment             (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeScalar              (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeBang                (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeToggle              (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeSliderVertical      (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeSliderHorizontal    (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeRadioVertical       (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeRadioHorizontal     (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeMenuButton          (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeVu                  (t_glist *, t_symbol *, int, t_atom *);
void canvas_makePanel               (t_glist *, t_symbol *, int, t_atom *);
void canvas_makeDial                (t_glist *, t_symbol *, int, t_atom *);

void canvas_key                     (t_glist *, t_symbol *, int, t_atom *);
void canvas_motion                  (t_glist *, t_symbol *, int, t_atom *);
void canvas_mouseDown               (t_glist *, t_symbol *, int, t_atom *);
void canvas_mouseUp                 (t_glist *, t_symbol *, int, t_atom *);

void canvas_close                   (t_glist *, t_float);
void canvas_save                    (t_glist *, t_float);
void canvas_saveAs                  (t_glist *, t_float);
void canvas_saveToFile              (t_glist *, t_symbol *, int, t_atom *);

void canvas_undo                    (t_glist *);
void canvas_redo                    (t_glist *);
void canvas_update                  (t_glist *);
void canvas_cut                     (t_glist *);
void canvas_copy                    (t_glist *);
void canvas_paste                   (t_glist *);
void canvas_duplicate               (t_glist *);
void canvas_encapsulate             (t_glist *);
void canvas_deencapsulate           (t_glist *);
void canvas_selectAll               (t_glist *);
void canvas_snap                    (t_glist *);
void canvas_bringToFront            (t_glist *);
void canvas_sendToBack              (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_functionProperties (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void glist_objectAddNextProceed (t_glist *, t_gobj *, t_gobj *);
void glist_objectAddUndoProceed (t_glist *, t_gobj *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_fromValue (t_scalar *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    POPUP_VALUE         = 0,
    POPUP_PROPERTIES    = 1,
    POPUP_OPEN          = 2,
    POPUP_HELP          = 3
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_click (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    glist_windowOpen (glist);
}

static void canvas_open (t_glist *glist)
{
    glist_windowOpen (glist);
}

static void canvas_loadbang (t_glist *glist)
{
    glist_loadbang (glist);
}

void canvas_clear (t_glist *glist)
{
    int undoable = glist_undoIsOk (glist);
    
    if (undoable) { glist_undoAppendSeparator (glist); }
    
    glist_objectRemoveAllScalars (glist);
    
    if (undoable) { glist_undoAppendSeparator (glist); }
}

static void canvas_editmode (t_glist *glist, t_float f)
{
    glist_windowEdit (glist, (f != 0.0));
}

static void canvas_dirty (t_glist *glist, t_float f)
{
    if (glist_isEditable (glist)) {
    //
    glist_setDirty (glist, (int)f);
    
    if (glist_undoIsOk (glist)) { glist_undoAppendSeparator (glist); }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_restore (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (glist_hasParent (glist));
    PD_ABORT (!glist_hasParent (glist));
    
    if (glist_hasParent (glist)) {
    //
    t_buffer *t = buffer_new(); 
    t_symbol *name = atom_getSymbolOrDollarSymbolAtIndex (3, argc, argv);
        
    if (argc > 2) { buffer_deserialize (t, argc - 2, argv + 2); }
    
    object_setBuffer (cast_object (glist), t);
    object_setX (cast_object (glist), atom_getFloatAtIndex (0, argc, argv));
    object_setY (cast_object (glist), atom_getFloatAtIndex (1, argc, argv));
    object_setWidth (cast_object (glist), 0);
    object_setType (cast_object (glist), TYPE_OBJECT);
    
    glist_setName (glist, dollar_expandSymbol (name, glist));
    glist_objectAdd (glist_getParent (glist), cast_gobj (glist));
    //
    }
    
    instance_stackPopPatch (glist, glist_isOpenedAtLoad (glist));
}

static void canvas_coords (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int flags   = (int)atom_getFloatAtIndex (6, argc, argv);
    int a       = (int)atom_getFloatAtIndex (7, argc, argv);
    int b       = (int)atom_getFloatAtIndex (8, argc, argv);
    int width   = (int)atom_getFloatAtIndex (4, argc, argv);
    int height  = (int)atom_getFloatAtIndex (5, argc, argv);
    
    int isGOP   = (flags & 1);
    
    t_rectangle r; t_bounds bounds;
    
    t_error err = bounds_setByAtoms (&bounds, argc, argv);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    if (!isGOP) {   /* Allow compatbility with legacy. */
    
        glist_setBounds (glist, &bounds);
            
        {
            t_float scaleX = glist_getValueForOnePixelX (glist);
            t_float scaleY = glist_getValueForOnePixelY (glist);
        
            bounds_set (&bounds, 0.0, 0.0, PD_ABS (scaleX), PD_ABS (scaleY));
        }
    }
    
    rectangle_setByWidthAndHeight (&r, a, b, width, height);
    
    glist_setGraphGeometry (glist, &r, &bounds, isGOP);
}

static void canvas_width (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    glist_objectSetWidthOfLast (glist, (int)atom_getFloatAtIndex (0, argc, argv));
}

static void canvas_tagcanvas (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    glist_setUnique (glist, argc, argv);
}

static void canvas_tagobject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    glist_objectSetUniqueOfLast (glist, argc, argv);
}

static void canvas_connect (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    int m = (int)atom_getFloat (argv + 0);
    int i = (int)atom_getFloat (argv + 1);
    int n = (int)atom_getFloat (argv + 2);
    int j = (int)atom_getFloat (argv + 3);
    
    if (glist_lineConnect (glist, m, i, n, j) == PD_ERROR_NONE) { return; }
    //
    }
    
    error_failed (sym_connect);
}

static void canvas_disconnect (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    int m = (int)atom_getFloat (argv + 0);
    int i = (int)atom_getFloat (argv + 1);
    int n = (int)atom_getFloat (argv + 2);
    int j = (int)atom_getFloat (argv + 3);
    
    if (glist_lineDisconnect (glist, m, i, n, j) == PD_ERROR_NONE) { return; }
    //
    }
    
    error_failed (sym_disconnect);
}

static void canvas_window (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    t_rectangle r; rectangle_setByAtoms (&r, argc, argv);
    
    glist_setWindowGeometry (glist, &r);
    
    if (glist_isGraphicArray (glist)) { glist_updateWindow (glist); }
    //
    }
}

static void canvas_scroll (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 2) {
    //
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    
    glist_setScroll (glist, a, b);
    //
    }
}

static void canvas_map (t_glist *glist, t_float f)
{
    glist_windowMapped (glist, (f != 0.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_properties (t_glist *glist)
{
    t_mouse m; mouse_init (&m);
    
    if (glist_objectGetNumberOfSelected (glist) > 0) {
        t_selection *s = NULL;
        for (s = editor_getSelection (glist_getEditor (glist)); s; s = selection_getNext (s)) {
            t_gobj *y = selection_getObject (s);
            if (class_hasPropertiesFunction (pd_class (y))) {
                (*class_getPropertiesFunction (pd_class (y))) (y, glist, &m);
            }
        }
    } else { canvas_functionProperties (cast_gobj (glist), NULL, &m); }
}

static void canvas_help (t_glist *glist)
{
    if (glist_objectGetNumberOfSelected (glist) == 1) {
        gobj_help (selection_getObject (editor_getSelection (glist_getEditor (glist))));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_requireArrayDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    char t[PD_STRING] = { 0 };
    
    int width  = GLIST_WIDTH / 2;
    int height = GLIST_HEIGHT / 2;
    
    int menu   = (atom_getSymbolAtIndex (0, argc, argv) == sym_menu);
    
    t_error err = string_sprintf (t, PD_STRING, 
                        "::ui_array::show %%s %s 100 %d %d 1 -1 0 1 0 0 %d\n",
                        utils_getUnusedBindName (garray_class, sym_array)->s_name,
                        width,
                        height,
                        menu);
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (glist), (void *)glist, t);
}

static void canvas_requireScalarDialog (t_glist *glist, t_symbol *s)
{
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (s);
    
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *dummy = scalar_new (glist, templateIdentifier);
    t_heapstring *h = heapstring_new (0);
    t_gpointer gp; gpointer_init (&gp);
    
    heapstring_add (h, "::ui_scalar::show %s scalar -1 invalid ");
    
    gpointer_setAsScalar (&gp, dummy);
    
    PD_ASSERT (gpointer_isValid (&gp));
    
    if (gpointer_getValues (&gp, h)) {
    //
    heapstring_add (h, "\n"); stub_new (cast_pd (glist), (void *)glist, heapstring_getRaw (h));
    //
    } else {
        PD_BUG;
    }
    
    heapstring_free (h); gpointer_unset (&gp);
    
    pd_free (cast_pd (dummy));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_fromPopupDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int k = (int)atom_getFloatAtIndex (0, argc, argv);
    int a = (int)atom_getFloatAtIndex (1, argc, argv);
    int b = (int)atom_getFloatAtIndex (2, argc, argv);
    
    int i, n = glist_objectGetNumberOf (glist);
    
    PD_ASSERT (argc == 3);
    
    t_mouse m; mouse_init (&m);
    
    m.m_x = a;
    m.m_y = b;
    m.m_clickedRight = 1;
    
    for (i = n - 1; i >= 0; i--) {
    //
    t_gobj *y = glist_objectGetAt (glist, i);
    
    t_rectangle t;
    
    if (gobj_hit (y, glist, a, b, 0, &t)) {
    //
    if (k == POPUP_VALUE) {
        if (class_hasValueFunction (pd_class (y))) {
            (*class_getValueFunction (pd_class (y))) (y, glist, &m); return;
        }
    }
    if (k == POPUP_PROPERTIES) {
        if (class_hasPropertiesFunction (pd_class (y))) {
            (*class_getPropertiesFunction (pd_class (y))) (y, glist, &m); return;
        }
    }
    if (k == POPUP_OPEN) {
        pd_message (cast_pd (y), gobj_isCanvas (y) ? sym_open : sym__open, 0, NULL); return;
    }
    if (k == POPUP_HELP) {
        gobj_help (y); return;
    }
    //
    }
    //
    }
    
    if (k == POPUP_PROPERTIES) { canvas_functionProperties (cast_gobj (glist), NULL, &m); }
}

static void canvas_fromArrayDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (argc == 11);
    
    canvas_makeArrayFromDialog (glist, s, argc, argv);
}

static void canvas_fromScalarDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 3) {
    //
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (atom_getSymbol (argv + 3));
    
    if (template_isValid (template_findByIdentifier (templateIdentifier))) {
    //
    t_scalar *scalar = scalar_new (glist, templateIdentifier);
    glist_objectAddNextProceed (glist, cast_gobj (scalar), NULL);
    scalar_fromValue (scalar, NULL, argc, argv);
    glist_objectAddUndoProceed (glist, cast_gobj (scalar));
    //
    }
    //
    }
}

static void canvas_fromDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_glist *manager = glist_hasWindow (glist) ? glist : glist_getParent (glist);
    int undoable     = glist_undoIsOk (manager);
    int isDirty      = 0;
    t_rectangle t1; t_bounds t2; int t3;
    
    t_undosnippet *s1 = NULL;
    t_undosnippet *s2 = NULL;
    
    if (undoable) { s1 = undosnippet_newProperties (cast_gobj (glist), glist_getParent (glist)); }
    
    rectangle_setCopy (&t1, glist_getGraphGeometry (glist)); 
    bounds_setCopy (&t2, glist_getBounds (glist));
    t3 = glist_isGraphOnParent (glist);
    
    PD_ASSERT (argc == 7);
    PD_ASSERT (!glist_isGraphicArray (glist));
    PD_ASSERT (manager != NULL);
    
    {
    //
    int a           = (int)atom_getFloatAtIndex (0, argc, argv);
    int b           = (int)atom_getFloatAtIndex (1, argc, argv);
    int width       = (int)atom_getFloatAtIndex (2, argc, argv);
    int height      = (int)atom_getFloatAtIndex (3, argc, argv);
    int isGOP       = (int)atom_getFloatAtIndex (4, argc, argv);
    t_float scaleX  = atom_getFloatAtIndex (5, argc, argv);
    t_float scaleY  = atom_getFloatAtIndex (6, argc, argv);
    
    t_rectangle r; t_bounds bounds;
    
    rectangle_setByWidthAndHeight (&r, a, b, width, height);

    if (scaleX == 0.0) { scaleX = (t_float)1.0; }
    if (scaleY == 0.0) { scaleY = (t_float)1.0; }
    
    bounds_set (&bounds, 0.0, 0.0, PD_ABS (scaleX), PD_ABS (scaleY));
    
    glist_setGraphGeometry (glist, &r, &bounds, isGOP);
    
    if (glist_hasWindow (glist)) { glist_updateWindow (glist); }
    //
    }
    
    if (undoable) { s2 = undosnippet_newProperties (cast_gobj (glist), glist_getParent (glist)); }
    
    isDirty |= !rectangle_areEquals (&t1, glist_getGraphGeometry (glist));
    isDirty |= !bounds_areEquals (&t2, glist_getBounds (glist)); 
    isDirty |= (t3 != glist_isGraphOnParent (glist));
    
    if (isDirty) { glist_setDirty (glist, 1); }
    
    if (undoable) {
    //
    if (isDirty) {
        glist_undoAppend (manager, undoproperties_new (cast_gobj (glist), s1, s2));
        glist_undoAppendSeparator (manager);
        
    } else {
        undosnippet_free (s1);
        undosnippet_free (s2);
    }
    
    s1 = NULL;
    s2 = NULL;
    //
    }
    
    PD_ASSERT (s1 == NULL);
    PD_ASSERT (s2 == NULL);
    
    if (isDirty) { glist_deselectAll (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void canvas_functionSave (t_gobj *x, t_buffer *b, int flags)
{
    int saveContents = !(glist_isAbstraction (cast_glist (x)));
    
    if (saveContents) { glist_serialize (cast_glist (x), b, flags); }
    else {
        buffer_appendSymbol (b, sym___hash__X);
        buffer_appendSymbol (b, sym_obj);
        buffer_appendFloat (b,  object_getX (cast_object (x)));
        buffer_appendFloat (b,  object_getY (cast_object (x)));
        buffer_serialize (b,    object_getBuffer (cast_object (x)));
        buffer_appendSemicolon (b);
        object_serializeWidth (cast_object (x), b);
        
        if (SAVED_UNDO (flags)) { gobj_serializeUnique (x, sym__tagobject, b); }
    }
}

/* Fake dialog message from interpreter. */

static void canvas_functionUndo (t_gobj *x, t_buffer *b)
{
    t_glist *glist   = cast_glist (x);
    t_bounds *bounds = glist_getBounds (glist);
    t_rectangle *r   = glist_getGraphGeometry (glist);
    
    PD_ASSERT (!glist_isGraphicArray (glist));
    
    buffer_appendSymbol (b, sym__canvasdialog);
    buffer_appendFloat (b,  rectangle_getTopLeftX (r));
    buffer_appendFloat (b,  rectangle_getTopLeftY (r));
    buffer_appendFloat (b,  rectangle_getWidth (r));
    buffer_appendFloat (b,  rectangle_getHeight (r));
    buffer_appendFloat (b,  glist_isGraphOnParent (glist));
    buffer_appendFloat (b,  bounds_getRight (bounds));
    buffer_appendFloat (b,  bounds_getBottom (bounds));
}

static void canvas_functionProperties (t_gobj *x, t_glist *dummy, t_mouse *m)
{
    t_glist *glist = cast_glist (x);
    
    if (glist_isGraphicArray (glist)) {
    
        /* Legacy patches can contains multiple arrays. */
        
        t_gobj *y = NULL;
        
        for (y = glist->gl_graphics; y; y = y->g_next) {
            if (gobj_isGraphicArray (y)) { garray_functionProperties ((t_garray *)y); }
        }
        
    } else {
    
        char t[PD_STRING] = { 0 };
        
        t_bounds *bounds  = glist_getBounds (glist);
        t_rectangle *r    = glist_getGraphGeometry (glist);
        
        t_error err = string_sprintf (t, PD_STRING, 
                            "::ui_canvas::show %%s %d %d %d %d %d %.9g %.9g\n",
                            rectangle_getTopLeftX (r),
                            rectangle_getTopLeftY (r),
                            rectangle_getWidth (r),
                            rectangle_getHeight (r),
                            glist_isGraphOnParent (glist),
                            bounds_getRight (bounds),
                            bounds_getBottom (bounds));

        PD_UNUSED (err); PD_ASSERT (!err);
        
        stub_new (cast_pd (glist), (void *)glist, t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *canvas_newSubpatch (t_symbol *s)
{
    return glist_newPatchPop (s, NULL, NULL, NULL, 0, 1, 0, 0);
}

void canvas_new (void *dummy, t_symbol *s, int argc, t_atom *argv)
{  
    t_rectangle r;
    
    rectangle_setByAtomsByWidthAndHeight (&r, argc, argv);
    
    glist_newPatch (atom_getSymbolAtIndex (4, argc, argv), 
        NULL, 
        NULL, 
        &r, 
        (int)atom_getFloatAtIndex (5, argc, argv),
        0,
        0, 
        (int)atom_getFloatAtIndex (4, argc, argv));
}

static void canvas_free (t_glist *glist)
{
    glist_undoDisable (glist);
    
    if (glist_hasView (glist) && glist_hasWindow (glist)) { glist_windowClose (glist); }
    
    stub_destroyWithKey ((void *)glist);
    
    glist_deselectAll (glist);
    glist_objectRemoveAll (glist);
    
    glist_free (glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void canvas_setup (void)
{
    t_class *c = NULL;
        
    c = class_new (sym_canvas,
            NULL, 
            (t_method)canvas_free, 
            sizeof (t_glist), 
            CLASS_DEFAULT | CLASS_NOINLET,
            A_NULL);

    class_addCreator ((t_newmethod)canvas_newSubpatch, sym_pd, A_DEFSYMBOL, A_NULL);
    
    class_addDSP (c, (t_method)canvas_dsp);
    class_addClick (c, (t_method)canvas_click);
    class_addLoadbang (c, (t_method)canvas_loadbang);
    
    class_addMethod (c, (t_method)canvas_save,                  sym_save,               A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_saveas,             A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_close,              A_DEFFLOAT, A_NULL);
    
    class_addMethod (c, (t_method)canvas_open,                  sym_open,               A_NULL);
    class_addMethod (c, (t_method)canvas_clear,                 sym_clear,              A_NULL);
    class_addMethod (c, (t_method)canvas_editmode,              sym_editmode,           A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,                 sym_dirty,              A_FLOAT, A_NULL);
        
    /* The methods below should stay private. */
    /* A safer approach for dynamic patching must be implemented. */
    
    class_addMethod (c, (t_method)canvas_restore,               sym_restore,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_coords,                sym_coords,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_width,                 sym_f,                  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_connect,               sym_connect,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_disconnect,            sym_disconnect,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeObject,            sym_obj,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeMessage,           sym_msg,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeArray,             sym_array,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeFloatAtom,         sym_floatatom,          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSymbolAtom,        sym_symbolatom,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_comment,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_text,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeScalar,            sym_scalar,             A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_makeBang,              sym_bng,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_tgl,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSliderVertical,    sym_vslider,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSliderHorizontal,  sym_hslider,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeRadioVertical,     sym_vradio,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeRadioHorizontal,   sym_hradio,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeMenuButton,        sym_menubutton,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vu,                 A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_cnv,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_nbx,                A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_tagcanvas,             sym__tagcanvas,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_tagobject,             sym__tagobject,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_key,                   sym__key,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_motion,                sym__motion,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseDown,             sym__mousedown,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseUp,               sym__mouseup,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_window,                sym__window,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_scroll,                sym__scroll,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_map,                   sym__map,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveToFile,            sym__savetofile,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_undo,                  sym__undo,              A_NULL);
    class_addMethod (c, (t_method)canvas_redo,                  sym__redo,              A_NULL);
    class_addMethod (c, (t_method)canvas_update,                sym__menu,              A_NULL);
    class_addMethod (c, (t_method)canvas_cut,                   sym__cut,               A_NULL);
    class_addMethod (c, (t_method)canvas_copy,                  sym__copy,              A_NULL);
    class_addMethod (c, (t_method)canvas_paste,                 sym__paste,             A_NULL);
    class_addMethod (c, (t_method)canvas_duplicate,             sym__duplicate,         A_NULL);
    class_addMethod (c, (t_method)canvas_encapsulate,           sym__encapsulate,       A_NULL);
    class_addMethod (c, (t_method)canvas_deencapsulate,         sym__deencapsulate,     A_NULL);
    class_addMethod (c, (t_method)canvas_selectAll,             sym__selectall,         A_NULL);
    class_addMethod (c, (t_method)canvas_snap,                  sym__snap,              A_NULL);
    class_addMethod (c, (t_method)canvas_bringToFront,          sym__front,             A_NULL);
    class_addMethod (c, (t_method)canvas_sendToBack,            sym__back,              A_NULL);
    class_addMethod (c, (t_method)canvas_properties,            sym__properties,        A_NULL);
    class_addMethod (c, (t_method)canvas_help,                  sym__help,              A_NULL);
    
    class_addMethod (c, (t_method)canvas_requireScalarDialog,   sym__scalar,            A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym__array,             A_GIMME,  A_NULL);
    
    class_addMethod (c, (t_method)canvas_fromPopupDialog,       sym__popupdialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromArrayDialog,       sym__arraydialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromScalarDialog,      sym__scalardialog,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromDialog,            sym__canvasdialog,      A_GIMME, A_NULL);
   
    class_setWidgetBehavior (c, &glist_widgetbehavior);
    class_setSaveFunction (c, canvas_functionSave);
    class_setUndoFunction (c, canvas_functionUndo);
    class_setPropertiesFunction (c, canvas_functionProperties);
    
    class_setHelpName (c, sym_pd);
    
    canvas_class = c;
}

void canvas_destroy (void)
{
    class_free (canvas_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
