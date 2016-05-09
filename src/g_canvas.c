
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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *text_class;
extern t_glist  *editor_pasteCanvas;

extern t_pd     pd_canvasMaker;
extern int      editor_pasteOnset;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *canvas_class;                              /* Shared. */
t_symbol        *canvas_fileName  = &s_;                    /* Shared. */
t_symbol        *canvas_directory = &s_;                    /* Shared. */
t_atom          *canvas_argv;                               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int             canvas_argc;                                /* Shared. */
int             canvas_magic = 10000;                       /* Shared. */

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
    
    if (s == &s_) { s = sym_subpatch; }
    
    SET_FLOAT  (a + 0, 0);
    SET_FLOAT  (a + 1, CANVAS_WINDOW_HEADER_HEIGHT);
    SET_FLOAT  (a + 2, CANVAS_WINDOW_DEFAULT_WIDTH);
    SET_FLOAT  (a + 3, CANVAS_WINDOW_DEFAULT_HEIGHT);
    SET_SYMBOL (a + 4, s);
    SET_FLOAT  (a + 5, 1);
    
    x = canvas_new (NULL, NULL, 6, a);
    x->gl_parent = z;
    
    canvas_pop (x, 1);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_click (t_glist *glist, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    canvas_visible (glist, 1);
}

void canvas_setBounds (t_glist *x, t_float a, t_float b, t_float c, t_float d)
{
    x->gl_windowTopLeftX     = a;
    x->gl_windowTopLeftY     = b;
    x->gl_windowBottomRightX = c;
    x->gl_windowBottomRightY = d;
}

/*
static void canvas_dosetbounds(t_glist *x, int x1, int y1, int x2, int y2)          // --
{
    int heightwas = y2 - y1;
    int heightchange = y2 - y1 - (x->gl_windowBottomRightY - x->gl_windowTopLef
    if (x->gl_windowTopLeftX == x1 && x->gl_windowTopLeftY == y1 &&                 // --
        x->gl_windowBottomRightX == x2 && x->gl_windowBottomRightY == y2)
            return;
    x->gl_windowTopLeftX = x1;
    x->gl_windowTopLeftY = y1;
    x->gl_windowBottomRightX = x2;
    x->gl_windowBottomRightY = y2;
    if (!canvas_isGraphOnParent(x) && (x->gl_valueDown < x->gl_valueUp))            // --
    {
        t_float diff = x->gl_valueUp - x->gl_valueDown;
        t_gobj *y;
        x->gl_valueUp = heightwas * diff;
        x->gl_valueDown = x->gl_valueUp - diff;
        for (y = x->gl_graphics; y; y = y->g_next)                                  // --
            if (canvas_castToObjectIfBox(&y->g_pd))                                 // --
                gobj_displace(y, x, 0, heightchange);                               // --
        canvas_redraw(x);                                                           // --
    }
}
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_coords (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int graphOnParent;
    
    glist->gl_indexStart    = atom_getFloatAtIndex (0, argc, argv);
    glist->gl_valueUp       = atom_getFloatAtIndex (1, argc, argv);
    glist->gl_indexEnd      = atom_getFloatAtIndex (2, argc, argv);
    glist->gl_valueDown     = atom_getFloatAtIndex (3, argc, argv);
    glist->gl_width         = (int)atom_getFloatAtIndex (4, argc, argv);
    glist->gl_height        = (int)atom_getFloatAtIndex (5, argc, argv);
    graphOnParent           = (int)atom_getFloatAtIndex (6, argc, argv);
    
    if (argc >= 8) {
    //
    glist->gl_marginX       = (int)atom_getFloatAtIndex (7, argc, argv);
    glist->gl_marginY       = (int)atom_getFloatAtIndex (8, argc, argv);
    //
    }
    
    canvas_setgraph (glist, graphOnParent, (argc < 8) ? 1 : 0);
}

void canvas_restore (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_pd *z = NULL;
    
    if ((argc > 3) && IS_SYMBOL (argv + 3)) {
    //
    t_environment *e = canvas_getEnvironment (canvas_getCurrent());
    t_symbol *name = dollar_expandDollarSymbol (GET_SYMBOL (argv + 3), e->ce_argc, e->ce_argv);
    canvas_setName (glist, name, NULL);
    //
    }
    
    canvas_pop (glist, glist->gl_willBeVisible);

    if (!(z = s__X.s_thing) || (pd_class (z) != canvas_class)) { PD_BUG; }
    else {
        t_glist *g = cast_glist (z);
        glist->gl_parent = g;
        canvas_objfor (g, cast_object (glist), argc, argv);
    }
}

static void canvas_graph (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_addGraph (glist, 
        atom_getSymbolAtIndex (0, argc, argv),
        atom_getFloatAtIndex (1, argc, argv),
        atom_getFloatAtIndex (2, argc, argv),
        atom_getFloatAtIndex (3, argc, argv),
        atom_getFloatAtIndex (4, argc, argv),
        atom_getFloatAtIndex (5, argc, argv),
        atom_getFloatAtIndex (6, argc, argv),
        atom_getFloatAtIndex (7, argc, argv),
        atom_getFloatAtIndex (8, argc, argv));
}

static void canvas_width (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (glist->gl_graphics) {
    //
    t_gobj *g1 = NULL;
    t_gobj *g2 = NULL;
    
    t_object *o = NULL;
    
    for (g1 = glist->gl_graphics; g2 = g1->g_next; g1 = g2) { }
    
    if (o = canvas_castToObjectIfBox (g1)) {
    //
    o->te_width = atom_getFloatAtIndex (0, argc, argv);
    
    if (canvas_isVisible (glist)) {
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
    int k = (editor_pasteCanvas == glist) ? editor_pasteOnset : 0;
    
    t_gobj *src  = canvas_getObjectAtIndex (glist, k + (int)indexOfObjectOut);
    t_gobj *dest = canvas_getObjectAtIndex (glist, k + (int)indexOfObjectIn);
    t_object *srcObject  = canvas_castToObjectIfBox (src);
    t_object *destObject = canvas_castToObjectIfBox (dest);
    
    if (srcObject && destObject) {
    //
    int m = (int)indexOfOutlet;
    int n = (int)indexOfOInlet;
    t_outconnect *connection = NULL;
    
    /* Creates dummy outlets and inlets mainly in case of failure at object creation. */
    
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
    if (canvas_isVisible (glist)) {
    
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

    post_error (PD_TRANSLATE ("connection: failed in '%s'"), glist->gl_name->s_name);
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
        sys_vGui (".x%lx.c delete %lxLINE\n", glist, connection);
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

void canvas_editmode (t_glist *glist, t_float f)
{
    int state = (int)(f != 0.0);
     
    if (glist->gl_isEditMode != state) {
    //
    glist->gl_isEditMode = state;
    
    if (state) {
    
        if (canvas_isVisible (glist) && canvas_canHaveWindow (glist)) {

            t_gobj *g = NULL;
            canvas_setcursor (glist, CURSOR_EDIT_NOTHING);
            
            for (g = glist->gl_graphics; g; g = g->g_next) {
                t_object *o = NULL;
                if ((o = canvas_castToObjectIfBox (g)) && o->te_type == TYPE_TEXT) {
                    t_boxtext *y = glist_findrtext (glist, o);
                    text_drawborder (o, glist, rtext_gettag (y), rtext_width (y), rtext_height (y), 1);
                }
            }
        }
        
    } else {
    
        canvas_deselectAll (glist);
        
        if (canvas_isVisible (glist) && canvas_canHaveWindow (glist)) {
            canvas_setcursor (glist, CURSOR_NOTHING);
            sys_vGui (".x%lx.c delete COMMENTBAR\n", canvas_getView (glist));
        }
    }
    
    if (canvas_isVisible (glist)) {
        sys_vGui ("::ui_patch::setEditMode .x%lx %d\n", canvas_getView (glist), glist->gl_isEditMode);
    }
    //
    }
}

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
    if (k == 1) { pd_free (cast_pd (glist)); }              /* Has been saved right before. */
    else {
        if (canvas_isDirty (glist)) {
            
            sys_vGui ("::ui_confirm::checkClose .x%lx"
                            " { ::ui_interface::pdsend $top menusave 1 }"
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
    
    if (canvas_isVisible (glist) && !canvas_canHaveWindow (glist)) {
    //
    PD_ASSERT (glist->gl_parent);
    
    gobj_visibilityChanged (cast_gobj (glist), glist->gl_parent, 0);
    
    canvas_destroyEditorIfAny (glist);

    glist->gl_haveWindow = 1;   /* Note that it modify how things are drawn below. */
    
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
        y->gl_isDirty = isDirty; if (y->gl_haveWindow) { canvas_updateTitle (y); }
    }
}

void canvas_visible (t_glist *glist, t_float f)
{
    int isVisible = (f != 0.0);
    
    if (isVisible) {

        if (glist->gl_editor && glist->gl_haveWindow) { sys_vGui ("::bringToFront .x%lx\n", glist); }
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
            
            glist->gl_haveWindow = 1;
        }
        
    } else {

        if (!glist->gl_haveWindow) { canvas_destroyEditorIfAny (glist); }
        else {
            t_glist *t = NULL;
            
            canvas_deselectAll (glist);
            if (canvas_isVisible (glist)) { canvas_map (glist, 0); }
            canvas_destroyEditorIfAny (glist);
            sys_vGui ("destroy .x%lx\n", glist);
            
            if (glist->gl_isGraphOnParent && (t = glist->gl_parent) && (!t->gl_isDeleting)) {
                if (canvas_isVisible (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 0); }
                glist->gl_haveWindow = 0;
                if (canvas_isVisible (t)) { gobj_visibilityChanged (cast_gobj (glist), t, 1); }
            } else {
                glist->gl_haveWindow = 0;
            }
        }
    }
}

void canvas_map (t_glist *glist, t_float f)
{
    int isMapped = (f != 0.0);

    if (isMapped != canvas_isVisible (glist)) {
    //
    if (!isMapped) { sys_vGui (".x%lx.c delete all\n", glist); glist->gl_isMapped = 0; }
    else {
    
        t_gobj *y = NULL;
        t_selection *selection = NULL;
        
        if (!glist->gl_haveWindow) { PD_BUG; canvas_visible (glist, 1); }
        for (y = glist->gl_graphics; y; y = y->g_next) { gobj_visibilityChanged (y, glist, 1); }
        for (selection = glist->gl_editor->e_selectedObjects; selection; selection = selection->sel_next) {
            gobj_select (selection->sel_what, glist, 1);
        }

        glist->gl_isMapped = 1;
        
        canvas_drawLines (glist);
        
        if (glist->gl_isGraphOnParent && glist->gl_hasRectangle) {
            canvas_drawGraphOnParentRectangle (glist);
        }
        
        sys_vGui ("::ui_patch::updateScrollRegion .x%lx.c\n", glist);
    }
    //
    }
}

void canvas_pop (t_glist *glist, t_float f)
{
    if (f != 0.0) { canvas_visible (glist, 1); }
    
    stack_pop (cast_pd (glist));
    
    canvas_resortinlets (glist);
    canvas_resortoutlets (glist);
    
    glist->gl_isLoading = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_rename (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_SYMBOL (argv)) { canvas_setName (glist, GET_SYMBOL (argv), NULL); }
    else if (argc && IS_DOLLARSYMBOL (argv)) {
    
        t_environment *e = canvas_getEnvironment (glist);
        stack_push (cast_pd (glist));
        {
            t_symbol *name = dollar_expandDollarSymbol (GET_DOLLARSYMBOL (argv), e->ce_argc, e->ce_argv);
            canvas_setName (glist, name, NULL); 
        }
        stack_pop (cast_pd (glist));
        
    } else { 
        canvas_setName (glist, gensym (PD_NAME_SHORT), NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_new (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    t_glist *x      = (t_glist *)pd_new (canvas_class);
    t_glist *owner  = canvas_getCurrent();
    t_symbol *name  = &s_;
    
    int visible     = 0;
    int width       = CANVAS_WINDOW_DEFAULT_WIDTH;
    int height      = CANVAS_WINDOW_DEFAULT_HEIGHT;
    int topLeftX    = 0;
    int topLeftY    = CANVAS_WINDOW_HEADER_HEIGHT;
    int fontSize    = (owner ? owner->gl_fontSize : font_getDefaultFontSize());
    
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

    x->gl_obj.te_type   = TYPE_OBJECT;
    x->gl_stub          = gstub_new (x, NULL);
    x->gl_parent        = owner;
    x->gl_name          = (name != &s_ ? name : (canvas_fileName ? canvas_fileName : gensym (PD_NAME_SHORT)));
    x->gl_magic         = ++canvas_magic;
    
    if (!owner) { instance_addToRoots (x); }
    
    if (canvas_directory == &s_) { x->gl_environment = NULL; }
    else {
    //
    static int dollarZero = 1000;   /* Shared. */
    
    x->gl_environment = (t_environment *)PD_MEMORY_GET (sizeof (t_environment));

    if (!canvas_argv) { 
        canvas_argv = PD_MEMORY_GET (0); 
    }
    
    x->gl_environment->ce_directory         = canvas_directory;
    x->gl_environment->ce_argc              = canvas_argc;
    x->gl_environment->ce_argv              = canvas_argv;
    x->gl_environment->ce_dollarZeroValue   = dollarZero++;

    canvas_directory = &s_;
    canvas_argc      = 0;
    canvas_argv      = NULL;
    //
    }

    topLeftX = PD_MAX (topLeftX, 0);
    topLeftY = PD_MAX (topLeftY, CANVAS_WINDOW_HEADER_HEIGHT);
        
    x->gl_indexStart    = 0;
    x->gl_valueUp       = 0.0;
    x->gl_indexEnd      = 1;
    x->gl_valueDown     = 1.0;
    
    canvas_setBounds (x, topLeftX, topLeftY, topLeftX + width, topLeftY + height);
    canvas_bind (x);
    
    x->gl_fontSize      = font_getNearestValidFontSize (fontSize);
    x->gl_isLoading     = 1;
    x->gl_hasRectangle  = 0;

    if (visible && s__X.s_thing && (pd_class (s__X.s_thing) == canvas_class)) {
        t_glist *g = canvas_getRoot (cast_glist (s__X.s_thing));
        PD_ASSERT (g);
        if (canvas_isAbstraction (g)) { visible = 0; }
    }
    
    x->gl_isEditMode    = 0;
    x->gl_willBeVisible = visible;
        
    stack_push (cast_pd (x));
    
    return x;
}

void canvas_free (t_glist *glist)
{
    int dspstate = dsp_suspend();
    t_gobj *y = NULL;
        
    canvas_deselectAll (glist);
    
    while (y = glist->gl_graphics) { glist_delete (glist, y); }
    if (glist == canvas_getView (glist)) { canvas_visible (glist, 0); }
    
    canvas_destroyEditorIfAny (glist);
    canvas_unbind (glist);

    if (glist->gl_environment) {
        PD_MEMORY_FREE (glist->gl_environment->ce_argv);
        PD_MEMORY_FREE (glist->gl_environment);
    }
    
    dsp_resume (dspstate);
    
    gstub_cutoff (glist->gl_stub);
    guistub_destroyWithKey (glist);
    
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
            CLASS_NOINLET,
            A_NULL);

    class_addCreator ((t_newmethod)subpatch_new, sym_pd, A_DEFSYMBOL, A_NULL);
    
    class_addKey (c, canvas_key);
    class_addClick (c, canvas_click);
    class_addMotion (c, canvas_motion);
    class_addMouse (c, canvas_mouse);
    class_addMouseUp (c, canvas_mouseup);
    class_addBounds (c, canvas_setBounds);

    class_addMethod (c, (t_method)canvas_coords,        sym_coords,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,       sym_restore,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_obj,           sym_obj,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_msg,           sym_msg,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_floatatom,     sym_floatatom,  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_symbolatom,    sym_symbolatom, A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_text,           sym_text,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_graph,         sym_graph,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_scalar,         sym_scalar,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_width,         sym_f,          A_GIMME, A_NULL);
    
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
     
    class_addMethod (c, (t_method)canvas_bng,           gensym ("bng"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_toggle,        gensym ("tgl"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vslider,       gensym ("vslider"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_hslider,       gensym ("hslider"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_hradio,        gensym ("hradio"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vradio,        gensym ("vradio"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vumeter,       gensym ("vu"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mycnv,         gensym ("cnv"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_numbox,        gensym ("nbx"),         A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_editmode,      gensym ("editmode"),    A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,         gensym ("close"),       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_open,          sym_open,               A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,      sym_loadbang,           A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,         gensym ("dirty"),       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,       gensym ("visible"),     A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_map,           gensym ("_map"),        A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_pop,           sym__pop,               A_DEFFLOAT, A_NULL);

    class_addMethod (c, (t_method)canvas_cut,           gensym ("_cut"),        A_NULL);
    class_addMethod (c, (t_method)canvas_copy,          gensym ("_copy"),       A_NULL);
    class_addMethod (c, (t_method)canvas_paste,         gensym ("_paste"),      A_NULL);
    class_addMethod (c, (t_method)canvas_duplicate,     gensym ("_duplicate"),  A_NULL);
    class_addMethod (c, (t_method)canvas_selectall,     gensym ("_selectall"),  A_NULL);
    
    class_addMethod (c, (t_method)glist_clear,          gensym ("clear"),       A_NULL);
    class_addMethod (c, (t_method)canvas_dsp,           sym_dsp,                A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_rename,        gensym ("rename"),      A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_done_popup,
        gensym ("done-popup"),
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)canvas_donecanvasdialog,
        gensym ("donecanvasdialog"),
        A_GIMME,
        A_NULL);
        
    class_addMethod (c, (t_method)glist_arraydialog,
        gensym ("arraydialog"),
        A_SYMBOL,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
   
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_open,          gensym ("menu-open"),   A_NULL);
    class_addMethod (c, (t_method)canvas_close,         gensym ("menuclose"),   A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_toggle,        gensym ("toggle"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vumeter,       gensym ("vumeter"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mycnv,         gensym ("mycnv"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_numbox,        gensym ("numbox"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_visible,       gensym ("vis"),         A_FLOAT, A_NULL);
    
    class_addCreator ((t_newmethod)subpatch_new,        gensym ("page"),        A_DEFSYMBOL, A_NULL);

    #endif
        
    class_setPropertiesFunction (c, canvas_properties);

    canvas_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
