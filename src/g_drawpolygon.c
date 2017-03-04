
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DRAWPOLYGON_NONE            0
#define DRAWPOLYGON_CLOSED          1
#define DRAWPOLYGON_BEZIER          2
#define DRAWPOLYGON_INHIBIT         4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DRAWPOLYGON_HANDLE_SIZE     8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          drawpolygon_field;                  /* Static. */
static t_float      drawpolygon_cumulativeX;            /* Static. */
static t_float      drawpolygon_cumulativeY;            /* Static. */
static t_float      drawpolygon_valueX;                 /* Static. */
static t_float      drawpolygon_valueY;                 /* Static. */
static t_float      drawpolygon_stepX;                  /* Static. */
static t_float      drawpolygon_stepY;                  /* Static. */
static t_gpointer   drawpolygon_gpointer;               /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *drawpolygon_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _drawpolygon {
    t_object            x_obj;                          /* Must be the first. */
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

static void drawpolygon_behaviorGetRectangle        (t_gobj *, t_gpointer *, t_float, t_float, t_rectangle *);
static void drawpolygon_behaviorVisibilityChanged   (t_gobj *, t_gpointer *, t_float, t_float, int);
static int  drawpolygon_behaviorMouse               (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_painterwidgetbehavior drawpolygon_widgetBehavior =
    {
        drawpolygon_behaviorGetRectangle,
        drawpolygon_behaviorVisibilityChanged,
        drawpolygon_behaviorMouse,
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void drawpolygon_release (void)
{
    gpointer_unset (&drawpolygon_gpointer);
}

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
    paint_erase();
    field_setAsFloatConstant (&x->x_isVisible, (t_float)k);
    paint_draw();
    //
    }
    //
    } else { error_unexpected (sym_drawpolygon, &s_float); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void drawpolygon_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_drawpolygon *x = (t_drawpolygon *)z;

    if (gpointer_isValid (&drawpolygon_gpointer)) {
    //
    t_fielddescriptor *fd = x->x_coordinates + drawpolygon_field;
    
    drawpolygon_cumulativeX += deltaX;
    drawpolygon_cumulativeY += deltaY;
    
    t_float positionX = drawpolygon_valueX + (drawpolygon_cumulativeX * drawpolygon_stepX);
    t_float positionY = drawpolygon_valueY + (drawpolygon_cumulativeY * drawpolygon_stepY);
    
    if (field_isVariable (fd + 0)) {
        gpointer_setFloatByDescriptor (&drawpolygon_gpointer, fd + 0, positionX); 
    }
    
    if (field_isVariable (fd + 1)) {
        gpointer_setFloatByDescriptor (&drawpolygon_gpointer, fd + 1, positionY);
    }
    
    PD_ASSERT (gpointer_isScalar (&drawpolygon_gpointer));
    
    template_notify (gpointer_getTemplate (&drawpolygon_gpointer),
        gpointer_getView (&drawpolygon_gpointer),
        gpointer_getScalar (&drawpolygon_gpointer),
        sym_change,
        0,
        NULL);
    
    gpointer_redraw (&drawpolygon_gpointer);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawpolygon_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    t_rectangle *r)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    rectangle_setNothing (r);
        
    if (visible && !(x->x_flags & DRAWPOLYGON_INHIBIT)) {
    //
    int i;
    t_glist *glist = gpointer_getView (gp);
    t_fielddescriptor *fd = x->x_coordinates;
        
    for (i = 0; i < x->x_size; i += 2) {
    //
    int a = canvas_valueToPixelX (glist, baseX + gpointer_getFloatByDescriptor (gp, fd + i));
    int b = canvas_valueToPixelY (glist, baseY + gpointer_getFloatByDescriptor (gp, fd + i + 1));
    
    rectangle_boundingBoxAddPoint (r, a, b);
    //
    }
    //
    }
}

static void drawpolygon_behaviorVisibilityChanged (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    if (!isVisible || visible) {
    //
    int n = x->x_numberOfPoints;
    
    if (n > 1) {
    //
    t_word *tag    = gpointer_getElement (gp);
    t_glist *glist = gpointer_getView (gp);
    t_glist *view  = canvas_getView (glist);
    
    if (!isVisible) { sys_vGui (".x%lx.c delete %lxCURVE\n", view, tag); }
    else {
    //
    int width            = gpointer_getFloatByDescriptor (gp, &x->x_width);
    t_float colorFill    = gpointer_getFloatByDescriptor (gp, &x->x_colorFill);
    t_float colorOutline = gpointer_getFloatByDescriptor (gp, &x->x_colorOutline);
    t_symbol *filled     = color_toEncodedSymbol (color_withDigits ((int)colorFill));
    t_symbol *outlined   = color_toEncodedSymbol (color_withDigits ((int)colorOutline));
    
    t_fielddescriptor *fd = x->x_coordinates;
    t_heapstring *t = heapstring_new (0);
    int i;
    
    if (x->x_flags & DRAWPOLYGON_CLOSED) { heapstring_addSprintf (t, ".x%lx.c create polygon", view); }
    else {
        heapstring_addSprintf (t, ".x%lx.c create line", view);
    }
    
    for (i = 0; i < x->x_size; i += 2) {
    //
    int a, b;
    
    a = canvas_valueToPixelX (glist, baseX + gpointer_getFloatByDescriptor (gp, fd + i));
    b = canvas_valueToPixelY (glist, baseY + gpointer_getFloatByDescriptor (gp, fd + i + 1));
        
    heapstring_addSprintf (t, " %d %d", a, b);
    //
    }
    
    if (x->x_flags & DRAWPOLYGON_BEZIER)  { heapstring_add (t, " -smooth 1"); }
    if (x->x_flags & DRAWPOLYGON_CLOSED)  {
    //
    heapstring_addSprintf (t, " -fill %s", filled->s_name);
    heapstring_addSprintf (t, " -outline %s", outlined->s_name);
    //
    } else {
    //
    heapstring_addSprintf (t, " -fill %s", outlined->s_name);
    //
    }

    width = PD_MAX (width, 1);
    
    heapstring_addSprintf (t, " -width %d", width);
    heapstring_addSprintf (t, " -tags %lxCURVE\n", tag);
    
    sys_gui (heapstring_getRaw (t));
    
    heapstring_free (t);
    //
    }
    //
    }
    //
    }
}

static int drawpolygon_behaviorMouse (t_gobj *z, t_gpointer *gp, t_float baseX, t_float baseY, t_mouse *m)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    if (visible) {
    //
    t_glist *glist = gpointer_getView (gp);
    
    int i;
    int bestField = -1;
    int bestError = PD_INT_MAX;
    
    t_fielddescriptor *fd = x->x_coordinates;

    for (i = 0; i < x->x_size; i += 2) {
    //
    if (field_isVariable (fd + i) || field_isVariable (fd + i + 1)) {
    //
    int valueX = gpointer_getFloatByDescriptor (gp, fd + i);
    int valueY = gpointer_getFloatByDescriptor (gp, fd + i + 1);
    int pixelX = canvas_valueToPixelX (glist, baseX + valueX);
    int pixelY = canvas_valueToPixelY (glist, baseY + valueY);
    int error  = (int)math_euclideanDistance (pixelX, pixelY, m->m_x, m->m_y);
    
    if (error < bestError) {
        drawpolygon_valueX = valueX;
        drawpolygon_valueY = valueY;
        bestError = error;
        bestField = i;
    }
    //
    }
    //
    }
    
    if (bestError <= DRAWPOLYGON_HANDLE_SIZE) {
    
        if (m->m_clicked) {
        
            drawpolygon_stepX       = canvas_valueForOnePixelX (glist);
            drawpolygon_stepY       = canvas_valueForOnePixelY (glist);
            drawpolygon_cumulativeX = (t_float)0.0;
            drawpolygon_cumulativeY = (t_float)0.0;
            drawpolygon_field       = bestField;
            
            gpointer_setByCopy (&drawpolygon_gpointer, gp);
            
            canvas_setMotionFunction (glist, z, (t_motionfn)drawpolygon_motion, m->m_x, m->m_y);
        }
    
        return 1;
    }
    //
    }
    
    return 0;
}

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
    
    field_setAsFloatConstant (&x->x_colorFill,    (t_float)0.0);
    field_setAsFloatConstant (&x->x_colorOutline, (t_float)0.0);
    field_setAsFloatConstant (&x->x_width,        (t_float)1.0);
    field_setAsFloatConstant (&x->x_isVisible,    (t_float)1.0);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && (t == sym___dash__v || t == sym___dash__visible)) {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (t == sym___dash__i || t == sym___dash__inhibit) {
            x->x_flags |= DRAWPOLYGON_INHIBIT;
            argc -= 1; argv += 1;
            
        } else { break; }
    }
    
    error__options (s, argc, argv);
    
    if (argc && (x->x_flags & DRAWPOLYGON_CLOSED)) { field_setAsFloat (&x->x_colorFill, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_colorOutline, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_width, argc--, argv++); }

    argc = PD_MAX (0, argc);
    
    x->x_numberOfPoints = argc / 2;
    x->x_size           = x->x_numberOfPoints * 2;
    x->x_coordinates    = (t_fielddescriptor *)PD_MEMORY_GET (x->x_size * sizeof (t_fielddescriptor));
    
    for (i = 0; i < x->x_size; i++) { field_setAsFloat (x->x_coordinates + i, 1, argv + i); }

    if (argc - x->x_size > 0) { warning_unusedArguments (s, argc - x->x_size, argv + x->x_size); }
    
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
    
    class_addFloat (c, (t_method)drawpolygon_float);
        
    class_setPainterWidgetBehavior (c, &drawpolygon_widgetBehavior);
    
    drawpolygon_class = c;
}

void drawpolygon_destroy (void)
{
    CLASS_FREE (drawpolygon_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
