
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

extern t_class      *canvas_class;
extern t_class      *array_define_class;
extern t_pd         *pd_newest;
extern t_symbol     *canvas_fileName;
extern t_symbol     *canvas_directory;
extern t_atom       *canvas_argv;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int          canvas_argc;
extern int          canvas_magic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_bind (t_glist *glist)
{
    if (strcmp (glist->gl_name->s_name, "Pd")) {
        pd_bind (cast_pd (glist), canvas_makeBindSymbol (glist->gl_name));
    }
}

void canvas_unbind (t_glist *glist)
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

t_glist *canvas_castToGlist (t_pd *x)
{
    if (pd_class (x) == canvas_class || pd_class (x) == array_define_class) {
        return cast_glist (x);
    } else {
        return NULL;
    }
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


    /* get the document containing this canvas */
t_glist *canvas_getroot(t_glist *x)
{
    if ((!x->gl_owner) || canvas_isabstraction(x))
        return (x);
    else return (canvas_getroot(x->gl_owner));
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
        sym = gensym (buf);
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
    buffer_vAppend(x->gl_obj.te_buffer, "s", gensym ("graph"));
    if (!menu)
        stack_push(&x->gl_obj.te_g.g_pd);
    glist_add(g, &x->gl_obj.te_g);
    return (x);
}

    /* return true if the glist should appear as a graph on parent;
    otherwise it appears as a text box. */
int glist_isgraph(t_glist *x)
{
  return (x->gl_isgraph|(x->gl_hidetext<<1));
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

int canvas_isabstraction(t_glist *x)
{
    return (x->gl_env != 0);
}

void canvas_popabstraction(t_glist *x)
{
    pd_newest = &x->gl_obj.te_g.g_pd;
    stack_pop(&x->gl_obj.te_g.g_pd);
    x->gl_loading = 0;
    canvas_resortinlets(x);
    canvas_resortoutlets(x);
}

    /* return true if the "canvas" object should be treated as a text
    object.  This is true for abstractions but also for "table"s... */
/* JMZ: add a flag to gop-abstractions to hide the title */
int canvas_showtext(t_glist *x)
{
    t_atom *argv = (x->gl_obj.te_buffer? buffer_atoms(x->gl_obj.te_buffer):0);
    int argc = (x->gl_obj.te_buffer? buffer_size(x->gl_obj.te_buffer) : 0);
    int isarray = (argc && argv[0].a_type == A_SYMBOL &&
        argv[0].a_w.w_symbol == gensym ("graph"));
    if(x->gl_hidetext)
      return 0;
    else
      return (!isarray);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int canvas_open(t_glist *x, const char *name, const char *ext,
    char *dirresult, char **nameresult, size_t size, int bin)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
