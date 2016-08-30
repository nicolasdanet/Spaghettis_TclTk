
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
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *drawnumber_class;                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float      drawnumber_cumulativeY;                 /* Shared. */
static t_gpointer   drawnumber_gpointer;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _drawnumber {
    t_object            x_obj;                              /* Must be the first. */
    t_fielddescriptor   x_positionX;
    t_fielddescriptor   x_positionY;
    t_fielddescriptor   x_color;
    t_fielddescriptor   x_isVisible;
    t_symbol            *x_fieldName;
    t_symbol            *x_label;
    } t_drawnumber;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error drawnumber_getContents (t_drawnumber *x,
    t_gpointer *gp,
    char *dest,
    int size,
    int *m,
    int *n)
{
    if (gpointer_fieldIsArray (gp, x->x_fieldName)) { return PD_ERROR; }
    else {
        t_error err = string_copy (dest, size, x->x_label->s_name);
        err |= gpointer_fieldToString (gp, x->x_fieldName, dest, (int)size);
        if (m && n) { string_getNumberOfColumnsAndLines (dest, m, n); }
        return err;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawnumber_float (t_drawnumber *x, t_float f)
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

static void drawnumber_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_drawnumber *x = (t_drawnumber *)z;

    if (gpointer_isValid (&drawnumber_gpointer)) {
    //
    drawnumber_cumulativeY -= deltaY;
    
    gpointer_setFloat (&drawnumber_gpointer, x->x_fieldName, drawnumber_cumulativeY);
    
    PD_ASSERT (gpointer_isScalar (&drawnumber_gpointer));
    
    template_notify (gpointer_getTemplate (&drawnumber_gpointer), 
        gpointer_getView (&drawnumber_gpointer), 
        gpointer_getScalar (&drawnumber_gpointer),
        sym_change,
        0,
        NULL);

    gpointer_redraw (&drawnumber_gpointer);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawnumber_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int *a,
    int *b,
    int *c,
    int *d)
{
    t_drawnumber *x = (t_drawnumber *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    if (visible) {
    //
    t_glist *glist = gpointer_getView (gp);
    
    char t[PD_STRING] = { 0 };
    int m, n;
        
    t_float valueX      = baseX + gpointer_getFloatByDescriptorAsPosition (gp, &x->x_positionX);
    t_float valueY      = baseY + gpointer_getFloatByDescriptorAsPosition (gp, &x->x_positionY);
    int pixelX          = canvas_valueToPixelX (glist, valueX);
    int pixelY          = canvas_valueToPixelY (glist, valueY);
    t_fontsize fontSize = canvas_getFontSize (glist);
    
    if (!drawnumber_getContents (x, gp, t, PD_STRING, &m, &n)) {
        *a = pixelX;
        *b = pixelY;
        *c = pixelX + (m * font_getHostFontWidth (fontSize));
        *d = pixelY + (n * font_getHostFontHeight (fontSize));
        return;
    }
    //
    }
    
    rectangle_setNowhere (a, b, c, d);
}

static void drawnumber_behaviorVisibilityChanged (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_drawnumber *x = (t_drawnumber *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);

    if (!isVisible || visible) {
    //
    t_word *tag    = gpointer_getData (gp);
    t_glist *glist = gpointer_getView (gp);
    
    if (!isVisible) { sys_vGui(".x%lx.c delete %lxNUMBER\n", canvas_getView (glist), tag); }
    else {
    //
    char t[PD_STRING] = { 0 };
    
    t_color color   = color_withDigits ((int)gpointer_getFloatByDescriptor (gp, &x->x_color));
    t_float valueX  = baseX + gpointer_getFloatByDescriptorAsPosition (gp, &x->x_positionX);
    t_float valueY  = baseY + gpointer_getFloatByDescriptorAsPosition (gp, &x->x_positionY);
    int pixelX      = canvas_valueToPixelX (glist, valueX);
    int pixelY      = canvas_valueToPixelY (glist, valueY);
    
    drawnumber_getContents (x, gp, t, PD_STRING, NULL, NULL);
    
    sys_vGui (".x%lx.c create text %d %d"
                    " -anchor nw"
                    " -fill %s"
                    " -font [::getFont %d]"
                    " -text {%s}"
                    " -tags %lxNUMBER\n",
                    canvas_getView (glist),
                    pixelX,
                    pixelY,
                    color_toEncodedSymbol (color)->s_name,
                    font_getHostFontSize (canvas_getFontSize (glist)),
                    t, 
                    tag);
    //
    }
    //
    }
}

static int drawnumber_behaviorMouse (t_gobj *z,
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
    t_drawnumber *x = (t_drawnumber *)z;
    
    int x1, y1, x2, y2;
     
    drawnumber_behaviorGetRectangle (z, gp, baseX, baseY, &x1, &y1, &x2, &y2);

    if (a >= x1 && a <= x2 && b >= y1 && b <= y2) {
    //
    if (gpointer_fieldIsFloat (gp, x->x_fieldName)) {
    //
    if (clicked) {
    
        drawnumber_cumulativeY = gpointer_getFloat (gp, x->x_fieldName);

        gpointer_setByCopy (gp, &drawnumber_gpointer);
        
        canvas_setMotionFunction (gpointer_getView (gp), z, (t_motionfn)drawnumber_motion, a, b);
    }
    
    return 1;
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_painterwidgetbehavior drawnumber_widgetBehavior =
    {
        drawnumber_behaviorGetRectangle,
        drawnumber_behaviorVisibilityChanged,
        drawnumber_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *drawnumber_new (t_symbol *s, int argc, t_atom *argv)
{
    t_drawnumber *x = (t_drawnumber *)pd_new (drawnumber_class);

    field_setAsFloatConstant (&x->x_positionX,  0.0);
    field_setAsFloatConstant (&x->x_positionY,  0.0);
    field_setAsFloatConstant (&x->x_color,      0.0);
    field_setAsFloatConstant (&x->x_isVisible,  1.0);
    
    x->x_label = &s_;
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        if (argc > 1 && (t == sym___dash__v || t == sym___dash__visible)) {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else {
            break;
        }
    }

    x->x_fieldName = atom_getSymbolAtIndex (0, argc, argv);
    
    if (argc) { argc--, argv++; }
    
    if (argc) { field_setAsFloat (&x->x_positionX,  argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_positionY,  argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_color,      argc--, argv++); }
    
    if (argc) { x->x_label = atom_getSymbolAtIndex (0, argc, argv);  }

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void drawnumber_initialize (void)
{
}

void drawnumber_release (void)
{
    if (gpointer_isSet (&drawnumber_gpointer)) { gpointer_unset (&drawnumber_gpointer); }
}

void drawnumber_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_drawtext,
            (t_newmethod)drawnumber_new,
            NULL,
            sizeof (t_drawnumber),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, drawnumber_float);
    
    class_addCreator ((t_newmethod)drawnumber_new, sym_drawsymbol, A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)drawnumber_new, sym_drawnumber, A_GIMME, A_NULL);
    
    class_setPainterWidgetBehavior (c, &drawnumber_widgetBehavior);
    
    drawnumber_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
