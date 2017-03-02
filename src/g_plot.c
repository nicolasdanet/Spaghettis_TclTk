
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *plot_class;                            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float              plot_cumulativeX;           /* Static. */
static t_float              plot_cumulativeY;           /* Static. */
static t_float              plot_stepX;                 /* Static. */
static t_float              plot_stepY;                 /* Static. */
static t_float              plot_relativeX;             /* Static. */
static t_float              plot_relativeY;             /* Static. */
static t_float              plot_incrementX;            /* Static. */
static t_float              plot_width;                 /* Static. */
static t_float              plot_style;                 /* Static. */
static int                  plot_startX;                /* Static. */
static int                  plot_previousX;             /* Static. */
static int                  plot_thickness;             /* Static. */
static t_float              plot_direction;             /* Static. */
static t_gpointer           plot_gpointer;              /* Static. */
static t_gpointer           plot_check;                 /* Static. */

static t_fielddescriptor    *plot_fieldDescriptorX;     /* Static. */
static t_fielddescriptor    *plot_fieldDescriptorY;     /* Static. */
static t_fielddescriptor    *plot_fieldArray;           /* Static. */

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

static void plot_motion                    (void *, t_float, t_float, t_float);
static void plot_behaviorGetRectangle      (t_gobj *, t_gpointer *, t_float, t_float, t_rectangle *);
static void plot_behaviorVisibilityChanged (t_gobj *, t_gpointer *, t_float, t_float, int);
static int  plot_behaviorMouse             (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_painterwidgetbehavior plot_widgetBehavior =
    {
        plot_behaviorGetRectangle,
        plot_behaviorVisibilityChanged,
        plot_behaviorMouse,
    };
    
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

typedef struct _plotproperties {
    t_array     *p_array;
    t_symbol    *p_fieldX;
    t_symbol    *p_fieldY;
    t_symbol    *p_fieldW;
    t_float     p_width;
    t_float     p_positionX;
    t_float     p_positionY;
    t_float     p_incrementX;
    t_float     p_style;
    int         p_step;
    int         p_visible;
    } t_plotproperties;

typedef struct _plotcoordinates {
    t_float     p_x;
    t_float     p_y;
    t_float     p_w;
    } t_plotcoordinates;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void plot_release (void)
{
    gpointer_unset (&plot_check);
    gpointer_unset (&plot_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error plot_fetchProperties (t_plot *x, t_gpointer *gp, t_plotproperties *p)
{
    if (field_isArray (&x->x_array)) {
    //
    t_symbol *s = field_getVariableName (&x->x_array);
    
    if (gpointer_hasField (gp, s) && gpointer_fieldIsArrayAndValid (gp, s)) {
    //
    t_template *t = NULL;
    int size;
    
    p->p_array      = gpointer_getArray (gp, s);
    p->p_fieldX     = sym_x;
    p->p_fieldY     = sym_y;
    p->p_fieldW     = sym_w;
    p->p_width      = gpointer_getFloatByDescriptor (gp, &x->x_width);
    p->p_positionX  = gpointer_getFloatByDescriptor (gp, &x->x_positionX);
    p->p_positionY  = gpointer_getFloatByDescriptor (gp, &x->x_positionY);
    p->p_incrementX = gpointer_getFloatByDescriptor (gp, &x->x_incrementX);
    p->p_style      = gpointer_getFloatByDescriptor (gp, &x->x_style);
    p->p_visible    = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    size = array_getSize (p->p_array); 
    
    p->p_step = (size <= 1000 ? 1 : (int)sqrt ((double)size));
    
    if (field_isVariable (&x->x_fieldX)) { p->p_fieldX = field_getVariableName (&x->x_fieldX); }
    if (field_isVariable (&x->x_fieldY)) { p->p_fieldY = field_getVariableName (&x->x_fieldY); }
    if (field_isVariable (&x->x_fieldW)) { p->p_fieldW = field_getVariableName (&x->x_fieldW); }
    
    t = array_getTemplate (p->p_array);
    
    if (!template_fieldIsFloat (t, p->p_fieldX)) { p->p_fieldX = NULL; }
    if (!template_fieldIsFloat (t, p->p_fieldY)) { p->p_fieldY = NULL; }
    if (!template_fieldIsFloat (t, p->p_fieldW)) { p->p_fieldW = NULL; }
    
    return PD_ERROR_NONE;
    //
    }
    //
    }
    
    error_unspecified (sym_plot, sym_array);
    
    return PD_ERROR;
}

t_float plot_getRelativeX (t_plotproperties *p, t_float baseX)
{
    return baseX + p->p_positionX;
}

t_float plot_getRelativeY (t_plotproperties *p, t_float baseY)
{
    return baseY + p->p_positionY;
}

static void plot_getCoordinates (t_plotproperties *p,
    t_float relativeX,
    t_float relativeY,
    int i,
    t_plotcoordinates *c)
{
    t_float valueX;
    t_float valueY;
    t_float valueW;

    int size = array_getSize (p->p_array);
    
    if (p->p_fieldX) { 
        valueX = (i < size) ? array_getFloatAtIndex (p->p_array, i, p->p_fieldX) : (t_float)0.0; 
    } else { 
        valueX = i * p->p_incrementX;
    }
    
    if (p->p_fieldY) {
        valueY = (i < size) ? array_getFloatAtIndex (p->p_array, i, p->p_fieldY) : (t_float)0.0; 
    } else { 
        valueY = (t_float)0.0;
    }
    
    if (p->p_fieldW) { 
        valueW = (i < size) ? array_getFloatAtIndex (p->p_array, i, p->p_fieldW) : (t_float)0.0; 
    } else {
        valueW = (t_float)0.0;
    }
    
    c->p_x = relativeX + valueX;
    c->p_y = relativeY + valueY;
    c->p_w = valueW;
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
    paint_erase();
    field_setAsFloatConstant (&x->x_isVisible, (t_float)k);
    paint_draw();
    //
    }
    //
    } else { error_unexpected (sym_plot, &s_float); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_motionHorizontalVertical (t_array *array)
{
    if (plot_fieldDescriptorX) {
        array_setFloatAtIndexByDescriptor (array, 
            plot_startX, 
            plot_fieldDescriptorX,
            plot_cumulativeX);
    }
    
    if (plot_fieldDescriptorY) {
        array_setFloatAtIndexByDescriptor (array, 
            plot_startX,
            plot_fieldDescriptorY,
            plot_cumulativeY);
    }
}

static void plot_motionVertical (t_array *array)
{
    t_float distanceX   = (t_float)((plot_cumulativeX / plot_incrementX) + 0.5);
    int currentX        = PD_CLAMP ((int)(plot_startX + distanceX), 0, array_getSize (array) - 1);
    
    int i = PD_MIN (currentX, plot_previousX);
    int j = PD_MAX (currentX, plot_previousX);
    int n = j - i;
    int back = currentX < plot_previousX;
        
    if (n > 0) {    /* Distribute change linearly between samples. */
    //
    t_float startY = array_getFloatAtIndexByDescriptor (array, back ? j : i, plot_fieldDescriptorY);
    t_float stepY  = (plot_cumulativeY - startY) / n;
    
    int k = back ? n : 0;
    
    for (; i <= j; i++) {
        array_setFloatAtIndexByDescriptor (array, i, plot_fieldDescriptorY, startY + (stepY * k));
        if (back) { k--; } else { k++; }
    }
    //
    } else {
        array_setFloatAtIndexByDescriptor (array, i, plot_fieldDescriptorY, plot_cumulativeY);
    }
    
    plot_previousX = currentX;
}

static void plot_motion (void *dummy, t_float deltaX, t_float deltaY, t_float modifier)
{
    if (gpointer_isValid (&plot_gpointer) && gpointer_isValid (&plot_check)) {
    //
    if (field_isArray (plot_fieldArray)) {
    //
    t_symbol *s = field_getVariableName (plot_fieldArray);
    
    if (gpointer_hasField (&plot_gpointer, s) && gpointer_fieldIsArrayAndValid (&plot_gpointer, s)) {
    //
    t_array *array = gpointer_getArray (&plot_gpointer, s);
    
    plot_cumulativeX += deltaX * plot_stepX;
    plot_cumulativeY += deltaY * plot_stepY * (plot_thickness ? plot_direction : (t_float)1.0);
    
    if (plot_fieldDescriptorX)      { plot_motionHorizontalVertical (array); }
    else if (plot_fieldDescriptorY) { plot_motionVertical (array); }
    
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
    //
    }
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
    t_rectangle *r)
{
    t_gobj *y = NULL;
                        
    for (y = view->gl_graphics; y; y = y->g_next) {
    
        t_painterwidgetbehavior *behavior = class_getPainterWidget (pd_class (y));
        
        if (behavior) {
        
            t_rectangle t;
            t_gpointer gp; GPOINTER_INIT (&gp);
            
            gpointer_setAsWord (&gp, array, array_getElementAtIndex (array, i));
            (*behavior->w_fnPainterGetRectangle) (y, &gp, baseX, baseY, &t);
            gpointer_unset (&gp);
            
            rectangle_boundingBoxAddRectangle (r, &t);
        }
    }
}

static void plot_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    t_rectangle *r)
{
    t_plot *x = (t_plot *)z;

    t_glist *glist = gpointer_getView (gp);
        
    rectangle_setNothing (r);
    
    if (garray_isSingle (glist)) { rectangle_setEverything (r); }
    else {
    //
    t_plotproperties p;
    
    if (!plot_fetchProperties (x, gp, &p) && (p.p_visible != 0)) {
    //
    int i;
    
    t_glist *view = template_getInstanceView (array_getTemplate (p.p_array));
    
    for (i = 0; i < array_getSize (p.p_array); i += p.p_step) {

        t_plotcoordinates c;
        
        t_float pixelX;
        t_float pixelY;
        t_float pixelW;
        
        plot_getCoordinates (&p, plot_getRelativeX (&p, baseX), plot_getRelativeY (&p, baseY), i, &c);
        
        pixelX = canvas_valueToPixelX (glist, c.p_x);
        pixelY = canvas_valueToPixelY (glist, c.p_y);
        pixelW = canvas_valueToPixelY (glist, c.p_y + c.p_w) - pixelY;
        
        pixelW = (t_float)PD_ABS (pixelW);
        pixelW = (t_float)PD_MAX (pixelW, p.p_width - 1.0);
        
        rectangle_boundingBoxAddPoint (r, pixelX, pixelY - pixelW);
        rectangle_boundingBoxAddPoint (r, pixelX, pixelY + pixelW);
        
        if (view) {
            plot_behaviorGetRectangleRecursive (x, view, p.p_array, i, c.p_x, c.p_y, r);
        }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void plot_behaviorVisibilityChangedDrawPoint (t_plot *x,
    t_plotproperties *p,
    t_float  relativeX,
    t_float  relativeY,
    t_glist  *glist,
    t_word   *w,
    t_symbol *color)
{
    int numberOfElements  = array_getSize (p->p_array);
    t_float minimumValueY = (t_float)PLOT_MAX;
    t_float maximumValueY = (t_float)-PLOT_MAX;
    int i;
    
    for (i = 0; i < numberOfElements; i++) {
    //
    t_plotcoordinates c;
    
    int pixelX, nextPixelX;
    
    plot_getCoordinates (p, relativeX, relativeY, i, &c);
    
    pixelX = (int)canvas_valueToPixelX (glist, c.p_x);
    
    minimumValueY = PD_MIN (minimumValueY, PLOT_CLIP (c.p_y));
    maximumValueY = PD_MAX (maximumValueY, PLOT_CLIP (c.p_y));
    
    plot_getCoordinates (p, relativeX, relativeY, i + 1, &c);
        
    nextPixelX = (int)canvas_valueToPixelX (glist, c.p_x);

    if (p->p_fieldX || i == numberOfElements - 1 || pixelX != nextPixelX) {

        sys_vGui (".x%lx.c create rectangle %d %d %d %d"
                        " -width %d"
                        " -fill %s"
                        " -outline %s"
                        " -tags %lxPLOT\n",
                        canvas_getView (glist),
                        (p->p_fieldX == NULL) ? pixelX : pixelX - 1,
                        (int)canvas_valueToPixelY (glist, minimumValueY),
                        (p->p_fieldX == NULL) ? nextPixelX : pixelX + 1,
                        (int)canvas_valueToPixelY (glist, maximumValueY),
                        (int)PD_MAX (0, p->p_width - 1.0),
                        color->s_name,
                        color->s_name,
                        w);
    
        minimumValueY = (t_float)PLOT_MAX;
        maximumValueY = (t_float)-PLOT_MAX;
    }
    //
    }
}

static void plot_behaviorVisibilityChangedDrawPolygonFill (t_plot *x,
    t_plotproperties *p,
    t_float  relativeX,
    t_float  relativeY,
    t_glist  *glist,
    t_word   *w,
    t_symbol *color)
{
    int numberOfElements = array_getSize (p->p_array);
    int coordinatesX[PLOT_MAXIMUM_DRAWN] = { 0 };     
    int coordinatesL[PLOT_MAXIMUM_DRAWN] = { 0 };
    int coordinatesH[PLOT_MAXIMUM_DRAWN] = { 0 };
    int elementsDrawn = 0;
    int i;
    
    int pixelX, previousPixelX = -PD_INT_MAX;
        
    for (i = 0; i < numberOfElements; i++) {
    //
    t_plotcoordinates c;
    
    plot_getCoordinates (p, relativeX, relativeY, i, &c);
    
    c.p_y = PLOT_CLIP (c.p_y);
    c.p_w = PLOT_CLIP (c.p_w);
    pixelX = (int)canvas_valueToPixelX (glist, c.p_x);
    
    if (p->p_fieldX || pixelX != previousPixelX) {
    //
    t_float pixelY = canvas_valueToPixelY (glist, c.p_y);
    t_float pixelW = canvas_valueToPixelY (glist, c.p_y + c.p_w) - pixelY;
    
    pixelW = (t_float)PD_ABS (pixelW);
    pixelW = (t_float)PD_MAX (pixelW, p->p_width - 1.0);

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

    if (p->p_style == PLOT_CURVES) { heapstring_addSprintf (t, " -width 1 -smooth 1 -tags %lxPLOT\n", w); }
    else { 
        heapstring_addSprintf (t,   " -width 1 -tags %lxPLOT\n", w);
    }
    
    sys_gui (heapstring_getRaw (t));

    heapstring_free (t);
    //
    }
}

static void plot_behaviorVisibilityChangedDrawPolygonSegment (t_plot *x,
    t_plotproperties *p,
    t_float  relativeX,
    t_float  relativeY,
    t_glist  *glist,
    t_word   *w,
    t_symbol *color)
{
    t_heapstring *t = heapstring_new (0);
    
    int numberOfElements = array_getSize (p->p_array);
    int elementsDrawn = 0;
    int i;
    
    int pixelY, pixelX, previousPixelX = -PD_INT_MAX;
        
    heapstring_addSprintf (t, ".x%lx.c create line", canvas_getView (glist));
    
    for (i = 0; i < numberOfElements; i++) {
    //
    t_plotcoordinates c;

    plot_getCoordinates (p, relativeX, relativeY, i, &c);
    
    c.p_y = PLOT_CLIP (c.p_y);
    pixelX = (int)canvas_valueToPixelX (glist, c.p_x);
    
    if (p->p_fieldX || pixelX != previousPixelX) {
        pixelY = (int)canvas_valueToPixelY (glist, c.p_y);
        heapstring_addSprintf (t, " %d %d", pixelX, pixelY);
        elementsDrawn++;
        previousPixelX = pixelX;
    }
    //
    }
    
    if (elementsDrawn > 1) {    /* Tk line requires at least two points. */

        heapstring_addSprintf (t, " -width %d", (int)(PD_MAX (0, p->p_width - 1.0)));
        heapstring_addSprintf (t, " -fill %s", color->s_name);

        if (p->p_style == PLOT_CURVES) { heapstring_addSprintf (t, " -smooth 1 -tags %lxPLOT\n", w); }
        else {
        //
        heapstring_addSprintf (t, " -tags %lxPLOT\n", w);
        //
        }

        sys_gui (heapstring_getRaw (t));
    }
    
    heapstring_free (t);
    
    if (elementsDrawn == 1) {

        plot_behaviorVisibilityChangedDrawPoint (x, p,
            relativeX,
            relativeY,
            glist,
            w,
            color);
    }
}

static void plot_behaviorVisibilityChangedRecursive (t_plot *x,
    t_plotproperties *p,
    t_float relativeX,
    t_float relativeY, 
    int isVisible)
{
    t_glist *view = template_getInstanceView (array_getTemplate (p->p_array));
    
    if (view) {
    //
    int numberOfElements = array_getSize (p->p_array);
    int i;
        
    for (i = 0; i < numberOfElements; i++) {
    //
    t_gobj *y = NULL;
    
    t_plotcoordinates c;

    plot_getCoordinates (p, relativeX, relativeY, i, &c);
    
    for (y = view->gl_graphics; y; y = y->g_next) {
    
        t_painterwidgetbehavior *behavior = class_getPainterWidget (pd_class (y));
        
        if (behavior) {
        
            t_gpointer gp; GPOINTER_INIT (&gp);
            
            gpointer_setAsWord (&gp, p->p_array, array_getElementAtIndex (p->p_array, i));
            
            (*behavior->w_fnPainterVisibilityChanged) (y, &gp, c.p_x, c.p_y, isVisible);
            
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
    t_word *w = gpointer_getElement (gp);
    t_glist *glist = gpointer_getView (gp);
    
    t_plotproperties p;
    
    if (!plot_fetchProperties (x, gp, &p)) {
    //
    if (!isVisible || p.p_visible) {
    //
    t_float relativeX = plot_getRelativeX (&p, baseX);
    t_float relativeY = plot_getRelativeY (&p, baseY);
    
    if (isVisible) {
    
        int color = (int)gpointer_getFloatByDescriptor (gp, &x->x_colorOutline);
                        
        if (p.p_style == PLOT_POINTS) { 
        
            plot_behaviorVisibilityChangedDrawPoint (x, &p,
                relativeX,
                relativeY,
                glist,
                w,
                color_toEncodedSymbol (color_withDigits (color)));
                
        } else {
            
            if (p.p_fieldW) {
            
                plot_behaviorVisibilityChangedDrawPolygonFill (x, &p,
                    relativeX,
                    relativeY,
                    glist,
                    w,
                    color_toEncodedSymbol (color_withDigits (color)));
                    
            } else {
            
                plot_behaviorVisibilityChangedDrawPolygonSegment (x, &p, 
                    relativeX,
                    relativeY,
                    glist,
                    w,
                    color_toEncodedSymbol (color_withDigits (color)));
            }
        }
        
    } else {
        sys_vGui (".x%lx.c delete %lxPLOT\n", canvas_getView (glist), w); 
    }
    
    plot_behaviorVisibilityChangedRecursive (x, &p, relativeX, relativeY, isVisible);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int plot_behaviorMouseRegularMatch (t_plot *x,
    t_plotproperties *p,
    int bestIndex,
    int bestDeltaY,
    int bestDeltaL,
    int bestDeltaH,
    t_mouse *m)
{
    if (bestIndex >= 0) {
    //
    int i = bestIndex;
    
    plot_thickness = PLOT_THICKNESS_NONE;
    plot_direction = (t_float)1.0;
    
    if (p->p_fieldW) {
        if (bestDeltaY < (PLOT_HANDLE_SIZE / 2)) { }
        else if ((bestDeltaY < bestDeltaL) && (bestDeltaY < bestDeltaH)) { } 
        else if (bestDeltaH < bestDeltaL) { plot_thickness = PLOT_THICKNESS_DOWN; }
        else {
            plot_thickness = PLOT_THICKNESS_UP;
        }
    }

    if (m->m_clicked) {
    //
    int a = m->m_x;
    int b = m->m_y;
    
    plot_cumulativeX          = (t_float)0.0;
    plot_cumulativeY          = (t_float)0.0;
    plot_startX               = i;
    plot_previousX            = i;
    plot_fieldDescriptorX     = NULL;
    plot_fieldDescriptorY     = NULL;
    
    if (p->p_fieldX) {
        plot_fieldDescriptorX = &x->x_fieldX;
        plot_cumulativeX      = array_getFloatAtIndexByDescriptor (p->p_array, i, &x->x_fieldX);
    }
    
    if (plot_thickness) {
        plot_fieldDescriptorY = &x->x_fieldW;
        plot_cumulativeY      = array_getFloatAtIndexByDescriptor (p->p_array, i, &x->x_fieldW);

    } else if (p->p_fieldY) {
        plot_fieldDescriptorY = &x->x_fieldY;
        plot_cumulativeY      = array_getFloatAtIndexByDescriptor (p->p_array, i, &x->x_fieldY);
    }

    if (plot_thickness == PLOT_THICKNESS_UP   && plot_cumulativeY >= 0.0) { plot_direction = (t_float)-1.0; }
    if (plot_thickness == PLOT_THICKNESS_DOWN && plot_cumulativeY <= 0.0) { plot_direction = (t_float)-1.0; }
        
    canvas_setMotionFunction (gpointer_getView (&plot_gpointer), NULL, (t_motionfn)plot_motion, a, b);
    //
    }
    
    return (plot_thickness ? CURSOR_THICKEN : CURSOR_CLICK);
    //
    }
    
    return 0;
}

static int plot_behaviorMouseRegular (t_plot *x, t_plotproperties *p, t_mouse *m)
{
    int a = m->m_x;
    int b = m->m_y;
    
    int best = PD_INT_MAX;
    int bestDeltaY = 0;
    int bestDeltaL = 0;
    int bestDeltaH = 0;
    int bestIndex = -1;
        
    int i;
    
    for (i = 0; i < array_getSize (p->p_array); i += p->p_step) {
    //
    t_plotcoordinates c;
    t_float pixelX;
    t_float pixelY;
    t_float pixelW;
    int deltaY;
    int deltaL;
    int deltaH;
    int k = 0;
    
    plot_getCoordinates (p, plot_relativeX, plot_relativeY, i, &c);
    
    pixelX = canvas_valueToPixelX (gpointer_getView (&plot_gpointer), c.p_x);
    pixelY = canvas_valueToPixelY (gpointer_getView (&plot_gpointer), c.p_y);
    pixelW = canvas_valueToPixelY (gpointer_getView (&plot_gpointer), c.p_y + c.p_w) - pixelY;
    pixelW = (t_float)PD_ABS (pixelW);
    pixelW = (t_float)PD_MAX (pixelW, plot_width - 1.0);
    
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

        return plot_behaviorMouseRegularMatch (x, p,
                    bestIndex,
                    bestDeltaY,
                    bestDeltaL,
                    bestDeltaH,
                    m);
    }
    
    return 0;
}

static int plot_behaviorMouseSingle (t_plot *x, t_plotproperties *p, t_mouse *m)
{
    int a = m->m_x;
    int b = m->m_y;
    
    t_float valueX = canvas_pixelToValueX (gpointer_getView (&plot_gpointer), a);
    t_float valueY = canvas_pixelToValueY (gpointer_getView (&plot_gpointer), b);
    
    PD_ASSERT (plot_relativeX  == 0.0);
    PD_ASSERT (plot_relativeY  == 0.0);
    PD_ASSERT (plot_incrementX == 1.0);
        
    int i = (int)(((int)plot_style == PLOT_POINTS) ? valueX : valueX + 0.5);
    
    i = PD_CLAMP (i, 0, array_getSize (p->p_array) - 1);
    
    plot_thickness          = PLOT_THICKNESS_NONE;
    plot_direction          = (t_float)1.0;
    plot_cumulativeX        = (t_float)0.0;
    plot_cumulativeY        = valueY;
    plot_startX             = i;
    plot_previousX          = i;
    plot_fieldDescriptorX   = NULL;
    plot_fieldDescriptorY   = &x->x_fieldY;

    if (m->m_clicked) {
        array_setFloatAtIndexByDescriptor (p->p_array, i, &x->x_fieldY, valueY);
        canvas_setMotionFunction (gpointer_getView (&plot_gpointer), NULL, (t_motionfn)plot_motion, a, b);
        gpointer_redraw (&plot_gpointer);
    }
    
    return 1;
}

static int plot_behaviorMouse (t_gobj *z, t_gpointer *gp, t_float baseX, t_float baseY, t_mouse *m)
{
    t_plot *x = (t_plot *)z; t_glist *glist = gpointer_getView (gp);
    
    t_plotproperties p;
    
    if (!plot_fetchProperties (x, gp, &p) && (p.p_visible != 0)) {
    //
    plot_stepX      = canvas_valueForOnePixelX (glist);
    plot_stepY      = canvas_valueForOnePixelY (glist);
    plot_relativeX  = plot_getRelativeX (&p, baseX);
    plot_relativeY  = plot_getRelativeY (&p, baseY);
    plot_incrementX = p.p_incrementX;
    plot_width      = p.p_width;
    plot_style      = p.p_style;
    plot_fieldArray = &x->x_array;
        
    gpointer_setByCopy (&plot_gpointer, gp);
    gpointer_setAsWord (&plot_check, p.p_array, array_getElements (p.p_array));
    
    if (garray_isSingle (glist)) { return plot_behaviorMouseSingle (x, &p, m); }
    else {
        return plot_behaviorMouseRegular (x, &p, m);
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *plot_new (t_symbol *s, int argc, t_atom *argv)
{
    t_plot *x = (t_plot *)pd_new (plot_class);
    
    field_setAsFloatConstant (&x->x_array,          (t_float)0.0);      /* Default is invalid. */
    field_setAsFloatConstant (&x->x_colorOutline,   (t_float)0.0);
    field_setAsFloatConstant (&x->x_width,          (t_float)1.0);
    field_setAsFloatConstant (&x->x_positionX,      (t_float)1.0);
    field_setAsFloatConstant (&x->x_positionY,      (t_float)1.0);
    field_setAsFloatConstant (&x->x_incrementX,     (t_float)1.0);
    field_setAsFloatConstant (&x->x_style,          (t_float)PLOT_POLYGONS);
    field_setAsFloatConstant (&x->x_isVisible,      (t_float)1.0);
    
    field_setAsFloatVariable (&x->x_fieldX,         sym_x);
    field_setAsFloatVariable (&x->x_fieldY,         sym_y);
    field_setAsFloatVariable (&x->x_fieldW,         sym_w);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        #if PD_WITH_LEGACY
        
        if (t == sym_curve) { t = sym___dash__curve; }
        
        #endif
        
        if (t == sym___dash__c || t == sym___dash__curve) {
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
            
        } else if (argc > 1 && (t == sym___dash__w || t == sym___dash__width)) {
            field_setAsFloat (&x->x_fieldW, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else { break; }
    }
    
    error__options (s, argc, argv);
    
    if (argc) { field_setAsArray (&x->x_array,          argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_colorOutline,   argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_width,          argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_positionX,      argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_positionY,      argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_incrementX,     argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_style,          argc--, argv++); }

    if (argc) { warning_unusedArguments (s, argc, argv); }
    
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
            
    class_addFloat (c, (t_method)plot_float);
    
    class_setPainterWidgetBehavior (c, &plot_widgetBehavior);
    
    plot_class = c;
}

void plot_destroy (void)
{
    CLASS_FREE (plot_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
