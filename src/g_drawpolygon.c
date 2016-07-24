
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DRAWPOLYGON_NONE            0
#define DRAWPOLYGON_CLOSED          1
#define DRAWPOLYGON_BEZIER          2
#define DRAWPOLYGON_NO_MOUSE        4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          drawpolygon_motionField;            /* Shared. */
static t_float      drawpolygon_motionCumulativeX;      /* Shared. */
static t_float      drawpolygon_motionCumulativeY;      /* Shared. */
static t_float      drawpolygon_motionBaseX;            /* Shared. */
static t_float      drawpolygon_motionBaseY;            /* Shared. */
static t_float      drawpolygon_motionStepX;            /* Shared. */
static t_float      drawpolygon_motionStepY;            /* Shared. */
static t_gpointer   drawpolygon_motionPointer;          /* Shared. */   /* ??? */

static t_glist      *drawpolygon_motionView;            /* Shared. */
static t_scalar     *drawpolygon_motionScalar;          /* Shared. */
static t_array      *drawpolygon_motionArray;           /* Shared. */
static t_word       *drawpolygon_motionData;            /* Shared. */
static t_template   *drawpolygon_motionTemplate;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *drawpolygon_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _drawpolygon {
    t_object            x_obj;
    int                 x_flags;
    t_fielddescriptor   x_colorFill;
    t_fielddescriptor   x_colorOutline;
    t_fielddescriptor   x_width;
    t_fielddescriptor   x_isVisible;
    int                 x_numberOfPoints;
    int                 x_size;
    t_fielddescriptor   *x_coordinates;
    } t_drawpolygon;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void drawpolygon_float (t_drawpolygon *x, t_float f)
{
    if (field_isFloatConstant (&x->x_isVisible)) {
    //
    int k = (f != 0.0);
    
    if (k != (int)(field_getFloatConstant (&x->x_isVisible))) {
    //
    paint_scalarsEraseAll();
    field_setAsFloatConstant (&x->x_isVisible, (t_float)k);
    paint_scalarsDrawAll();
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void drawpolygon_motion (void *z, t_float dx, t_float dy, t_float modifier)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    t_fielddescriptor *f = x->x_coordinates + drawpolygon_motionField;
    //t_atom at;
    if (!gpointer_isValid(&drawpolygon_motionPointer, 0))
    {
        post("drawpolygon_motion: scalar disappeared");
        return;
    }
    drawpolygon_motionCumulativeX += dx;
    drawpolygon_motionCumulativeY += dy;
    if (field_isVariable (f) && (dx != 0))
    {
        word_setFloatByFieldAsPosition(
            drawpolygon_motionData,
            drawpolygon_motionTemplate,
            f,
            drawpolygon_motionBaseX + drawpolygon_motionCumulativeX * drawpolygon_motionStepX); 
    }
    if (field_isVariable (f+1) && (dy != 0))
    {
        word_setFloatByFieldAsPosition(
            drawpolygon_motionData,
            drawpolygon_motionTemplate,
            f+1,
            drawpolygon_motionBaseY + drawpolygon_motionCumulativeY * drawpolygon_motionStepY); 
    }
        /* LATER figure out what to do to notify for an array? */
    if (drawpolygon_motionScalar)
        template_notify(drawpolygon_motionTemplate, drawpolygon_motionView, 
            drawpolygon_motionScalar, sym_change, 0, NULL);
    if (drawpolygon_motionScalar)
        scalar_redraw(drawpolygon_motionScalar, drawpolygon_motionView);
    if (drawpolygon_motionArray)
        array_redraw(drawpolygon_motionArray, drawpolygon_motionView);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawpolygon_behaviorGetRectangle(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    int i, n = x->x_numberOfPoints;
    t_fielddescriptor *f = x->x_coordinates;
    int x1 = PD_INT_MAX, x2 = -PD_INT_MAX, y1 = PD_INT_MAX, y2 = -PD_INT_MAX;
    if (!word_getFloatByField(data, template, &x->x_isVisible) ||
        (x->x_flags & DRAWPOLYGON_NO_MOUSE))
    {
        *xp1 = *yp1 = PD_INT_MAX;
        *xp2 = *yp2 = -PD_INT_MAX;
        return;
    }
    for (i = 0, f = x->x_coordinates; i < n; i++, f += 2)
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

static void drawpolygon_behaviorDisplaced(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int dx, int dy)
{
    /* refuse */
}

static void drawpolygon_behaviorSelected(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* fill in later */
}

static void drawpolygon_behaviorActivated(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* fill in later */
}

static void drawpolygon_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_float basex, t_float basey,
    int vis)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    int i, n = x->x_numberOfPoints;
    t_fielddescriptor *f = x->x_coordinates;
    
        /* see comment in plot_vis() */
    if (vis && !word_getFloatByField(data, template, &x->x_isVisible))
        return;
    if (vis)
    {
        if (n > 1)
        {
            int flags = x->x_flags, closed = (flags & DRAWPOLYGON_CLOSED);
            t_float width = word_getFloatByField(data, template, &x->x_width);
            char outline[20], fill[20];
            int pix[200];
            if (n > 100)
                n = 100;
                /* calculate the pixel values before we start printing
                out the TK message so that "error" printout won't be
                interspersed with it.  Only show up to 100 points so we don't
                have to allocate memory here. */
            for (i = 0, f = x->x_coordinates; i < n; i++, f += 2)
            {
                pix[2*i] = canvas_valueToPositionX(glist,
                    basex + word_getFloatByFieldAsPosition(data, template, f));
                pix[2*i+1] = canvas_valueToPositionY(glist,
                    basey + word_getFloatByFieldAsPosition(data, template, f+1));
            }
            if (width < 1) width = 1;
            color_toEncodedString(outline, 20,
                color_withDigits (word_getFloatByField(data, template, &x->x_colorOutline)));
            if (flags & DRAWPOLYGON_CLOSED)
            {
                color_toEncodedString(fill, 20,
                    color_withDigits (word_getFloatByField(data, template, &x->x_colorFill)));
                sys_vGui(".x%lx.c create polygon\\\n",
                    canvas_getView(glist));
            }
            else sys_vGui(".x%lx.c create line\\\n", canvas_getView(glist));
            for (i = 0; i < n; i++)
                sys_vGui("%d %d\\\n", pix[2*i], pix[2*i+1]);
            sys_vGui("-width %f\\\n", width);
            if (flags & DRAWPOLYGON_CLOSED) sys_vGui("-fill %s -outline %s\\\n",
                fill, outline);
            else sys_vGui("-fill %s\\\n", outline);
            if (flags & DRAWPOLYGON_BEZIER) sys_vGui("-smooth 1\\\n");
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

static int drawpolygon_behaviorClicked(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    int i, n = x->x_numberOfPoints;
    int bestn = -1;
    int besterror = PD_INT_MAX;
    t_fielddescriptor *f;
    if (!word_getFloatByField(data, template, &x->x_isVisible))
        return (0);
    for (i = 0, f = x->x_coordinates; i < n; i++, f += 2)
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
            drawpolygon_motionBaseX = xval;
            drawpolygon_motionBaseY = yval;
            besterror = xerr;
            bestn = i;
        }
    }
    if (besterror > 6)
        return (0);
    if (doit)
    {
        drawpolygon_motionStepX = canvas_positionToValueX(glist, 1)
            - canvas_positionToValueX(glist, 0);
        drawpolygon_motionStepY = canvas_positionToValueY(glist, 1)
            - canvas_positionToValueY(glist, 0);
        drawpolygon_motionCumulativeX = 0;
        drawpolygon_motionCumulativeY = 0;
        drawpolygon_motionView = glist;
        drawpolygon_motionScalar = sc;
        drawpolygon_motionArray = ap;
        drawpolygon_motionData = data;
        drawpolygon_motionField = 2*bestn;
        drawpolygon_motionTemplate = template;
        if (drawpolygon_motionScalar)
            gpointer_setAsScalar(&drawpolygon_motionPointer, drawpolygon_motionView,
                drawpolygon_motionScalar);
        else gpointer_setAsWord(&drawpolygon_motionPointer,
                drawpolygon_motionArray, drawpolygon_motionData);
        canvas_setMotionFunction(glist, z, (t_motionfn)drawpolygon_motion, xpix, ypix);
    }
    return (1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_parentwidgetbehavior drawpolygon_parentWidgetBehavior =
    {
        drawpolygon_behaviorGetRectangle,
        drawpolygon_behaviorDisplaced,
        drawpolygon_behaviorSelected,
        drawpolygon_behaviorActivated,
        drawpolygon_behaviorVisibilityChanged,
        drawpolygon_behaviorClicked,
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *drawpolygon_new (t_symbol *s, int argc, t_atom *argv)
{
    int i;
        
    t_drawpolygon *x = (t_drawpolygon *)pd_new (drawpolygon_class);

    x->x_flags = DRAWPOLYGON_NONE;
    
    if (s == sym_filledcurve || s == sym_filledpolygon) { x->x_flags |= DRAWPOLYGON_CLOSED; }
    if (s == sym_filledcurve || s == sym_drawcurve)     { x->x_flags |= DRAWPOLYGON_BEZIER; }
    
    field_setAsFloatConstant (&x->x_colorFill, 0.0);
    field_setAsFloatConstant (&x->x_colorOutline, 0.0);
    field_setAsFloatConstant (&x->x_width, 1.0);
    field_setAsFloatConstant (&x->x_isVisible, 1.0);
    
    while (argc > 0) {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
    
    if (argc > 1 && (t == sym___dash__v || t == sym___dash__visible)) {
        field_setAsFloat (&x->x_isVisible, 1, argv + 1);
        argc -= 2; argv += 2;
        
    } else if (t == sym___dash__x || t == sym___dash__inhibit) {
        x->x_flags |= DRAWPOLYGON_NO_MOUSE;
        argc -= 1; argv += 1;
        
    } else { break; }
    //
    }
    
    if (argc && (x->x_flags & DRAWPOLYGON_CLOSED)) { field_setAsFloat (&x->x_colorFill, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_colorOutline, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_width, argc--, argv++); }

    argc = PD_MAX (0, argc);
    
    x->x_numberOfPoints = argc / 2;
    x->x_size           = x->x_numberOfPoints * 2;
    x->x_coordinates    = (t_fielddescriptor *)PD_MEMORY_GET (x->x_size * sizeof (t_fielddescriptor));
    
    for (i = 0; i < x->x_size; i++) { field_setAsFloat (x->x_coordinates + i, 1, argv + i); }

    return x;
}

static void drawpolygon_free (t_drawpolygon *x)
{
    PD_MEMORY_FREE (x->x_coordinates);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void drawpolygon_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_drawpolygon,
            (t_newmethod)drawpolygon_new,
            (t_method)drawpolygon_free,
            sizeof (t_drawpolygon),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addCreator ((t_newmethod)drawpolygon_new, sym_drawcurve,      A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)drawpolygon_new, sym_filledpolygon,  A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)drawpolygon_new, sym_filledcurve,    A_GIMME, A_NULL);
    
    class_addFloat (c, drawpolygon_float);
        
    class_setParentWidgetBehavior (c, &drawpolygon_parentWidgetBehavior);
    
    drawpolygon_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
