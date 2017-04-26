
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

void canvas_cut                     (t_glist *);
void canvas_copy                    (t_glist *);
void canvas_paste                   (t_glist *);
void canvas_duplicate               (t_glist *);
void canvas_selectAll               (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_functionProperties (t_gobj *, t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

enum {
    POPUP_PROPERTIES    = 0,
    POPUP_OPEN          = 1,
    POPUP_HELP          = 2
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    glist_objectRemoveAllScalars (glist);
}

static void canvas_editmode (t_glist *glist, t_float f)
{
    glist_windowEdit (glist, (f != 0.0));
}

static void canvas_dirty (t_glist *glist, t_float f)
{
    if (glist_isEditable (glist)) { glist_setDirty (glist, (int)f); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_restore (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
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
    
    glist_setName (glist, dollar_expandDollarSymbolByEnvironment (name, glist));
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
    
    PD_ASSERT (!err);
    
    #if PD_WITH_LEGACY
    
    if (!isGOP) {
    
        glist_setBounds (glist, &bounds);
            
        {
            t_float scaleX = glist_getValueForOnePixelX (glist);
            t_float scaleY = glist_getValueForOnePixelY (glist);
        
            bounds_set (&bounds, (t_float)0.0, (t_float)0.0, PD_ABS (scaleX), PD_ABS (scaleY));
        }
    }
    
    #endif
    
    rectangle_setByWidthAndHeight (&r, a, b, width, height);
    
    glist_setGraphGeometry (glist, &r, &bounds, isGOP);
}

static void canvas_width (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    glist_objectSetWidthOfLast (glist, (int)atom_getFloatAtIndex (0, argc, argv));
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
    
    if (glist_isArray (glist)) { glist_updateWindow (glist); }
    //
    }
}

static void canvas_map (t_glist *glist, t_float f)
{
    glist_windowMapped (glist, (f != 0.0));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_requireArrayDialog (t_glist *glist)
{
    char t[PD_STRING] = { 0 };
    
    t_error err = string_sprintf (t, PD_STRING, 
                        "::ui_array::show %%s %s 100 1 -1 1 1\n", 
                        utils_getDefaultBindName (garray_class, sym_array)->s_name);
    
    PD_ASSERT (!err);
    
    stub_new (cast_pd (glist), (void *)glist, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_fromPopupDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int k = (int)atom_getFloatAtIndex (0, argc, argv);
    int a = (int)atom_getFloatAtIndex (1, argc, argv);
    int b = (int)atom_getFloatAtIndex (2, argc, argv);
    
    t_gobj *y = NULL;
    
    PD_ASSERT (argc == 3);
    
    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    t_rectangle t;
    
    if (gobj_hit (y, glist, a, b, &t)) {
    //
    if (k == POPUP_PROPERTIES) {
        if (class_hasPropertiesFunction (pd_class (y))) {
            (*class_getPropertiesFunction (pd_class (y))) (y, glist); return;
        }
    } 
    if (k == POPUP_OPEN) {
        if (class_hasMethod (pd_class (y), sym_open)) {
            pd_message (cast_pd (y), sym_open, 0, NULL); return;
        }
    }
    if (k == POPUP_HELP) {
        file_openHelpPatch (y); return;
    }
    //
    }
    //
    }
    
    if (k == POPUP_PROPERTIES) { canvas_functionProperties (cast_gobj (glist), NULL); }
}

static void canvas_fromArrayDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeArrayFromDialog (glist, s, argc, argv);
}

static void canvas_fromDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty = 0;
    t_rectangle t1; t_bounds t2; int t3;
    
    rectangle_setCopy (&t1, glist_getGraphGeometry (glist)); 
    bounds_setCopy (&t2, glist_getBounds (glist));
    t3 = glist_isGraphOnParent (glist);
    
    PD_ASSERT (argc == 7);
    PD_ASSERT (!glist_isArray (glist));
    
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
    
    bounds_set (&bounds, (t_float)0.0, (t_float)0.0, PD_ABS (scaleX), PD_ABS (scaleY));
    
    glist_setGraphGeometry (glist, &r, &bounds, isGOP);
    
    if (glist_hasWindow (glist)) { glist_updateWindow (glist); }
    //
    }
    
    isDirty |= !rectangle_areEquals (&t1, glist_getGraphGeometry (glist));
    isDirty |= !bounds_areEquals (&t2, glist_getBounds (glist)); 
    isDirty |= (t3 != glist_isGraphOnParent (glist));
    
    if (isDirty) { glist_setDirty (glist, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_functionSave (t_gobj *x, t_buffer *b)
{
    int saveContents = !(glist_isAbstraction (cast_glist (x)));
    
    if (saveContents) { glist_serialize (cast_glist (x), b); }
    else {
        buffer_vAppend (b, "ssii",
            sym___hash__X,
            sym_obj,
            object_getX (cast_object (x)),
            object_getY (cast_object (x)));
        
        buffer_serialize (b, object_getBuffer (cast_object (x)));
        buffer_appendSemicolon (b);
        object_saveWidth (cast_object (x), b);
    }
}

static void canvas_functionProperties (t_gobj *x, t_glist *dummy)
{
    t_glist *glist = cast_glist (x);
    
    if (glist_isArray (glist)) {
    
        /* Legacy patches can contains multiple arrays. */
        
        t_gobj *y = NULL;
        
        for (y = glist->gl_graphics; y; y = y->g_next) {
            if (pd_class (y) == garray_class) { garray_functionProperties ((t_garray *)y); }
        }
        
    } else {
    
        char t[PD_STRING] = { 0 };
        
        t_bounds *bounds  = glist_getBounds (glist);
        t_rectangle *r    = glist_getGraphGeometry (glist);
        
        t_error err = string_sprintf (t, PD_STRING, 
                            "::ui_canvas::show %%s %d %d %d %d %d %g %g\n",
                            rectangle_getTopLeftX (r),
                            rectangle_getTopLeftY (r),
                            rectangle_getWidth (r),
                            rectangle_getHeight (r),
                            glist_isGraphOnParent (glist),
                            bounds_getRight (bounds),
                            bounds_getBottom (bounds));

        PD_ASSERT (!err);
    
        stub_new (cast_pd (glist), (void *)glist, t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *canvas_newSubpatch (t_symbol *s)
{
    return glist_newPatchPop (s, NULL, NULL, NULL, 0, 0, 0);
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
        (int)atom_getFloatAtIndex (4, argc, argv));
}

static void canvas_free (t_glist *glist)
{
    if (glist_hasView (glist) && glist_hasWindow (glist)) { glist_windowClose (glist); } 
    
    stub_destroyWithKey ((void *)glist);
    
    glist_deselectAll (glist);
    glist_objectRemoveAll (glist);
    
    glist_free (glist);
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

    class_addCreator ((t_newmethod)canvas_newSubpatch, sym_pd, A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)canvas_newSubpatch, sym_page, A_DEFSYMBOL, A_NULL);
        
    #endif
    
    class_addDSP (c, (t_method)canvas_dsp);
    class_addClick (c, (t_method)canvas_click);
    
    class_addMethod (c, (t_method)canvas_save,                  sym_save,               A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_saveas,             A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_close,              A_DEFFLOAT, A_NULL);
    
    class_addMethod (c, (t_method)canvas_open,                  sym_open,               A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,              sym_loadbang,           A_NULL);
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
    class_addMethod (c, (t_method)canvas_makeScalar,            sym_scalar,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeBang,              sym_bng,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_tgl,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSliderVertical,    sym_vslider,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeSliderHorizontal,  sym_hslider,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeRadioVertical,     sym_vradio,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeRadioHorizontal,   sym_hradio,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vu,                 A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_cnv,                A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_nbx,                A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_key,                   sym__key,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_motion,                sym__motion,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseDown,             sym__mousedown,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mouseUp,               sym__mouseup,           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_window,                sym__window,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_map,                   sym__map,               A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveToFile,            sym__savetofile,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_cut,                   sym__cut,               A_NULL);
    class_addMethod (c, (t_method)canvas_copy,                  sym__copy,              A_NULL);
    class_addMethod (c, (t_method)canvas_paste,                 sym__paste,             A_NULL);
    class_addMethod (c, (t_method)canvas_duplicate,             sym__duplicate,         A_NULL);
    class_addMethod (c, (t_method)canvas_selectAll,             sym__selectall,         A_NULL);
    
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym__array,             A_NULL);
    class_addMethod (c, (t_method)canvas_fromPopupDialog,       sym__popupdialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromArrayDialog,       sym__arraydialog,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_fromDialog,            sym__canvasdialog,      A_GIMME, A_NULL);
   
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_open,                  sym_menu__dash__open,   A_NULL);
    class_addMethod (c, (t_method)canvas_makeComment,           sym_text,               A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeToggle,            sym_toggle,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeVu,                sym_vumeter,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makePanel,             sym_mycnv,              A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_makeDial,              sym_numbox,             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_close,                 sym_menuclose,          A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,                  sym_menusave,           A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,                sym_menusaveas,         A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_requireArrayDialog,    sym_menuarray,          A_NULL);

    #endif
    
    class_setWidgetBehavior (c, &glist_widgetbehavior);
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
