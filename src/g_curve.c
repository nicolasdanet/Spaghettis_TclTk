
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
#include "s_system.h"    /* for font_getHostFontSize */
#include "g_graphics.h"

extern t_class *garray_class;
extern t_class *scalar_class;
extern t_pd pd_canvasMaker;
extern t_class *canvas_class;
extern t_pdinstance *pd_this;

/* ---------------- curves and polygons (joined segments) ---------------- */

/*
curves belong to templates and describe how the data in the template are to
be drawn.  The coordinates of the curve (and other display features) can
be attached to fields in the template.
*/

t_class *curve_class;

typedef struct _curve
{
    t_object x_obj;
    int x_flags;            /* CLOSED and/or BEZ and/or NOMOUSE */
    t_fielddescriptor x_fillcolor;
    t_fielddescriptor x_outlinecolor;
    t_fielddescriptor x_width;
    t_fielddescriptor x_vis;
    int x_npoints;
    t_fielddescriptor *x_vec;
    t_glist *x_canvas;
} t_curve;

static void *curve_new(t_symbol *classsym, int argc, t_atom *argv)
{
    t_curve *x = (t_curve *)pd_new(curve_class);
    char *classname = classsym->s_name;
    int flags = 0;
    int nxy, i;
    t_fielddescriptor *fd;
    x->x_canvas = canvas_getCurrent();
    if (classname[0] == 'f')
    {
        classname += 6;
        flags |= CLOSED;
    }
    else classname += 4;
    if (classname[0] == 'c') flags |= BEZ;
    fielddesc_setfloat_const(&x->x_vis, 1);
    while (1)
    {
        t_symbol *firstarg = atom_getSymbolAtIndex(0, argc, argv);
        if (!strcmp(firstarg->s_name, "-v") && argc > 1)
        {
            fielddesc_setfloatarg(&x->x_vis, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-x"))
        {
            flags |= NOMOUSE;
            argc -= 1; argv += 1;
        }
        else break;
    }
    x->x_flags = flags;
    if ((flags & CLOSED) && argc)
        fielddesc_setfloatarg(&x->x_fillcolor, argc--, argv++);
    else fielddesc_setfloat_const(&x->x_fillcolor, 0); 
    if (argc) fielddesc_setfloatarg(&x->x_outlinecolor, argc--, argv++);
    else fielddesc_setfloat_const(&x->x_outlinecolor, 0);
    if (argc) fielddesc_setfloatarg(&x->x_width, argc--, argv++);
    else fielddesc_setfloat_const(&x->x_width, 1);
    if (argc < 0) argc = 0;
    nxy =  (argc + (argc & 1));
    x->x_npoints = (nxy>>1);
    x->x_vec = (t_fielddescriptor *)PD_MEMORY_GET(nxy * sizeof(t_fielddescriptor));
    for (i = 0, fd = x->x_vec; i < argc; i++, fd++, argv++)
        fielddesc_setfloatarg(fd, 1, argv);
    if (argc & 1) fielddesc_setfloat_const(fd, 0);

    return (x);
}

void curve_float(t_curve *x, t_float f)
{
    int viswas;
    if (x->x_vis.fd_type != A_FLOAT || x->x_vis.fd_var)
    {
        post_error ("global vis/invis for a template with variable visibility");
        return;
    }
    viswas = (x->x_vis.fd_un.fd_float != 0);
    
    if ((f != 0 && viswas) || (f == 0 && !viswas))
        return;
    canvas_paintAllScalarsByView(x->x_canvas, SCALAR_ERASE);
    fielddesc_setfloat_const(&x->x_vis, (f != 0));
    canvas_paintAllScalarsByView(x->x_canvas, SCALAR_DRAW);
}

/* -------------------- widget behavior for curve ------------ */

static void curve_getrect(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_curve *x = (t_curve *)z;
    int i, n = x->x_npoints;
    t_fielddescriptor *f = x->x_vec;
    int x1 = PD_INT_MAX, x2 = -PD_INT_MAX, y1 = PD_INT_MAX, y2 = -PD_INT_MAX;
    if (!fielddesc_getfloat(&x->x_vis, template, data, 0) ||
        (x->x_flags & NOMOUSE))
    {
        *xp1 = *yp1 = PD_INT_MAX;
        *xp2 = *yp2 = -PD_INT_MAX;
        return;
    }
    for (i = 0, f = x->x_vec; i < n; i++, f += 2)
    {
        int xloc = canvas_valueToPositionX(glist,
            basex + fielddesc_getcoord(f, template, data, 0));
        int yloc = canvas_valueToPositionY(glist,
            basey + fielddesc_getcoord(f+1, template, data, 0));
        if (xloc < x1) x1 = xloc;
        if (xloc > x2) x2 = xloc;
        if (yloc < y1) y1 = yloc;
        if (yloc > y2) y2 = yloc;
    }
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2; 
}

static void curve_displace(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int dx, int dy)
{
    /* refuse */
}

static void curve_select(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* fill in later */
}

static void curve_activate(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* fill in later */
}

#if 0
static int rangecolor(int n)    /* 0 to 9 in 5 steps */
{
    int n2 = n/2;               /* 0 to 4 */
    int ret = (n2 << 6);        /* 0 to 256 in 5 steps */
    if (ret > 255) ret = 255;
    return (ret);
}
#endif

static int rangecolor(int n)    /* 0 to 9 in 5 steps */
{
    int n2 = (n == 9 ? 8 : n);               /* 0 to 8 */
    int ret = (n2 << 5);        /* 0 to 256 in 9 steps */
    if (ret > 255) ret = 255;
    return (ret);
}

void numbertocolor(int n, char *s)
{
    int red, blue, green;
    if (n < 0) n = 0;
    red = n / 100;
    blue = ((n / 10) % 10);
    green = n % 10;
    sprintf(s, "#%2.2x%2.2x%2.2x", rangecolor(red), rangecolor(blue),
        rangecolor(green));
}

static void curve_vis(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_float basex, t_float basey,
    int vis)
{
    t_curve *x = (t_curve *)z;
    int i, n = x->x_npoints;
    t_fielddescriptor *f = x->x_vec;
    
        /* see comment in plot_vis() */
    if (vis && !fielddesc_getfloat(&x->x_vis, template, data, 0))
        return;
    if (vis)
    {
        if (n > 1)
        {
            int flags = x->x_flags, closed = (flags & CLOSED);
            t_float width = fielddesc_getfloat(&x->x_width, template, data, 1);
            char outline[20], fill[20];
            int pix[200];
            if (n > 100)
                n = 100;
                /* calculate the pixel values before we start printing
                out the TK message so that "error" printout won't be
                interspersed with it.  Only show up to 100 points so we don't
                have to allocate memory here. */
            for (i = 0, f = x->x_vec; i < n; i++, f += 2)
            {
                pix[2*i] = canvas_valueToPositionX(glist,
                    basex + fielddesc_getcoord(f, template, data, 1));
                pix[2*i+1] = canvas_valueToPositionY(glist,
                    basey + fielddesc_getcoord(f+1, template, data, 1));
            }
            if (width < 1) width = 1;
            numbertocolor(
                fielddesc_getfloat(&x->x_outlinecolor, template, data, 1),
                outline);
            if (flags & CLOSED)
            {
                numbertocolor(
                    fielddesc_getfloat(&x->x_fillcolor, template, data, 1),
                    fill);
                sys_vGui(".x%lx.c create polygon\\\n",
                    canvas_getView(glist));
            }
            else sys_vGui(".x%lx.c create line\\\n", canvas_getView(glist));
            for (i = 0; i < n; i++)
                sys_vGui("%d %d\\\n", pix[2*i], pix[2*i+1]);
            sys_vGui("-width %f\\\n", width);
            if (flags & CLOSED) sys_vGui("-fill %s -outline %s\\\n",
                fill, outline);
            else sys_vGui("-fill %s\\\n", outline);
            if (flags & BEZ) sys_vGui("-smooth 1\\\n");
            sys_vGui("-tags curve%lx\n", data);
        }
        else post("warning: curves need at least two points to be graphed");
    }
    else
    {
        if (n > 1) sys_vGui(".x%lx.c delete curve%lx\n",
            canvas_getView(glist), data);      
    }
}

static int curve_motion_field;
static t_float curve_motion_xcumulative;
static t_float curve_motion_xbase;
static t_float curve_motion_xper;
static t_float curve_motion_ycumulative;
static t_float curve_motion_ybase;
static t_float curve_motion_yper;
static t_glist *curve_motion_glist;
static t_scalar *curve_motion_scalar;
static t_array *curve_motion_array;
static t_word *curve_motion_wp;
static t_template *curve_motion_template;
static t_gpointer curve_motion_gpointer;

    /* LATER protect against the template changing or the scalar disappearing
    probably by attaching a gpointer here ... */

static void curve_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    t_curve *x = (t_curve *)z;
    t_fielddescriptor *f = x->x_vec + curve_motion_field;
    t_atom at;
    if (!gpointer_isValid(&curve_motion_gpointer, 0))
    {
        post("curve_motion: scalar disappeared");
        return;
    }
    curve_motion_xcumulative += dx;
    curve_motion_ycumulative += dy;
    if (f->fd_var && (dx != 0))
    {
        fielddesc_setcoord(f, curve_motion_template, curve_motion_wp,
            curve_motion_xbase + curve_motion_xcumulative * curve_motion_xper,
                1); 
    }
    if ((f+1)->fd_var && (dy != 0))
    {
        fielddesc_setcoord(f+1, curve_motion_template, curve_motion_wp,
            curve_motion_ybase + curve_motion_ycumulative * curve_motion_yper,
                1); 
    }
        /* LATER figure out what to do to notify for an array? */
    if (curve_motion_scalar)
        template_notifyforscalar(curve_motion_template, curve_motion_glist, 
            curve_motion_scalar, sym_change, 1, &at);
    if (curve_motion_scalar)
        scalar_redraw(curve_motion_scalar, curve_motion_glist);
    if (curve_motion_array)
        array_redraw(curve_motion_array, curve_motion_glist);
}

static int curve_click(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_curve *x = (t_curve *)z;
    int i, n = x->x_npoints;
    int bestn = -1;
    int besterror = PD_INT_MAX;
    t_fielddescriptor *f;
    if (!fielddesc_getfloat(&x->x_vis, template, data, 0))
        return (0);
    for (i = 0, f = x->x_vec; i < n; i++, f += 2)
    {
        int xval = fielddesc_getcoord(f, template, data, 0),
            xloc = canvas_valueToPositionX(glist, basex + xval);
        int yval = fielddesc_getcoord(f+1, template, data, 0),
            yloc = canvas_valueToPositionY(glist, basey + yval);
        int xerr = xloc - xpix, yerr = yloc - ypix;
        if (!f->fd_var && !(f+1)->fd_var)
            continue;
        if (xerr < 0)
            xerr = -xerr;
        if (yerr < 0)
            yerr = -yerr;
        if (yerr > xerr)
            xerr = yerr;
        if (xerr < besterror)
        {
            curve_motion_xbase = xval;
            curve_motion_ybase = yval;
            besterror = xerr;
            bestn = i;
        }
    }
    if (besterror > 6)
        return (0);
    if (doit)
    {
        curve_motion_xper = canvas_positionToValueX(glist, 1)
            - canvas_positionToValueX(glist, 0);
        curve_motion_yper = canvas_positionToValueY(glist, 1)
            - canvas_positionToValueY(glist, 0);
        curve_motion_xcumulative = 0;
        curve_motion_ycumulative = 0;
        curve_motion_glist = glist;
        curve_motion_scalar = sc;
        curve_motion_array = ap;
        curve_motion_wp = data;
        curve_motion_field = 2*bestn;
        curve_motion_template = template;
        if (curve_motion_scalar)
            gpointer_setAsScalarType(&curve_motion_gpointer, curve_motion_glist,
                curve_motion_scalar);
        else gpointer_setAsWordType(&curve_motion_gpointer,
                curve_motion_array, curve_motion_wp);
        canvas_setMotionFunction(glist, z, (t_motionfn)curve_motion, xpix, ypix);
    }
    return (1);
}

t_parentwidgetbehavior curve_widgetbehavior =
{
    curve_getrect,
    curve_displace,
    curve_select,
    curve_activate,
    curve_vis,
    curve_click,
};

static void curve_free(t_curve *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

void curve_setup(void)
{
    curve_class = class_new(sym_drawpolygon, (t_newmethod)curve_new,
        (t_method)curve_free, sizeof(t_curve), 0, A_GIMME, 0);
    class_setDrawCommand(curve_class);
    class_addCreator((t_newmethod)curve_new, sym_drawcurve,
        A_GIMME, 0);
    class_addCreator((t_newmethod)curve_new, sym_filledpolygon,
        A_GIMME, 0);
    class_addCreator((t_newmethod)curve_new, sym_filledcurve,
        A_GIMME, 0);
    class_setParentWidgetBehavior(curve_class, &curve_widgetbehavior);
    class_addFloat(curve_class, curve_float);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
