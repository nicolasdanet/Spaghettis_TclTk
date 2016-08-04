
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
    t_fielddescriptor   x_array;
    t_fielddescriptor   x_colorOutline;
    t_fielddescriptor   x_width;
    t_fielddescriptor   x_positionX;
    t_fielddescriptor   x_positionY;
    t_fielddescriptor   x_incrementX;
    t_fielddescriptor   x_style;
    t_fielddescriptor   x_fieldX;
    t_fielddescriptor   x_fieldY;
    t_fielddescriptor   x_fieldW;
    t_fielddescriptor   x_isVisible;
    } t_plot;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_motion (void *, t_float, t_float, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_MAX            1e20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PLOT_CLIP(x)        (((x) > -PLOT_MAX && (x) < PLOT_MAX) ? (x) : 0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_BUFFER_SIZE    4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error plot_fetchScalarFields (t_plot *x, t_word *w, t_template *tmpl, 
    t_array **array,
    t_float *width,
    t_float *positionX,
    t_float *positionY,
    t_float *incrementX,
    t_float *style,
    int *visible)
{
    if (field_isArray (&x->x_array)) {
    //
    t_symbol *s = field_getVariableName (&x->x_array);
    
    if (template_hasField (tmpl, s) && template_fieldIsArrayAndValid (tmpl, s)) {
    //
    *array      = word_getArray (w, tmpl, s);
    *width      = word_getFloatByDescriptor (w, tmpl, &x->x_width);
    *positionX  = word_getFloatByDescriptor (w, tmpl, &x->x_positionX);
    *positionY  = word_getFloatByDescriptor (w, tmpl, &x->x_positionY);
    *incrementX = word_getFloatByDescriptor (w, tmpl, &x->x_incrementX);
    *style      = word_getFloatByDescriptor (w, tmpl, &x->x_style);
    *visible    = (int)word_getFloatByDescriptor (w, tmpl, &x->x_isVisible);
    
    return PD_ERROR_NONE;
    //
    }
    //
    }
    
    post_error (PD_TRANSLATE ("plot: needs a valid array field"));
    
    return PD_ERROR;
}

static t_error plot_fetchElementFieldNames (t_plot *x, t_array *array,
    t_symbol **fieldX,
    t_symbol **fieldY,
    t_symbol **fieldW)
{
    t_template *t = array_getTemplate (array);
    
    if (field_isVariable (&x->x_fieldX)) { *fieldX = field_getVariableName (&x->x_fieldX); }
    else { *fieldX = sym_x; }
    
    if (field_isVariable (&x->x_fieldY)) { *fieldY = field_getVariableName (&x->x_fieldY); }
    else { *fieldY = sym_y; }

    if (field_isVariable (&x->x_fieldW)) { *fieldW = field_getVariableName (&x->x_fieldW); }
    else { *fieldW = sym_w; }
    
    if (!template_fieldIsFloat (t, *fieldX)) { *fieldX = NULL; }
    if (!template_fieldIsFloat (t, *fieldY)) { *fieldY = NULL; }
    if (!template_fieldIsFloat (t, *fieldW)) { *fieldW = NULL; }
    
    return PD_ERROR_NONE;
}

static int plot_getStep (t_array *array)
{
    int size = array_getSize (array); return (size <= 1000 ? 1 : (int)sqrt ((double)size));
}

static void plot_getCoordinates (t_plot *x,
    t_array *array,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    int i,
    t_float relativeX,
    t_float relativeY,
    t_float incrementX,
    t_float *a,
    t_float *b,
    t_float *c)
{
    t_float valueX;
    t_float valueY;
    t_float valueW;

    if (fieldX) { valueX = array_getFloatInElementAtIndex (array, i, fieldX); }
    else { 
        valueX = i * incrementX;
    }
    
    if (fieldY) { valueY = array_getFloatInElementAtIndex (array, i, fieldY); }
    else { 
        valueY = 0.0;
    }
    
    if (fieldW) { valueW = array_getFloatInElementAtIndex (array, i, fieldW); }
    else {
        valueW = 0.0;
    }
    
    *a = relativeX + field_convertValueToPosition (&x->x_fieldX, valueX);
    *b = relativeY + field_convertValueToPosition (&x->x_fieldY, valueY);
    *c = field_convertValueToPosition (&x->x_fieldY, valueW);
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

static void plot_behaviorGetRectangleRecursive (t_plot *x,
    t_glist *glist,
    t_array *array,
    int i,
    t_float baseX,
    t_float baseY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    t_glist *view = template_getFirstInstanceView (array_getTemplate (array));
    
    if (view) {
    //
    t_gobj *y = NULL;
                        
    for (y = view->gl_graphics; y; y = y->g_next) {
    
        t_parentwidgetbehavior *behavior = class_getParentWidget (pd_class (y));
        
        if (behavior) {
        
            int x1, y1, x2, y2;
            
            (*behavior->w_fnParentGetRectangle) (y,
                glist,
                array_getElementAtIndex (array, i),
                array_getTemplate (array),
                baseX,
                baseY, 
                &x1,
                &y1,
                &x2,
                &y2);
            
            *a = PD_MIN (*a, x1);
            *b = PD_MIN (*b, y1);
            *c = PD_MAX (*c, x2);
            *d = PD_MAX (*d, y2);
        }
    }
    //
    }
}

static void plot_behaviorGetRectangle (t_gobj *z,
    t_glist *glist,
    t_word *w,
    t_template *tmpl,
    t_float baseX,
    t_float baseY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    t_plot *x = (t_plot *)z;
    
    int x1 = PD_INT_MAX;
    int y1 = PD_INT_MAX;
    int x2 = -x1;
    int y2 = -y2;
    
    if (garray_isSingle (glist)) { x1 = -PD_INT_MAX; y1 = -PD_INT_MAX; x2 = PD_INT_MAX; y2 = PD_INT_MAX; }
    else {
    //
    t_array *array = NULL;
    t_float width;
    t_float positionX;
    t_float positionY;
    t_float incrementX;
    t_float style;
    int visible;
    
    if (!plot_fetchScalarFields (x, w, tmpl,
            &array,
            &width,
            &positionX,
            &positionY,
            &incrementX,
            &style,
            &visible) && (visible != 0)) {
    //
    t_symbol *fieldX = NULL;
    t_symbol *fieldY = NULL;
    t_symbol *fieldW = NULL;
    
    if (!plot_fetchElementFieldNames (x, array, &fieldX, &fieldY, &fieldW)) {
    //
    int i, k = plot_getStep (array);
    
    for (i = 0; i < array_getSize (array); i += k) {

        t_float valueX;
        t_float valueY;
        t_float valueW;
        t_float pixelX;
        t_float pixelY;
        t_float pixelW;
        
        plot_getCoordinates (x, array, fieldX, fieldY, fieldW,
            i,
            baseX + positionX,
            baseY + positionY,
            incrementX,
            &valueX,
            &valueY,
            &valueW);
        
        pixelX = canvas_valueToPixelX (glist, valueX);
        pixelY = canvas_valueToPixelY (glist, valueY);
        pixelW = canvas_valueToPixelY (glist, valueY + valueW) - pixelY;
        
        pixelW = PD_ABS (pixelW);
        pixelW = PD_MAX (pixelW, width);

        x1 = PD_MIN (x1, pixelX);
        x2 = PD_MAX (x2, pixelX);
        y1 = PD_MIN (y1, pixelY - pixelW);
        y2 = PD_MAX (y2, pixelY + pixelW);
        
        plot_behaviorGetRectangleRecursive (x, glist, array, i, valueX, valueY, &x1, &y1, &x2, &y2);
    }
    //
    }
    //
    }
    //
    }
    
    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if 0

static void plot_behaviorVisibilityChangedDrawPoint (t_plot *x,
    t_array *array,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    t_float relativeX,
    t_float relativeY, 
    t_float incrementX)
{
    char t[PLOT_BUFFER_SIZE] = { 0 };
    
    int numberOfElements     = array_getSize (array);
    int numberOfElementDrawn = 0;
    t_float minimumValueY    =  PLOT_MAX;
    t_float maximumValueY    = -PLOT_MAX;
    
    int i;
    
    for (i = 0; i < numberOfElements; i++) {
    //
    t_float valueY;
    t_float valueX;
    t_float width;
    t_float pixelX;
    t_float pixelY;
    
    int draw = (i == numberOfElements - 1);
    
    plot_getCoordinates (x, 
        array,
        fieldX,
        fieldY,
        NULL,
        i,
        relativeX,
        relativeY,
        incrementX, 
        &valueX, 
        &valueY, 
        &width);
    
    int ixpix;
    int inextx;
    /*
    if (fieldX) {
        valueX = baseX + positionX + *(t_float *)((elem + elemsize * i) + xonset);
        ixpix = canvas_valueToPixelX(glist, field_convertValueToPosition(xfielddesc, valueX));
        inextx = ixpix + 2;
    }
    else
    {
        valueX = relativeX;
        relativeX += xinc;
        ixpix = canvas_valueToPixelX(glist,
            field_convertValueToPosition(xfielddesc, valueX));
        inextx = canvas_valueToPixelX(glist,
            field_convertValueToPosition(xfielddesc, relativeX));
    }

    if (yonset >= 0)
        valueY = positionY + *(t_float *)((elem + elemsize * i) + yonset);
    else valueY = 0;
    
    valueY = PLOT_CLIP(valueY);
    
    if (valueY < minimumValueY)
        minimumValueY = valueY;
    if (valueY > maximumValueY)
        maximumValueY = valueY;
        
    if (i == numberOfElements-1 || inextx != ixpix)
    {
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill black -width 0 -tags [list PLOT%lx array]\n",
            canvas_getView(glist),
            ixpix,
            (int)canvas_valueToPixelY (glist, baseY + field_convertValueToPosition(yfielddesc, minimumValueY)),
            inextx,
            (int)(canvas_valueToPixelY (glist, baseY + field_convertValueToPosition(yfielddesc, maximumValueY)) + linewidth),
            w);
        numberOfElementDrawn++;
        minimumValueY = PLOT_MAX;
        maximumValueY = -PLOT_MAX;
    }
    
    if (numberOfElementDrawn > 2000 || ixpix >= 3000) { break; }
    */
    //
    }
}

static void plot_behaviorVisibilityChangedDrawPolygon (void)
{
    /*
    char outline[20];
    int lastpixel = -1, ndrawn = 0;
    t_float yval = 0, wval = 0, xpix;
    int ixpix = 0;
        
    color_toEncodedString(outline, 20,
        color_withDigits (word_getFloatByDescriptor (w, tmpl, &x->x_colorOutline)));
    if (wonset >= 0)
    {
            // found "w" field which controls linewidth.  The trace is
            // a filled polygon with 2n points. 
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
            xpix = canvas_valueToPixelX(glist,
                baseX + field_convertValueToPosition(xfielddesc, usexloc));
            ixpix = xpix + 0.5;
            if (xonset >= 0 || ixpix != lastpixel)
            {
                sys_vGui("%d %f \\\n", ixpix,
                    canvas_valueToPixelY(glist,
                        baseY + field_convertValueToPosition(yfielddesc, 
                            yloc + yval) -
                                field_convertValueToPosition(wfielddesc,wval)));
                ndrawn++;
            }
            lastpixel = ixpix;
            if (ndrawn >= 1000) { post_log ("toto"); goto ouch; }
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
            xpix = canvas_valueToPixelX(glist,
                baseX + field_convertValueToPosition(xfielddesc, usexloc));
            ixpix = xpix + 0.5;
            if (xonset >= 0 || ixpix != lastpixel)
            {
                sys_vGui("%d %f \\\n", ixpix, canvas_valueToPixelY(glist,
                    baseY + yloc + field_convertValueToPosition(yfielddesc,
                        yval) +
                            field_convertValueToPosition(wfielddesc, wval)));
                ndrawn++;
            }
            lastpixel = ixpix;
            if (ndrawn >= 1000) { post_log ("toto"); goto ouch; }
        }
            // TK will complain if there aren't at least 3 points.
            // There should be at least two already.
        if (ndrawn < 4)
        {
            sys_vGui("%d %f \\\n", ixpix + 10, canvas_valueToPixelY(glist,
                baseY + yloc + field_convertValueToPosition(yfielddesc,
                    yval) +
                        field_convertValueToPosition(wfielddesc, wval)));
            sys_vGui("%d %f \\\n", ixpix + 10, canvas_valueToPixelY(glist,
                baseY + yloc + field_convertValueToPosition(yfielddesc,
                    yval) -
                        field_convertValueToPosition(wfielddesc, wval)));
        }
    ouch:
        sys_vGui(" -width 1 -fill %s -outline %s\\\n",
            outline, outline);
        if (style == PLOT_CURVES) sys_vGui("-smooth 1\\\n");

        sys_vGui("-tags [list PLOT%lx array]\n", w);
    }
    else if (linewidth > 0)
    {
            //no "w" field.  If the linewidth is positive, draw a
            //segmented line with the requested width; otherwise don't
            // draw the trace at all.
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
            xpix = canvas_valueToPixelX(glist,
                baseX + field_convertValueToPosition(xfielddesc, usexloc));
            ixpix = xpix + 0.5;
            if (xonset >= 0 || ixpix != lastpixel)
            {
                sys_vGui("%d %f \\\n", ixpix,
                    canvas_valueToPixelY(glist,
                        baseY + yloc + field_convertValueToPosition(yfielddesc,
                            yval)));
                ndrawn++;
            }
            lastpixel = ixpix;
            if (ndrawn >= 1000) break;
        }
            // TK will complain if there aren't at least 2 points...
        if (ndrawn == 0) sys_vGui("0 0 0 0 \\\n");
        else if (ndrawn == 1) sys_vGui("%d %f \\\n", ixpix + 10,
            canvas_valueToPixelY(glist, baseY + yloc + 
                field_convertValueToPosition(yfielddesc, yval)));

        sys_vGui("-width %f\\\n", linewidth);
        sys_vGui("-fill %s\\\n", outline);
        if (style == PLOT_CURVES) sys_vGui("-smooth 1\\\n");

        sys_vGui("-tags [list PLOT%lx array]\n", w);
    }
    
    post_log ("! %d", ndrawn);
    */
}

static void plot_behaviorVisibilityChangedRecursive (void)
{
    // Visible
    /*
    for (xsum = xloc, i = 0; i < nelem; i++)
    {
        t_float usexloc, useyloc;
        t_gobj *y;
        if (xonset >= 0)
            usexloc = baseX + xloc +
                *(t_float *)((elem + elemsize * i) + xonset);
        else usexloc = baseX + xsum, xsum += xinc;
        if (yonset >= 0)
            yval = *(t_float *)((elem + elemsize * i) + yonset);
        else yval = 0;
        useyloc = baseY + yloc +
            field_convertValueToPosition(yfielddesc, yval);
        for (y = elemtemplatecanvas->gl_graphics; y; y = y->g_next)
        {
            t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
            if (!wb) continue;
            (*wb->w_fnParentVisibilityChanged)(y, glist,
                (t_word *)(elem + elemsize * i),
                    elemtemplate, usexloc, useyloc, isVisible);
        }
    }*/
    
    // NOT Visible
    /*
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
    */
}

static void plot_behaviorVisibilityChanged (t_gobj *z,
    t_glist *glist, 
    t_word *w,
    t_template *tmpl,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_plot *x = (t_plot *)z;

    t_array *array = NULL;
    t_float width;
    t_float positionX;
    t_float positionY;
    t_float incrementX;
    t_float style;
    int visible;
    
    if (!plot_fetchScalarFields (x, w, tmpl,
            &array,
            &width,
            &positionX,
            &positionY,
            &incrementX,
            &style,
            &visible)) {
    //
    t_symbol *fieldX = NULL;
    t_symbol *fieldY = NULL;
    t_symbol *fieldW = NULL;
    
    if (!isVisible || visible) {
    //
    if (!plot_fetchElementFieldNames (x, array, &fieldX, &fieldY, &fieldW)) {

        if (isVisible) {
        
            if (style == PLOT_POINTS) { 
            
                plot_behaviorVisibilityChangedDrawPoint (x,
                    array,
                    fieldX, 
                    fieldY,
                    fieldW,
                    baseX + positionX,
                    baseY + positionY,
                    incrementX);
                    
            } else {
                plot_behaviorVisibilityChangedDrawPolygon();
            }
            
        } else {
            sys_vGui (".x%lx.c delete PLOT%lx\n", canvas_getView (glist), w);      
        }
        
        plot_behaviorVisibilityChangedRecursive();
    }
    //
    }
    //
    }
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error plot_getFields (t_plot *x, t_symbol *elementTemplateIdentifier,
    t_glist **elementView,
    t_template **elementTemplate,
    int *elementSize,
    int *onsetX,
    int *onsetY,
    int *onsetW)
{
    t_fielddescriptor *xfielddesc = &x->x_fieldX;
    t_fielddescriptor *yfielddesc = &x->x_fieldY;
    t_fielddescriptor *wfielddesc = &x->x_fieldW;
        
    int arrayonset;
    int elemsize;
    int yonset;
    int wonset;
    int xonset;
    int type;
    t_template *elemtemplate;
    t_symbol *dummy;
    t_symbol *varname;
    t_glist *elemtemplatecanvas = NULL;

    if (!(elemtemplate = template_findByIdentifier(elementTemplateIdentifier)))
    {
        post_error ("plot: %s: no such template", elementTemplateIdentifier->s_name);
        return PD_ERROR;
    }
    if (!(elemtemplatecanvas = template_getFirstInstanceView (elemtemplate)))
    {
        post_error ("plot: %s: no canvas for this template", elementTemplateIdentifier->s_name);
        return PD_ERROR;
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
        
    *elementView = elemtemplatecanvas;
    *elementTemplate = elemtemplate;
    *elementSize = elemsize;
    *onsetX = xonset;
    *onsetY = yonset;
    *onsetW = wonset;
    
    return PD_ERROR_NONE;
}

static void array_getcoordinate (t_plot *x, t_glist *glist,
    char *elem, int xonset, int yonset, int wonset, int indx,
    t_float basex, t_float basey, t_float xinc,
    t_float *xp, t_float *yp, t_float *wp)
{
    t_fielddescriptor *xfielddesc = &x->x_fieldX;
    t_fielddescriptor *yfielddesc = &x->x_fieldY;
    t_fielddescriptor *wfielddesc = &x->x_fieldW;
        
    t_float xval, yval, ypix, wpix;
    if (xonset >= 0)
        xval = *(t_float *)(elem + xonset);
    else xval = indx * xinc;
    if (yonset >= 0)
        yval = *(t_float *)(elem + yonset);
    else yval = 0;
    
    ypix = canvas_valueToPixelY(glist, basey + field_convertValueToPosition(yfielddesc, yval));
        
    if (wonset >= 0)
    {
            /* found "w" field which controls linewidth. */
        t_float wval = *(t_float *)(elem + wonset);
        t_float t = basey + field_convertValueToPosition (yfielddesc, yval) + field_convertValueToPosition (wfielddesc, wval);
        wpix = canvas_valueToPixelY (glist, t) - ypix;
        if (wpix < 0)
            wpix = -wpix;
    }
    else wpix = 1;
    
    *xp = canvas_valueToPixelX(glist, basex + field_convertValueToPosition(xfielddesc, xval));
    *yp = ypix;
    *wp = wpix;
}

static void plot_behaviorVisibilityChanged (t_gobj *z,
    t_glist *glist, 
    t_word *w,
    t_template *tmpl,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_plot *x = (t_plot *)z;
    int elemsize;
    int yonset;
    int wonset;
    int xonset;
    int i;
    t_glist *elemtemplatecanvas;
    t_template *elemtemplate;
    t_float linewidth;
    t_float xloc;
    t_float xinc;
    t_float yloc;
    t_float style;
    t_float usexloc;
    t_float xsum;
    t_float yval;
    t_float vis;
    t_float scalarvis;
    t_array *array;
    int nelem;
    char *elem;
    t_fielddescriptor *xfielddesc = &x->x_fieldX;
    t_fielddescriptor *yfielddesc = &x->x_fieldY;
    t_fielddescriptor *wfielddesc = &x->x_fieldW;
    
    if (plot_fetchScalarFields (x, w, tmpl, 
        &array, &linewidth, &xloc, &yloc, &xinc, &style,
        &vis) ||
            ((vis == 0) && isVisible) /* see above for 'tovis' */
            || plot_getFields(x, array_getTemplateIdentifier (array), &elemtemplatecanvas,
                &elemtemplate, &elemsize,
                &xonset, &yonset, &wonset))
                    return;
    nelem = array->a_size;
    elem = (char *)array->a_vector;

    if (isVisible)
    {
        if (style == PLOT_POINTS)
        {
            t_float minyval = PLOT_MAX, maxyval = -PLOT_MAX;
            int ndrawn = 0;
            for (xsum = baseX + xloc, i = 0; i < nelem; i++)
            {
                t_float yval, xpix, ypix, nextxloc;
                int ixpix, inextx;

                if (xonset >= 0)
                {
                    usexloc = baseX + xloc +
                        *(t_float *)((elem + elemsize * i) + xonset);
                    ixpix = canvas_valueToPixelX(glist, 
                        field_convertValueToPosition(xfielddesc, usexloc));
                    inextx = ixpix + 2;
                }
                else
                {
                    usexloc = xsum;
                    xsum += xinc;
                    ixpix = canvas_valueToPixelX(glist,
                        field_convertValueToPosition(xfielddesc, usexloc));
                    inextx = canvas_valueToPixelX(glist,
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
                        ixpix, (int)canvas_valueToPixelY(glist, 
                            baseY + field_convertValueToPosition(yfielddesc, minyval)),
                        inextx, (int)(canvas_valueToPixelY(glist, 
                            baseY + field_convertValueToPosition(yfielddesc, maxyval))
                                + linewidth), w);
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
                color_withDigits (word_getFloatByDescriptor (w, tmpl, &x->x_colorOutline)));
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
                    xpix = canvas_valueToPixelX(glist,
                        baseX + field_convertValueToPosition(xfielddesc, usexloc));
                    ixpix = xpix + 0.5;
                    if (xonset >= 0 || ixpix != lastpixel)
                    {
                        sys_vGui("%d %f \\\n", ixpix,
                            canvas_valueToPixelY(glist,
                                baseY + field_convertValueToPosition(yfielddesc, 
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
                    xpix = canvas_valueToPixelX(glist,
                        baseX + field_convertValueToPosition(xfielddesc, usexloc));
                    ixpix = xpix + 0.5;
                    if (xonset >= 0 || ixpix != lastpixel)
                    {
                        sys_vGui("%d %f \\\n", ixpix, canvas_valueToPixelY(glist,
                            baseY + yloc + field_convertValueToPosition(yfielddesc,
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
                    sys_vGui("%d %f \\\n", ixpix + 10, canvas_valueToPixelY(glist,
                        baseY + yloc + field_convertValueToPosition(yfielddesc,
                            yval) +
                                field_convertValueToPosition(wfielddesc, wval)));
                    sys_vGui("%d %f \\\n", ixpix + 10, canvas_valueToPixelY(glist,
                        baseY + yloc + field_convertValueToPosition(yfielddesc,
                            yval) -
                                field_convertValueToPosition(wfielddesc, wval)));
                }
            ouch:
                sys_vGui(" -width 1 -fill %s -outline %s\\\n",
                    outline, outline);
                if (style == PLOT_CURVES) sys_vGui("-smooth 1\\\n");

                sys_vGui("-tags [list plot%lx array]\n", w);
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
                    xpix = canvas_valueToPixelX(glist,
                        baseX + field_convertValueToPosition(xfielddesc, usexloc));
                    ixpix = xpix + 0.5;
                    if (xonset >= 0 || ixpix != lastpixel)
                    {
                        sys_vGui("%d %f \\\n", ixpix,
                            canvas_valueToPixelY(glist,
                                baseY + yloc + field_convertValueToPosition(yfielddesc,
                                    yval)));
                        ndrawn++;
                    }
                    lastpixel = ixpix;
                    if (ndrawn >= 1000) break;
                }
                    /* TK will complain if there aren't at least 2 points... */
                if (ndrawn == 0) sys_vGui("0 0 0 0 \\\n");
                else if (ndrawn == 1) sys_vGui("%d %f \\\n", ixpix + 10,
                    canvas_valueToPixelY(glist, baseY + yloc + 
                        field_convertValueToPosition(yfielddesc, yval)));

                sys_vGui("-width %f\\\n", linewidth);
                sys_vGui("-fill %s\\\n", outline);
                if (style == PLOT_CURVES) sys_vGui("-smooth 1\\\n");

                sys_vGui("-tags [list plot%lx array]\n", w);
            }
        }
            /* We're done with the outline; now draw all the points.
            This code is inefficient since the template has to be
            searched for drawing instructions for every last point. */
        if (1)
        {
            for (xsum = xloc, i = 0; i < nelem; i++)
            {
                t_float usexloc, useyloc;
                t_gobj *y;
                if (xonset >= 0)
                    usexloc = baseX + xloc +
                        *(t_float *)((elem + elemsize * i) + xonset);
                else usexloc = baseX + xsum, xsum += xinc;
                if (yonset >= 0)
                    yval = *(t_float *)((elem + elemsize * i) + yonset);
                else yval = 0;
                useyloc = baseY + yloc +
                    field_convertValueToPosition(yfielddesc, yval);
                for (y = elemtemplatecanvas->gl_graphics; y; y = y->g_next)
                {
                    t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
                    if (!wb) continue;
                    (*wb->w_fnParentVisibilityChanged)(y, glist,
                        (t_word *)(elem + elemsize * i),
                            elemtemplate, usexloc, useyloc, isVisible);
                }
            }
        }
    }
    else
    {
            /* un-draw the individual points */
        if (1)
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
            canvas_getView(glist), w);      
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -


    /* try clicking on an element of the array as a scalar (if clicking
    on the trace of the array failed) */
static int array_doclick_element(t_plot *x, t_array *array, t_glist *glist,
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
    if (plot_getFields(x, elemtemplatesym, &elemtemplatecanvas,
        &elemtemplate, &elemsize,
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

static int array_doclick (t_plot *x, t_array *array, t_glist *glist, t_scalar *sc,
    t_array *ap, t_symbol *elemtemplatesym,
    t_float linewidth, t_float xloc, t_float xinc, t_float yloc, t_float scalarvis,
    t_fielddescriptor *xfield, t_fielddescriptor *yfield, t_fielddescriptor *wfield,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_glist *elemtemplatecanvas;
    t_template *elemtemplate;
    int elemsize, yonset, wonset, xonset, i, callmotion = 0;

    if (!plot_getFields(x, elemtemplatesym, &elemtemplatecanvas,
        &elemtemplate, &elemsize,
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
        plot_stepX = canvas_valueForOnePixelX (glist);
        plot_stepY = canvas_valueForOnePixelY (glist);
            /* if we're a garray, the only one here, and if we appear to have
            only a 'y' field, click always succeeds and furthermore we'll
            call "motion" later. */
        if (glist->gl_graphics && pd_class(&glist->gl_graphics->g_pd) == garray_class
            && !glist->gl_graphics->g_next &&
                elemsize == ARRAY_WORD)
        {
            int xval = canvas_pixelToValueX(glist, xpix);
            if (xval < 0)
                xval = 0;
            else if (xval >= array->a_size)
                xval = array->a_size - 1;
            plot_fieldY = yfield;
            plot_cumulativeY = canvas_pixelToValueY(glist, ypix);
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
                    canvas_pixelToValueY(glist, ypix));
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
                array_getcoordinate(x, glist,
                    (char *)(array->a_vector) + i * elemsize,
                    xonset, yonset, wonset, i, xloc, yloc, xinc,
                    &pxpix, &pypix, &pwpix);
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
                if (1)
                    return (array_doclick_element(x, array, glist, sc, ap,
                        elemtemplatesym, linewidth, xloc, xinc, yloc,
                            xfield, yfield, wfield,
                            xpix, ypix, shift, alt, dbl, doit));
                else return (0);
            }
            best += 0.001;  /* add truncation error margin */
            for (i = 0; i < array->a_size; i += incr)
            {
                t_float pxpix, pypix, pwpix, dx, dy, dy2, dy3;
                array_getcoordinate(x, glist, (char *)(array->a_vector) + i * elemsize,
                    xonset, yonset, wonset, i, xloc, yloc, xinc,
                    &pxpix, &pypix, &pwpix);
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

static int plot_behaviorClicked(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_plot *x = (t_plot *)z;
    t_float linewidth, xloc, xinc, yloc, style, scalarvis;
    int vis;
    t_array *array;
    
    t_fielddescriptor *xfielddesc = &x->x_fieldX;
    t_fielddescriptor *yfielddesc = &x->x_fieldY;
    t_fielddescriptor *wfielddesc = &x->x_fieldW;

    if (!plot_fetchScalarFields(x, data, template, 
        &array, &linewidth, &xloc, &yloc, &xinc, &style,
        &vis) && (vis != 0))
    {
        return (array_doclick(x, array, glist, sc, ap,
            array_getTemplateIdentifier (array),
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

static void *plot_new (t_symbol *s, int argc, t_atom *argv)
{
    t_plot *x = (t_plot *)pd_new (plot_class);
    
    field_setAsFloatConstant (&x->x_array,          0.0);       /* Default is invalid. */
    field_setAsFloatConstant (&x->x_colorOutline,   0.0);
    field_setAsFloatConstant (&x->x_width,          1.0);
    field_setAsFloatConstant (&x->x_positionX,      1.0);
    field_setAsFloatConstant (&x->x_positionY,      1.0);
    field_setAsFloatConstant (&x->x_incrementX,     1.0);
    field_setAsFloatConstant (&x->x_style,          (t_float)PLOT_POLYGONS);
    field_setAsFloatConstant (&x->x_isVisible,      1.0);
    
    field_setAsFloatVariable (&x->x_fieldX,         sym_x);
    field_setAsFloatVariable (&x->x_fieldY,         sym_y);
    field_setAsFloatVariable (&x->x_fieldW,         sym_w);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if ((PD_WITH_LEGACY && t == sym_curve) || t == sym___dash__c || t == sym___dash__curve) {
            field_setAsFloatConstant (&x->x_style, (t_float)PLOT_CURVES);
            argc -= 1; argv += 1;
            
        } else if (argc > 1 && (t == sym___dash__v || t == sym___dash__visible)) {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__x) {
            field_setAsFloat (&x->x_fieldX, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__y) {
            field_setAsFloat (&x->x_fieldY, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__w) {
            field_setAsFloat (&x->x_fieldW, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else { break; }
    }
    
    if (argc) { field_setAsArray (&x->x_array,          argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_colorOutline,   argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_width,          argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_positionX,      argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_positionY,      argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_incrementX,     argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_style,          argc--, argv++); }

    return x;
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
