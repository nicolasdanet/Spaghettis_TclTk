
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

#define CANVAS_DEFAULT_WIDTH    450
#define CANVAS_DEFAULT_HEIGHT   300

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     pd_canvasMaker;
extern int      editor_reloading;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

static void canvas_dosetbounds(t_glist *x, int x1, int y1, int x2, int y2)
{
    int heightwas = y2 - y1;
    int heightchange = y2 - y1 - (x->gl_screeny2 - x->gl_screeny1);
    if (x->gl_screenx1 == x1 && x->gl_screeny1 == y1 &&
        x->gl_screenx2 == x2 && x->gl_screeny2 == y2)
            return;
    x->gl_screenx1 = x1;
    x->gl_screeny1 = y1;
    x->gl_screenx2 = x2;
    x->gl_screeny2 = y2;
    if (!glist_isgraph(x) && (x->gl_y2 < x->gl_y1)) 
    {
            /* if it's flipped so that y grows upward,
            fix so that zero is bottom edge and redraw.  This is
            only appropriate if we're a regular "text" object on the
            parent. */
        t_float diff = x->gl_y1 - x->gl_y2;
        t_gobj *y;
        x->gl_y1 = heightwas * diff;
        x->gl_y2 = x->gl_y1 - diff;
            /* and move text objects accordingly; they should stick
            to the bottom, not the top. */
        for (y = x->gl_list; y; y = y->g_next)
            if (canvas_castToObjectIfBox(&y->g_pd))
                gobj_displace(y, x, 0, heightchange);
        canvas_redraw(x);
    }
}

static void canvas_loadbangabstractions(t_glist *x)
{
    t_gobj *y;
    t_symbol *s = gensym ("loadbang");
    for (y = x->gl_list; y; y = y->g_next)
        if (pd_class(&y->g_pd) == canvas_class)
    {
        if (canvas_isabstraction((t_glist *)y))
            canvas_loadbang((t_glist *)y);
        else
            canvas_loadbangabstractions((t_glist *)y);
    }
}

static void canvas_loadbangsubpatches(t_glist *x)
{
    t_gobj *y;
    t_symbol *s = gensym ("loadbang");
    for (y = x->gl_list; y; y = y->g_next)
        if (pd_class(&y->g_pd) == canvas_class)
    {
        if (!canvas_isabstraction((t_glist *)y))
            canvas_loadbangsubpatches((t_glist *)y);
    }
    for (y = x->gl_list; y; y = y->g_next)
        if ((pd_class(&y->g_pd) != canvas_class) &&
            class_hasMethod(pd_class (&y->g_pd), s))
                pd_vMessage(&y->g_pd, s, "");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *subcanvas_new(t_symbol *s)
{
    t_atom a[6];
    t_glist *x, *z = canvas_getCurrent();
    if (!*s->s_name) s = gensym ("/SUBPATCH/");
    SET_FLOAT(a, 0);
    SET_FLOAT(a+1, CANVAS_DEFAULT_Y);
    SET_FLOAT(a+2, CANVAS_DEFAULT_WIDTH);
    SET_FLOAT(a+3, CANVAS_DEFAULT_HEIGHT);
    SET_SYMBOL(a+4, s);
    SET_FLOAT(a+5, 1);
    x = canvas_new(0, 0, 6, a);
    x->gl_owner = z;
    canvas_pop(x, 1);
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_click(t_glist *x,
    t_float xpos, t_float ypos,
        t_float shift, t_float ctrl, t_float alt)
{
    canvas_vis(x, 1);
}

    /* This is sent from the GUI to inform a toplevel that its window has been
    moved or resized. */
void canvas_setbounds(t_glist *x, t_float left, t_float top, 
                             t_float right, t_float bottom)
{
    canvas_dosetbounds(x, (int)left, (int)top, (int)right, (int)bottom);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_coords(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    x->gl_x1 = atom_getFloatAtIndex(0, argc, argv);
    x->gl_y1 = atom_getFloatAtIndex(1, argc, argv);
    x->gl_x2 = atom_getFloatAtIndex(2, argc, argv);
    x->gl_y2 = atom_getFloatAtIndex(3, argc, argv);
    x->gl_pixwidth = (t_int)atom_getFloatAtIndex(4, argc, argv);
    x->gl_pixheight = (t_int)atom_getFloatAtIndex(5, argc, argv);
    if (argc <= 7)
        canvas_setgraph(x, (t_int)atom_getFloatAtIndex(6, argc, argv), 1);
    else
    {
        x->gl_xmargin = (t_int)atom_getFloatAtIndex(7, argc, argv);
        x->gl_ymargin = (t_int)atom_getFloatAtIndex(8, argc, argv);
        canvas_setgraph(x, (t_int)atom_getFloatAtIndex(6, argc, argv), 0);
    }
}

void canvas_restore(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_pd *z;
    if (argc > 3)
    {
        t_atom *ap=argv+3;
        if (ap->a_type == A_SYMBOL)
        {
            t_canvasenvironment *e = canvas_getEnvironment(canvas_getCurrent());
            canvas_rename(x, dollar_expandDollarSymbol(ap->a_w.w_symbol,
                e->ce_argc, e->ce_argv/*, 1*/), 0);
        }
    }
    canvas_pop(x, x->gl_willvis);

    if (!(z = gensym ("#X")->s_thing)) post_error ("canvas_restore: out of context");
    else if (*z != canvas_class) post_error ("canvas_restore: wasn't a canvas");
    else
    {
        t_glist *x2 = (t_glist *)z;
        x->gl_owner = x2;
        canvas_objfor(x2, &x->gl_obj, argc, argv);
    }
}

    /* call glist_addglist from a Pd message */
static void glist_glist(t_glist *g, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *sym = atom_getSymbolAtIndex(0, argc, argv);   
    t_float x1 = atom_getFloatAtIndex(1, argc, argv);  
    t_float y1 = atom_getFloatAtIndex(2, argc, argv);  
    t_float x2 = atom_getFloatAtIndex(3, argc, argv);  
    t_float y2 = atom_getFloatAtIndex(4, argc, argv);  
    t_float px1 = atom_getFloatAtIndex(5, argc, argv);  
    t_float py1 = atom_getFloatAtIndex(6, argc, argv);  
    t_float px2 = atom_getFloatAtIndex(7, argc, argv);  
    t_float py2 = atom_getFloatAtIndex(8, argc, argv);
    glist_addglist(g, sym, x1, y1, x2, y2, px1, py1, px2, py2);
}

static void canvas_f(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    static int warned;
    t_gobj *g, *g2;
    t_object *ob;
    if (argc > 1 && !warned)
    {
        post("** ignoring width or font settings from future Pd version **");
        warned = 1;
    }
    if (!x->gl_list)
        return;
    for (g = x->gl_list; g2 = g->g_next; g = g2)
        ;
    if (ob = canvas_castToObjectIfBox(&g->g_pd))
    {
        ob->te_width = atom_getFloatAtIndex(0, argc, argv);
        if (glist_isvisible(x))
        {
            gobj_vis(g, x, 0);
            gobj_vis(g, x, 1);
        }
    }
}

    /* we call this on a non-toplevel glist to "open" it into its
    own window. */
void glist_menu_open(t_glist *x)
{
    if (glist_isvisible(x) && !glist_istoplevel(x))
    {
        t_glist *gl2 = x->gl_owner;
        if (!gl2) { PD_BUG; }
        else {
                /* erase ourself in parent window */
            gobj_vis(&x->gl_obj.te_g, gl2, 0);
                    /* get rid of our editor (and subeditors) */
            if (x->gl_editor)
                canvas_destroy_editor(x);
            x->gl_havewindow = 1;
                    /* redraw ourself in parent window (blanked out this time) */
            gobj_vis(&x->gl_obj.te_g, gl2, 1);
        }
    }
    canvas_vis(x, 1);
}

void canvas_loadbang(t_glist *x)
{
    t_gobj *y;
    canvas_loadbangabstractions(x);
    canvas_loadbangsubpatches(x);
}

    /* the window becomes "mapped" (visible and not miniaturized) or
    "unmapped" (either miniaturized or just plain gone.)  This should be
    called from the GUI after the fact to "notify" us that we're mapped. */
void canvas_map(t_glist *x, t_float f)
{
    int flag = (f != 0);
    t_gobj *y;
    if (flag)
    {
        if (!glist_isvisible(x))
        {
            t_selection *sel;
            if (!x->gl_havewindow)
            {
                PD_BUG;
                canvas_vis(x, 1);
            }
            for (y = x->gl_list; y; y = y->g_next)
                gobj_vis(y, x, 1);
            for (sel = x->gl_editor->e_selection; sel; sel = sel->sel_next)
                gobj_select(sel->sel_what, x, 1);
            x->gl_mapped = 1;
            canvas_drawlines(x);
            if (x->gl_isgraph && x->gl_goprect)
                canvas_drawredrect(x, 1);
            sys_vGui("::ui_patch::updateScrollRegion .x%lx.c\n", x);
        }
    }
    else
    {
        if (glist_isvisible(x))
        {
                /* just clear out the whole canvas */
            sys_vGui(".x%lx.c delete all\n", x);
            x->gl_mapped = 0;
        }
    }
}

    /* mark a glist dirty or clean */
void canvas_dirty(t_glist *x, t_float n)
{
    t_glist *x2 = canvas_getroot(x);
    if (editor_reloading)
        return;
    if ((unsigned)n != x2->gl_dirty)
    {
        x2->gl_dirty = n;
        if (x2->gl_havewindow)
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
    x->gl_loading = 0;
}

static void canvas_rename_method(t_glist *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac && av->a_type == A_SYMBOL)
        canvas_rename(x, av->a_w.w_symbol, 0);
    else if (ac && av->a_type == A_DOLLARSYMBOL)
    {
        t_canvasenvironment *e = canvas_getEnvironment(x);
        //canvas_setCurrent(x);
        stack_push (cast_pd (x));
        canvas_rename(x, dollar_expandDollarSymbol(av->a_w.w_symbol,
            e->ce_argc, e->ce_argv/*, 1*/), 0); 
        //canvas_unsetCurrent(x);
        stack_pop (cast_pd (x));
    }
    else canvas_rename(x, gensym ("Pd"), 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_new (void *dummy, t_symbol *sel, int argc, t_atom *argv)
{
    t_glist *x = (t_glist *)pd_new(canvas_class);
    t_glist *owner = canvas_getCurrent();
    t_symbol *s = &s_;
    int vis = 0, width = CANVAS_DEFAULT_WIDTH, height = CANVAS_DEFAULT_HEIGHT;
    int xloc = 0, yloc = CANVAS_DEFAULT_Y;
    int font = (owner ? owner->gl_font : font_getDefaultFontSize());
    x->gl_stub = gstub_new (x, NULL);
    x->gl_valid = ++canvas_magic;
    x->gl_obj.te_type = TYPE_OBJECT;
    if (!owner)
        instance_addToRoots (x);
    /* post("canvas %lx, owner %lx", x, owner); */

    if (argc == 5)  /* toplevel: x, y, w, h, font */
    {
        xloc = (t_int)atom_getFloatAtIndex(0, argc, argv);
        yloc = (t_int)atom_getFloatAtIndex(1, argc, argv);
        width = (t_int)atom_getFloatAtIndex(2, argc, argv);
        height = (t_int)atom_getFloatAtIndex(3, argc, argv);
        font = (t_int)atom_getFloatAtIndex(4, argc, argv);
    }
    else if (argc == 6)  /* subwindow: x, y, w, h, name, vis */
    {
        xloc = (t_int)atom_getFloatAtIndex(0, argc, argv);
        yloc = (t_int)atom_getFloatAtIndex(1, argc, argv);
        width = (t_int)atom_getFloatAtIndex(2, argc, argv);
        height = (t_int)atom_getFloatAtIndex(3, argc, argv);
        s = atom_getSymbolAtIndex(4, argc, argv);
        vis = (t_int)atom_getFloatAtIndex(5, argc, argv);
    }
        /* (otherwise assume we're being created from the menu.) */

    if (canvas_directory->s_name[0])
    {
        static int dollarzero = 1000;
        t_canvasenvironment *env = x->gl_env =
            (t_canvasenvironment *)PD_MEMORY_GET(sizeof(*x->gl_env));
        if (!canvas_argv)
            canvas_argv = PD_MEMORY_GET(0);
        env->ce_directory = canvas_directory;
        env->ce_argc = canvas_argc;
        env->ce_argv = canvas_argv;
        env->ce_dollarZeroValue = dollarzero++;
        //env->ce_path = 0;
        canvas_directory = &s_;
        canvas_argc = 0;
        canvas_argv = 0;
    }
    else x->gl_env = 0;

    if (yloc < CANVAS_DEFAULT_Y)
        yloc = CANVAS_DEFAULT_Y;
    if (xloc < 0)
        xloc = 0;
    x->gl_x1 = 0;
    x->gl_y1 = 0;
    x->gl_x2 = 1;
    x->gl_y2 = 1;
    canvas_dosetbounds(x, xloc, yloc, xloc + width, yloc + height);
    x->gl_owner = owner;
    x->gl_name = (*s->s_name ? s : 
        (canvas_fileName ? canvas_fileName : gensym ("Pd")));
    canvas_bind(x);
    x->gl_loading = 1;
    x->gl_goprect = 0;      /* no GOP rectangle unless it's turned on later */
        /* cancel "vis" flag if we're a subpatch of an
         abstraction inside another patch.  A separate mechanism prevents
         the toplevel abstraction from showing up. */
    if (vis && gensym ("#X")->s_thing && 
        ((*gensym ("#X")->s_thing) == canvas_class))
    {
        t_glist *zzz = (t_glist *)(gensym ("#X")->s_thing);
        while (zzz && !zzz->gl_env)
            zzz = zzz->gl_owner;
        if (zzz && canvas_isabstraction(zzz) && zzz->gl_owner)
            vis = 0;
    }
    x->gl_willvis = vis;
    x->gl_edit = !strncmp(x->gl_name->s_name, "Untitled", 8);
    x->gl_font = font_getNearestValidFontSize(font);
    stack_push(&x->gl_obj.te_g.g_pd);
    return(x);
}

void canvas_free(t_glist *x)
{
    t_gobj *y;
    int dspstate = dsp_suspend();
    canvas_noundo(x);
    glist_noselect(x);
    while (y = x->gl_list)
        glist_delete(x, y);
    if (x == glist_getcanvas(x))
        canvas_vis(x, 0);
    if (x->gl_editor)
        canvas_destroy_editor(x);   /* bug workaround; should already be gone*/
    canvas_unbind(x);

    if (x->gl_env)
    {
        PD_MEMORY_FREE(x->gl_env->ce_argv);
        PD_MEMORY_FREE(x->gl_env);
    }
    dsp_resume(dspstate);
    //PD_MEMORY_FREE(x->gl_xlabel);
    //PD_MEMORY_FREE(x->gl_ylabel);
    gstub_cutoff(x->gl_stub);
    guistub_destroyWithKey(x);        /* probably unnecessary */
    if (!x->gl_owner)
        instance_removeFromRoots (x);
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

    class_addCreator ((t_newmethod)subcanvas_new, gensym ("pd"), A_DEFSYMBOL, A_NULL);
    
    class_addClick (c, canvas_click);
    class_addBounds (c, canvas_setbounds);
    
    class_addMethod (c, (t_method)canvas_coords,        gensym ("coords"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,       gensym ("restore"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_obj,           gensym ("obj"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_msg,           gensym ("msg"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_floatatom,     gensym ("floatatom"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_symbolatom,    gensym ("symbolatom"),  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_text,           gensym ("text"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_glist,          gensym ("graph"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)glist_scalar,         gensym ("scalar"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_f,             gensym ("f"),           A_GIMME, A_NULL);
        
    class_addMethod (c, (t_method)canvas_bng,           gensym ("bng"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_toggle,        gensym ("tgl"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vslider,       gensym ("vslider"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_hslider,       gensym ("hslider"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_hradio,        gensym ("hradio"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vradio,        gensym ("vradio"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vumeter,       gensym ("vu"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mycnv,         gensym ("cnv"),         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_numbox,        gensym ("nbx"),         A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)glist_menu_open,      gensym ("open"),        A_NULL);
    class_addMethod (c, (t_method)canvas_loadbang,      gensym ("loadbang"),    A_NULL);
    class_addMethod (c, (t_method)canvas_vis,           gensym ("visible"),     A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_map,           gensym ("map"),         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_dirty,         gensym ("dirty"),       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_pop,           gensym ("pop"),         A_DEFFLOAT, A_NULL);

    class_addMethod (c, (t_method)glist_clear,          gensym ("clear"),       A_NULL);
    class_addMethod (c, (t_method)canvas_dsp,           gensym ("dsp"),         A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_rename_method, gensym ("rename"),      A_GIMME, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)glist_menu_open,      gensym ("menu-open"),   A_NULL);
    class_addMethod (c, (t_method)canvas_toggle,        gensym ("toggle"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vumeter,       gensym ("vumeter"),     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_mycnv,         gensym ("mycnv"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_numbox,        gensym ("numbox"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_vis,           gensym ("vis"),         A_FLOAT, A_NULL);
    
    class_addCreator ((t_newmethod)subcanvas_new,       gensym ("page"),        A_DEFSYMBOL, A_NULL);

    #endif
        
    class_setPropertiesFunction (c, canvas_properties);

    canvas_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
