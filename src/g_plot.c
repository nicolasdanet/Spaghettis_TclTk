
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
static int                  plot_numberOfElements;      /* Shared. */
static int                  plot_fatten;                /* Shared. */

static t_fielddescriptor    *plot_fieldDescriptorX;     /* Shared. */
static t_fielddescriptor    *plot_fieldDescriptorY;     /* Shared. */
static t_glist              *plot_glist;                /* Shared. */
static t_scalar             *plot_asScalar;             /* Shared. */
static t_array              *plot_asArray;              /* Shared. */
static t_word               *plot_data;                 /* Shared. */
static t_array              *plot_array;                /* Shared. */

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

#define PLOT_MAX                1e20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PLOT_CLIP(x)            (((x) > -PLOT_MAX && (x) < PLOT_MAX) ? (x) : 0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_HANDLE_SIZE        8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_BUFFER_SIZE        4096
#define PLOT_MAXIMUM_DRAWN      256

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

    int size = array_getSize (array);
    
    if (fieldX) { valueX = (i < size) ? array_getFloatInElementAtIndex (array, i, fieldX) : 0.0; }
    else { 
        valueX = i * incrementX;
    }
    
    if (fieldY) { valueY = (i < size) ? array_getFloatInElementAtIndex (array, i, fieldY) : 0.0; }
    else { 
        valueY = 0.0;
    }
    
    if (fieldW) { valueW = (i < size) ? array_getFloatInElementAtIndex (array, i, fieldW) : 0.0; }
    else {
        valueW = 0.0;
    }
    
    *a = relativeX + field_convertValueToPosition (&x->x_fieldX, valueX);
    *b = relativeY + field_convertValueToPosition (&x->x_fieldY, valueY);
    *c = field_convertValueToPosition (&x->x_fieldY, valueW);
}

static int plot_getStep (t_array *array)
{
    int size = array_getSize (array); return (size <= 1000 ? 1 : (int)sqrt ((double)size));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void plot_float (t_plot *x, t_float f)
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

static void plot_motion (void *dummy, t_float deltaX, t_float deltaY, t_float modifier)
{
    /*
    plot_cumulativeX += deltaX * plot_stepX;
    plot_cumulativeY += deltaY * plot_stepY;
    if (plot_fieldDescriptorX)
    {
            // it's an x, y plot
        int i;
        for (i = 0; i < plot_numberOfElements; i++)
        {
            t_word *thisword = (t_word *)(((char *)plot_data) +
                i * plot_elementSize);
            t_float xwas = word_getFloatByDescriptorAsPosition(
                thisword,
                plot_template,
                plot_fieldDescriptorX);
            t_float ywas = (plot_fieldDescriptorY ?
                word_getFloatByDescriptorAsPosition(
                    thisword, 
                    plot_template,
                    plot_fieldDescriptorY) : 0);
            word_setFloatByDescriptorAsPosition(
                thisword,
                plot_template,
                plot_fieldDescriptorX,
                xwas + deltaX);
            if (plot_fieldDescriptorY)
            {
                if (plot_fatten)
                {
                    if (i == 0)
                    {
                        t_float newy = ywas + deltaY * plot_stepY;
                        if (newy < 0)
                            newy = 0;
                        word_setFloatByDescriptorAsPosition(
                            thisword,
                            plot_template,
                            plot_fieldDescriptorY,
                            newy);
                    }
                }
                else
                {
                    word_setFloatByDescriptorAsPosition(
                        thisword,
                        plot_template,
                        plot_fieldDescriptorY,
                        ywas + deltaY * plot_stepY);
                }
            }
        }
    }
    else if (plot_fieldDescriptorY)
    {
            // a y-only plot.
        int thisx = plot_currentX + plot_cumulativeX + 0.5, x2;
        int increment, i, nchange;
        t_float newy = plot_cumulativeY,
            oldy = word_getFloatByDescriptorAsPosition(
                (t_word *)(((char *)plot_data) + plot_elementSize * plot_previousX),
                plot_template,
                plot_fieldDescriptorY);
        t_float ydiff = newy - oldy;
        if (thisx < 0) thisx = 0;
        else if (thisx >= plot_numberOfElements)
            thisx = plot_numberOfElements - 1;
        increment = (thisx > plot_previousX ? -1 : 1);
        nchange = 1 + increment * (plot_previousX - thisx);

        for (i = 0, x2 = thisx; i < nchange; i++, x2 += increment)
        {
            word_setFloatByDescriptorAsPosition(
                (t_word *)(((char *)plot_data) + plot_elementSize * x2),
                plot_template,
                plot_fieldDescriptorY,
                newy);
            if (nchange > 1)
                newy -= ydiff * (1./(nchange - 1));
         }
         plot_previousX = thisx;
    }
    if (plot_asScalar)
        scalar_redraw(plot_asScalar, plot_glist);
    if (plot_asArray)
        array_redraw(plot_asArray, plot_glist);
    */
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
        pixelW = PD_MAX (pixelW, width - 1);

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

static void plot_behaviorVisibilityChangedDrawPoint (t_plot *x,
    t_glist *glist,
    t_word  *w,
    t_array *array,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    t_float relativeX,
    t_float relativeY, 
    t_float incrementX, 
    t_float width)
{
    int numberOfElements  = array_getSize (array);
    t_float minimumValueY =  PLOT_MAX;
    t_float maximumValueY = -PLOT_MAX;
    int i;
    
    for (i = 0; i < numberOfElements; i++) {
    //
    t_float valueX;
    t_float valueY;
    t_float valueW;
    int pixelX, nextPixelX;
    
    plot_getCoordinates (x,
        array,
        fieldX,
        fieldY,
        fieldW,
        i,
        relativeX,
        relativeY,
        incrementX, 
        &valueX, 
        &valueY, 
        &valueW);
    
    pixelX = (int)canvas_valueToPixelX (glist, valueX);
    
    minimumValueY = PD_MIN (minimumValueY, PLOT_CLIP (valueY));
    maximumValueY = PD_MAX (maximumValueY, PLOT_CLIP (valueY));
    
    plot_getCoordinates (x, 
        array,
        fieldX,
        fieldY,
        fieldW,
        i + 1,
        relativeX,
        relativeY,
        incrementX, 
        &valueX, 
        &valueY, 
        &valueW);
        
    nextPixelX = (int)canvas_valueToPixelX (glist, valueX);

    if (i == numberOfElements - 1 || pixelX != nextPixelX) {
        
        t_float min = relativeY + field_convertValueToPosition (&x->x_fieldY, minimumValueY);
        t_float max = relativeY + field_convertValueToPosition (&x->x_fieldY, maximumValueY);
        
        sys_vGui (".x%lx.c create rectangle %d %d %d %d"
                        " -width %d"
                        " -fill #%06x"
                        " -tags PLOT%lx\n",
                        canvas_getView (glist),
                        (fieldX == NULL) ? pixelX : pixelX - 1,
                        (int)canvas_valueToPixelY (glist, min),
                        (fieldX == NULL) ? nextPixelX : pixelX + 1,
                        (int)canvas_valueToPixelY (glist, max),
                        (int)PD_MAX (0, width - 1),
                        COLOR_NORMAL,
                        (t_int)w);
    
        minimumValueY =  PLOT_MAX;
        maximumValueY = -PLOT_MAX;
    }
    //
    }
}

static void plot_behaviorVisibilityChangedDrawPolygonFill (t_plot *x,
    t_glist *glist,
    t_word  *w,
    t_array *array,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    t_float relativeX,
    t_float relativeY, 
    t_float incrementX, 
    t_float width, 
    int style, 
    t_symbol *color)
{
    int numberOfElements = array_getSize (array);
    int coordinatesX[PLOT_MAXIMUM_DRAWN] = { 0 };     
    int coordinatesL[PLOT_MAXIMUM_DRAWN] = { 0 };
    int coordinatesH[PLOT_MAXIMUM_DRAWN] = { 0 };
    int elementsDrawn = 0;
    int i;
    
    int pixelX, previousPixelX = -PD_INT_MAX;
        
    for (i = 0; i < numberOfElements; i++) {
    //
    t_float valueX;
    t_float valueY;
    t_float valueW;
    
    plot_getCoordinates (x,
        array,
        fieldX,
        fieldY,
        fieldW,
        i,
        relativeX,
        relativeY,
        incrementX, 
        &valueX, 
        &valueY, 
        &valueW);
    
    valueY = PLOT_CLIP (valueY);
    valueW = PLOT_CLIP (valueW);
    pixelX = (int)canvas_valueToPixelX (glist, valueX);
    
    if (fieldX || pixelX != previousPixelX) {
    //
    t_float pixelY = canvas_valueToPixelY (glist, valueY);
    t_float pixelW = canvas_valueToPixelY (glist, valueY + valueW) - pixelY;
    
    pixelW = PD_ABS (pixelW);
    pixelW = PD_MAX (pixelW, width - 1.0);

    coordinatesX[elementsDrawn] = pixelX;
    coordinatesL[elementsDrawn] = pixelY - pixelW;
    coordinatesH[elementsDrawn] = pixelY + pixelW;
    elementsDrawn++; 
    
    if (elementsDrawn >= PLOT_MAXIMUM_DRAWN) { break; }
    //
    }

    previousPixelX = pixelX;
    //
    }
    
    if (elementsDrawn) {        /* Tk requires at least three points (i.e. two elements). */
    //
    char t[PLOT_BUFFER_SIZE] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    err |= string_sprintf (t, PLOT_BUFFER_SIZE,         ".x%lx.c create polygon", canvas_getView (glist));
  
    for (i = 0; i < elementsDrawn; i++) {
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " %d %d", coordinatesX[i], coordinatesL[i]);
    }
    
    if (elementsDrawn == 1) { 
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " %d %d", coordinatesX[0] + 1, coordinatesL[0]);
    } 
    
    for (i = elementsDrawn - 1; i >= 0; i--) {
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " %d %d", coordinatesX[i], coordinatesH[i]);
    }
    
    if (elementsDrawn == 1) { 
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " %d %d", coordinatesX[0] + 1, coordinatesH[0]);
    } 
    
    err |= string_addSprintf (t, PLOT_BUFFER_SIZE,      " -fill %s", color->s_name);
    err |= string_addSprintf (t, PLOT_BUFFER_SIZE,      " -outline %s", color->s_name);

    if (style == PLOT_CURVES) { 
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " -width 1 -smooth 1 -tags PLOT%lx\n", (t_int)w);
    } else { 
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " -width 1 -tags PLOT%lx\n", (t_int)w);
    }
    
    if (!err) { sys_gui (t); }
    else {
        PD_BUG;
    }
    //
    }
}

static void plot_behaviorVisibilityChangedDrawPolygonSegment (t_plot *x,
    t_glist *glist,
    t_word  *w,
    t_array *array,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    t_float relativeX,
    t_float relativeY, 
    t_float incrementX, 
    t_float width, 
    int style, 
    t_symbol *color)
{
    char t[PLOT_BUFFER_SIZE] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    int numberOfElements = array_getSize (array);
    int elementsDrawn = 0;
    int i;
    
    int pixelY, pixelX, previousPixelX = -PD_INT_MAX;
        
    err |= string_sprintf (t, PLOT_BUFFER_SIZE,         ".x%lx.c create line", canvas_getView (glist));
    
    for (i = 0; i < numberOfElements; i++) {
    //
    t_float valueX;
    t_float valueY;
    t_float valueW;

    plot_getCoordinates (x,
        array,
        fieldX,
        fieldY,
        fieldW,
        i,
        relativeX,
        relativeY,
        incrementX, 
        &valueX, 
        &valueY, 
        &valueW);
    
    valueY = PLOT_CLIP (valueY);
    pixelX = (int)canvas_valueToPixelX (glist, valueX);
    
    if (fieldX || pixelX != previousPixelX) {
        pixelY = (int)canvas_valueToPixelY (glist, valueY);
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " %d %d", pixelX, pixelY);
        elementsDrawn++;
        previousPixelX = pixelX;
    }
    //
    }
    
    
    if (elementsDrawn) {    /* Tk requires at least two points. */
    //
    if (elementsDrawn == 1) { 
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " %d %d", pixelX + 1, pixelY);
    }
    
    err |= string_addSprintf (t, PLOT_BUFFER_SIZE,      " -width %d", (int)(PD_MAX (0, width - 1)));
    err |= string_addSprintf (t, PLOT_BUFFER_SIZE,      " -fill %s", color->s_name);

    if (style == PLOT_CURVES) {
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " -smooth 1 -tags PLOT%lx\n", (t_int)w);
    } else {
        err |= string_addSprintf (t, PLOT_BUFFER_SIZE,  " -tags PLOT%lx\n", (t_int)w);
    }

    if (!err) { sys_gui (t); }
    else {
        PD_BUG;
    }
    //
    }
}

static void plot_behaviorVisibilityChangedRecursive (t_plot *x,
    t_glist *glist,
    t_word  *w,
    t_array *array,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    t_float relativeX,
    t_float relativeY, 
    t_float incrementX, 
    t_float width, 
    int isVisible)
{
    t_glist *view = template_getFirstInstanceView (array_getTemplate (array));
    
    if (view) {
    //
    int numberOfElements = array_getSize (array);
    int i;
        
    for (i = 0; i < numberOfElements; i++) {
    //
    t_gobj *y = NULL;
    
    t_float valueX;
    t_float valueY;
    t_float valueW;

    plot_getCoordinates (x,
        array,
        fieldX,
        fieldY,
        fieldW,
        i,
        relativeX,
        relativeY,
        incrementX, 
        &valueX, 
        &valueY, 
        &valueW);
    
    for (y = view->gl_graphics; y; y = y->g_next) {
        t_parentwidgetbehavior *behavior = class_getParentWidget (pd_class (y));
        if (behavior) {
            (*behavior->w_fnParentVisibilityChanged) (y,
                glist,
                array_getElementAtIndex (array, i),
                array_getTemplate (array),
                valueX,
                valueY,
                isVisible);
        }
    }
    //
    }
    //
    }
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
                    glist,
                    w,
                    array,
                    fieldX, 
                    fieldY,
                    fieldW,
                    baseX + positionX,
                    baseY + positionY,
                    incrementX, 
                    width);
                    
            } else {
                
                int color = (int)word_getFloatByDescriptor (w, tmpl, &x->x_colorOutline);
                
                if (fieldW) {
                
                    plot_behaviorVisibilityChangedDrawPolygonFill (x,
                        glist,
                        w,
                        array,
                        fieldX, 
                        fieldY, 
                        fieldW, 
                        baseX + positionX, 
                        baseY + positionY, 
                        incrementX,
                        width, 
                        style, 
                        color_toEncodedSymbol (color_withDigits (color)));
                        
                } else {
                    plot_behaviorVisibilityChangedDrawPolygonSegment (x,
                        glist,
                        w,
                        array,
                        fieldX, 
                        fieldY, 
                        fieldW, 
                        baseX + positionX, 
                        baseY + positionY, 
                        incrementX,
                        width, 
                        style, 
                        color_toEncodedSymbol (color_withDigits (color)));
                }
            }
            
        } else {
            sys_vGui (".x%lx.c delete PLOT%lx\n", canvas_getView (glist), w); 
        }
        
        plot_behaviorVisibilityChangedRecursive (x,
            glist,
            w,
            array,
            fieldX, 
            fieldY,
            fieldW,
            baseX + positionX,
            baseY + positionY,
            incrementX, 
            width,
            isVisible);
    }
    //
    }
    //
    }
}

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

static int plot_behaviorClickedPerformRegularRecursive (t_plot *x,
    t_array *array,
    t_glist *glist,
    t_scalar *sc,
    t_array *ap,
    t_float xloc,
    t_float yloc,
    t_float xinc,
    t_float linewidth,
    int xpix,
    int ypix,
    int shift,
    int alt,
    int dbl,
    int doit)
{
    /*
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
        // if it has more than 2000 points, just check 300 of them.
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
    */
    
    return 0;
}

static int plot_behaviorClickedPerformRegularMatch (t_plot *x,
    t_array  *array,
    t_glist  *glist,
    t_scalar *asScalar,
    t_array  *asArray,
    t_float relativeX,
    t_float relativeY,
    t_float incrementX,
    t_float width,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
        /*
    bestError += 0.001;  // add truncation error margin
    for (i = 0; i < array->a_size; i += incr)
    {
        t_float pxpix, pypix, pwpix, dx, dy, dy2, dy3;
        array_getcoordinate(x, glist, (char *)(array->a_vector) + i * elemsize,
            xonset, yonset, wonset, i, relativeX, relativeY, incrementX,
            &pxpix, &pypix, &pwpix);
        if (pwpix < 4)
            pwpix = 4;
        dx = pxpix - a;
        if (dx < 0) dx = -dx;
        dy = pypix - b;
        if (dy < 0) dy = -dy;
        if (wonset >= 0)
        {
            dy2 = (pypix + pwpix) - b;
            if (dy2 < 0) dy2 = -dy2;
            dy3 = (pypix - pwpix) - b;
            if (dy3 < 0) dy3 = -dy3;
            if (yonset < 0)
                dy = 100;
        }
        else dy2 = dy3 = 100;
        if (dx + dy <= bestError || dx + dy2 <= bestError || dx + dy3 <= bestError)
        {
            if (dy < dy2 && dy < dy3)
                plot_fatten = 0;
            else if (dy2 < dy3)
                plot_fatten = -1;
            else plot_fatten = 1;
            if (clicked)
            {
                char *elem = (char *)array->a_vector;
                if (alt && a < pxpix) // delete a point
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
                    // add a point (after the clicked-on one)
                    array_resizeAndRedraw(array, glist, array->a_size + 1);
                    elem = (char *)array->a_vector;
                    memmove(elem + elemsize * (i+1), 
                        elem + elemsize * i,
                            (array->a_size - i - 1) * elemsize);
                    i++;
                }
                if (xonset >= 0)
                {
                    plot_fieldDescriptorX = xfield;
                    plot_cumulativeX = 
                        word_getFloatByDescriptorAsPosition(
                            (t_word *)(elem + i * elemsize),
                            plot_template,
                            xfield);
                        plot_data = (t_word *)(elem + i * elemsize);
                    if (shift)
                        plot_numberOfElements = array->a_size - i;
                    else plot_numberOfElements = 1;
                }
                else
                {
                    plot_fieldDescriptorX = 0;
                    plot_cumulativeX = 0;
                    plot_data = (t_word *)elem;
                    plot_numberOfElements = array->a_size;

                    plot_currentX = i;
                    plot_previousX = i;
                    plot_stepX *= (incrementX == 0 ? 1 : 1./incrementX);
                }
                if (plot_fatten)
                {
                    plot_fieldDescriptorY = wfield;
                    plot_cumulativeY = 
                        word_getFloatByDescriptorAsPosition(
                            (t_word *)(elem + i * elemsize),
                            plot_template,
                            wfield);
                    plot_stepY *= -plot_fatten;
                }
                else if (yonset >= 0)
                {
                    plot_fieldDescriptorY = yfield;
                    plot_cumulativeY = 
                        word_getFloatByDescriptorAsPosition(
                            (t_word *)(elem + i * elemsize),
                            plot_template,
                            yfield);
                        // *(t_float *)((elem + elemsize * i) + yonset);
                }
                else
                {
                    plot_fieldDescriptorY = 0;
                    plot_cumulativeY = 0;
                }
                canvas_setMotionFunction(glist, 0, (t_motionfn)plot_motion, a, b);
            }
            if (alt)
            {
                if (a < pxpix)
                    return (CURSOR_THICKEN); // CURSOR_EDIT_DISCONNECT
                else return (CURSOR_ADD);
            }
            else return (plot_fatten ?
                CURSOR_THICKEN : CURSOR_CLICK);
        }
    } */
    
    return 0;
}

static int plot_behaviorClickedPerformRegular (t_plot *x,
    t_array  *array,
    t_glist  *glist,
    t_scalar *asScalar,
    t_array  *asArray,
    t_float relativeX,
    t_float relativeY,
    t_float incrementX,
    t_float width,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    t_symbol *fieldX = NULL;
    t_symbol *fieldY = NULL;
    t_symbol *fieldW = NULL;
    
    if (!plot_fetchElementFieldNames (x, array, &fieldX, &fieldY, &fieldW)) {
    //
    int bestError = PD_INT_MAX;
    int bestErrorIndex = -1;
    
    int i, k = plot_getStep (array);
    
    for (i = 0; i < array_getSize (array); i += k) {
    //
    t_float valueX;
    t_float valueY;
    t_float valueW;
    t_float pixelX;
    t_float pixelY;
    t_float pixelW;
    
    int deltaX;
    
    plot_getCoordinates (x, array, fieldX, fieldY, fieldW,
        i,
        relativeX,
        relativeY,
        incrementX,
        &valueX,
        &valueY,
        &valueW);
    
    pixelX = canvas_valueToPixelX (glist, valueX);
    pixelY = canvas_valueToPixelY (glist, valueY);
    pixelW = canvas_valueToPixelY (glist, valueY + valueW) - pixelY;
    
    pixelW = PD_ABS (pixelW);
    pixelW = PD_MAX (pixelW, width - 1);
    
    deltaX = PD_ABS ((int)pixelX - a);

    if (deltaX <= PLOT_HANDLE_SIZE) {
    
        int deltaY = PD_ABS ((int)pixelY - b);
        int deltaL = PD_ABS ((int)pixelY - b - pixelW);
        int deltaH = PD_ABS ((int)pixelY - b + pixelW);
    
        if (deltaX + deltaY < bestError) { bestError = deltaX + deltaY; bestErrorIndex = i; }
        if (deltaX + deltaL < bestError) { bestError = deltaX + deltaL; bestErrorIndex = i; }
        if (deltaX + deltaH < bestError) { bestError = deltaX + deltaH; bestErrorIndex = i; }
    }
    //
    }
    
    if (bestError > PLOT_HANDLE_SIZE) {
    
        return (plot_behaviorClickedPerformRegularRecursive (x,
                    array,
                    glist,
                    asScalar,
                    asArray,
                    relativeX,
                    relativeY,
                    incrementX,
                    width,
                    a,
                    b,
                    shift,
                    alt, 
                    dbl,
                    clicked));
                    
    } else {
        
        return (plot_behaviorClickedPerformRegularMatch (x,
                    array,
                    glist,
                    asScalar,
                    asArray,
                    relativeX,
                    relativeY,
                    incrementX,
                    width,
                    a,
                    b,
                    shift,
                    alt, 
                    dbl,
                    clicked));
    }
    //
    }
    
    return 0;
}

static int plot_behaviorClickedPerformSingle (t_plot *x,
    t_array  *array,
    t_glist  *glist,
    t_scalar *asScalar,
    t_array  *asArray,
    t_float style,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    int size = array_getSize (array);
    t_float valueX = canvas_pixelToValueX (glist, a);
    t_float valueY = canvas_pixelToValueY (glist, b);
    
    int i = ((int)style == PLOT_POINTS) ? valueX : valueX + 0.5;
    
    i = PD_CLAMP (i, 0, size - 1);
    
    plot_fatten             = 0;
    plot_cumulativeX        = 0;
    plot_cumulativeY        = valueY;
    plot_previousX          = i;
    plot_currentX           = i;
    plot_numberOfElements   = size;
    plot_fieldDescriptorX   = NULL;
    plot_fieldDescriptorY   = &x->x_fieldY;
    plot_data               = array_getData (array);

    if (clicked) {
    //
    word_setFloatByDescriptorAsPosition (array_getElementAtIndex (array, i),
        array_getTemplate (array),
        &x->x_fieldY,
        valueY);
            
    canvas_setMotionFunction (glist, NULL, (t_motionfn)plot_motion, a, b);

    if (plot_asScalar) { scalar_redraw (plot_asScalar, plot_glist); }
    else {
        PD_ASSERT (plot_asArray); array_redraw (plot_asArray, plot_glist);
    }
    //
    }
    
    return 1;
}

static int plot_behaviorClickedPerform (t_plot *x,
    t_array  *array,
    t_glist  *glist,
    t_scalar *asScalar,
    t_array  *asArray,
    t_float relativeX,
    t_float relativeY,
    t_float incrementX,
    t_float width,
    t_float style,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    t_symbol *fieldX = NULL;
    t_symbol *fieldY = NULL;
    t_symbol *fieldW = NULL;

    if (!plot_fetchElementFieldNames (x, array, &fieldX, &fieldY, &fieldW)) {
    //
    plot_stepX    = canvas_valueForOnePixelX (glist);
    plot_stepY    = canvas_valueForOnePixelY (glist);
    plot_glist    = glist;
    plot_asScalar = asScalar;
    plot_asArray  = asArray;
    plot_array    = array;

    if (garray_isSingle (glist)) {
    
        return (plot_behaviorClickedPerformSingle (x,
            array,
            glist,
            asScalar,
            asArray,
            style,
            a,
            b,
            shift,
            alt,
            dbl,
            clicked));
            
    } else {

        return (plot_behaviorClickedPerformRegular (x,
            array,
            glist,
            asScalar,
            asArray,
            relativeX,
            relativeY,
            incrementX,
            width,
            a,
            b,
            shift,
            alt,
            dbl,
            clicked));
    }
    //
    }
    
    return 0;
}

static int plot_behaviorClicked (t_gobj *z,
    t_glist *glist, 
    t_word *w,
    t_template *tmpl,
    t_scalar *asScalar,
    t_array *asArray,
    t_float baseX,
    t_float baseY,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
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
            &visible) && (visible != 0)) {
        
    return (plot_behaviorClickedPerform (x, 
                array,
                glist,
                asScalar,
                asArray,
                baseX + positionX,
                baseY + positionY,
                incrementX,
                width,
                style,
                a,
                b,
                shift,
                alt,
                dbl,
                clicked));
    }
    
    return 0;
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
