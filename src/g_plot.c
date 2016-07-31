
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

extern t_class  *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *plot_class;                            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float              plot_cumulativeX;           /* Shared. */
static t_float              plot_cumulativeY;           /* Shared. */
static t_float              plot_stepX;                 /* Shared. */
static t_float              plot_stepY;                 /* Shared. */
static int                  plot_currentX;              /* Shared. */
static int                  plot_previousX;             /* Shared. */
static int                  plot_numberOfPoints;        /* Shared. */
static int                  plot_elementSize;           /* Shared. */
static int                  plot_fatten;                /* Shared. */

static t_fielddescriptor    *plot_fieldX;               /* Shared. */
static t_fielddescriptor    *plot_fieldY;               /* Shared. */
static t_glist              *plot_glist;                /* Shared. */
static t_scalar             *plot_scalar;               /* Shared. */
static t_array              *plot_array;                /* Shared. */
static t_word               *plot_data;                 /* Shared. */
static t_template           *plot_template;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _plot {
    t_object            x_obj;                          /* Must be the first. */
    t_fielddescriptor   x_colorOutline;
    t_fielddescriptor   x_width;
    t_fielddescriptor   x_positionX;
    t_fielddescriptor   x_positionY;
    t_fielddescriptor   x_incrementX;
    t_fielddescriptor   x_style;
    t_fielddescriptor   x_array;
    t_fielddescriptor   x_fieldX;
    t_fielddescriptor   x_fieldY;
    t_fielddescriptor   x_fieldW;
    t_fielddescriptor   x_isVisible;
    t_fielddescriptor   x_isScalarsVisible;
    } t_plot;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_motion (void *, t_float, t_float, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_MAX        1e20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PLOT_CLIP(x)    (((x) > -PLOT_MAX && (x) < PLOT_MAX) ? (x) : 0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void array_getcoordinate (t_glist *glist,
    char *elem, int xonset, int yonset, int wonset, int indx,
    t_float basex, t_float basey, t_float xinc,
    t_fielddescriptor *xfielddesc, t_fielddescriptor *yfielddesc, t_fielddescriptor *wfielddesc,
    t_float *xp, t_float *yp, t_float *wp)
{
    t_float xval, yval, ypix, wpix;
    if (xonset >= 0)
        xval = *(t_float *)(elem + xonset);
    else xval = indx * xinc;
    if (yonset >= 0)
        yval = *(t_float *)(elem + yonset);
    else yval = 0;
    ypix = canvas_valueToPositionY(glist, basey +
        field_convertValueToPosition(yfielddesc, yval));
    if (wonset >= 0)
    {
            /* found "w" field which controls linewidth. */
        t_float wval = *(t_float *)(elem + wonset);
        wpix = canvas_valueToPositionY(glist, basey + 
            field_convertValueToPosition(yfielddesc, yval) +
                field_convertValueToPosition(wfielddesc, wval)) - ypix;
        if (wpix < 0)
            wpix = -wpix;
    }
    else wpix = 1;
    *xp = canvas_valueToPositionX(glist, basex +
        field_convertValueToPosition(xfielddesc, xval));
    *yp = ypix;
    *wp = wpix;
}

static int plot_readownertemplate(t_plot *x,
    t_word *data, t_template *ownertemplate, 
    t_symbol **elemtemplatesymp, t_array **arrayp,
    t_float *linewidthp, t_float *xlocp, t_float *xincp, t_float *ylocp, t_float *stylep,
    t_float *visp, t_float *scalarvisp,
    t_fielddescriptor **xfield, t_fielddescriptor **yfield, t_fielddescriptor **wfield)
{
    int arrayonset, type;
    t_symbol *elemtemplatesym;
    t_array *array;

    if (!field_isArray (&x->x_array))
    {
        post_error ("plot: needs an array field");
        return (-1);
    }
    if (!template_findField(ownertemplate, field_getVariableName (&x->x_array),
        &arrayonset, &type, &elemtemplatesym))
    {
        post_error ("plot: %s: no such field", field_getVariableName (&x->x_array)->s_name);
        return (-1);
    }
    if (type != DATA_ARRAY)
    {
        post_error ("plot: %s: not an array", field_getVariableName (&x->x_array)->s_name);
        return (-1);
    }
    array = *(t_array **)(((char *)data) + arrayonset);
    *linewidthp = word_getFloatByDescriptor(data, ownertemplate, &x->x_width);
    *xlocp = word_getFloatByDescriptor(data, ownertemplate, &x->x_positionX);
    *xincp = word_getFloatByDescriptor(data, ownertemplate, &x->x_incrementX);
    *ylocp = word_getFloatByDescriptor(data, ownertemplate, &x->x_positionY);
    *stylep = word_getFloatByDescriptor(data, ownertemplate, &x->x_style);
    *visp = word_getFloatByDescriptor(data, ownertemplate, &x->x_isVisible);
    *scalarvisp = word_getFloatByDescriptor(data, ownertemplate, &x->x_isScalarsVisible);
    *elemtemplatesymp = elemtemplatesym;
    *arrayp = array;
    *xfield = &x->x_fieldX;
    *yfield = &x->x_fieldY;
    *wfield = &x->x_fieldW;
    return (0);
}

    /* get everything else you could possibly need about a plot,
    either for plot's own purposes or for plotting a "garray" */
static int array_getfields(t_symbol *elemtemplatesym,
    t_glist **elemtemplatecanvasp,
    t_template **elemtemplatep, int *elemsizep,
    t_fielddescriptor *xfielddesc, t_fielddescriptor *yfielddesc, t_fielddescriptor *wfielddesc, 
    int *xonsetp, int *yonsetp, int *wonsetp)
{
    int arrayonset, elemsize, yonset, wonset, xonset, type;
    t_template *elemtemplate;
    t_symbol *dummy, *varname;
    t_glist *elemtemplatecanvas = 0;

        /* the "float" template is special in not having to have a canvas;
        template_findByIdentifier is hardwired to return a predefined 
        template. */

    if (!(elemtemplate =  template_findByIdentifier(elemtemplatesym)))
    {
        post_error ("plot: %s: no such template", elemtemplatesym->s_name);
        return (-1);
    }
    if (!((elemtemplatesym == &s_float) ||
        (elemtemplatecanvas = template_getFirstInstanceView (elemtemplate))))
    {
        post_error ("plot: %s: no canvas for this template", elemtemplatesym->s_name);
        return (-1);
    }
    elemsize = template_getSize (elemtemplate) * ARRAY_WORD;
    if (yfielddesc && field_isVariable (yfielddesc))
        varname = field_getVariableName (yfielddesc);
    else varname = sym_y;
    if (!template_findField(elemtemplate, varname, &yonset, &type, &dummy)
        || type != DATA_FLOAT)    
            yonset = -1;
    if (xfielddesc && field_isVariable (xfielddesc))
        varname = field_getVariableName (xfielddesc);
    else varname = sym_x;
    if (!template_findField(elemtemplate, varname, &xonset, &type, &dummy)
        || type != DATA_FLOAT) 
            xonset = -1;
    if (wfielddesc && field_isVariable (wfielddesc))
        varname = field_getVariableName (wfielddesc);
    else varname = sym_w;
    if (!template_findField(elemtemplate, varname, &wonset, &type, &dummy)
        || type != DATA_FLOAT) 
            wonset = -1;

        /* fill in slots for return values */
    *elemtemplatecanvasp = elemtemplatecanvas;
    *elemtemplatep = elemtemplate;
    *elemsizep = elemsize;
    *xonsetp = xonset;
    *yonsetp = yonset;
    *wonsetp = wonset;
    return (0);
}





    /* try clicking on an element of the array as a scalar (if clicking
    on the trace of the array failed) */
static int array_doclick_element(t_array *array, t_glist *glist,
    t_scalar *sc, t_array *ap,
    t_symbol *elemtemplatesym,
    t_float linewidth, t_float xloc, t_float xinc, t_float yloc,
    t_fielddescriptor *xfield, t_fielddescriptor *yfield, t_fielddescriptor *wfield,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_glist *elemtemplatecanvas;
    t_template *elemtemplate;
    int elemsize, yonset, wonset, xonset, i, incr, hit;
    t_float xsum;

    if (elemtemplatesym == &s_float)
        return (0);
    if (array_getfields(elemtemplatesym, &elemtemplatecanvas,
        &elemtemplate, &elemsize, xfield, yfield, wfield,
            &xonset, &yonset, &wonset))
                return (0);
        /* if it has more than 2000 points, just check 300 of them. */
    if (array->a_size < 2000)
        incr = 1;
    else incr = array->a_size / 300;
    for (i = 0, xsum = 0; i < array->a_size; i += incr)
    {
        t_float usexloc, useyloc;
        if (xonset >= 0)
            usexloc = xloc + field_convertValueToPosition(xfield, 
                *(t_float *)(((char *)(array->a_vector) + elemsize * i) + xonset));
        else usexloc = xloc + xsum, xsum += xinc;
        useyloc = yloc + (yonset >= 0 ? field_convertValueToPosition(yfield,
            *(t_float *)(((char *)(array->a_vector) + elemsize * i) + yonset)) : 0);
        
        if (hit = scalar_performClick(
            (t_word *)((char *)(array->a_vector) + i * elemsize),
            elemtemplate,
            0,
            array,
            glist,
            usexloc,
            useyloc,
            xpix,
            ypix,
            shift,
            alt,
            dbl,
            doit)) {
                return (hit);
            }
    }
    return (0);
}

static int array_doclick(t_array *array, t_glist *glist, t_scalar *sc,
    t_array *ap, t_symbol *elemtemplatesym,
    t_float linewidth, t_float xloc, t_float xinc, t_float yloc, t_float scalarvis,
    t_fielddescriptor *xfield, t_fielddescriptor *yfield, t_fielddescriptor *wfield,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_glist *elemtemplatecanvas;
    t_template *elemtemplate;
    int elemsize, yonset, wonset, xonset, i, callmotion = 0;

    if (!array_getfields(elemtemplatesym, &elemtemplatecanvas,
        &elemtemplate, &elemsize, xfield, yfield, wfield,
        &xonset, &yonset, &wonset))
    {
        t_float best = 100;
            /* if it has more than 2000 points, just check 1000 of them. */
        int incr = (array->a_size <= 2000 ? 1 : array->a_size / 1000);
        plot_elementSize = elemsize;
        plot_glist = glist;
        plot_scalar = sc;
        plot_array = ap;
        plot_template = elemtemplate;
        plot_stepX = canvas_stepX (glist);
        plot_stepY = canvas_stepY (glist);
            /* if we're a garray, the only one here, and if we appear to have
            only a 'y' field, click always succeeds and furthermore we'll
            call "motion" later. */
        if (glist->gl_graphics && pd_class(&glist->gl_graphics->g_pd) == garray_class
            && !glist->gl_graphics->g_next &&
                elemsize == ARRAY_WORD)
        {
            int xval = canvas_positionToValueX(glist, xpix);
            if (xval < 0)
                xval = 0;
            else if (xval >= array->a_size)
                xval = array->a_size - 1;
            plot_fieldY = yfield;
            plot_cumulativeY = canvas_positionToValueY(glist, ypix);
            plot_fatten = 0;
            plot_fieldX = 0;
            plot_cumulativeX = 0;
            plot_previousX = plot_currentX = xval;
            plot_numberOfPoints = array->a_size;
            plot_data = (t_word *)((char *)array->a_vector);
            if (doit)
            {
                word_setFloatByDescriptorAsPosition(
                    (t_word *)(((char *)array->a_vector) + elemsize * xval),
                    elemtemplate,
                    yfield,
                    canvas_positionToValueY(glist, ypix));
                canvas_setMotionFunction(glist, 0, (t_motionfn)plot_motion, xpix, ypix);
                if (plot_scalar)
                    scalar_redraw(plot_scalar, plot_glist);
                if (plot_array)
                    array_redraw(plot_array, plot_glist);
            }
        }
        else
        {
            for (i = 0; i < array->a_size; i += incr)
            {
                t_float pxpix, pypix, pwpix, dx, dy;
                array_getcoordinate(glist,
                    (char *)(array->a_vector) + i * elemsize,
                    xonset, yonset, wonset, i, xloc, yloc, xinc,
                    xfield, yfield, wfield, &pxpix, &pypix, &pwpix);
                if (pwpix < 4)
                    pwpix = 4;
                dx = pxpix - xpix;
                if (dx < 0) dx = -dx;
                if (dx > 8)
                    continue;   
                dy = pypix - ypix;
                if (dy < 0) dy = -dy;
                if (dx + dy < best)
                    best = dx + dy;
                if (wonset >= 0)
                {
                    dy = (pypix + pwpix) - ypix;
                    if (dy < 0) dy = -dy;
                    if (dx + dy < best)
                        best = dx + dy;
                    dy = (pypix - pwpix) - ypix;
                    if (dy < 0) dy = -dy;
                    if (dx + dy < best)
                        best = dx + dy;
                }
            }
            if (best > 8)
            {
                if (scalarvis != 0)
                    return (array_doclick_element(array, glist, sc, ap,
                        elemtemplatesym, linewidth, xloc, xinc, yloc,
                            xfield, yfield, wfield,
                            xpix, ypix, shift, alt, dbl, doit));
                else return (0);
            }
            best += 0.001;  /* add truncation error margin */
            for (i = 0; i < array->a_size; i += incr)
            {
                t_float pxpix, pypix, pwpix, dx, dy, dy2, dy3;
                array_getcoordinate(glist, (char *)(array->a_vector) + i * elemsize,
                    xonset, yonset, wonset, i, xloc, yloc, xinc,
                    xfield, yfield, wfield, &pxpix, &pypix, &pwpix);
                if (pwpix < 4)
                    pwpix = 4;
                dx = pxpix - xpix;
                if (dx < 0) dx = -dx;
                dy = pypix - ypix;
                if (dy < 0) dy = -dy;
                if (wonset >= 0)
                {
                    dy2 = (pypix + pwpix) - ypix;
                    if (dy2 < 0) dy2 = -dy2;
                    dy3 = (pypix - pwpix) - ypix;
                    if (dy3 < 0) dy3 = -dy3;
                    if (yonset < 0)
                        dy = 100;
                }
                else dy2 = dy3 = 100;
                if (dx + dy <= best || dx + dy2 <= best || dx + dy3 <= best)
                {
                    if (dy < dy2 && dy < dy3)
                        plot_fatten = 0;
                    else if (dy2 < dy3)
                        plot_fatten = -1;
                    else plot_fatten = 1;
                    if (doit)
                    {
                        char *elem = (char *)array->a_vector;
                        if (alt && xpix < pxpix) /* delete a point */
                        {
                            if (array->a_size <= 1)
                                return (0);
                            memmove((char *)(array->a_vector) + elemsize * i, 
                                (char *)(array->a_vector) + elemsize * (i+1),
                                    (array->a_size - 1 - i) * elemsize);
                            array_resizeAndRedraw(array, glist, array->a_size - 1);
                            return (0);
                        }
                        else if (alt)
                        {
                            /* add a point (after the clicked-on one) */
                            array_resizeAndRedraw(array, glist, array->a_size + 1);
                            elem = (char *)array->a_vector;
                            memmove(elem + elemsize * (i+1), 
                                elem + elemsize * i,
                                    (array->a_size - i - 1) * elemsize);
                            i++;
                        }
                        if (xonset >= 0)
                        {
                            plot_fieldX = xfield;
                            plot_cumulativeX = 
                                word_getFloatByDescriptorAsPosition(
                                    (t_word *)(elem + i * elemsize),
                                    plot_template,
                                    xfield);
                                plot_data = (t_word *)(elem + i * elemsize);
                            if (shift)
                                plot_numberOfPoints = array->a_size - i;
                            else plot_numberOfPoints = 1;
                        }
                        else
                        {
                            plot_fieldX = 0;
                            plot_cumulativeX = 0;
                            plot_data = (t_word *)elem;
                            plot_numberOfPoints = array->a_size;

                            plot_currentX = i;
                            plot_previousX = i;
                            plot_stepX *= (xinc == 0 ? 1 : 1./xinc);
                        }
                        if (plot_fatten)
                        {
                            plot_fieldY = wfield;
                            plot_cumulativeY = 
                                word_getFloatByDescriptorAsPosition(
                                    (t_word *)(elem + i * elemsize),
                                    plot_template,
                                    wfield);
                            plot_stepY *= -plot_fatten;
                        }
                        else if (yonset >= 0)
                        {
                            plot_fieldY = yfield;
                            plot_cumulativeY = 
                                word_getFloatByDescriptorAsPosition(
                                    (t_word *)(elem + i * elemsize),
                                    plot_template,
                                    yfield);
                                /* *(t_float *)((elem + elemsize * i) + yonset); */
                        }
                        else
                        {
                            plot_fieldY = 0;
                            plot_cumulativeY = 0;
                        }
                        canvas_setMotionFunction(glist, 0, (t_motionfn)plot_motion, xpix, ypix);
                    }
                    if (alt)
                    {
                        if (xpix < pxpix)
                            return (CURSOR_THICKEN /* CURSOR_EDIT_DISCONNECT */);
                        else return (CURSOR_ADD);
                    }
                    else return (plot_fatten ?
                        CURSOR_THICKEN : CURSOR_CLICK);
                }
            }   
        }
    }
    return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void plot_float(t_plot *x, t_float f)
{
    int viswas;
    if (!field_isFloatConstant (&x->x_isVisible))
    {
        post_error ("global vis/invis for a template with variable visibility");
        return;
    }

    viswas = (field_getFloatConstant (&x->x_isVisible) != 0);
    
    if ((f != 0 && viswas) || (f == 0 && !viswas))
        return;
    paint_scalarsEraseAll();
    field_setAsFloatConstant(&x->x_isVisible, (f != 0));
    paint_scalarsDrawAll();
}

static void plot_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    plot_cumulativeX += dx * plot_stepX;
    plot_cumulativeY += dy * plot_stepY;
    if (plot_fieldX)
    {
            /* it's an x, y plot */
        int i;
        for (i = 0; i < plot_numberOfPoints; i++)
        {
            t_word *thisword = (t_word *)(((char *)plot_data) +
                i * plot_elementSize);
            t_float xwas = word_getFloatByDescriptorAsPosition(
                thisword,
                plot_template,
                plot_fieldX);
            t_float ywas = (plot_fieldY ?
                word_getFloatByDescriptorAsPosition(
                    thisword, 
                    plot_template,
                    plot_fieldY) : 0);
            word_setFloatByDescriptorAsPosition(
                thisword,
                plot_template,
                plot_fieldX,
                xwas + dx);
            if (plot_fieldY)
            {
                if (plot_fatten)
                {
                    if (i == 0)
                    {
                        t_float newy = ywas + dy * plot_stepY;
                        if (newy < 0)
                            newy = 0;
                        word_setFloatByDescriptorAsPosition(
                            thisword,
                            plot_template,
                            plot_fieldY,
                            newy);
                    }
                }
                else
                {
                    word_setFloatByDescriptorAsPosition(
                        thisword,
                        plot_template,
                        plot_fieldY,
                        ywas + dy * plot_stepY);
                }
            }
        }
    }
    else if (plot_fieldY)
    {
            /* a y-only plot. */
        int thisx = plot_currentX + plot_cumulativeX + 0.5, x2;
        int increment, i, nchange;
        t_float newy = plot_cumulativeY,
            oldy = word_getFloatByDescriptorAsPosition(
                (t_word *)(((char *)plot_data) + plot_elementSize * plot_previousX),
                plot_template,
                plot_fieldY);
        t_float ydiff = newy - oldy;
        if (thisx < 0) thisx = 0;
        else if (thisx >= plot_numberOfPoints)
            thisx = plot_numberOfPoints - 1;
        increment = (thisx > plot_previousX ? -1 : 1);
        nchange = 1 + increment * (plot_previousX - thisx);

        for (i = 0, x2 = thisx; i < nchange; i++, x2 += increment)
        {
            word_setFloatByDescriptorAsPosition(
                (t_word *)(((char *)plot_data) + plot_elementSize * x2),
                plot_template,
                plot_fieldY,
                newy);
            if (nchange > 1)
                newy -= ydiff * (1./(nchange - 1));
         }
         plot_previousX = thisx;
    }
    if (plot_scalar)
        scalar_redraw(plot_scalar, plot_glist);
    if (plot_array)
        array_redraw(plot_array, plot_glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_behaviorGetRectangle(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_plot *x = (t_plot *)z;
    int elemsize, yonset, wonset, xonset;
    t_glist *elemtemplatecanvas;
    t_template *elemtemplate;
    t_symbol *elemtemplatesym;
    t_float linewidth, xloc, xinc, yloc, style, xsum, yval, vis, scalarvis;
    t_array *array;
    int x1 = PD_INT_MAX, y1 = PD_INT_MAX, x2 = -PD_INT_MAX, y2 = -PD_INT_MAX;
    int i;
    t_float xpix, ypix, wpix;
    t_fielddescriptor *xfielddesc, *yfielddesc, *wfielddesc;
        /* if we're the only plot in the glist claim the whole thing */
    if (glist->gl_graphics && !glist->gl_graphics->g_next)
    {
        *xp1 = *yp1 = -PD_INT_MAX;
        *xp2 = *yp2 = PD_INT_MAX;
        return;
    }
    if (!plot_readownertemplate(x, data, template, 
        &elemtemplatesym, &array, &linewidth, &xloc, &xinc, &yloc, &style,
            &vis, &scalarvis, &xfielddesc, &yfielddesc, &wfielddesc) &&
                (vis != 0) &&
            !array_getfields(elemtemplatesym, &elemtemplatecanvas,
                &elemtemplate, &elemsize, 
                xfielddesc, yfielddesc, wfielddesc,
                &xonset, &yonset, &wonset))
    {
            /* if it has more than 2000 points, just check 1000 of them. */
        int incr = (array->a_size <= 2000 ? 1 : array->a_size / 1000);
        for (i = 0, xsum = 0; i < array->a_size; i += incr)
        {
            t_float usexloc, useyloc;
            t_gobj *y;
                /* get the coords of the point proper */
            array_getcoordinate(glist, (char *)(array->a_vector) + i * elemsize,
                xonset, yonset, wonset, i, basex + xloc, basey + yloc, xinc,
                xfielddesc, yfielddesc, wfielddesc, &xpix, &ypix, &wpix);
            if (xpix < x1)
                x1 = xpix;
            if (xpix > x2)
                x2 = xpix;
            if (ypix - wpix < y1)
                y1 = ypix - wpix;
            if (ypix + wpix > y2)
                y2 = ypix + wpix;
            
            if (scalarvis != 0)
            {
                    /* check also the drawing instructions for the scalar */ 
                if (xonset >= 0)
                    usexloc = basex + xloc + field_convertValueToPosition(xfielddesc, 
                        *(t_float *)(((char *)(array->a_vector) + elemsize * i)
                            + xonset));
                else usexloc = basex + xsum, xsum += xinc;
                if (yonset >= 0)
                    yval = *(t_float *)(((char *)(array->a_vector) + elemsize * i)
                        + yonset);
                else yval = 0;
                useyloc = basey + yloc + field_convertValueToPosition(yfielddesc, yval);
                for (y = elemtemplatecanvas->gl_graphics; y; y = y->g_next)
                {
                    int xx1, xx2, yy1, yy2;
                    t_parentwidgetbehavior *wb = class_getParentWidget(pd_class (&y->g_pd));
                    if (!wb) continue;
                    (*wb->w_fnParentGetRectangle)(y, glist,
                        (t_word *)((char *)(array->a_vector) + elemsize * i),
                            elemtemplate, usexloc, useyloc, 
                                &xx1, &yy1, &xx2, &yy2);
                    if (xx1 < x1)
                        x1 = xx1;
                    if (yy1 < y1)
                        y1 = yy1;
                     if (xx2 > x2)
                        x2 = xx2;
                    if (yy2 > y2)
                        y2 = yy2;   
                }
            }
        }
    }

    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2;
}

static void plot_behaviorVisibilityChanged(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_float basex, t_float basey,
    int tovis)
{
    t_plot *x = (t_plot *)z;
    int elemsize, yonset, wonset, xonset, i;
    t_glist *elemtemplatecanvas;
    t_template *elemtemplate;
    t_symbol *elemtemplatesym;
    t_float linewidth, xloc, xinc, yloc, style, usexloc, xsum, yval, vis,
        scalarvis;
    t_array *array;
    int nelem;
    char *elem;
    t_fielddescriptor *xfielddesc, *yfielddesc, *wfielddesc;
        /* even if the array is "invisible", if its visibility is
        set by an instance variable you have to explicitly erase it,
        because the flag could earlier have been on when we were getting
        drawn.  Rather than look to try to find out whether we're
        visible we just do the erasure.  At the TK level this should
        cause no action because the tag matches nobody.  LATER we
        might want to optimize this somehow.  Ditto the "vis()" routines
        for other drawing instructions. */
        
    if (plot_readownertemplate(x, data, template, 
        &elemtemplatesym, &array, &linewidth, &xloc, &xinc, &yloc, &style,
        &vis, &scalarvis, &xfielddesc, &yfielddesc, &wfielddesc) ||
            ((vis == 0) && tovis) /* see above for 'tovis' */
            || array_getfields(elemtemplatesym, &elemtemplatecanvas,
                &elemtemplate, &elemsize, xfielddesc, yfielddesc, wfielddesc,
                &xonset, &yonset, &wonset))
                    return;
    nelem = array->a_size;
    elem = (char *)array->a_vector;

    if (tovis)
    {
        if (style == PLOT_POINTS)
        {
            t_float minyval = PLOT_MAX, maxyval = -PLOT_MAX;
            int ndrawn = 0;
            for (xsum = basex + xloc, i = 0; i < nelem; i++)
            {
                t_float yval, xpix, ypix, nextxloc;
                int ixpix, inextx;

                if (xonset >= 0)
                {
                    usexloc = basex + xloc +
                        *(t_float *)((elem + elemsize * i) + xonset);
                    ixpix = canvas_valueToPositionX(glist, 
                        field_convertValueToPosition(xfielddesc, usexloc));
                    inextx = ixpix + 2;
                }
                else
                {
                    usexloc = xsum;
                    xsum += xinc;
                    ixpix = canvas_valueToPositionX(glist,
                        field_convertValueToPosition(xfielddesc, usexloc));
                    inextx = canvas_valueToPositionX(glist,
                        field_convertValueToPosition(xfielddesc, xsum));
                }

                if (yonset >= 0)
                    yval = yloc + *(t_float *)((elem + elemsize * i) + yonset);
                else yval = 0;
                yval = PLOT_CLIP(yval);
                if (yval < minyval)
                    minyval = yval;
                if (yval > maxyval)
                    maxyval = yval;
                if (i == nelem-1 || inextx != ixpix)
                {
                    sys_vGui(".x%lx.c create rectangle %d %d %d %d \
-fill black -width 0  -tags [list plot%lx array]\n",
                        canvas_getView(glist),
                        ixpix, (int)canvas_valueToPositionY(glist, 
                            basey + field_convertValueToPosition(yfielddesc, minyval)),
                        inextx, (int)(canvas_valueToPositionY(glist, 
                            basey + field_convertValueToPosition(yfielddesc, maxyval))
                                + linewidth), data);
                    ndrawn++;
                    minyval = PLOT_MAX;
                    maxyval = -PLOT_MAX;
                }
                if (ndrawn > 2000 || ixpix >= 3000) break;
            }
        }
        else
        {
            char outline[20];
            int lastpixel = -1, ndrawn = 0;
            t_float yval = 0, wval = 0, xpix;
            int ixpix = 0;
                /* draw the trace */
            color_toEncodedString(outline, 20,
                color_withDigits (word_getFloatByDescriptor (data, template, &x->x_colorOutline)));
            if (wonset >= 0)
            {
                    /* found "w" field which controls linewidth.  The trace is
                    a filled polygon with 2n points. */
                sys_vGui(".x%lx.c create polygon \\\n",
                    canvas_getView(glist));

                for (i = 0, xsum = xloc; i < nelem; i++)
                {
                    if (xonset >= 0)
                        usexloc = xloc + *(t_float *)((elem + elemsize * i)
                            + xonset);
                    else usexloc = xsum, xsum += xinc;
                    if (yonset >= 0)
                        yval = *(t_float *)((elem + elemsize * i) + yonset);
                    else yval = 0;
                    yval = PLOT_CLIP(yval);
                    wval = *(t_float *)((elem + elemsize * i) + wonset);
                    wval = PLOT_CLIP(wval);
                    xpix = canvas_valueToPositionX(glist,
                        basex + field_convertValueToPosition(xfielddesc, usexloc));
                    ixpix = xpix + 0.5;
                    if (xonset >= 0 || ixpix != lastpixel)
                    {
                        sys_vGui("%d %f \\\n", ixpix,
                            canvas_valueToPositionY(glist,
                                basey + field_convertValueToPosition(yfielddesc, 
                                    yloc + yval) -
                                        field_convertValueToPosition(wfielddesc,wval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) goto ouch;
                }
                lastpixel = -1;
                for (i = nelem-1; i >= 0; i--)
                {
                    t_float usexloc;
                    if (xonset >= 0)
                        usexloc = xloc + *(t_float *)((elem + elemsize * i)
                            + xonset);
                    else xsum -= xinc, usexloc = xsum;
                    if (yonset >= 0)
                        yval = *(t_float *)((elem + elemsize * i) + yonset);
                    else yval = 0;
                    yval = PLOT_CLIP(yval);
                    wval = *(t_float *)((elem + elemsize * i) + wonset);
                    wval = PLOT_CLIP(wval);
                    xpix = canvas_valueToPositionX(glist,
                        basex + field_convertValueToPosition(xfielddesc, usexloc));
                    ixpix = xpix + 0.5;
                    if (xonset >= 0 || ixpix != lastpixel)
                    {
                        sys_vGui("%d %f \\\n", ixpix, canvas_valueToPositionY(glist,
                            basey + yloc + field_convertValueToPosition(yfielddesc,
                                yval) +
                                    field_convertValueToPosition(wfielddesc, wval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) goto ouch;
                }
                    /* TK will complain if there aren't at least 3 points.
                    There should be at least two already. */
                if (ndrawn < 4)
                {
                    sys_vGui("%d %f \\\n", ixpix + 10, canvas_valueToPositionY(glist,
                        basey + yloc + field_convertValueToPosition(yfielddesc,
                            yval) +
                                field_convertValueToPosition(wfielddesc, wval)));
                    sys_vGui("%d %f \\\n", ixpix + 10, canvas_valueToPositionY(glist,
                        basey + yloc + field_convertValueToPosition(yfielddesc,
                            yval) -
                                field_convertValueToPosition(wfielddesc, wval)));
                }
            ouch:
                sys_vGui(" -width 1 -fill %s -outline %s\\\n",
                    outline, outline);
                if (style == PLOT_CURVES) sys_vGui("-smooth 1\\\n");

                sys_vGui("-tags [list plot%lx array]\n", data);
            }
            else if (linewidth > 0)
            {
                    /* no "w" field.  If the linewidth is positive, draw a
                    segmented line with the requested width; otherwise don't
                    draw the trace at all. */
                sys_vGui(".x%lx.c create line \\\n", canvas_getView(glist));

                for (xsum = xloc, i = 0; i < nelem; i++)
                {
                    t_float usexloc;
                    if (xonset >= 0)
                        usexloc = xloc + *(t_float *)((elem + elemsize * i) +
                            xonset);
                    else usexloc = xsum, xsum += xinc;
                    if (yonset >= 0)
                        yval = *(t_float *)((elem + elemsize * i) + yonset);
                    else yval = 0;
                    yval = PLOT_CLIP(yval);
                    xpix = canvas_valueToPositionX(glist,
                        basex + field_convertValueToPosition(xfielddesc, usexloc));
                    ixpix = xpix + 0.5;
                    if (xonset >= 0 || ixpix != lastpixel)
                    {
                        sys_vGui("%d %f \\\n", ixpix,
                            canvas_valueToPositionY(glist,
                                basey + yloc + field_convertValueToPosition(yfielddesc,
                                    yval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) break;
                }
                    /* TK will complain if there aren't at least 2 points... */
                if (ndrawn == 0) sys_vGui("0 0 0 0 \\\n");
                else if (ndrawn == 1) sys_vGui("%d %f \\\n", ixpix + 10,
                    canvas_valueToPositionY(glist, basey + yloc + 
                        field_convertValueToPosition(yfielddesc, yval)));

                sys_vGui("-width %f\\\n", linewidth);
                sys_vGui("-fill %s\\\n", outline);
                if (style == PLOT_CURVES) sys_vGui("-smooth 1\\\n");

                sys_vGui("-tags [list plot%lx array]\n", data);
            }
        }
            /* We're done with the outline; now draw all the points.
            This code is inefficient since the template has to be
            searched for drawing instructions for every last point. */
        if (scalarvis != 0)
        {
            for (xsum = xloc, i = 0; i < nelem; i++)
            {
                t_float usexloc, useyloc;
                t_gobj *y;
                if (xonset >= 0)
                    usexloc = basex + xloc +
                        *(t_float *)((elem + elemsize * i) + xonset);
                else usexloc = basex + xsum, xsum += xinc;
                if (yonset >= 0)
                    yval = *(t_float *)((elem + elemsize * i) + yonset);
                else yval = 0;
                useyloc = basey + yloc +
                    field_convertValueToPosition(yfielddesc, yval);
                for (y = elemtemplatecanvas->gl_graphics; y; y = y->g_next)
                {
                    t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
                    if (!wb) continue;
                    (*wb->w_fnParentVisibilityChanged)(y, glist,
                        (t_word *)(elem + elemsize * i),
                            elemtemplate, usexloc, useyloc, tovis);
                }
            }
        }
    }
    else
    {
            /* un-draw the individual points */
        if (scalarvis != 0)
        {
            int i;
            for (i = 0; i < nelem; i++)
            {
                t_gobj *y;
                for (y = elemtemplatecanvas->gl_graphics; y; y = y->g_next)
                {
                    t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
                    if (!wb) continue;
                    (*wb->w_fnParentVisibilityChanged)(y, glist,
                        (t_word *)(elem + elemsize * i), elemtemplate,
                            0, 0, 0);
                }
            }
        }
            /* and then the trace */
        sys_vGui(".x%lx.c delete plot%lx\n",
            canvas_getView(glist), data);      
    }
}

static int plot_behaviorClicked(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_plot *x = (t_plot *)z;
    t_symbol *elemtemplatesym;
    t_float linewidth, xloc, xinc, yloc, style, vis, scalarvis;
    t_array *array;
    t_fielddescriptor *xfielddesc, *yfielddesc, *wfielddesc;

    if (!plot_readownertemplate(x, data, template, 
        &elemtemplatesym, &array, &linewidth, &xloc, &xinc, &yloc, &style,
        &vis, &scalarvis,
        &xfielddesc, &yfielddesc, &wfielddesc) && (vis != 0))
    {
        return (array_doclick(array, glist, sc, ap,
            elemtemplatesym,
            linewidth, basex + xloc, xinc, basey + yloc, scalarvis,
            xfielddesc, yfielddesc, wfielddesc,
            xpix, ypix, shift, alt, dbl, doit));
    }
    else return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_parentwidgetbehavior plot_widgetBehavior =
    {
        plot_behaviorGetRectangle,
        plot_behaviorVisibilityChanged,
        plot_behaviorClicked,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *plot_new(t_symbol *classsym, int argc, t_atom *argv)
{
    t_plot *x = (t_plot *)pd_new(plot_class);
    int defstyle = PLOT_POLYGONS;

    field_setAsFloatVariable(&x->x_fieldX, sym_x);
    field_setAsFloatVariable(&x->x_fieldY, sym_y);
    field_setAsFloatVariable(&x->x_fieldW, sym_w);
    
    field_setAsFloatConstant(&x->x_isVisible, 1);
    field_setAsFloatConstant(&x->x_isScalarsVisible, 1);
    while (1)
    {
        t_symbol *firstarg = atom_getSymbolAtIndex(0, argc, argv);
        if (!strcmp(firstarg->s_name, "curve") ||
            !strcmp(firstarg->s_name, "-c"))
        {
            defstyle = PLOT_CURVES;
            argc--, argv++;
        }
        else if (!strcmp(firstarg->s_name, "-v") && argc > 1)
        {
            field_setAsFloat(&x->x_isVisible, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-vs") && argc > 1)
        {
            field_setAsFloat(&x->x_isScalarsVisible, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-x") && argc > 1)
        {
            field_setAsFloat(&x->x_fieldX, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-y") && argc > 1)
        {
            field_setAsFloat(&x->x_fieldY, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-w") && argc > 1)
        {
            field_setAsFloat(&x->x_fieldW, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else break;
    }
    if (argc) field_setAsArray (&x->x_array, argc--, argv++);
    else field_setAsFloatConstant(&x->x_array, 1);
    if (argc) field_setAsFloat(&x->x_colorOutline, argc--, argv++);
    else field_setAsFloatConstant(&x->x_colorOutline, 0);
    if (argc) field_setAsFloat(&x->x_width, argc--, argv++);
    else field_setAsFloatConstant(&x->x_width, 1);
    if (argc) field_setAsFloat(&x->x_positionX, argc--, argv++);
    else field_setAsFloatConstant(&x->x_positionX, 1);
    if (argc) field_setAsFloat(&x->x_positionY, argc--, argv++);
    else field_setAsFloatConstant(&x->x_positionY, 1);
    if (argc) field_setAsFloat(&x->x_incrementX, argc--, argv++);
    else field_setAsFloatConstant(&x->x_incrementX, 1);
    if (argc) field_setAsFloat(&x->x_style, argc--, argv++);
    else field_setAsFloatConstant(&x->x_style, defstyle);
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void plot_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_plot,
            (t_newmethod)plot_new,
            NULL,
            sizeof (t_plot),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, plot_float);
    
    class_setParentWidgetBehavior (c, &plot_widgetBehavior);
    
    plot_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
