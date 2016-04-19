
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
#include "s_system.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define CANVAS_DEFAULT_WIDTH        450
#define CANVAS_DEFAULT_HEIGHT       300

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef __APPLE__
    #define CANVAS_DEFAULT_Y        22
#else
    #define CANVAS_DEFAULT_Y        50
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class          *scalar_class;
extern t_pdinstance     *pd_this;
extern t_pd             *pd_newest;

extern t_pd             pd_canvasMaker;
extern int              editor_reloading;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_drawlines    (t_glist *x);
static void canvas_dosetbounds  (t_glist *x, int a, int b, int c, int d);
static void canvas_pop          (t_glist *x, t_float fvis);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *canvas_class;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int canvas_magic = 10000;                               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      canvas_argc;                            /* Shared. */
static t_atom   *canvas_argv;                           /* Shared. */
static t_symbol *canvas_fileName  = &s_;                /* Shared. */
static t_symbol *canvas_directory = &s_;                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_bind (t_glist *glist)
{
    if (strcmp (glist->gl_name->s_name, "Pd")) {
        pd_bind (cast_pd (glist), canvas_makeBindSymbol (glist->gl_name));
    }
}

static void canvas_unbind (t_glist *glist)
{
    if (strcmp (glist->gl_name->s_name, "Pd")) {
        pd_unbind (cast_pd (glist), canvas_makeBindSymbol (glist->gl_name));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_setFileNameAndDirectory (t_symbol *name, t_symbol *directory)
{
    canvas_fileName  = name;
    canvas_directory = directory;
}

void canvas_setArguments (int argc, t_atom *argv)
{
    if (canvas_argv) { PD_MEMORY_FREE (canvas_argv); }
    
    canvas_argc = argc;
    canvas_argv = PD_MEMORY_GET_COPY (argv, argc * sizeof (t_atom));
}

void canvas_newPatch (void *dummy, t_symbol *name, t_symbol *directory)
{
    canvas_setFileNameAndDirectory (name, directory);
    canvas_new (NULL, NULL, 0, NULL);
    canvas_pop (cast_glist (s__X.s_thing), 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist *canvas_getCurrent (void)
{
    return (cast_glist (pd_findByClass (&s__X, canvas_class)));
}

t_canvasenvironment *canvas_getEnvironment (t_glist *glist)
{
    PD_ASSERT (glist);
    
    while (!glist->gl_env) { if (!(glist = glist->gl_owner)) { PD_BUG; } }
    
    return glist->gl_env;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *canvas_expandDollar (t_glist *glist, t_symbol *s)
{
    t_symbol *t = s;
    
    if (strchr (s->s_name, '$')) {
    //
    t_canvasenvironment *environment = canvas_getEnvironment (glist);
    stack_push (cast_pd (glist));
    t = dollar_expandDollarSymbol (s, environment->ce_argc, environment->ce_argv);
    stack_pop (cast_pd (glist));
    //
    }

    return t;
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

void canvas_rename (t_glist *glist, t_symbol *name, t_symbol *directory)
{
    canvas_unbind (glist);
    glist->gl_name = name;
    canvas_bind (glist);
    
    if (glist->gl_havewindow) { canvas_updateTitle (glist); }
    if (directory && directory != &s_) {
        canvas_getEnvironment (glist)->ce_directory = directory; 
    }
}

void canvas_updateTitle (t_glist *glist)
{
    sys_vGui ("::ui_patch::setTitle .x%lx {%s} {%s} %d\n",
        glist,
        canvas_getEnvironment (glist)->ce_directory->s_name,
        glist->gl_name->s_name,
        glist->gl_dirty);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_getIndexOfObject (t_glist *glist, t_gobj *object)
{
    t_gobj *t = NULL;
    int n = 0;
    for (t = glist->gl_list; t && t != object; t = t->g_next) { n++; }
    return n;
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
    
    if (!t->tr_srcObject) { y = cast_gobj (t->tr_owner->gl_list); }
    else {
        y = cast_gobj (t->tr_srcObject)->g_next;
    }
    
    for (; y; y = y->g_next) {
        if ((o = canvas_castToObjectIfBox (y))) { break; }     /* Only box objects are considered. */
    }
    
    if (!o) { return NULL; }
    
    t->tr_srcObject          = o;
    t->tr_srcNumberOfOutlets = object_numberOfOutlets (o);
    n = 0;
    
    if (glist_isvisible (t->tr_owner)) {
    
        gobj_getrect (y, t->tr_owner,
            &t->tr_srcTopLeftX,
            &t->tr_srcTopLeftY,
            &t->tr_srcBottomRightX,
            &t->tr_srcBottomRightY);
            
    } else {
        t->tr_srcTopLeftX     = 0;
        t->tr_srcTopLeftY     = 0;
        t->tr_srcBottomRightX = 0;
        t->tr_srcBottomRightY = 0;
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
    
    if (glist_isvisible (t->tr_owner)) {

        gobj_getrect (cast_gobj (t->tr_destObject), t->tr_owner,
            &t->tr_destTopLeftX,
            &t->tr_destTopLeftY,
            &t->tr_destBottomRightX,
            &t->tr_destBottomRightY);
        
        {
            int w = t->tr_srcBottomRightX - t->tr_srcTopLeftX;
            int i = t->tr_srcIndexOfOutlet;
            int j = t->tr_srcNumberOfOutlets;
        
            t->tr_lineStartX = t->tr_srcTopLeftX + INLETS_MIDDLE (w, i, j);
            t->tr_lineStartY = t->tr_srcBottomRightY;
        }
        {
            int w = t->tr_destBottomRightX - t->tr_destTopLeftX;
            int i = t->tr_destIndexOfInlet;
            int j = t->tr_destNumberOfInlets;
        
            t->tr_lineEndX = t->tr_destTopLeftX + INLETS_MIDDLE (w, i, j);
            t->tr_lineEndY = t->tr_destTopLeftY;
        }
        
    } else {
        t->tr_lineStartX        = 0;
        t->tr_lineStartY        = 0;
        t->tr_lineEndX          = 0;
        t->tr_lineEndY          = 0;
        t->tr_destTopLeftX      = 0;
        t->tr_destTopLeftY      = 0;
        t->tr_destBottomRightX  = 0;
        t->tr_destBottomRightY  = 0;
    }
    
    return connection;
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
        (canvas_fileName ? canvas_fileName : gensym("Pd")));
    canvas_bind(x);
    x->gl_loading = 1;
    x->gl_goprect = 0;      /* no GOP rectangle unless it's turned on later */
        /* cancel "vis" flag if we're a subpatch of an
         abstraction inside another patch.  A separate mechanism prevents
         the toplevel abstraction from showing up. */
    if (vis && gensym("#X")->s_thing && 
        ((*gensym("#X")->s_thing) == canvas_class))
    {
        t_glist *zzz = (t_glist *)(gensym("#X")->s_thing);
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

void canvas_setgraph(t_glist *x, int flag, int nogoprect);

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

    /* make a new glist and add it to this glist.  It will appear as
    a "graph", not a text object.  */
t_glist *glist_addglist(t_glist *g, t_symbol *sym,
    t_float x1, t_float y1, t_float x2, t_float y2,
    t_float px1, t_float py1, t_float px2, t_float py2)
{
    static int gcount = 0;
    int zz;
    int menu = 0;
    char *str;
    t_glist *x = (t_glist *)pd_new(canvas_class);
    x->gl_stub = gstub_new (x, NULL);
    x->gl_valid = ++canvas_magic;
    x->gl_obj.te_type = TYPE_OBJECT;
    if (!*sym->s_name)
    {
        char buf[40];
        sprintf(buf, "graph%d", ++gcount);
        sym = gensym(buf);
        menu = 1;
    }
    else if (!strncmp((str = sym->s_name), "graph", 5)
        && (zz = atoi(str + 5)) > gcount)
            gcount = zz;
        /* in 0.34 and earlier, the pixel rectangle and the y bounds were
        reversed; this would behave the same, except that the dialog window
        would be confusing.  The "correct" way is to have "py1" be the value
        that is higher on the screen. */
    if (py2 < py1)
    {
        t_float zz;
        zz = y2;
        y2 = y1;
        y1 = zz;
        zz = py2;
        py2 = py1;
        py1 = zz;
    }
    if (x1 == x2 || y1 == y2)
        x1 = 0, x2 = 100, y1 = 1, y2 = -1;
    if (px1 >= px2 || py1 >= py2)
        px1 = 100, py1 = 20, px2 = 100 + GLIST_DEFAULT_WIDTH,
            py2 = 20 + GLIST_DEFAULT_HEIGHT;
    x->gl_name = sym;
    x->gl_x1 = x1;
    x->gl_x2 = x2;
    x->gl_y1 = y1;
    x->gl_y2 = y2;
    x->gl_obj.te_xCoordinate = px1;
    x->gl_obj.te_yCoordinate = py1;
    x->gl_pixwidth = px2 - px1;
    x->gl_pixheight = py2 - py1;
    x->gl_font =  (canvas_getCurrent() ?
        canvas_getCurrent()->gl_font : font_getDefaultFontSize());
    x->gl_screenx1 = 0;
    x->gl_screeny1 = CANVAS_DEFAULT_Y;
    x->gl_screenx2 = 450;
    x->gl_screeny2 = 300;
    x->gl_owner = g;
    canvas_bind(x);
    x->gl_isgraph = 1;
    x->gl_goprect = 0;
    x->gl_obj.te_buffer = buffer_new();
    buffer_vAppend(x->gl_obj.te_buffer, "s", gensym("graph"));
    if (!menu)
        stack_push(&x->gl_obj.te_g.g_pd);
    glist_add(g, &x->gl_obj.te_g);
    return (x);
}

    /* call glist_addglist from a Pd message */
void glist_glist(t_glist *g, t_symbol *s, int argc, t_atom *argv)
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

    /* return true if the glist should appear as a graph on parent;
    otherwise it appears as a text box. */
int glist_isgraph(t_glist *x)
{
  return (x->gl_isgraph|(x->gl_hidetext<<1));
}

    /* This is sent from the GUI to inform a toplevel that its window has been
    moved or resized. */
void canvas_setbounds(t_glist *x, t_float left, t_float top, 
                             t_float right, t_float bottom)
{
    canvas_dosetbounds(x, (int)left, (int)top, (int)right, (int)bottom);
}

/* this is the internal version using ints */
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

void canvas_drawredrect(t_glist *x, int doit)
{
    if (doit)
        sys_vGui(".x%lx.c create line\
            %d %d %d %d %d %d %d %d %d %d -fill #ff8080 -tags GOP\n",
            glist_getcanvas(x),
            x->gl_xmargin, x->gl_ymargin,
            x->gl_xmargin + x->gl_pixwidth, x->gl_ymargin,
            x->gl_xmargin + x->gl_pixwidth, x->gl_ymargin + x->gl_pixheight,
            x->gl_xmargin, x->gl_ymargin + x->gl_pixheight,
            x->gl_xmargin, x->gl_ymargin);
    else sys_vGui(".x%lx.c delete GOP\n",  glist_getcanvas(x));
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

void canvas_redraw(t_glist *x)
{
    if (glist_isvisible(x))
    {
        canvas_map(x, 0);
        canvas_map(x, 1);
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

int glist_isvisible(t_glist *x)
{
    return ((!x->gl_loading) && glist_getcanvas(x)->gl_mapped);
}

int glist_istoplevel(t_glist *x)
{
        /* we consider a graph "toplevel" if it has its own window
        or if it appears as a box in its parent window so that we
        don't draw the actual contents there. */
    return (x->gl_havewindow || !x->gl_isgraph);
}

int glist_getfont(t_glist *x)
{
    while (!x->gl_env)
        if (!(x = x->gl_owner)) { PD_BUG; }
    return (x->gl_font);
}

void canvas_free(t_glist *x)
{
    t_gobj *y;
    int dspstate = canvas_suspend_dsp();
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
    canvas_resume_dsp(dspstate);
    //PD_MEMORY_FREE(x->gl_xlabel);
    //PD_MEMORY_FREE(x->gl_ylabel);
    gstub_cutoff(x->gl_stub);
    guistub_destroyWithKey(x);        /* probably unnecessary */
    if (!x->gl_owner)
        instance_removeFromRoots (x);
}

/* ----------------- lines ---------- */

static void canvas_drawlines(t_glist *x)
{
    t_linetraverser t;
    t_outconnect *oc;
    {
        canvas_traverseLinesStart(&t, x);
        while (oc = canvas_traverseLinesNext(&t))
            sys_vGui(".x%lx.c create line %d %d %d %d -width %d -tags [list l%lx cord]\n",
                    glist_getcanvas(x),
                        t.tr_lineStartX, t.tr_lineStartY, t.tr_lineEndX, t.tr_lineEndY, 
                            (outlet_isSignal(t.tr_srcOutlet) ? 2:1),
                                oc);
    }
}

void canvas_fixlines(t_glist *x, t_object *text)
{
    t_linetraverser t;
    t_outconnect *oc;

    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if (t.tr_srcObject == text || t.tr_destObject == text)
        {
            sys_vGui(".x%lx.c coords l%lx %d %d %d %d\n",
                glist_getcanvas(x), oc,
                    t.tr_lineStartX, t.tr_lineStartY, t.tr_lineEndX, t.tr_lineEndY);
        }
    }
}

    /* kill all lines for the object */
void canvas_deletelines(t_glist *x, t_object *text)
{
    t_linetraverser t;
    t_outconnect *oc;
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if (t.tr_srcObject == text || t.tr_destObject == text)
        {
            if (glist_isvisible(x))
            {
                sys_vGui(".x%lx.c delete l%lx\n",
                    glist_getcanvas(x), oc);
            }
            object_disconnect(t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
        }
    }
}

    /* kill all lines for one inlet or outlet */
void canvas_deletelinesforio(t_glist *x, t_object *text,
    t_inlet *inp, t_outlet *outp)
{
    t_linetraverser t;
    t_outconnect *oc;
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
    {
        if ((t.tr_srcObject == text && t.tr_srcOutlet == outp) ||
            (t.tr_destObject == text && t.tr_destInlet == inp))
        {
            if (glist_isvisible(x))
            {
                sys_vGui(".x%lx.c delete l%lx\n",
                    glist_getcanvas(x), oc);
            }
            object_disconnect(t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);
        }
    }
}

static void canvas_pop(t_glist *x, t_float fvis)
{
    if (fvis != 0)
        canvas_vis(x, 1);
    stack_pop(&x->gl_obj.te_g.g_pd);
    canvas_resortinlets(x);
    canvas_resortoutlets(x);
    x->gl_loading = 0;
}

void canvas_objfor(t_glist *gl, t_object *x, int argc, t_atom *argv);


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

    if (!(z = gensym("#X")->s_thing)) post_error ("canvas_restore: out of context");
    else if (*z != canvas_class) post_error ("canvas_restore: wasn't a canvas");
    else
    {
        t_glist *x2 = (t_glist *)z;
        x->gl_owner = x2;
        canvas_objfor(x2, &x->gl_obj, argc, argv);
    }
}

static void canvas_loadbangabstractions(t_glist *x)
{
    t_gobj *y;
    t_symbol *s = gensym("loadbang");
    for (y = x->gl_list; y; y = y->g_next)
        if (pd_class(&y->g_pd) == canvas_class)
    {
        if (canvas_isabstraction((t_glist *)y))
            canvas_loadbang((t_glist *)y);
        else
            canvas_loadbangabstractions((t_glist *)y);
    }
}

void canvas_loadbangsubpatches(t_glist *x)
{
    t_gobj *y;
    t_symbol *s = gensym("loadbang");
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

void canvas_loadbang(t_glist *x)
{
    t_gobj *y;
    canvas_loadbangabstractions(x);
    canvas_loadbangsubpatches(x);
}

/* no longer used by 'pd-gui', but kept here for backwards compatibility.  The
 * new method calls canvas_setbounds() directly. */
static void canvas_relocate(t_glist *x, t_symbol *canvasgeom,
    t_symbol *topgeom)
{
    int cxpix, cypix, cw, ch, txpix, typix, tw, th;
    if (sscanf(canvasgeom->s_name, "%dx%d+%d+%d", &cw, &ch, &cxpix, &cypix)
        < 4 ||
        sscanf(topgeom->s_name, "%dx%d+%d+%d", &tw, &th, &txpix, &typix) < 4) { PD_BUG; }
            /* for some reason this is initially called with cw=ch=1 so
            we just suppress that here. */
    if (cw > 5 && ch > 5)
        canvas_dosetbounds(x, txpix, typix,
            txpix + cw, typix + ch);
}

void canvas_popabstraction(t_glist *x)
{
    pd_newest = &x->gl_obj.te_g.g_pd;
    stack_pop(&x->gl_obj.te_g.g_pd);
    x->gl_loading = 0;
    canvas_resortinlets(x);
    canvas_resortoutlets(x);
}

void canvas_logerror(t_object *y)
{
#ifdef LATER
    canvas_vis(x, 1);
    if (!glist_isselected(x, &y->te_g))
        glist_select(x, &y->te_g);
#endif
}

/* -------------------------- subcanvases ---------------------- */

static void *subcanvas_new(t_symbol *s)
{
    t_atom a[6];
    t_glist *x, *z = canvas_getCurrent();
    if (!*s->s_name) s = gensym("/SUBPATCH/");
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

void canvas_click(t_glist *x,
    t_float xpos, t_float ypos,
        t_float shift, t_float ctrl, t_float alt)
{
    canvas_vis(x, 1);
}


    /* find out from subcanvas contents how much to fatten the box */
void canvas_fattensub(t_glist *x,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_gobj *y;
    *xp2 += 50;     /* fake for now */
    *yp2 += 50;
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
    else canvas_rename(x, gensym("Pd"), 0);
}


    /* return true if the "canvas" object is an abstraction (so we don't
    save its contents, for example.)  */
int canvas_isabstraction(t_glist *x)
{
    return (x->gl_env != 0);
}

    /* return true if the "canvas" object should be treated as a text
    object.  This is true for abstractions but also for "table"s... */
/* JMZ: add a flag to gop-abstractions to hide the title */
int canvas_showtext(t_glist *x)
{
    t_atom *argv = (x->gl_obj.te_buffer? buffer_atoms(x->gl_obj.te_buffer):0);
    int argc = (x->gl_obj.te_buffer? buffer_size(x->gl_obj.te_buffer) : 0);
    int isarray = (argc && argv[0].a_type == A_SYMBOL &&
        argv[0].a_w.w_symbol == gensym("graph"));
    if(x->gl_hidetext)
      return 0;
    else
      return (!isarray);
}

    /* get the document containing this canvas */
t_glist *canvas_getroot(t_glist *x)
{
    if ((!x->gl_owner) || canvas_isabstraction(x))
        return (x);
    else return (canvas_getroot(x->gl_owner));
}

/* ------------------------- DSP chain handling ------------------------- */

struct _dspcontext;
#define t_dspcontext struct _dspcontext

void ugen_start(void);
void ugen_stop(void);

t_dspcontext *ugen_start_graph(int toplevel, t_signal **sp,
    int ninlets, int noutlets);
void ugen_add(t_dspcontext *dc, t_object *x);
void ugen_connect(t_dspcontext *dc, t_object *x1, int outno,
    t_object *x2, int inno);
void ugen_done_graph(t_dspcontext *dc);

    /* schedule one canvas for DSP.  This is called below for all "root"
    canvases, but is also called from the "dsp" method for sub-
    canvases, which are treated almost like any other tilde object.  */

static void canvas_dodsp(t_glist *x, int toplevel, t_signal **sp)
{
    t_linetraverser t;
    t_outconnect *oc;
    t_gobj *y;
    t_object *ob;
    t_symbol *dspsym = gensym("dsp");
    t_dspcontext *dc;    

        /* create a new "DSP graph" object to use in sorting this canvas.
        If we aren't toplevel, there are already other dspcontexts around. */

    dc = ugen_start_graph(toplevel, sp,
        object_numberOfSignalInlets(&x->gl_obj),
        object_numberOfSignalOutlets(&x->gl_obj));

        /* find all the "dsp" boxes and add them to the graph */
    
    for (y = x->gl_list; y; y = y->g_next)
        if ((ob = canvas_castToObjectIfBox(&y->g_pd)) && class_hasMethod (pd_class (&y->g_pd), dspsym))
            ugen_add(dc, ob);

        /* ... and all dsp interconnections */
    canvas_traverseLinesStart(&t, x);
    while (oc = canvas_traverseLinesNext(&t))
        if (object_isSignalOutlet(t.tr_srcObject, t.tr_srcIndexOfOutlet))
            ugen_connect(dc, t.tr_srcObject, t.tr_srcIndexOfOutlet, t.tr_destObject, t.tr_destIndexOfInlet);

        /* finally, sort them and add them to the DSP chain */
    ugen_done_graph(dc);
}

void canvas_dsp(t_glist *x, t_signal **sp)
{
    canvas_dodsp(x, 0, sp);
}

    /* this routine starts DSP for all root canvases. */
static void canvas_start_dsp(void)
{
    t_glist *x;
    if (pd_this->pd_dspState) ugen_stop();
    else sys_gui("set ::var(isDsp) 1\n");
    ugen_start();
    
    for (x = pd_this->pd_roots; x; x = x->gl_next)
        canvas_dodsp(x, 1, 0);
    
    pd_this->pd_dspState = 1;
}

static void canvas_stop_dsp(void)
{
    if (pd_this->pd_dspState)
    {
        ugen_stop();
        sys_gui("set ::var(isDsp) 0\n");
        pd_this->pd_dspState = 0;
    }
}

    /* DSP can be suspended before, and resumed after, operations which
    might affect the DSP chain.  For example, we suspend before loading and
    resume afterward, so that DSP doesn't get resorted for every DSP object
    int the patch. */

int canvas_suspend_dsp(void)
{
    int rval = pd_this->pd_dspState;
    if (rval) canvas_stop_dsp();
    return (rval);
}

void canvas_resume_dsp(int oldstate)
{
    if (oldstate) canvas_start_dsp();
}

    /* this is equivalent to suspending and resuming in one step. */
void canvas_update_dsp(void)
{
    if (pd_this->pd_dspState) canvas_start_dsp();
}

void global_dsp(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int newstate;
    
    if (argc)
    {
        newstate = (t_int)atom_getFloatAtIndex (0, argc, argv);
        
        if (newstate && !pd_this->pd_dspState)
        {
            if (audio_startDSP() == PD_ERROR_NONE) { canvas_start_dsp(); }
        }
        else if (!newstate && pd_this->pd_dspState)
        {
            canvas_stop_dsp();
            audio_stopDSP();
        }
    }
}

void *canvas_getblock(t_class *blockclass, t_glist **canvasp)
{
    t_glist *canvas = *canvasp;
    t_gobj *g;
    void *ret = 0;
    for (g = canvas->gl_list; g; g = g->g_next)
    {
        if (g->g_pd == blockclass)
            ret = g;
    }
    *canvasp = canvas->gl_owner;
    return(ret);
}
    
/******************* redrawing  data *********************/

    /* redraw all "scalars" (do this if a drawing command is changed.) 
    LATER we'll use the "template" information to select which ones we
    redraw.   Action = 0 for redraw, 1 for draw only, 2 for erase. */
static void glist_redrawall(t_glist *gl, int action)
{
    t_gobj *g;
    int vis = glist_isvisible(gl);
    for (g = gl->gl_list; g; g = g->g_next)
    {
        t_class *cl;
        if (vis && g->g_pd == scalar_class)
        {
            if (action == 1)
            {
                if (glist_isvisible(gl))
                    gobj_vis(g, gl, 1);
            }
            else if (action == 2)
            {
                if (glist_isvisible(gl))
                    gobj_vis(g, gl, 0);
            }
            else scalar_redraw((t_scalar *)g, gl);
        }
        else if (g->g_pd == canvas_class)
            glist_redrawall((t_glist *)g, action);
    }
}

    /* public interface for above. */
void canvas_redrawallfortemplate(t_template *template, int action)
{
    t_glist *x;
        /* find all root canvases */
    for (x = pd_this->pd_roots; x; x = x->gl_next)
        glist_redrawall(x, action);
}

    /* find the template defined by a canvas, and redraw all elements
    for that */
void canvas_redrawallfortemplatecanvas(t_glist *x, int action)
{
    t_gobj *g;
    t_template *tmpl;
    t_symbol *s1 = gensym("struct");
    for (g = x->gl_list; g; g = g->g_next)
    {
        t_object *ob = canvas_castToObjectIfBox(&g->g_pd);
        t_atom *argv;
        if (!ob || ob->te_type != TYPE_OBJECT ||
            buffer_size(ob->te_buffer) < 2)
            continue;
        argv = buffer_atoms(ob->te_buffer);
        if (argv[0].a_type != A_SYMBOL || argv[1].a_type != A_SYMBOL
            || argv[0].a_w.w_symbol != s1)
                continue;
        tmpl = template_findbyname(argv[1].a_w.w_symbol);
        canvas_redrawallfortemplate(tmpl, action);
    }
    canvas_redrawallfortemplate(0, action);
}

/* ------------------------------- declare ------------------------ */

/* put "declare" objects in a patch to tell it about the environment in
which objects should be created in this canvas.  This includes directories to
search ("-path", "-stdpath") and object libraries to load
("-lib" and "-stdlib").  These must be set before the patch containing
the "declare" object is filled in with its contents; so when the patch is
saved,  we throw early messages to the canvas to set the environment
before any objects are created in it. */

static t_class *declare_class;

typedef struct _declare
{
    t_object x_obj;
    t_glist *x_canvas;
    int x_useme;
} t_declare;

static void *declare_new(t_symbol *s, int argc, t_atom *argv)
{
    t_declare *x = (t_declare *)pd_new(declare_class);
    x->x_useme = 1;
    x->x_canvas = canvas_getCurrent();
        /* LATER update environment and/or load libraries */
    return (x);
}

static void declare_free(t_declare *x)
{
    x->x_useme = 0;
        /* LATER update environment */
}

void canvas_savedeclarationsto(t_glist *x, t_buffer *b)
{
    t_gobj *y;

    for (y = x->gl_list; y; y = y->g_next)
    {
        if (pd_class(&y->g_pd) == declare_class)
        {
            buffer_vAppend(b, "s", gensym("#X"));
            buffer_serialize(b, ((t_declare *)y)->x_obj.te_buffer);
            buffer_vAppend(b, ";");
        }
        else if (pd_class(&y->g_pd) == canvas_class)
            canvas_savedeclarationsto((t_glist *)y, b);
    }
}

/*
static void canvas_completepath(char *from, char *to, int bufsize)
{
    if (path_isAbsoluteConsideringEnvironment(from))
    {
        to[0] = '\0';
    }
    else
    {   // if not absolute path, append Pd lib dir
        strncpy(to, main_directoryRoot->s_name, bufsize-4);
        to[bufsize-3] = '\0';
        strcat(to, "/");
    }
    strncat(to, from, bufsize-strlen(to));
    to[bufsize-1] = '\0';
}
*/

/* maybe we should rename check_exists() to sys_access() and move it to s_path */
#ifdef _WIN32
static int check_exists(const char *filepath)
{
    char t[PD_STRING] = { 0 };
    wchar_t ucs2path[PD_STRING];
    
    if (string_copy (t, PD_STRING, filepath)) { PD_BUG; }
    path_slashToBackslashIfNecessary (t, t);
    u8_utf8toucs2(ucs2path, PD_STRING, t, -1);
    return (0 ==  _waccess(ucs2path, 0));
}
#else
#include <unistd.h>
static int check_exists(const char *filepath)
{
    return (0 == access (filepath, 0));
}
#endif

//extern t_pathlist *path_extra;

/*
static void canvas_stdpath(t_canvasenvironment *e, char *stdpath)
{
    t_pathlist*nl;
    char strbuf[PD_STRING];
    if (path_isAbsoluteConsideringEnvironment(stdpath))
    {
        e->ce_path = pathlist_newAppend(e->ce_path, stdpath);
        return;
    }

    canvas_completepath(stdpath, strbuf, PD_STRING);
    if (check_exists(strbuf))
    {
        e->ce_path = pathlist_newAppend(e->ce_path, strbuf);
        return;
    }

    if (!strncmp("extra/", stdpath, 6))
        stdpath+=6;

    for (nl=path_extra; nl; nl=nl->pl_next)
    {
        snprintf(strbuf, PD_STRING-1, "%s/%s/", nl->pl_string, stdpath);
        strbuf[PD_STRING-1]=0;
        if (check_exists(strbuf))
        {
            e->ce_path = pathlist_newAppend(e->ce_path, strbuf);
            return;
        }
    }
}

static void canvas_stdlib(t_canvasenvironment *e, char *stdlib)
{
    t_pathlist*nl;
    char strbuf[PD_STRING];
    if (path_isAbsoluteConsideringEnvironment(stdlib))
    {
        loader_loadExternal(0, stdlib);
        return;
    }

    canvas_completepath(stdlib, strbuf, PD_STRING);
    if (loader_loadExternal(0, strbuf))
        return;

    if (!strncmp("extra/", stdlib, 6))
        stdlib+=6;

    for (nl=path_extra; nl; nl=nl->pl_next)
    {
        snprintf(strbuf, PD_STRING-1, "%s/%s", nl->pl_string, stdlib);
        strbuf[PD_STRING-1]=0;
        if (loader_loadExternal(0, strbuf))
            return;
    }
}

*/
/*
static void canvas_declare(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    t_canvasenvironment *e = canvas_getEnvironment(x);
#if 0
    post("declare:: %s", s->s_name);
    post_atoms(argc, argv);
#endif
    for (i = 0; i < argc; i++)
    {
        char *flag = atom_getSymbolAtIndex(i, argc, argv)->s_name;
        if ((argc > i+1) && !strcmp(flag, "-path"))
        {
            e->ce_path = pathlist_newAppend(e->ce_path, atom_getSymbolAtIndex(i+1, argc, argv)->s_name);
            i++;
        }
        else if ((argc > i+1) && !strcmp(flag, "-stdpath"))
        {
            canvas_stdpath(e, atom_getSymbolAtIndex(i+1, argc, argv)->s_name);
            i++;
        }
        else if ((argc > i+1) && !strcmp(flag, "-lib"))
        {
            loader_loadExternal(x, atom_getSymbolAtIndex(i+1, argc, argv)->s_name);
            i++;
        }
        else if ((argc > i+1) && !strcmp(flag, "-stdlib"))
        {
            canvas_stdlib(e, atom_getSymbolAtIndex(i+1, argc, argv)->s_name);
            i++;
        }
        else post("declare: %s: unknown declaration", flag);
    }
}
*/
    /* utility function to read a file, looking first down the canvas's search
    path (set with "declare" objects in the patch and recursively in calling
    patches), then down the system one.  The filename is the concatenation of
    "name" and "ext".  "Name" may be absolute, or may be relative with
    slashes.  If anything can be opened, the true directory
    ais put in the buffer dirresult (provided by caller), which should
    be "size" bytes.  The "nameresult" pointer will be set somewhere in
    the interior of "dirresult" and will give the file basename (with
    slashes trimmed).  If "bin" is set a 'binary' open is
    attempted, otherwise ASCII (this only matters on Microsoft.) 
    If "x" is zero, the file is sought in the directory "." or in the
    global path.*/

int canvas_open(t_glist *x, const char *name, const char *ext,
    char *dirresult, char **nameresult, unsigned int size, int bin)
{
    t_pathlist *nl, thislist;
    int fd = -1;
    char listbuf[PD_STRING];
    t_glist *y;

        /* first check if "name" is absolute (and if so, try to open) */
    /* if ((fd = file_openWithAbsolutePath(name, ext, dirresult, nameresult, size)) >= 0)
        return (fd); */
    
        /* otherwise "name" is relative; start trying in directories named
        in this and parent environments */
    /*
    for (y = x; y; y = y->gl_owner)
        if (y->gl_env)
    {
        t_pathlist *nl;
        t_glist *x2 = x;
        char *dir;
        while (x2 && x2->gl_owner)
            x2 = x2->gl_owner;
        dir = (x2 ? canvas_getDirectory(x2)->s_name : ".");
        
        for (nl = y->gl_env->ce_path; nl; nl = nl->pl_next)
        {
            char realname[PD_STRING];
            if (0)
            {
                realname[0] = '\0';
            }
            else
            { 
                strncpy(realname, dir, PD_STRING);
                realname[PD_STRING-3] = 0;
                strcat(realname, "/");
            }
            strncat(realname, nl->pl_string, PD_STRING-strlen(realname));
            realname[PD_STRING-1] = 0;
            if ((fd = file_openWithDirectoryAndName(realname, name, ext,
                dirresult, nameresult, size)) >= 0)
                    return (fd);
        }
    }*/
    return (file_openConsideringSearchPath((x ? canvas_getEnvironment (x)->ce_directory->s_name : "."), name, ext,
        dirresult, nameresult, size));
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

extern t_class *array_define_class;     /* LATER datum class too */

    /* check if a pd can be treated as a glist - true if we're of any of
    the glist classes, which all have 'glist' as the first item in struct */
t_glist *pd_checkglist(t_pd *x)
{
    if (*x == canvas_class || *x == array_define_class)
        return ((t_glist *)x);
    else return (0);
}

/* ------------------------------- setup routine ------------------------ */

    /* why are some of these "glist" and others "canvas"? */
extern void glist_text(t_glist *x, t_symbol *s, int argc, t_atom *argv);
extern void canvas_obj(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_bng(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_toggle(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_vslider(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_hslider(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_hradio(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_vradio(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_vumeter(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_mycnv(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_numbox(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_msg(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_floatatom(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void canvas_symbolatom(t_glist *gl, t_symbol *s, int argc, t_atom *argv);
extern void glist_scalar(t_glist *canvas, t_symbol *s, int argc, t_atom *argv);

void g_graph_setup(void);
void g_editor_setup(void);
void g_readwrite_setup(void);
extern void canvas_properties(t_gobj *z, t_glist *canvas);

void g_canvas_setup(void)
{
        /* we prevent the user from typing "canvas" in an object box
        by sending 0 for a creator function. */
    canvas_class = class_new(gensym("canvas"), 0,
        (t_method)canvas_free, sizeof(t_glist), CLASS_NOINLET, 0);
            /* here is the real creator function, invoked in patch files
            by sending the "canvas" message to #N, which is bound
            to pd_camvasmaker. */
    class_addMethod(pd_canvasMaker, (t_method)canvas_new, gensym("canvas"),
        A_GIMME, 0);
    class_addMethod(canvas_class, (t_method)canvas_restore,
        gensym("restore"), A_GIMME, 0);
    class_addMethod(canvas_class, (t_method)canvas_coords,
        gensym("coords"), A_GIMME, 0);

/* -------------------------- objects ----------------------------- */
    class_addMethod(canvas_class, (t_method)canvas_obj,
        gensym("obj"), A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_msg,
        gensym("msg"), A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_floatatom,
        gensym("floatatom"), A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_symbolatom,
        gensym("symbolatom"), A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_text,
        gensym("text"), A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_glist, gensym("graph"),
        A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_scalar,
        gensym("scalar"), A_GIMME, A_NULL);

/* -------------- IEMGUI: button, toggle, slider, etc.  ------------ */
    class_addMethod(canvas_class, (t_method)canvas_bng, gensym("bng"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_toggle, gensym("toggle"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_vslider, gensym("vslider"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_hslider, gensym("hslider"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_hradio, gensym("hradio"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_vradio, gensym("vradio"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_vumeter, gensym("vumeter"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_mycnv, gensym("mycnv"),
                    A_GIMME, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_numbox, gensym("numbox"),
                    A_GIMME, A_NULL);

/* ------------------------ gui stuff --------------------------- */
    class_addMethod(canvas_class, (t_method)canvas_pop, gensym("pop"),
        A_DEFFLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_loadbang,
        gensym("loadbang"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_setbounds,
        gensym("setbounds"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_relocate,
        gensym("relocate"), A_SYMBOL, A_SYMBOL, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_vis,
        gensym("vis"), A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_menu_open,
        gensym("menu-open"), A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_map,
        gensym("map"), A_FLOAT, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_dirty,
        gensym("dirty"), A_FLOAT, A_NULL);
    class_setPropertiesFunction(canvas_class, canvas_properties);

/* ---------------------- list handling ------------------------ */
    class_addMethod(canvas_class, (t_method)glist_clear, gensym("clear"),
        A_NULL);

/* ----- subcanvases, which you get by typing "pd" in a box ---- */
    class_addCreator((t_newmethod)subcanvas_new, gensym("pd"), A_DEFSYMBOL, 0);
    class_addCreator((t_newmethod)subcanvas_new, gensym("page"),  A_DEFSYMBOL, 0);

    class_addMethod(canvas_class, (t_method)canvas_click,
        gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(canvas_class, (t_method)canvas_dsp,
        gensym("dsp"), A_CANT, 0);
    class_addMethod(canvas_class, (t_method)canvas_rename_method,
        gensym("rename"), A_GIMME, 0);

/*---------------------------- declare ------------------- */
    declare_class = class_new(gensym("declare"), (t_newmethod)declare_new,
        (t_method)declare_free, sizeof(t_declare), CLASS_NOINLET, A_GIMME, 0);
    /*class_addMethod(canvas_class, (t_method)canvas_declare,
        gensym("declare"), A_GIMME, 0);*/

/*--------------- future message to set formatting  -------------- */
    class_addMethod(canvas_class, (t_method)canvas_f,
        gensym("f"), A_GIMME, 0);
/* -------------- setups from other files for canvas_class ---------------- */
    g_graph_setup();
    g_editor_setup();
    g_readwrite_setup();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
