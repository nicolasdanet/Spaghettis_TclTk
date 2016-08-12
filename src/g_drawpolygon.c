
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
#pragma mark -

#define DRAWPOLYGON_HANDLE_SIZE     8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          drawpolygon_field;                  /* Shared. */
static t_float      drawpolygon_cumulativeX;            /* Shared. */
static t_float      drawpolygon_cumulativeY;            /* Shared. */
static t_float      drawpolygon_coordinateX;            /* Shared. */
static t_float      drawpolygon_coordinateY;            /* Shared. */
static t_float      drawpolygon_stepX;                  /* Shared. */
static t_float      drawpolygon_stepY;                  /* Shared. */
static t_gpointer   drawpolygon_gpointer;               /* Shared. */

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

static void drawpolygon_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_drawpolygon *x = (t_drawpolygon *)z;

    if (gpointer_isValid (&drawpolygon_gpointer)) {
    //
    t_fielddescriptor *fd = x->x_coordinates + drawpolygon_field;
    
    drawpolygon_cumulativeX += deltaX;
    drawpolygon_cumulativeY += deltaY;
    
    t_float positionX = drawpolygon_coordinateX + (drawpolygon_cumulativeX * drawpolygon_stepX);
    t_float positionY = drawpolygon_coordinateY + (drawpolygon_cumulativeY * drawpolygon_stepY);
    
    if (field_isVariable (fd + 0)) {
        word_setFloatByDescriptorAsPosition (gpointer_getData (&drawpolygon_gpointer),
            gpointer_getTemplate (&drawpolygon_gpointer),
            fd + 0,
            positionX); 
    }
    
    if (field_isVariable (fd + 1)) {
        word_setFloatByDescriptorAsPosition (gpointer_getData (&drawpolygon_gpointer),
            gpointer_getTemplate (&drawpolygon_gpointer),
            fd + 1,
            positionY);
    }
    
    if (gpointer_isScalar (&drawpolygon_gpointer)) {
        template_notify (gpointer_getTemplate (&drawpolygon_gpointer),
            gpointer_getView (&drawpolygon_gpointer),
            gpointer_getScalar (&drawpolygon_gpointer),
            sym_change,
            0,
            NULL);
    }
    
    gpointer_redraw (&drawpolygon_gpointer);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawpolygon_behaviorGetRectangle (t_gobj *z,
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
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int x1 = PD_INT_MAX;
    int y1 = PD_INT_MAX;
    int x2 = -x1;
    int y2 = -y1;
    
    int visible = (int)word_getFloatByDescriptor (w, tmpl, &x->x_isVisible);
    
    if (visible && !(x->x_flags & DRAWPOLYGON_NO_MOUSE)) {
    //
    int i;
    t_fielddescriptor *fd = x->x_coordinates;
    
    for (i = 0; i < x->x_size; i += 2) {
    //
    int m, n;
    
    m = canvas_valueToPixelX (glist, baseX + word_getFloatByDescriptorAsPosition (w, tmpl, fd + i));
    n = canvas_valueToPixelY (glist, baseY + word_getFloatByDescriptorAsPosition (w, tmpl, fd + i + 1));
    
    x1 = PD_MIN (m, x1);
    x2 = PD_MAX (m, x2);
    y1 = PD_MIN (n, y1);
    y2 = PD_MAX (n, y2);
    //
    }
    //
    }
    
    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2; 
}

static void drawpolygon_behaviorVisibilityChanged (t_gobj *z, 
    t_glist *glist, 
    t_word *w,
    t_template *tmpl,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)word_getFloatByDescriptor (w, tmpl, &x->x_isVisible);
    
    if (!isVisible || visible) {
    //
    int i, n = x->x_numberOfPoints;
    
    if (n > 1) {
    //
    if (!isVisible) { sys_vGui (".x%lx.c delete %lxCURVE\n", canvas_getView (glist), w); }
    else {
    //
    t_float width        = word_getFloatByDescriptor (w, tmpl, &x->x_width);
    t_float colorFill    = word_getFloatByDescriptor (w, tmpl, &x->x_colorFill);
    t_float colorOutline = word_getFloatByDescriptor (w, tmpl, &x->x_colorOutline);
    t_symbol *filled     = color_toEncodedSymbol (color_withDigits ((int)colorFill));
    t_symbol *outlined   = color_toEncodedSymbol (color_withDigits ((int)colorOutline));
    
    t_fielddescriptor *fd = x->x_coordinates;
    t_heapstring *t = heapstring_new (0);
    int i;
    
    t_glist *view = canvas_getView (glist);
    
    if (x->x_flags & DRAWPOLYGON_CLOSED) { heapstring_addSprintf (t, ".x%lx.c create polygon", view); }
    else {
        heapstring_addSprintf (t, ".x%lx.c create line", view);
    }
    
    for (i = 0; i < x->x_size; i += 2) {
    //
    int a, b;
    
    a = canvas_valueToPixelX (glist, baseX + word_getFloatByDescriptorAsPosition (w, tmpl, fd + i));
    b = canvas_valueToPixelY (glist, baseY + word_getFloatByDescriptorAsPosition (w, tmpl, fd + i + 1));
        
    heapstring_addSprintf (t, " %d %d", a, b);
    //
    }
    
    if (x->x_flags & DRAWPOLYGON_BEZIER)  { heapstring_add (t, " -smooth 1"); }
    if (x->x_flags & DRAWPOLYGON_CLOSED)  {
        heapstring_addSprintf (t, " -fill %s", filled->s_name);
        heapstring_addSprintf (t, " -outline %s", outlined->s_name);
    } else {
        heapstring_addSprintf (t, " -fill %s", outlined->s_name);
    }

    heapstring_addSprintf (t,  " -width %f", PD_MAX (width, 1.0));
    heapstring_addSprintf (t,  " -tags %lxCURVE\n", w);
    
    sys_gui (heapstring_getRaw (t));
    
    heapstring_free (t);
    //
    }
    //
    }
    //
    }
}

static int drawpolygon_behaviorClicked (t_gobj *z,
    t_glist *glist, 
    t_word *w,
    t_template *tmpl,
    t_scalar *asScalar,
    t_array *dummy,
    t_float baseX,
    t_float baseY,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    t_drawpolygon *x = (t_drawpolygon *)z;
    
    int visible = (int)word_getFloatByDescriptor (w, tmpl, &x->x_isVisible);
    
    if (visible) {
    //
    int i;
    int bestField = -1;
    int bestError = PD_INT_MAX;
    
    t_fielddescriptor *fd = x->x_coordinates;

    for (i = 0; i < x->x_size; i += 2) {
    //
    if (field_isVariable (fd + i) || field_isVariable (fd + i + 1)) {
    //
    int valueX = word_getFloatByDescriptorAsPosition (w, tmpl, fd + i);
    int valueY = word_getFloatByDescriptorAsPosition (w, tmpl, fd + i + 1);
    int pixelX = canvas_valueToPixelX (glist, baseX + valueX);
    int pixelY = canvas_valueToPixelY (glist, baseY + valueY);
    int errorX = PD_ABS (pixelX - a);
    int errorY = PD_ABS (pixelY - b);
    int error  = PD_MAX (errorX, errorY);

    if (error < bestError) {
        drawpolygon_coordinateX = valueX;
        drawpolygon_coordinateY = valueY;
        bestError = error;
        bestField = i;
    }
    //
    }
    //
    }
    
    if (bestError <= DRAWPOLYGON_HANDLE_SIZE) {
    
        if (clicked) {
        
            drawpolygon_stepX       = canvas_valueForOnePixelX (glist);
            drawpolygon_stepY       = canvas_valueForOnePixelY (glist);
            drawpolygon_cumulativeX = 0.0;
            drawpolygon_cumulativeY = 0.0;
            drawpolygon_field       = bestField;
            
            gpointer_setAsScalar (&drawpolygon_gpointer, glist, asScalar);
            
            canvas_setMotionFunction (glist, z, (t_motionfn)drawpolygon_motion, a, b);
        }
    
        return 1;
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_parentwidgetbehavior drawpolygon_parentWidgetBehavior =
    {
        drawpolygon_behaviorGetRectangle,
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

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && (t == sym___dash__v || t == sym___dash__visible)) {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (t == sym___dash__i || t == sym___dash__inhibit) {
            x->x_flags |= DRAWPOLYGON_NO_MOUSE;
            argc -= 1; argv += 1;
            
        } else { break; }
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
#pragma mark -

void drawpolygon_initialize (void)
{
}

void drawpolygon_release (void)
{
    if (gpointer_isSet (&drawpolygon_gpointer)) { gpointer_unset (&drawpolygon_gpointer); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
