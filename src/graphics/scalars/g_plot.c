
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

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
    int                 x_once;
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
// MARK: -

static void plot_motion                     (void *, t_float, t_float, t_float);
static void plot_behaviorGetRectangle       (t_gobj *, t_gpointer *, t_float, t_float, t_rectangle *);
static void plot_behaviorVisibilityChanged  (t_gobj *, t_gpointer *, t_float, t_float, int);
static int  plot_behaviorMouse              (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_painterbehavior plot_painterBehavior =         /* Shared. */
    {
        plot_behaviorGetRectangle,
        plot_behaviorVisibilityChanged,
        plot_behaviorMouse,
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PLOT_MAXIMUM_DRAWN      256
#define PLOT_HANDLE_SIZE        8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PLOT_THICKNESS_NONE     0
#define PLOT_THICKNESS_UP       1
#define PLOT_THICKNESS_DOWN     2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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

typedef struct _plotpixels {
    t_float     p_pixelX;
    t_float     p_pixelY;
    t_float     p_pixelW;
    t_float     p_x;
    t_float     p_y;
    t_float     p_w;
    } t_plotpixels;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void plot_release (void)
{
    gpointer_unset (&plot_check);
    gpointer_unset (&plot_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    p->p_step = (size <= 1000 ? 1 : (int)sqrt ((double)size));  /* Waveforms for instance. */
    
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
    
    /* Avoid overzealous reporting. */
    
    if (!x->x_once) { x->x_once = 1; error_unspecified (sym_plot, sym_array); }
    
    return PD_ERROR;
}

static t_float plot_getRelativeX (t_plotproperties *p, t_float baseX)
{
    return baseX + p->p_positionX;
}

static t_float plot_getRelativeY (t_plotproperties *p, t_float baseY)
{
    return baseY + p->p_positionY;
}

static void plot_getPixelsAtIndex (t_plotproperties *p,
    t_float relativeX,
    t_float relativeY,
    int i, 
    t_glist *view, 
    t_float maximumWidth, 
    t_plotpixels *c) 
{
    /* Fetch values for the geometry fields. */
    
    {
        t_float valueX;
        t_float valueY;
        t_float valueW;

        int size = array_getSize (p->p_array);
        
        if (p->p_fieldX) { 
            valueX = (i < size) ? array_getFloatAtIndex (p->p_array, i, p->p_fieldX) : 0.0; 
        } else { 
            valueX = i * p->p_incrementX;
        }
        
        if (p->p_fieldY) {
            valueY = (i < size) ? array_getFloatAtIndex (p->p_array, i, p->p_fieldY) : 0.0; 
        } else { 
            valueY = 0.0;
        }
        
        if (p->p_fieldW) { 
            valueW = (i < size) ? array_getFloatAtIndex (p->p_array, i, p->p_fieldW) : 0.0; 
        } else {
            valueW = 0.0;
        }
        
        c->p_x = c->p_pixelX = relativeX + valueX;
        c->p_y = c->p_pixelY = relativeY + valueY;
        c->p_w = c->p_pixelW = valueW;
    }
    
    /* Convert to pixels. */
    
    {
        t_float pixelX = glist_valueToPixelX (view, c->p_pixelX);
        t_float pixelY = glist_valueToPixelY (view, c->p_pixelY);
        t_float pixelW = glist_valueToPixelY (view, c->p_pixelY + c->p_pixelW) - pixelY;

        pixelW = (t_float)PD_ABS (pixelW);
        pixelW = (t_float)PD_MAX (pixelW, maximumWidth - 1.0);
        
        c->p_pixelX = pixelX;
        c->p_pixelY = pixelY;
        c->p_pixelW = pixelW;
    }
}

/* Reduce floating-point precision to avoid artefacts with infinitesimal values. */

int plot_toInteger (t_float f)
{
    return (int)round ((round (f * 100.0)) / 100.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

static void plot_motionHorizontal (t_array *array)
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

        t_array *array = gpointer_getArray (&plot_gpointer, s);
        int isArray = (gpointer_getTemplateIdentifier (&plot_gpointer) == sym___TEMPLATE__float__dash__array);
        
        plot_cumulativeX += deltaX * plot_stepX;
        plot_cumulativeY += deltaY * plot_stepY * (plot_thickness ? plot_direction : (t_float)1.0);
        
        if (!isArray) { gpointer_erase (&plot_gpointer); }
        
        if (plot_fieldDescriptorX)      { plot_motionHorizontal (array); }
        else if (plot_fieldDescriptorY) { plot_motionVertical (array); }
        
        if (!isArray) {

            PD_ASSERT (gpointer_isScalar (&plot_gpointer));
            
            template_notify (gpointer_getTemplate (&plot_gpointer), 
                gpointer_getView (&plot_gpointer), 
                gpointer_getScalar (&plot_gpointer),
                sym_change,
                0,
                NULL);
            
        } else {
        
            t_garray *a = gpointer_getGraphicArray (&plot_gpointer);
            
            PD_ASSERT (a);
            
            if (a) { garray_setNextTag (a); }
        }
        
        if (!isArray) { gpointer_draw (&plot_gpointer); } else { gpointer_redraw (&plot_gpointer); }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void plot_behaviorGetRectangleRecursive (t_plot *x,
    t_glist *view,
    t_array *array,
    int i,
    t_float relativeX,
    t_float relativeY,
    t_rectangle *r)
{
    t_gobj *y = NULL;
                        
    for (y = view->gl_graphics; y; y = y->g_next) {
    
        t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
        
        if (behavior) {
        
            t_rectangle t;
            
            t_gpointer gp; gpointer_init (&gp);
            gpointer_setAsWord (&gp, array, array_getElementAtIndex (array, i));
            (*behavior->w_fnPainterGetRectangle) (y, &gp, relativeX, relativeY, &t);
            gpointer_unset (&gp);
            
            rectangle_addRectangle (r, &t);
        }
    }
}

static void plot_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    t_rectangle *r)
{
    t_plot *x = (t_plot *)z; t_glist *glist = gpointer_getView (gp);
        
    rectangle_setNothing (r);
    
    if (glist_isArray (glist)) { rectangle_setEverything (r); }
    else {
    //
    t_plotproperties p;
    
    if (!plot_fetchProperties (x, gp, &p) && (p.p_visible != 0)) {
    //
    int i;
    
    t_glist *view = template_getInstanceViewIfPainters (array_getTemplate (p.p_array));
    
    for (i = 0; i < array_getSize (p.p_array); i += p.p_step) {

        t_plotpixels c;
    
        plot_getPixelsAtIndex (&p,
            plot_getRelativeX (&p, baseX), 
            plot_getRelativeY (&p, baseY),
            i,
            glist, 
            p.p_width, 
            &c);
        
        rectangle_addPoint (r, c.p_pixelX, c.p_pixelY - c.p_pixelW);
        rectangle_addPoint (r, c.p_pixelX, c.p_pixelY + c.p_pixelW);
        
        if (view) { plot_behaviorGetRectangleRecursive (x, view, p.p_array, i, c.p_x, c.p_y, r); }
    }
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void plot_behaviorVisibilityDrawPoint (t_plot *x,
    t_plotproperties *p,
    t_float  relativeX,
    t_float  relativeY,
    t_glist  *glist,
    t_word   *w,
    t_symbol *color)
{
    int size = array_getSize (p->p_array);
    t_float minY = PD_FLT_MAX;
    t_float maxY = -minY;
    int i;
    
    t_plotpixels here;
    t_plotpixels next;
    
    plot_getPixelsAtIndex (p, relativeX, relativeY, 0, glist, p->p_width, &next);
    
    for (i = 0; i < size; i++) {
    //
    here = next;
    
    plot_getPixelsAtIndex (p, relativeX, relativeY, i + 1, glist, p->p_width, &next);
    
    minY = PD_MIN (minY, here.p_pixelY);
    maxY = PD_MAX (maxY, here.p_pixelY);

    if (p->p_fieldX || i == size - 1 || (int)here.p_pixelX != (int)next.p_pixelX) {

        gui_vAdd ("%s.c create rectangle %d %d %d %d"
                        " -width %d"
                        " -fill %s"
                        " -outline %s"
                        " -tags %lxPLOT\n",
                        glist_getTagAsString (glist_getView (glist)),
                        (p->p_fieldX == NULL) ? (int)here.p_pixelX : (int)here.p_pixelX - 1,
                        plot_toInteger (minY),
                        (p->p_fieldX == NULL) ? (int)next.p_pixelX : (int)here.p_pixelX + 1,
                        plot_toInteger (maxY),
                        (int)PD_MAX (0, p->p_width - 1.0),
                        color->s_name,
                        color->s_name,
                        w);
    
        minY = (t_float)PD_FLT_MAX;
        maxY = (t_float)-minY;
    }
    //
    }
}

static void plot_behaviorVisibilityDrawPolygonFill (t_plot *x,
    t_plotproperties *p,
    t_float  relativeX,
    t_float  relativeY,
    t_glist  *glist,
    t_word   *w,
    t_symbol *color)
{
    int size = array_getSize (p->p_array);
    int cX[PLOT_MAXIMUM_DRAWN] = { 0 };     
    int cL[PLOT_MAXIMUM_DRAWN] = { 0 };
    int cH[PLOT_MAXIMUM_DRAWN] = { 0 };
    int elementsDrawn = 0;
    int i;
    
    int previous = -PD_INT_MAX;
        
    for (i = 0; i < size; i++) {
    //
    t_plotpixels c;
    
    plot_getPixelsAtIndex (p, relativeX, relativeY, i, glist, p->p_width, &c);
    
    if (p->p_fieldX || (int)c.p_pixelX != previous) {
        cX[elementsDrawn] = (int)(c.p_pixelX);
        cL[elementsDrawn] = plot_toInteger (c.p_pixelY - c.p_pixelW);
        cH[elementsDrawn] = plot_toInteger (c.p_pixelY + c.p_pixelW);
        if (++elementsDrawn >= PLOT_MAXIMUM_DRAWN) { PD_BUG; break; }
    }

    previous = (int)c.p_pixelX;
    //
    }
    
    if (elementsDrawn) {
    //
    t_heapstring *t = heapstring_new (0);
    
    heapstring_addSprintf (t, "%s.c create polygon", glist_getTagAsString (glist_getView (glist)));
  
    /* Tk requires at least three points (i.e. two elements). */
    
    if (elementsDrawn > 1) {
        for (i = 0; i < elementsDrawn; i++)      { heapstring_addSprintf (t, " %d %d", cX[i], cL[i]); }
        for (i = elementsDrawn - 1; i >= 0; i--) { heapstring_addSprintf (t, " %d %d", cX[i], cH[i]); }
        
    } else {
        heapstring_addSprintf (t, " %d %d %d %d", cX[0] - 1, cL[0], cX[0] + 1, cL[0]);
        heapstring_addSprintf (t, " %d %d %d %d", cX[0] + 1, cH[0], cX[0] - 1, cH[0]);
    }

    heapstring_addSprintf (t, " -fill %s", color->s_name);
    heapstring_addSprintf (t, " -outline %s", color->s_name);

    if (p->p_style == PLOT_CURVES) { heapstring_addSprintf (t, " -width 1 -smooth 1 -tags %lxPLOT\n", w); }
    else { 
        heapstring_addSprintf (t, " -width 1 -tags %lxPLOT\n", w);
    }
    
    gui_add (heapstring_getRaw (t));

    heapstring_free (t);
    //
    }
}

static void plot_behaviorVisibilityDrawPolygonSegment (t_plot *x,
    t_plotproperties *p,
    t_float  relativeX,
    t_float  relativeY,
    t_glist  *glist,
    t_word   *w,
    t_symbol *color)
{
    t_heapstring *t = heapstring_new (0);
    
    int size = array_getSize (p->p_array);
    int elementsDrawn = 0;
    int i;
    
    int previous = -PD_INT_MAX;
        
    heapstring_addSprintf (t, "%s.c create line", glist_getTagAsString (glist_getView (glist)));
    
    for (i = 0; i < size; i++) {
    //
    t_plotpixels c;

    plot_getPixelsAtIndex (p, relativeX, relativeY, i, glist, p->p_width, &c);
    
    if (p->p_fieldX || (int)c.p_pixelX != previous) {
        heapstring_addSprintf (t, " %d %d", (int)c.p_pixelX, plot_toInteger (c.p_pixelY));
        elementsDrawn++;
        previous = (int)c.p_pixelX;
    }
    //
    }
    
    /* Tk requires at least three points (i.e. two elements). */
    
    if (elementsDrawn > 1) {
    //
    heapstring_addSprintf (t, " -width %d", (int)(PD_MAX (0, p->p_width - 1.0)));
    heapstring_addSprintf (t, " -fill %s", color->s_name);

    if (p->p_style == PLOT_CURVES) { heapstring_addSprintf (t, " -smooth 1 -tags %lxPLOT\n", w); }
    else {
        heapstring_addSprintf (t, " -tags %lxPLOT\n", w);
    }

    gui_add (heapstring_getRaw (t));
    //
    } else {
        plot_behaviorVisibilityDrawPoint (x, p, relativeX, relativeY, glist, w, color);
    }
    
    heapstring_free (t);
}

static void plot_behaviorVisibilityChangedRecursive (t_plot *x,
    t_plotproperties *p,
    t_float relativeX,
    t_float relativeY, 
    int isVisible)
{
    t_glist *view = template_getInstanceViewIfPainters (array_getTemplate (p->p_array));
    
    if (view) {
    //
    int i;
        
    for (i = 0; i < array_getSize (p->p_array); i++) {
    //
    t_gobj *y = NULL;
    
    t_plotpixels c; plot_getPixelsAtIndex (p, relativeX, relativeY, i, view, p->p_width, &c);
    
    for (y = view->gl_graphics; y; y = y->g_next) {

        t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
        
        if (behavior) {
        
            t_gpointer gp; gpointer_init (&gp);
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
    t_plot *x = (t_plot *)z; t_glist *glist = gpointer_getView (gp);
    
    t_word *w = gpointer_getElement (gp);
    
    t_plotproperties p;
    
    if (!plot_fetchProperties (x, gp, &p)) {
    //
    if (!isVisible || p.p_visible) {
    //
    t_float relativeX = plot_getRelativeX (&p, baseX);
    t_float relativeY = plot_getRelativeY (&p, baseY);
    
    if (!isVisible) { gui_vAdd ("%s.c delete %lxPLOT\n", glist_getTagAsString (glist_getView (glist)), w); }
    else {
    //
    int t = (int)gpointer_getFloatByDescriptor (gp, &x->x_colorOutline);
    t_symbol *color = color_toEncoded (color_withDigits (t));
    
    /* Draw simply as points. */
    /* Draw a filled line with variable thickness. */
    /* Segment can be a straight line or a curved one. */
    
    if (p.p_style == PLOT_POINTS) {     
            plot_behaviorVisibilityDrawPoint (x, &p, relativeX, relativeY, glist, w, color);
    } else {
        if (p.p_fieldW) {               
            plot_behaviorVisibilityDrawPolygonFill (x, &p, relativeX, relativeY, glist, w, color);
        } else {                        
            plot_behaviorVisibilityDrawPolygonSegment (x, &p, relativeX, relativeY, glist, w, color);
        }
    }
    //
    }
    
    /* Recursively change the visibility of the elements. */
    
    plot_behaviorVisibilityChangedRecursive (x, &p, relativeX, relativeY, isVisible);
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void plot_behaviorMouseThickness (t_plot *x, t_plotproperties *p, int deltaY, int deltaL, int deltaH)
{
    plot_thickness = PLOT_THICKNESS_NONE;
    
    /* Does it match the middle, up or down point? */
    
    if (p->p_fieldW) {
        if (deltaY < (PLOT_HANDLE_SIZE / 2)) { }
        else if ((deltaY < deltaL) && (deltaY < deltaH)) { } 
        else if (deltaH < deltaL) { plot_thickness = PLOT_THICKNESS_DOWN; }
        else {
            plot_thickness = PLOT_THICKNESS_UP;
        }
    }
}

static int plot_behaviorMouseMatch (t_plot *x,
    t_plotproperties *p,
    int i,
    int deltaY, 
    int deltaL,
    int deltaH,
    t_mouse *m)
{
    plot_behaviorMouseThickness (x, p, deltaY, deltaL, deltaH);
    
    if (m->m_clicked) {
    //
    plot_direction        = (t_float)1.0;
    plot_cumulativeX      = 0.0;
    plot_cumulativeY      = 0.0;
    plot_startX           = i;
    plot_previousX        = i;
    plot_fieldDescriptorX = NULL;
    plot_fieldDescriptorY = NULL;
    
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
        
    glist_setMotion (gpointer_getView (&plot_gpointer), 
        NULL, 
        (t_motionfn)plot_motion, 
        m->m_x, 
        m->m_y);
    //
    }
    
    return (plot_thickness ? CURSOR_THICKEN : CURSOR_OVER);
}

static int plot_behaviorMouseGrab (t_plot *x, t_plotproperties *p, t_mouse *m)
{
    int d = PD_INT_MAX;
    int dY, dL, dH;
    int deltaY = 0;
    int deltaL = 0;
    int deltaH = 0;
    int best   = 0;
    
    int i;
    
    for (i = 0; i < array_getSize (p->p_array); i += p->p_step) {
    //
    t_plotpixels c;
    
    plot_getPixelsAtIndex (p, 
        plot_relativeX, 
        plot_relativeY, 
        i,
        gpointer_getView (&plot_gpointer),
        plot_width, 
        &c);
    
    /* Compute the distance between the mouse and this point. */
    /* Up and down width is also considered. */
    
    dY = (int)math_euclideanDistance (c.p_pixelX, c.p_pixelY, m->m_x, m->m_y);
    dL = (int)math_euclideanDistance (c.p_pixelX, c.p_pixelY - c.p_pixelW, m->m_x, m->m_y);
    dH = (int)math_euclideanDistance (c.p_pixelX, c.p_pixelY + c.p_pixelW, m->m_x, m->m_y);
    
    /* The nearest point is matched. */
    
    if (dY < d) { d = dY; best = i; deltaY = dY; deltaL = dL; deltaH = dH; }
    if (dL < d) { d = dL; best = i; deltaY = dY; deltaL = dL; deltaH = dH; }
    if (dH < d) { d = dH; best = i; deltaY = dY; deltaL = dL; deltaH = dH; }
    //
    }
    
    if (d > PLOT_HANDLE_SIZE) { return 0; }
    else {
        return plot_behaviorMouseMatch (x, p, best, deltaY, deltaL, deltaH, m); 
    }
}

static int plot_behaviorMouseArray (t_plot *x, t_plotproperties *p, t_mouse *m)
{
    t_float valueX = glist_pixelToValueX (gpointer_getView (&plot_gpointer), m->m_x);
    t_float valueY = glist_pixelToValueY (gpointer_getView (&plot_gpointer), m->m_y);
    
    PD_ASSERT (plot_relativeX  == 0.0);
    PD_ASSERT (plot_relativeY  == 0.0);
    PD_ASSERT (plot_incrementX == 1.0);
        
    int i = (int)(((int)plot_style == PLOT_POINTS) ? valueX : valueX + 0.5);
    
    i = PD_CLAMP (i, 0, array_getSize (p->p_array) - 1);
    
    plot_thickness        = PLOT_THICKNESS_NONE;
    plot_direction        = (t_float)1.0;
    plot_cumulativeX      = 0.0;
    plot_cumulativeY      = valueY;
    plot_startX           = i;
    plot_previousX        = i;
    plot_fieldDescriptorX = NULL;
    plot_fieldDescriptorY = &x->x_fieldY;

    if (m->m_clicked) {

        t_garray *a = gpointer_getGraphicArray (&plot_gpointer);
        
        array_setFloatAtIndexByDescriptor (p->p_array, i, &x->x_fieldY, valueY);
        
        glist_setMotion (gpointer_getView (&plot_gpointer), 
            NULL, 
            (t_motionfn)plot_motion, 
            m->m_x, 
            m->m_y);
        
        PD_ASSERT (a);
        
        if (a) { garray_setNextTag (a); }
        
        gpointer_redraw (&plot_gpointer);
    }
    
    return 1;
}

/* Note that the mouse do NOT recursively affect the element's geometry. */

static int plot_behaviorMouse (t_gobj *z, t_gpointer *gp, t_float baseX, t_float baseY, t_mouse *m)
{
    t_plot *x = (t_plot *)z; t_glist *glist = gpointer_getView (gp);
    
    t_plotproperties p;
    
    if (!plot_fetchProperties (x, gp, &p) && (p.p_visible != 0)) {

        plot_stepX      = glist_getValueForOnePixelX (glist);
        plot_stepY      = glist_getValueForOnePixelY (glist);
        plot_relativeX  = plot_getRelativeX (&p, baseX);
        plot_relativeY  = plot_getRelativeY (&p, baseY);
        plot_incrementX = p.p_incrementX;
        plot_width      = p.p_width;
        plot_style      = p.p_style;
        plot_fieldArray = &x->x_array;
            
        gpointer_setByCopy (&plot_gpointer, gp);
        gpointer_setAsWord (&plot_check, p.p_array, array_getElements (p.p_array));
        
        /* The garray case is handled differently. */
        
        if (glist_isArray (glist)) { return plot_behaviorMouseArray (x, &p, m); }
        else {
            return plot_behaviorMouseGrab (x, &p, m);
        }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *plot_new (t_symbol *s, int argc, t_atom *argv)
{
    t_plot *x = (t_plot *)pd_new (plot_class);
    
    field_setAsFloatConstant (&x->x_array,          0.0);      /* Default is invalid. */
    field_setAsFloatConstant (&x->x_colorOutline,   0.0);
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
// MARK: -

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
    
    class_setPainterBehavior (c, &plot_painterBehavior);
    
    plot_class = c;
}

void plot_destroy (void)
{
    class_free (plot_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
