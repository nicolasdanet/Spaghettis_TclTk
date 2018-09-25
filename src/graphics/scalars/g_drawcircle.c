
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define DRAWCIRCLE_NONE         0
#define DRAWCIRCLE_TOP          1
#define DRAWCIRCLE_BOTTOM       2
#define DRAWCIRCLE_LEFT         3
#define DRAWCIRCLE_RIGHT        4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DRAWCIRCLE_TINY         24
#define DRAWCIRCLE_HANDLE       0.3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class      *drawcircle_class;                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float      drawcircle_cumulativeX;                 /* Static. */
static t_float      drawcircle_cumulativeY;                 /* Static. */
static t_float      drawcircle_stepX;                       /* Static. */
static t_float      drawcircle_stepY;                       /* Static. */
static t_float      drawcircle_radius;                      /* Static. */
static int          drawcircle_side;                        /* Static. */
static int          drawcircle_opposite;                    /* Static. */

static t_gpointer   drawcircle_gpointer;                    /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _drawcircle {
    t_object            x_obj;                              /* Must be the first. */
    int                 x_isFilled;
    t_fielddescriptor   x_positionX;
    t_fielddescriptor   x_positionY;
    t_fielddescriptor   x_colorFill;
    t_fielddescriptor   x_colorOutline;
    t_fielddescriptor   x_width;
    t_fielddescriptor   x_isVisible;
    t_fielddescriptor   x_radius;
    } t_drawcircle;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawcircle_behaviorGetRectangle      (t_gobj *, t_gpointer *, t_float, t_float, t_rectangle *);
static void drawcircle_behaviorVisibilityChanged (t_gobj *, t_gpointer *, t_float, t_float, int);
static int  drawcircle_behaviorMouse             (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_painterbehavior drawcircle_painterBehavior =       /* Shared. */
    {
        drawcircle_behaviorGetRectangle,
        drawcircle_behaviorVisibilityChanged,
        drawcircle_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawcircle_release (void)
{
    gpointer_unset (&drawcircle_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawcircle_float (t_drawcircle *x, t_float f)
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
    } else { error_unexpected (sym_drawcircle, &s_float); }
}


static void drawcircle_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_drawcircle *x = (t_drawcircle *)z;

    if (gpointer_isValid (&drawcircle_gpointer)) {
    //
    drawcircle_cumulativeX += deltaX;
    drawcircle_cumulativeY += deltaY;
    
    t_float dX = drawcircle_cumulativeX * drawcircle_stepX;
    t_float dY = drawcircle_cumulativeY * drawcircle_stepY;
    t_float d  = 0.0;
    
    switch (drawcircle_side) {
        case DRAWCIRCLE_TOP     : d = drawcircle_opposite ? dY : -dY; break;
        case DRAWCIRCLE_BOTTOM  : d = drawcircle_opposite ? -dY : dY; break;
        case DRAWCIRCLE_LEFT    : d = drawcircle_opposite ? dX : -dX; break;
        case DRAWCIRCLE_RIGHT   : d = drawcircle_opposite ? -dX : dX; break;
        default                 : PD_BUG;
    }
    
    gpointer_erase (&drawcircle_gpointer);
    
    gpointer_setFloatByDescriptor (&drawcircle_gpointer, &x->x_radius, drawcircle_radius + d);

    gpointer_draw (&drawcircle_gpointer);
    
    gpointer_notify (&drawcircle_gpointer, sym_change, 0, NULL);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawcircle_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    t_rectangle *r)
{
    t_drawcircle *x = (t_drawcircle *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    rectangle_setNothing (r);
    
    if (visible) {
    //
    t_glist *glist = gpointer_getView (gp);
    
    int width      = (gpointer_getFloatByDescriptor (gp, &x->x_width) / 2.0);
    t_float radius = gpointer_getFloatByDescriptor (gp, &x->x_radius);
    t_float valueX = baseX + gpointer_getFloatByDescriptor (gp, &x->x_positionX);
    t_float valueY = baseY + gpointer_getFloatByDescriptor (gp, &x->x_positionY);
    
    if (radius < 0.0) { width *= -1; }
    
    int a = glist_valueToPixelX (glist, valueX - (radius + width));
    int b = glist_valueToPixelY (glist, valueY - (radius + width));
    int c = glist_valueToPixelX (glist, valueX + (radius + width));
    int d = glist_valueToPixelY (glist, valueY + (radius + width));
    
    rectangle_set (r, a, b, c, d);
    //
    }
}

static void drawcircle_behaviorVisibilityChanged (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_drawcircle *x = (t_drawcircle *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);

    if (!isVisible || visible) {
    //
    t_word *tag    = gpointer_getElement (gp);
    t_glist *glist = gpointer_getView (gp);
    t_glist *view  = glist_getView (glist);
    
    if (!isVisible) { gui_vAdd ("%s.c delete %lxCIRCLE\n", glist_getTagAsString (view), tag); }    // --
    else {
    //
    t_rectangle r;
    
    int width            = gpointer_getFloatByDescriptor (gp, &x->x_width);
    t_float colorFill    = gpointer_getFloatByDescriptor (gp, &x->x_colorFill);
    t_float colorOutline = gpointer_getFloatByDescriptor (gp, &x->x_colorOutline);
    t_symbol *filled     = color_toEncoded (color_withDigits ((int)colorFill));
    t_symbol *outlined   = color_toEncoded (color_withDigits ((int)colorOutline));
    
    t_float radius = gpointer_getFloatByDescriptor (gp, &x->x_radius);
    t_float valueX = baseX + gpointer_getFloatByDescriptor (gp, &x->x_positionX);
    t_float valueY = baseY + gpointer_getFloatByDescriptor (gp, &x->x_positionY);
    
    int a = glist_valueToPixelX (glist, valueX - radius);
    int b = glist_valueToPixelY (glist, valueY - radius);
    int c = glist_valueToPixelX (glist, valueX + radius);
    int d = glist_valueToPixelY (glist, valueY + radius);
    
    rectangle_set (&r, a, b, c, d);
    
    if (x->x_isFilled) {
    
        gui_vAdd ("%s.c create oval"
                        " %d"
                        " %d"
                        " %d"
                        " %d"
                        " -width %d"
                        " -fill %s"
                        " -outline %s"
                        " -tags %lxCIRCLE\n",
                        glist_getTagAsString (view),
                        rectangle_getTopLeftX (&r),
                        rectangle_getTopLeftY (&r),
                        rectangle_getBottomRightX (&r),
                        rectangle_getBottomRightY (&r),
                        width,
                        filled->s_name,
                        outlined->s_name,
                        tag);
    
    } else {
    
        gui_vAdd ("%s.c create oval"
                        " %d"
                        " %d"
                        " %d"
                        " %d"
                        " -width %d"
                        " -outline %s"
                        " -tags %lxCIRCLE\n",
                        glist_getTagAsString (view),
                        rectangle_getTopLeftX (&r),
                        rectangle_getTopLeftY (&r),
                        rectangle_getBottomRightX (&r),
                        rectangle_getBottomRightY (&r),
                        width,
                        outlined->s_name,
                        tag);
    }
    
    //
    }
    //
    }
}

/* Note that it can be an ellipse with asymmetric rescaling. */

static int drawcircle_behaviorMouse (t_gobj *z, t_gpointer *gp, t_float baseX, t_float baseY, t_mouse *m)
{
    t_drawcircle *x = (t_drawcircle *)z;
    
    int a = m->m_x;
    int b = m->m_y;
    
    drawcircle_side = DRAWCIRCLE_NONE;
    
    int match = 0;
    
    if (!field_isFloatConstant (&x->x_radius)) {
    //
    t_rectangle t;
     
    drawcircle_behaviorGetRectangle (z, gp, baseX, baseY, &t);

    if (!rectangle_isNothing (&t)) {
    //
    if (rectangle_contains (&t, a, b)) {
    //
    int w = rectangle_getWidth (&t);
    int h = rectangle_getHeight (&t);
    
    if (w > DRAWCIRCLE_TINY && h > DRAWCIRCLE_TINY) {
    //
    t_float u = rectangle_getMiddleX (&t);
    t_float v = rectangle_getMiddleY (&t);
    
    if (b < (v - (h / 4.0)))      { drawcircle_side = DRAWCIRCLE_TOP;    }
    else if (b > (v + (h / 4.0))) { drawcircle_side = DRAWCIRCLE_BOTTOM; }
    else if (a < u)               { drawcircle_side = DRAWCIRCLE_LEFT;   }
    else                          { drawcircle_side = DRAWCIRCLE_RIGHT;  }
    
    if (drawcircle_side) {
    
        /* Fit cartesian equation a an ellipse. */
        
        t_float t1 = (a - u) / (w / 2.0);
        t_float t2 = (b - v) / (h / 2.0);
        t_float e  = (t1*t1 + t2*t2) - 1.0;

        if (PD_ABS (e) < DRAWCIRCLE_HANDLE) { match = 1; }
    }
    //
    } else {
        drawcircle_side = DRAWCIRCLE_TOP; match = 1;    /* Tiny radius. */
    }
    //
    }
    //
    }
    //
    }

    if (match) {
    //
    t_glist *glist = gpointer_getView (gp);
    
    PD_ASSERT (drawcircle_side != DRAWCIRCLE_NONE);
    
    if (m->m_clicked) {
        
        drawcircle_radius      = gpointer_getFloatByDescriptor (gp, &x->x_radius);
        drawcircle_stepX       = glist_getValueForOnePixelX (glist);
        drawcircle_stepY       = glist_getValueForOnePixelY (glist);
        drawcircle_cumulativeX = 0.0;
        drawcircle_cumulativeY = 0.0;
        drawcircle_opposite    = (drawcircle_radius < 0.0);
        
        gpointer_setByCopy (&drawcircle_gpointer, gp);
        
        glist_setMotion (glist, z, (t_motionfn)drawcircle_motion, a, b);
    }
    
    if (drawcircle_side == DRAWCIRCLE_TOP || drawcircle_side == DRAWCIRCLE_BOTTOM) {
        return CURSOR_RESIZE_Y;
    } else {
        return CURSOR_RESIZE_X;
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *drawcircle_new (t_symbol *s, int argc, t_atom *argv)
{
    t_drawcircle *x = (t_drawcircle *)pd_new (drawcircle_class);

    field_setAsFloatConstant (&x->x_positionX,      0.0);
    field_setAsFloatConstant (&x->x_positionY,      0.0);
    field_setAsFloatConstant (&x->x_colorFill,      0.0);
    field_setAsFloatConstant (&x->x_colorOutline,   0.0);
    field_setAsFloatConstant (&x->x_width,          1.0);
    field_setAsFloatConstant (&x->x_isVisible,      1.0);
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        #if PD_WITH_LEGACY
        
        if (t == sym___dash__v) { t = sym___dash__visible; }
        
        #endif
        
        if (argc > 1 && t == sym___dash__visible) {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__x) {
            field_setAsFloatExtended (&x->x_positionX, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__y) {
            field_setAsFloatExtended (&x->x_positionY, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else if (argc > 1 && t == sym___dash__color) {
            field_setAsFloat (&x->x_colorOutline, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__width) {
            field_setAsFloat (&x->x_width, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (argc > 1 && t == sym___dash__fillcolor) {
            field_setAsFloat (&x->x_colorFill, 1, argv + 1);
            argc -= 2; argv += 2;
        
        } else if (t == sym___dash__fill) {
            x->x_isFilled = 1;
            argc--; argv++;
        
        } else if (t == sym___dash____dash__) {
            argc--; argv++;
            break;
        
        } else {
            break;
        }
    }

    error__options (s, argc, argv);

    if (argc) { field_setAsFloat (&x->x_radius, argc--, argv++); }
    
    if (argc) {
        warning_unusedArguments (s, argc, argv);
    }

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawcircle_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_drawcircle,
            (t_newmethod)drawcircle_new,
            NULL,
            sizeof (t_drawcircle),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, (t_method)drawcircle_float);
    
    class_setPainterBehavior (c, &drawcircle_painterBehavior);
    
    drawcircle_class = c;
}

void drawcircle_destroy (void)
{
    class_free (drawcircle_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
