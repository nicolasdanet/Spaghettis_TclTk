
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
#include "g_graphics.h"

/* ---------------- curves and polygons (joined segments) ---------------- */

/*
curves belong to templates and describe how the data in the template are to
be drawn.  The coordinates of the curve (and other display features) can
be attached to fields in the template.
*/

t_class *curve_class;

#define CURVE_CLOSED    1
#define CURVE_BEZIER    2
#define CURVE_NO_MOUSE  4

typedef struct _curve
{
    t_object x_obj;
    int x_flags;
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
        flags |= CURVE_CLOSED;
    }
    else classname += 4;
    if (classname[0] == 'c') flags |= CURVE_BEZIER;
    field_setAsFloatConstant(&x->x_vis, 1);
    while (1)
    {
        t_symbol *firstarg = atom_getSymbolAtIndex(0, argc, argv);
        if (!strcmp(firstarg->s_name, "-v") && argc > 1)
        {
            field_setAsFloat(&x->x_vis, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-x"))
        {
            flags |= CURVE_NO_MOUSE;
            argc -= 1; argv += 1;
        }
        else break;
    }
    x->x_flags = flags;
    if ((flags & CURVE_CLOSED) && argc)
        field_setAsFloat(&x->x_fillcolor, argc--, argv++);
    else field_setAsFloatConstant(&x->x_fillcolor, 0); 
    if (argc) field_setAsFloat(&x->x_outlinecolor, argc--, argv++);
    else field_setAsFloatConstant(&x->x_outlinecolor, 0);
    if (argc) field_setAsFloat(&x->x_width, argc--, argv++);
    else field_setAsFloatConstant(&x->x_width, 1);
    if (argc < 0) argc = 0;
    nxy =  (argc + (argc & 1));
    x->x_npoints = (nxy>>1);
    x->x_vec = (t_fielddescriptor *)PD_MEMORY_GET(nxy * sizeof(t_fielddescriptor));
    for (i = 0, fd = x->x_vec; i < argc; i++, fd++, argv++)
        field_setAsFloat(fd, 1, argv);
    if (argc & 1) field_setAsFloatConstant(fd, 0);

    return (x);
}

void curve_float(t_curve *x, t_float f)
{
    int viswas;
    if (!field_isFloatConstant (&x->x_vis))
    {
        post_error ("global vis/invis for a template with variable visibility");
        return;
    }
    
    viswas = (field_getFloatConstant (&x->x_vis) != 0);
    
    if ((f != 0 && viswas) || (f == 0 && !viswas))
        return;
    canvas_paintAllScalarsByView(x->x_canvas, SCALAR_ERASE);
    field_setAsFloatConstant(&x->x_vis, (f != 0));
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
    if (!word_getFloatByField(data, template, &x->x_vis) ||
        (x->x_flags & CURVE_NO_MOUSE))
    {
        *xp1 = *yp1 = PD_INT_MAX;
        *xp2 = *yp2 = -PD_INT_MAX;
        return;
    }
    for (i = 0, f = x->x_vec; i < n; i++, f += 2)
    {
        int xloc = canvas_valueToPositionX(glist,
            basex + word_getFloatByFieldAsPosition (data, template, f));
        int yloc = canvas_valueToPositionY(glist,
            basey + word_getFloatByFieldAsPosition(data, template, f+1));
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
    if (vis && !word_getFloatByField(data, template, &x->x_vis))
        return;
    if (vis)
    {
        if (n > 1)
        {
            int flags = x->x_flags, closed = (flags & CURVE_CLOSED);
            t_float width = word_getFloatByField(data, template, &x->x_width);
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
                    basex + word_getFloatByFieldAsPosition(data, template, f));
                pix[2*i+1] = canvas_valueToPositionY(glist,
                    basey + word_getFloatByFieldAsPosition(data, template, f+1));
            }
            if (width < 1) width = 1;
            numbertocolor(
                word_getFloatByField(data, template, &x->x_outlinecolor),
                outline);
            if (flags & CURVE_CLOSED)
            {
                numbertocolor(
                    word_getFloatByField(data, template, &x->x_fillcolor),
                    fill);
                sys_vGui(".x%lx.c create polygon\\\n",
                    canvas_getView(glist));
            }
            else sys_vGui(".x%lx.c create line\\\n", canvas_getView(glist));
            for (i = 0; i < n; i++)
                sys_vGui("%d %d\\\n", pix[2*i], pix[2*i+1]);
            sys_vGui("-width %f\\\n", width);
            if (flags & CURVE_CLOSED) sys_vGui("-fill %s -outline %s\\\n",
                fill, outline);
            else sys_vGui("-fill %s\\\n", outline);
            if (flags & CURVE_BEZIER) sys_vGui("-smooth 1\\\n");
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
    //t_atom at;
    if (!gpointer_isValid(&curve_motion_gpointer, 0))
    {
        post("curve_motion: scalar disappeared");
        return;
    }
    curve_motion_xcumulative += dx;
    curve_motion_ycumulative += dy;
    if (field_isVariable (f) && (dx != 0))
    {
        word_setFloatByFieldAsPosition(
            curve_motion_wp,
            curve_motion_template,
            f,
            curve_motion_xbase + curve_motion_xcumulative * curve_motion_xper); 
    }
    if (field_isVariable (f+1) && (dy != 0))
    {
        word_setFloatByFieldAsPosition(
            curve_motion_wp,
            curve_motion_template,
            f+1,
            curve_motion_ybase + curve_motion_ycumulative * curve_motion_yper); 
    }
        /* LATER figure out what to do to notify for an array? */
    if (curve_motion_scalar)
        template_notify(curve_motion_template, curve_motion_glist, 
            curve_motion_scalar, sym_change, 0, NULL);
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
    if (!word_getFloatByField(data, template, &x->x_vis))
        return (0);
    for (i = 0, f = x->x_vec; i < n; i++, f += 2)
    {
        int xval = word_getFloatByFieldAsPosition(data, template, f),
            xloc = canvas_valueToPositionX(glist, basex + xval);
        int yval = word_getFloatByFieldAsPosition(data, template, f+1),
            yloc = canvas_valueToPositionY(glist, basey + yval);
        int xerr = xloc - xpix, yerr = yloc - ypix;
        if (!field_isVariable (f) && !field_isVariable (f+1))
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
