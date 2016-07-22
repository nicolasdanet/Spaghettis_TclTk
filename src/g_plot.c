
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

extern t_class *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* --------- plots for showing arrays --------------- */

t_class *plot_class;

typedef struct _plot
{
    t_object x_obj;
    t_glist *x_canvas;
    t_fielddescriptor x_outlinecolor;
    t_fielddescriptor x_width;
    t_fielddescriptor x_xloc;
    t_fielddescriptor x_yloc;
    t_fielddescriptor x_xinc;
    t_fielddescriptor x_style;
    t_fielddescriptor x_data;
    t_fielddescriptor x_xpoints;
    t_fielddescriptor x_ypoints;
    t_fielddescriptor x_wpoints;
    t_fielddescriptor x_vis;          /* visible */
    t_fielddescriptor x_scalarvis;    /* true if drawing the scalar at each point */
} t_plot;

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

static void *plot_new(t_symbol *classsym, int argc, t_atom *argv)
{
    t_plot *x = (t_plot *)pd_new(plot_class);
    int defstyle = PLOT_POLYGONS;
    x->x_canvas = canvas_getCurrent();

    field_setAsFloatVariable(&x->x_xpoints, sym_x);
    field_setAsFloatVariable(&x->x_ypoints, sym_y);
    field_setAsFloatVariable(&x->x_wpoints, sym_w);
    
    field_setAsFloatConstant(&x->x_vis, 1);
    field_setAsFloatConstant(&x->x_scalarvis, 1);
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
            field_setAsFloat(&x->x_vis, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-vs") && argc > 1)
        {
            field_setAsFloat(&x->x_scalarvis, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-x") && argc > 1)
        {
            field_setAsFloat(&x->x_xpoints, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-y") && argc > 1)
        {
            field_setAsFloat(&x->x_ypoints, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else if (!strcmp(firstarg->s_name, "-w") && argc > 1)
        {
            field_setAsFloat(&x->x_wpoints, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else break;
    }
    if (argc) field_setAsArray (&x->x_data, argc--, argv++);
    else field_setAsFloatConstant(&x->x_data, 1);
    if (argc) field_setAsFloat(&x->x_outlinecolor, argc--, argv++);
    else field_setAsFloatConstant(&x->x_outlinecolor, 0);
    if (argc) field_setAsFloat(&x->x_width, argc--, argv++);
    else field_setAsFloatConstant(&x->x_width, 1);
    if (argc) field_setAsFloat(&x->x_xloc, argc--, argv++);
    else field_setAsFloatConstant(&x->x_xloc, 1);
    if (argc) field_setAsFloat(&x->x_yloc, argc--, argv++);
    else field_setAsFloatConstant(&x->x_yloc, 1);
    if (argc) field_setAsFloat(&x->x_xinc, argc--, argv++);
    else field_setAsFloatConstant(&x->x_xinc, 1);
    if (argc) field_setAsFloat(&x->x_style, argc--, argv++);
    else field_setAsFloatConstant(&x->x_style, defstyle);
    return (x);
}

void plot_float(t_plot *x, t_float f)
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

/* -------------------- widget behavior for plot ------------ */


    /* get everything we'll need from the owner template of the array being
    plotted. Not used for garrays, but see below */
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

    if (!field_isArray (&x->x_data))
    {
        post_error ("plot: needs an array field");
        return (-1);
    }
    if (!template_findField(ownertemplate, field_getVariableName (&x->x_data),
        &arrayonset, &type, &elemtemplatesym))
    {
        post_error ("plot: %s: no such field", field_getVariableName (&x->x_data)->s_name);
        return (-1);
    }
    if (type != DATA_ARRAY)
    {
        post_error ("plot: %s: not an array", field_getVariableName (&x->x_data)->s_name);
        return (-1);
    }
    array = *(t_array **)(((char *)data) + arrayonset);
    *linewidthp = word_getFloatByField(&x->x_width, ownertemplate, data);
    *xlocp = word_getFloatByField(&x->x_xloc, ownertemplate, data);
    *xincp = word_getFloatByField(&x->x_xinc, ownertemplate, data);
    *ylocp = word_getFloatByField(&x->x_yloc, ownertemplate, data);
    *stylep = word_getFloatByField(&x->x_style, ownertemplate, data);
    *visp = word_getFloatByField(&x->x_vis, ownertemplate, data);
    *scalarvisp = word_getFloatByField(&x->x_scalarvis, ownertemplate, data);
    *elemtemplatesymp = elemtemplatesym;
    *arrayp = array;
    *xfield = &x->x_xpoints;
    *yfield = &x->x_ypoints;
    *wfield = &x->x_wpoints;
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

static void plot_getrect(t_gobj *z, t_glist *glist,
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

static void plot_displace(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int dx, int dy)
{
        /* not yet */
}

static void plot_select(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
    /* not yet */
}

static void plot_activate(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int state)
{
        /* not yet */
}

#define CLIP(x) ((x) < 1e20 && (x) > -1e20 ? x : 0)

static void plot_vis(t_gobj *z, t_glist *glist, 
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
            t_float minyval = 1e20, maxyval = -1e20;
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
                yval = CLIP(yval);
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
                    minyval = 1e20;
                    maxyval = -1e20;
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
            numbertocolor(word_getFloatByField(&x->x_outlinecolor, template,
                data), outline);
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
                    yval = CLIP(yval);
                    wval = *(t_float *)((elem + elemsize * i) + wonset);
                    wval = CLIP(wval);
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
                    yval = CLIP(yval);
                    wval = *(t_float *)((elem + elemsize * i) + wonset);
                    wval = CLIP(wval);
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
                    yval = CLIP(yval);
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

static t_float array_motion_xcumulative;
static t_float array_motion_ycumulative;
static t_fielddescriptor *array_motion_xfield;
static t_fielddescriptor *array_motion_yfield;
static t_glist *array_motion_glist;
static t_scalar *array_motion_scalar;
static t_array *array_motion_array;
static t_word *array_motion_wp;
static t_template *array_motion_template;
static int array_motion_npoints;
static int array_motion_elemsize;
static int array_motion_altkey;
static t_float array_motion_initx;
static t_float array_motion_xperpix;
static t_float array_motion_yperpix;
static int array_motion_lastx;
static int array_motion_fatten;

    /* LATER protect against the template changing or the scalar disappearing
    probably by attaching a gpointer here ... */

static void array_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    array_motion_xcumulative += dx * array_motion_xperpix;
    array_motion_ycumulative += dy * array_motion_yperpix;
    if (array_motion_xfield)
    {
            /* it's an x, y plot */
        int i;
        for (i = 0; i < array_motion_npoints; i++)
        {
            t_word *thisword = (t_word *)(((char *)array_motion_wp) +
                i * array_motion_elemsize);
            t_float xwas = word_getFloatByFieldAsPosition(array_motion_xfield, 
                array_motion_template, thisword);
            t_float ywas = (array_motion_yfield ?
                word_getFloatByFieldAsPosition(array_motion_yfield, 
                    array_motion_template, thisword) : 0);
            word_setFloatByFieldAsPosition(array_motion_xfield,
                array_motion_template, thisword, xwas + dx);
            if (array_motion_yfield)
            {
                if (array_motion_fatten)
                {
                    if (i == 0)
                    {
                        t_float newy = ywas + dy * array_motion_yperpix;
                        if (newy < 0)
                            newy = 0;
                        word_setFloatByFieldAsPosition(array_motion_yfield,
                            array_motion_template, thisword, newy);
                    }
                }
                else
                {
                    word_setFloatByFieldAsPosition(array_motion_yfield,
                        array_motion_template, thisword,
                            ywas + dy * array_motion_yperpix);
                }
            }
        }
    }
    else if (array_motion_yfield)
    {
            /* a y-only plot. */
        int thisx = array_motion_initx + array_motion_xcumulative + 0.5, x2;
        int increment, i, nchange;
        t_float newy = array_motion_ycumulative,
            oldy = word_getFloatByFieldAsPosition(array_motion_yfield,
                array_motion_template,
                    (t_word *)(((char *)array_motion_wp) +
                        array_motion_elemsize * array_motion_lastx));
        t_float ydiff = newy - oldy;
        if (thisx < 0) thisx = 0;
        else if (thisx >= array_motion_npoints)
            thisx = array_motion_npoints - 1;
        increment = (thisx > array_motion_lastx ? -1 : 1);
        nchange = 1 + increment * (array_motion_lastx - thisx);

        for (i = 0, x2 = thisx; i < nchange; i++, x2 += increment)
        {
            word_setFloatByFieldAsPosition(array_motion_yfield,
                array_motion_template,
                    (t_word *)(((char *)array_motion_wp) +
                        array_motion_elemsize * x2), newy);
            if (nchange > 1)
                newy -= ydiff * (1./(nchange - 1));
         }
         array_motion_lastx = thisx;
    }
    if (array_motion_scalar)
        scalar_redraw(array_motion_scalar, array_motion_glist);
    if (array_motion_array)
        array_redraw(array_motion_array, array_motion_glist);
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
        array_motion_elemsize = elemsize;
        array_motion_glist = glist;
        array_motion_scalar = sc;
        array_motion_array = ap;
        array_motion_template = elemtemplate;
        array_motion_xperpix = canvas_deltaPositionToValueX (glist, 1.0);
        array_motion_yperpix = canvas_deltaPositionToValueY (glist, 1.0);
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
            array_motion_yfield = yfield;
            array_motion_ycumulative = canvas_positionToValueY(glist, ypix);
            array_motion_fatten = 0;
            array_motion_xfield = 0;
            array_motion_xcumulative = 0;
            array_motion_lastx = array_motion_initx = xval;
            array_motion_npoints = array->a_size;
            array_motion_wp = (t_word *)((char *)array->a_vector);
            if (doit)
            {
                word_setFloatByFieldAsPosition(yfield, elemtemplate,
                    (t_word *)(((char *)array->a_vector) + elemsize * xval),
                        canvas_positionToValueY(glist, ypix));
                canvas_setMotionFunction(glist, 0, (t_motionfn)array_motion, xpix, ypix);
                if (array_motion_scalar)
                    scalar_redraw(array_motion_scalar, array_motion_glist);
                if (array_motion_array)
                    array_redraw(array_motion_array, array_motion_glist);
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
                        array_motion_fatten = 0;
                    else if (dy2 < dy3)
                        array_motion_fatten = -1;
                    else array_motion_fatten = 1;
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
                            array_motion_xfield = xfield;
                            array_motion_xcumulative = 
                                word_getFloatByFieldAsPosition(xfield, array_motion_template,
                                    (t_word *)(elem + i * elemsize));
                                array_motion_wp = (t_word *)(elem + i * elemsize);
                            if (shift)
                                array_motion_npoints = array->a_size - i;
                            else array_motion_npoints = 1;
                        }
                        else
                        {
                            array_motion_xfield = 0;
                            array_motion_xcumulative = 0;
                            array_motion_wp = (t_word *)elem;
                            array_motion_npoints = array->a_size;

                            array_motion_initx = i;
                            array_motion_lastx = i;
                            array_motion_xperpix *= (xinc == 0 ? 1 : 1./xinc);
                        }
                        if (array_motion_fatten)
                        {
                            array_motion_yfield = wfield;
                            array_motion_ycumulative = 
                                word_getFloatByFieldAsPosition(wfield, array_motion_template,
                                    (t_word *)(elem + i * elemsize));
                            array_motion_yperpix *= -array_motion_fatten;
                        }
                        else if (yonset >= 0)
                        {
                            array_motion_yfield = yfield;
                            array_motion_ycumulative = 
                                word_getFloatByFieldAsPosition(yfield, array_motion_template,
                                    (t_word *)(elem + i * elemsize));
                                /* *(t_float *)((elem + elemsize * i) + yonset); */
                        }
                        else
                        {
                            array_motion_yfield = 0;
                            array_motion_ycumulative = 0;
                        }
                        canvas_setMotionFunction(glist, 0, (t_motionfn)array_motion, xpix, ypix);
                    }
                    if (alt)
                    {
                        if (xpix < pxpix)
                            return (CURSOR_THICKEN /* CURSOR_EDIT_DISCONNECT */);
                        else return (CURSOR_ADD);
                    }
                    else return (array_motion_fatten ?
                        CURSOR_THICKEN : CURSOR_CLICK);
                }
            }   
        }
    }
    return (0);
}

static int plot_click(t_gobj *z, t_glist *glist, 
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

t_parentwidgetbehavior plot_widgetbehavior =
{
    plot_getrect,
    plot_displace,
    plot_select,
    plot_activate,
    plot_vis,
    plot_click,
};

void plot_setup(void)
{
    plot_class = class_new(sym_plot, (t_newmethod)plot_new, 0,
        sizeof(t_plot), 0, A_GIMME, 0);
    class_setDrawCommand(plot_class);
    class_addFloat(plot_class, plot_float);
    class_setParentWidgetBehavior(plot_class, &plot_widgetbehavior);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
