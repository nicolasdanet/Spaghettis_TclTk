
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
static t_float              plot_relativeX;             /* Shared. */
static t_float              plot_relativeY;             /* Shared. */
static t_float              plot_incrementX;            /* Shared. */
static t_float              plot_width;                 /* Shared. */
static t_float              plot_style;                 /* Shared. */
static int                  plot_startX;                /* Shared. */
static int                  plot_previousX;             /* Shared. */
static int                  plot_thickness;             /* Shared. */
static t_float              plot_direction;             /* Shared. */
static t_gpointer           plot_gpointer;              /* Shared. */

static t_fielddescriptor    *plot_fieldDescriptorX;     /* Shared. */
static t_fielddescriptor    *plot_fieldDescriptorY;     /* Shared. */
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

#define PLOT_MAXIMUM_DRAWN      256
#define PLOT_HANDLE_SIZE        8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PLOT_THICKNESS_NONE     0
#define PLOT_THICKNESS_UP       1
#define PLOT_THICKNESS_DOWN     2

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_motionHorizontalVertical (void)
{
    if (plot_fieldDescriptorX) {
    
        word_setFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, plot_startX),
                array_getTemplate (plot_array),
                plot_fieldDescriptorX,
                plot_cumulativeX);
    }
    
    if (plot_fieldDescriptorY) {
    
        word_setFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, plot_startX),
                array_getTemplate (plot_array),
                plot_fieldDescriptorY,
                plot_cumulativeY);
    }
}

static void plot_motionVertical (void)
{
    t_float distanceX   = (plot_cumulativeX / plot_incrementX) + 0.5;
    int currentX        = PD_CLAMP ((int)(plot_startX + distanceX), 0, array_getSize (plot_array) - 1);
    
    int i = PD_MIN (currentX, plot_previousX);
    int j = PD_MAX (currentX, plot_previousX);
    int n = j - i;
    int back = currentX < plot_previousX;
        
    if (n > 0) {    /* Distribute change linearly between samples. */
    //
    t_float startY  = word_getFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, back ? j : i),
                            array_getTemplate (plot_array),
                            plot_fieldDescriptorY);
    t_float stepY   = (plot_cumulativeY - startY) / n;
    
    int k = back ? n : 0;
    
    for (; i <= j; i++) {
    
        word_setFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, i),
                array_getTemplate (plot_array),
                plot_fieldDescriptorY,
                startY + (stepY * k));
        
        if (back) { k--; } else { k++; }
    }
    //
    } else {
        word_setFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, i),
                array_getTemplate (plot_array),
                plot_fieldDescriptorY,
                plot_cumulativeY);
    }
    
    plot_previousX = currentX;
}

static void plot_motion (void *dummy, t_float deltaX, t_float deltaY, t_float modifier)
{
    if (gpointer_isValid (&plot_gpointer)) {
    //
    plot_cumulativeX += deltaX * plot_stepX;
    plot_cumulativeY += deltaY * plot_stepY * (plot_thickness ? plot_direction : 1.0);
        
    if (plot_fieldDescriptorX)      { plot_motionHorizontalVertical(); }
    else if (plot_fieldDescriptorY) { plot_motionVertical(); }
    
    if (gpointer_getTemplateIdentifier (&plot_gpointer) != sym___TEMPLATE__float__dash__array) {
    //
    PD_ASSERT (gpointer_isScalar (&plot_gpointer));
    
    template_notify (gpointer_getTemplate (&plot_gpointer), 
        gpointer_getView (&plot_gpointer), 
        gpointer_getScalar (&plot_gpointer),
        sym_change,
        0,
        NULL);
    //
    }
    
    gpointer_redraw (&plot_gpointer);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_behaviorGetRectangleRecursive (t_plot *x,
    t_glist *view,
    t_array *array,
    int i,
    t_float baseX,
    t_float baseY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    t_gobj *y = NULL;
                        
    for (y = view->gl_graphics; y; y = y->g_next) {
    
        t_parentwidgetbehavior *behavior = class_getParentWidget (pd_class (y));
        
        if (behavior) {
        
            int x1, y1, x2, y2;
            
            t_gpointer gp = GPOINTER_INIT;
            
            gpointer_setAsWord (&gp, array, array_getElementAtIndex (array, i));
            
            (*behavior->w_fnParentGetRectangle) (y, &gp, baseX, baseY, &x1, &y1, &x2, &y2);
            
            gpointer_unset (&gp);
            
            *a = PD_MIN (*a, x1);
            *b = PD_MIN (*b, y1);
            *c = PD_MAX (*c, x2);
            *d = PD_MAX (*d, y2);
        }
    }
}

static void plot_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    t_plot *x = (t_plot *)z;

    t_template *template = gpointer_getTemplate (gp);
    t_word *w = gpointer_getData (gp);
    t_glist *glist = gpointer_getView (gp);
    
    int x1, y1, x2, y2;
        
    rectangle_initialize (&x1, &y1, &x2, &y2);
    
    if (garray_isSingle (glist)) { rectangle_setEverything (&x1, &y1, &x2, &y2); }
    else {
    //
    t_array *array = NULL;
    t_float width;
    t_float positionX;
    t_float positionY;
    t_float incrementX;
    t_float style;
    int visible;
    
    if (!plot_fetchScalarFields (x, w, template,
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
    
    t_glist *view = template_getFirstInstanceView (array_getTemplate (array));
    
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
        pixelW = PD_MAX (pixelW, width - 1.0);

        x1 = PD_MIN (x1, pixelX);
        x2 = PD_MAX (x2, pixelX);
        y1 = PD_MIN (y1, pixelY - pixelW);
        y2 = PD_MAX (y2, pixelY + pixelW);
        
        if (view) {
            plot_behaviorGetRectangleRecursive (x, view, array, i, valueX, valueY, &x1, &y1, &x2, &y2);
        }
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
    t_float width, 
    int style, 
    t_symbol *color)
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

    if (fieldX || i == numberOfElements - 1 || pixelX != nextPixelX) {

        sys_vGui (".x%lx.c create rectangle %d %d %d %d"
                        " -width %d"
                        " -fill %s"
                        " -outline %s"
                        " -tags %lxPLOT\n",
                        canvas_getView (glist),
                        (fieldX == NULL) ? pixelX : pixelX - 1,
                        (int)canvas_valueToPixelY (glist, minimumValueY),
                        (fieldX == NULL) ? nextPixelX : pixelX + 1,
                        (int)canvas_valueToPixelY (glist, maximumValueY),
                        (int)PD_MAX (0, width - 1.0),
                        color->s_name,
                        color->s_name,
                        w);
    
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
    t_heapstring *t = heapstring_new (0);
    
    heapstring_addSprintf (t,       ".x%lx.c create polygon", canvas_getView (glist));
  
    if (elementsDrawn == 1) {
        heapstring_addSprintf (t,   " %d %d", coordinatesX[0] - 1, coordinatesL[0]);
        heapstring_addSprintf (t,   " %d %d", coordinatesX[0] + 1, coordinatesL[0]);
    } else {
    //
    for (i = 0; i < elementsDrawn; i++) {
        heapstring_addSprintf (t,   " %d %d", coordinatesX[i], coordinatesL[i]);
    }
    //
    }

    if (elementsDrawn == 1) {
        heapstring_addSprintf (t,   " %d %d", coordinatesX[0] + 1, coordinatesH[0]);
        heapstring_addSprintf (t,   " %d %d", coordinatesX[0] - 1, coordinatesH[0]);
    } else { 
    //
    for (i = elementsDrawn - 1; i >= 0; i--) {
        heapstring_addSprintf (t,   " %d %d", coordinatesX[i], coordinatesH[i]);
    }
    //
    }
    
    heapstring_addSprintf (t,       " -fill %s", color->s_name);
    heapstring_addSprintf (t,       " -outline %s", color->s_name);

    if (style == PLOT_CURVES) { heapstring_addSprintf (t, " -width 1 -smooth 1 -tags %lxPLOT\n", w); }
    else { 
        heapstring_addSprintf (t,   " -width 1 -tags %lxPLOT\n", w);
    }
    
    sys_gui (heapstring_getRaw (t));

    heapstring_free (t);
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
    t_heapstring *t = heapstring_new (0);
    
    int numberOfElements = array_getSize (array);
    int elementsDrawn = 0;
    int i;
    
    int pixelY, pixelX, previousPixelX = -PD_INT_MAX;
        
    heapstring_addSprintf (t, ".x%lx.c create line", canvas_getView (glist));
    
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
        heapstring_addSprintf (t, " %d %d", pixelX, pixelY);
        elementsDrawn++;
        previousPixelX = pixelX;
    }
    //
    }
    
    if (elementsDrawn > 1) {    /* Tk line requires at least two points. */

        heapstring_addSprintf (t,     " -width %d", (int)(PD_MAX (0, width - 1.0)));
        heapstring_addSprintf (t,     " -fill %s", color->s_name);

        if (style == PLOT_CURVES) { heapstring_addSprintf (t, " -smooth 1 -tags %lxPLOT\n", w); }
        else {
            heapstring_addSprintf (t, " -tags %lxPLOT\n", w);
        }

        sys_gui (heapstring_getRaw (t));
    }
    
    heapstring_free (t);
    
    if (elementsDrawn == 1) {

        plot_behaviorVisibilityChangedDrawPoint (x,
            glist,
            w,
            array,
            fieldX, 
            fieldY,
            fieldW,
            relativeX,
            relativeY,
            incrementX, 
            width,
            style, 
            color);
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
        
            t_gpointer gp = GPOINTER_INIT;
            
            gpointer_setAsWord (&gp, array, array_getElementAtIndex (array, i));
            
            (*behavior->w_fnParentVisibilityChanged) (y, &gp, valueX, valueY, isVisible);
            
            gpointer_unset (&gp);
        }
    }
    //
    }
    //
    }
}

static void plot_behaviorVisibilityChanged (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_plot *x = (t_plot *)z;

    t_template *template = gpointer_getTemplate (gp);
    t_word *w = gpointer_getData (gp);
    t_glist *glist = gpointer_getView (gp);
    
    t_array *array = NULL;
    t_float width;
    t_float positionX;
    t_float positionY;
    t_float incrementX;
    t_float style;
    int visible;
    
    if (!plot_fetchScalarFields (x, w, template,
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
        
            int color = (int)word_getFloatByDescriptor (w, template, &x->x_colorOutline);
                            
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
                    width,
                    style, 
                    color_toEncodedSymbol (color_withDigits (color)));
                    
            } else {
                
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
            sys_vGui (".x%lx.c delete %lxPLOT\n", canvas_getView (glist), w); 
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

static int plot_behaviorClickedRegularMatch (t_plot *x,
    t_symbol *fieldX,
    t_symbol *fieldY,
    t_symbol *fieldW,
    int bestIndex,
    int bestDeltaY,
    int bestDeltaL,
    int bestDeltaH,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    if (bestIndex >= 0) {
    //
    int i = bestIndex;
    
    plot_thickness = PLOT_THICKNESS_NONE;
    plot_direction = 1.0;
    
    if (fieldW) {
        if (bestDeltaY < (PLOT_HANDLE_SIZE / 2)) { }
        else if ((bestDeltaY < bestDeltaL) && (bestDeltaY < bestDeltaH)) { } 
        else if (bestDeltaH < bestDeltaL) { plot_thickness = PLOT_THICKNESS_DOWN; }
        else {
            plot_thickness = PLOT_THICKNESS_UP;
        }
    }

    if (clicked) {
    //
    plot_cumulativeX          = 0.0;
    plot_cumulativeY          = 0.0;
    plot_startX               = i;
    plot_previousX            = i;
    plot_fieldDescriptorX     = NULL;
    plot_fieldDescriptorY     = NULL;
    
    if (fieldX) {
        plot_fieldDescriptorX = &x->x_fieldX;
        plot_cumulativeX      = word_getFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, i),
                                        array_getTemplate (plot_array),
                                        &x->x_fieldX);
    }
    
    if (plot_thickness) {
        plot_fieldDescriptorY = &x->x_fieldW;
        plot_cumulativeY      = word_getFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, i),
                                        array_getTemplate (plot_array),
                                        &x->x_fieldW);

    } else if (fieldY) {
        plot_fieldDescriptorY = &x->x_fieldY;
        plot_cumulativeY      = word_getFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, i),
                                        array_getTemplate (plot_array),
                                        &x->x_fieldY);
    }

    if (plot_thickness == PLOT_THICKNESS_UP   && plot_cumulativeY >= 0.0) { plot_direction = -1.0; }
    if (plot_thickness == PLOT_THICKNESS_DOWN && plot_cumulativeY <= 0.0) { plot_direction = -1.0; }
        
    canvas_setMotionFunction (gpointer_getView (&plot_gpointer), NULL, (t_motionfn)plot_motion, a, b);
    //
    }
    
    return (plot_thickness ? CURSOR_THICKEN : CURSOR_CLICK);
    //
    }
    
    return 0;
}

static int plot_behaviorClickedRegular (t_plot *x,
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
    
    if (!plot_fetchElementFieldNames (x, plot_array, &fieldX, &fieldY, &fieldW)) {
    //
    int best = PD_INT_MAX;
    int bestDeltaY;
    int bestDeltaL;
    int bestDeltaH;
    
    int bestIndex = -1;
        
    int i, k = plot_getStep (plot_array);
    
    for (i = 0; i < array_getSize (plot_array); i += k) {
    //
    t_float valueX;
    t_float valueY;
    t_float valueW;
    t_float pixelX;
    t_float pixelY;
    t_float pixelW;
    int deltaY;
    int deltaL;
    int deltaH;
    int k = 0;
    
    plot_getCoordinates (x, plot_array, fieldX, fieldY, fieldW,
        i,
        plot_relativeX,
        plot_relativeY,
        plot_incrementX,
        &valueX,
        &valueY,
        &valueW);
    
    pixelX = canvas_valueToPixelX (gpointer_getView (&plot_gpointer), valueX);
    pixelY = canvas_valueToPixelY (gpointer_getView (&plot_gpointer), valueY);
    pixelW = canvas_valueToPixelY (gpointer_getView (&plot_gpointer), valueY + valueW) - pixelY;
    pixelW = PD_ABS (pixelW);
    pixelW = PD_MAX (pixelW, plot_width - 1.0);
    
    deltaY = (int)math_euclideanDistance (pixelX, pixelY, a, b);
    deltaL = (int)math_euclideanDistance (pixelX, pixelY - pixelW, a, b);
    deltaH = (int)math_euclideanDistance (pixelX, pixelY + pixelW, a, b);
    
    if (deltaY < best) { best = deltaY; k = 1; }
    if (deltaL < best) { best = deltaL; k = 1; }
    if (deltaH < best) { best = deltaH; k = 1; }
    
    if (k) { bestIndex = i; bestDeltaY = deltaY; bestDeltaL = deltaL; bestDeltaH = deltaH; }
    //
    }
    
    if (best <= PLOT_HANDLE_SIZE) {

        return (plot_behaviorClickedRegularMatch (x,
                    fieldX,
                    fieldY, 
                    fieldW,
                    bestIndex,
                    bestDeltaY,
                    bestDeltaL,
                    bestDeltaH,
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

static int plot_behaviorClickedSingle (t_plot *x,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    t_float valueX = canvas_pixelToValueX (gpointer_getView (&plot_gpointer), a);
    t_float valueY = canvas_pixelToValueY (gpointer_getView (&plot_gpointer), b);
    
    PD_ASSERT (plot_relativeX  == 0.0);
    PD_ASSERT (plot_relativeY  == 0.0);
    PD_ASSERT (plot_incrementX == 1.0);
        
    int i = ((int)plot_style == PLOT_POINTS) ? valueX : valueX + 0.5;
    
    i = PD_CLAMP (i, 0, array_getSize (plot_array) - 1);
    
    plot_thickness          = PLOT_THICKNESS_NONE;
    plot_direction          = 1.0;
    plot_cumulativeX        = 0.0;
    plot_cumulativeY        = valueY;
    plot_startX             = i;
    plot_previousX          = i;
    plot_fieldDescriptorX   = NULL;
    plot_fieldDescriptorY   = &x->x_fieldY;

    if (clicked) {
    //
    word_setFloatByDescriptorAsPosition (array_getElementAtIndex (plot_array, i),
        array_getTemplate (plot_array),
        &x->x_fieldY,
        valueY);
            
    canvas_setMotionFunction (gpointer_getView (&plot_gpointer), NULL, (t_motionfn)plot_motion, a, b);

    gpointer_redraw (&plot_gpointer);
    //
    }
    
    return 1;
}

static int plot_behaviorClicked (t_gobj *z,
    t_gpointer *gp,
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
    
    t_template *template = gpointer_getTemplate (gp);
    t_word *w = gpointer_getData (gp);
    t_glist *glist = gpointer_getView (gp);
    
    t_array *array = NULL;
    t_float width;
    t_float positionX;
    t_float positionY;
    t_float incrementX;
    t_float style;
    int visible;
    
    if (!plot_fetchScalarFields (x, w, template,
            &array,
            &width,
            &positionX,
            &positionY,
            &incrementX,
            &style,
            &visible) && (visible != 0)) {
    //
    plot_stepX      = canvas_valueForOnePixelX (glist);
    plot_stepY      = canvas_valueForOnePixelY (glist);
    plot_relativeX  = baseX + positionX;
    plot_relativeY  = baseY + positionY;
    plot_incrementX = incrementX;
    plot_width      = width;
    plot_style      = style;
    plot_array      = array;
    
    gpointer_setByCopy (gp, &plot_gpointer);
    
    if (garray_isSingle (glist)) { return (plot_behaviorClickedSingle (x, a, b, shift, alt, dbl, clicked)); } 
    else {
        return (plot_behaviorClickedRegular (x, a, b, shift, alt, dbl, clicked));
    }
    //
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

void plot_initialize (void)
{
}

void plot_release (void)
{
    if (gpointer_isSet (&plot_gpointer)) { gpointer_unset (&plot_gpointer); }
}

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
