
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

extern t_pd     pd_canvasMaker;
extern int      editor_reloading;

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
    t_symbol *s = gensym ("loadbang");
    
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
    
    if (s == &s_) { s = gensym ("/SUBPATCH/"); }
    
    SET_FLOAT  (a + 0, 0);
    SET_FLOAT  (a + 1, CANVAS_WINDOW_HEADER_HEIGHT);
    SET_FLOAT  (a + 2, CANVAS_WINDOW_DEFAULT_WIDTH);
    SET_FLOAT  (a + 3, CANVAS_WINDOW_DEFAULT_HEIGHT);
    SET_SYMBOL (a + 4, s);
    SET_FLOAT  (a + 5, 1);
    
    x = canvas_new (NULL, NULL, 6, a);
    x->gl_owner = z;
    
    canvas_pop (x, 1);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_click (t_glist *glist, t_float a, t_float b, t_float shift, t_float ctrl, t_float alt)
{
    canvas_vis (glist, 1);
}

void canvas_setBounds (t_glist *x, t_float a, t_float b, t_float c, t_float d)
{
    x->gl_windowTopLeftX     = a;
    x->gl_windowTopLeftY     = b;
    x->gl_windowBottomRightX = c;
    x->gl_windowBottomRightY = d;
}

/*
static void canvas_dosetbounds(t_glist *x, int x1, int y1, int x2, int y2)
{
    int heightwas = y2 - y1;
    int heightchange = y2 - y1 - (x->gl_windowBottomRightY - x->gl_windowTopLef
    if (x->gl_windowTopLeftX == x1 && x->gl_windowTopLeftY == y1 &&
        x->gl_windowBottomRightX == x2 && x->gl_windowBottomRightY == y2)
            return;
    x->gl_windowTopLeftX = x1;
    x->gl_windowTopLeftY = y1;
    x->gl_windowBottomRightX = x2;
    x->gl_windowBottomRightY = y2;
    if (!canvas_isGraphOnParent(x) && (x->gl_valueDown < x->gl_valueUp)) 
    {
        t_float diff = x->gl_valueUp - x->gl_valueDown;
        t_gobj *y;
        x->gl_valueUp = heightwas * diff;
        x->gl_valueDown = x->gl_valueUp - diff;
        for (y = x->gl_graphics; y; y = y->g_next)
            if (canvas_castToObjectIfBox(&y->g_pd))
                gobj_displace(y, x, 0, heightchange);
        canvas_redraw(x);
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
    canvas_rename (glist, name, NULL);
    //
    }
    
    canvas_pop (glist, glist->gl_willBeVisible);

    if (!(z = gensym ("#X")->s_thing) || (pd_class (z) != canvas_class)) { PD_BUG; }
    else {
        t_glist *g = cast_glist (z);
        glist->gl_owner = g;
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
        gobj_vis (g1, glist, 0);
        gobj_vis (g1, glist, 1);
    }
    //
    }
    //
    }
}

static void canvas_open (t_glist *glist)
{
    if (canvas_isVisible (glist) && !canvas_isTopLevel (glist)) {
    //
    PD_ASSERT (glist->gl_owner);
    
    gobj_vis (cast_gobj (glist), glist->gl_owner, 0);
    
    if (glist->gl_editor) { canvas_destroy_editor (glist); }

    glist->gl_haveWindow = 1;
    
    gobj_vis (cast_gobj (glist), glist->gl_owner, 1);
    //
    }
    
    canvas_vis (glist, 1);
}

void canvas_loadbang (t_glist *glist)
{
    canvas_loadbangAbstractions (glist);
    canvas_loadbangSubpatches (glist);
}

void canvas_map (t_glist *glist, t_float f)
{
    int isMapped = (f != 0.0);

    if (isMapped != canvas_isVisible (glist)) {
    //
    if (isMapped) {
    
        t_gobj *y = NULL;
        t_selection *selection = NULL;
        
        if (!glist->gl_haveWindow) { PD_BUG; canvas_vis (glist, 1); }
        for (y = glist->gl_graphics; y; y = y->g_next) { gobj_vis (y, glist, 1); }
        for (selection = glist->gl_editor->e_selection; selection; selection = selection->sel_next) {
            gobj_select (selection->sel_what, glist, 1);
        }

        glist->gl_isMapped = 1;
        
        canvas_drawLines (glist);
        
        if (glist->gl_isGraphOnParent && glist->gl_hasRectangle) {
            canvas_drawGraphOnParentRectangle (glist);
        }
        
        sys_vGui ("::ui_patch::updateScrollRegion .x%lx.c\n", glist);
        
    } else {
        sys_vGui (".x%lx.c delete all\n", glist); glist->gl_isMapped = 0;
    }
    //
    }
}

    /* mark a glist dirty or clean */
void canvas_dirty(t_glist *x, t_float n)
{
    t_glist *x2 = canvas_getRoot(x);
    if (editor_reloading)
        return;
    if ((unsigned)n != x2->gl_isDirty)
    {
        x2->gl_isDirty = n;
        if (x2->gl_haveWindow)
            canvas_updateTitle(x2);
    }
}

void canvas_pop(t_glist *x, t_float fvis)
{
    if (fvis != 0)
        canvas_vis(x, 1);
    stack_pop(&x->gl_obj.te_g.g_pd);
    canvas_resortinlets(x);
    canvas_resortoutlets(x);
    x->gl_isLoading = 0;
}

static void canvas_rename_method(t_glist *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_SYMBOL)
        canvas_rename(x, av->a_w.w_symbol, 0);
    else if (ac && av->a_type == A_DOLLARSYMBOL)
    {
        t_environment *e = canvas_getEnvironment(x);
        //canvas_setCurrent(x);
        stack_push (cast_pd (x));
        canvas_rename(x, dollar_expandDollarSymbol(av->a_w.w_symbol,
            e->ce_argc, e->ce_argv/*, 1*/), 0); 
        //canvas_unsetCurrent(x);
        stack_pop (cast_pd (x));
    }
    else canvas_rename(x, gensym (PD_NAME_SHORT), 0);
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
    x->gl_owner         = owner;
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

    if (visible && gensym ("#X")->s_thing && (pd_class (gensym ("#X")->s_thing) == canvas_class)) {
        t_glist *g = cast_glist (gensym ("#X")->s_thing);
        while (g && !g->gl_environment) { g = g->gl_owner; }
        if (g && canvas_isAbstraction (g) && g->gl_owner) { visible = 0; }
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
        
    canvas_noundo (glist);
    glist_noselect (glist);
    
    while (y = glist->gl_graphics) { glist_delete (glist, y); }
    if (glist == glist_getcanvas (glist)) { canvas_vis (glist, 0); }
    if (glist->gl_editor) { canvas_destroy_editor (glist); }
    
    canvas_unbind (glist);

    if (glist->gl_environment) {
        PD_MEMORY_FREE (glist->gl_environment->ce_argv);
        PD_MEMORY_FREE (glist->gl_environment);
    }
    
    dsp_resume (dspstate);
    
    gstub_cutoff (glist->gl_stub);
    guistub_destroyWithKey (glist);
    
    if (!glist->gl_owner) { instance_removeFromRoots (glist); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setup (void)
{
    t_class *c = NULL;
        
    /* Creator function by sending the "canvas" message to #N. */
            
    class_addMethod (pd_canvasMaker, (t_method)canvas_new, gensym ("canvas"), A_GIMME, A_NULL);
    
    c = class_new (gensym ("canvas"),
        NULL, 
        (t_method)canvas_free, 
        sizeof (t_glist), 
        CLASS_NOINLET,
        A_NULL);

    class_addCreator ((t_newmethod)subpatch_new, gensym ("pd"), A_DEFSYMBOL, A_NULL);
    
    class_addClick (c, canvas_click);
    class_addBounds (c, canvas_setBounds);
    
    class_addMethod (c, (t_method)canvas_coords,        gensym ("coords"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,       gensym ("restore"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_obj,           gensym ("obj"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_msg,           gensym ("msg"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_floatatom,     gensym ("floatatom"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_symbolatom,    gensym ("symbolatom"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_text,           gensym ("text"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_graph,         gensym ("graph"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_scalar,         gensym ("scalar"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_width,         gensym ("f"),           A_GIMME, A_NULL);
        
    class_addMethod (c, (t_method)canvas_bng,           gensym ("bng"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_toggle,        gensym ("tgl"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vslider,       gensym ("vslider"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_hslider,       gensym ("hslider"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_hradio,        gensym ("hradio"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vradio,        gensym ("vradio"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vumeter,       gensym ("vu"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mycnv,         gensym ("cnv"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_numbox,        gensym ("nbx"),         A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_open,          gensym ("open"),        A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,      gensym ("loadbang"),    A_NULL);
    class_addMethod (c, (t_method)canvas_vis,           gensym ("visible"),     A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_map,           gensym ("map"),         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,         gensym ("dirty"),       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_pop,           gensym ("pop"),         A_DEFFLOAT, A_NULL);

    class_addMethod (c, (t_method)glist_clear,          gensym ("clear"),       A_NULL);
    class_addMethod (c, (t_method)canvas_dsp,           gensym ("dsp"),         A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_rename_method, gensym ("rename"),      A_GIMME, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_open,          gensym ("menu-open"),   A_NULL);
    class_addMethod (c, (t_method)canvas_toggle,        gensym ("toggle"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vumeter,       gensym ("vumeter"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mycnv,         gensym ("mycnv"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_numbox,        gensym ("numbox"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vis,           gensym ("vis"),         A_FLOAT, A_NULL);
    
    class_addCreator ((t_newmethod)subpatch_new,        gensym ("page"),        A_DEFSYMBOL, A_NULL);

    #endif
        
    class_setPropertiesFunction (c, canvas_properties);

    canvas_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
