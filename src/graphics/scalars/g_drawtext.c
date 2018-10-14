
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

static t_class      *drawtext_class;                        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float      drawtext_factor;                        /* Static. */
static t_float      drawtext_cumulativeY;                   /* Static. */
static t_gpointer   drawtext_gpointer;                      /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _drawtext {
    t_object            x_obj;                              /* Must be the first. */
    t_fielddescriptor   x_positionX;
    t_fielddescriptor   x_positionY;
    t_fielddescriptor   x_color;
    t_fielddescriptor   x_isVisible;
    t_symbol            *x_fieldName;
    t_symbol            *x_label;
    } t_drawtext;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawtext_behaviorGetRectangle      (t_gobj *, t_gpointer *, t_float, t_float, t_rectangle *);
static void drawtext_behaviorVisibilityChanged (t_gobj *, t_gpointer *, t_float, t_float, int);
static int  drawtext_behaviorMouse             (t_gobj *, t_gpointer *, t_float, t_float, t_mouse *);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_painterbehavior drawtext_painterBehavior =     /* Shared. */
    {
        drawtext_behaviorGetRectangle,
        drawtext_behaviorVisibilityChanged,
        drawtext_behaviorMouse,
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawtext_release (void)
{
    gpointer_unset (&drawtext_gpointer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error drawtext_getContents (t_drawtext *x, t_gpointer *gp, char *dest, int size)
{
    if (gpointer_hasField (gp, x->x_fieldName)) {
    //
    t_error err = string_copy (dest, size, x->x_label->s_name);
    err |= gpointer_getFieldAsString (gp, x->x_fieldName, dest, size);
    return err;
    //
    }
    
    return PD_ERROR;
}

static t_float drawtext_getFactor (t_drawtext *x, t_gpointer *gp, t_rectangle *r, t_mouse *m)
{
    double factor = 1.0;
    
    if (gpointer_fieldIsFloat (gp, x->x_fieldName)) {
    //
    char s[PD_STRING] = { 0 }; t_error err = gpointer_getFieldAsString (gp, x->x_fieldName, s, PD_STRING);
    
    if (!err && !string_containsOccurrence (s, "eE")) {
    //
    int dot = string_indexOfFirstOccurrenceFromEnd (s, ".");
    
    if (dot != -1) {
    
        int decimals = (int)strlen (s) - (dot + 1);
        int h = font_getWidth (glist_getFontSize (gpointer_getView (gp)));
        int a = rectangle_getTopRightX (r);
        int b = rectangle_getTopRightY (r);
        int c = rectangle_getBottomRightX (r);
        int d = rectangle_getBottomRightY (r);
        t_rectangle t; rectangle_set (&t, a, b, c, d);
        int i, k = -1;
    
        for (i = 0; i < decimals; i++) {
            rectangle_enlargeLeft (&t, h); if (rectangle_contains (&t, m->m_x, m->m_y)) { k = i; break; }
        }
        
        if (k >= 0) { int n = decimals - k; while (n--) { factor *= 0.1; } }
    }
    //
    }
    //
    } else { PD_BUG; }
    
    return factor;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawtext_float (t_drawtext *x, t_float f)
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

static void drawtext_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_drawtext *x = (t_drawtext *)z;

    if (gpointer_isValid (&drawtext_gpointer)) {
    //
    drawtext_cumulativeY -= (drawtext_factor * deltaY);
    
    gpointer_erase (&drawtext_gpointer);
    
    gpointer_setFloat (&drawtext_gpointer, x->x_fieldName, drawtext_cumulativeY);
    
    gpointer_draw (&drawtext_gpointer);
    
    gpointer_notify (&drawtext_gpointer, sym_change, 0, NULL);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void drawtext_behaviorGetRectangle (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    t_rectangle *r)
{
    t_drawtext *x = (t_drawtext *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);
    
    if (visible) {
    //
    t_glist *glist = gpointer_getView (gp);
    
    char t[PD_STRING] = { 0 };
        
    t_float valueX      = baseX + gpointer_getFloatByDescriptor (gp, &x->x_positionX);
    t_float valueY      = baseY + gpointer_getFloatByDescriptor (gp, &x->x_positionY);
    int pixelX          = glist_valueToPixelX (glist, valueX);
    int pixelY          = glist_valueToPixelY (glist, valueY);
    int fontSize        = glist_getFontSize (glist);
    
    if (!drawtext_getContents (x, gp, t, PD_STRING)) {
    
        int m = (int)strlen (t);
        int n = 1;
        int a = pixelX;
        int b = pixelY;
        int c = (int)(pixelX + (m * font_getWidth (fontSize)));
        int d = (int)(pixelY + (n * font_getHeight (fontSize)));
        
        rectangle_set (r, a, b, c, d); 
        
        return;
    }
    //
    }
    
    rectangle_setNothing (r);
}

static void drawtext_behaviorVisibilityChanged (t_gobj *z,
    t_gpointer *gp,
    t_float baseX,
    t_float baseY,
    int isVisible)
{
    t_drawtext *x = (t_drawtext *)z;
    
    int visible = (int)gpointer_getFloatByDescriptor (gp, &x->x_isVisible);

    if (!isVisible || visible) {
    //
    t_word *tag    = gpointer_getElement (gp);
    t_glist *glist = gpointer_getView (gp);
    t_glist *view  = glist_getView (glist);
    
    if (!isVisible) { gui_vAdd ("%s.c delete %lxNUMBER\n", glist_getTagAsString (view), tag); }    // --
    else {
    //
    char t[PD_STRING] = { 0 };
    
    t_color color   = color_withDigits ((int)gpointer_getFloatByDescriptor (gp, &x->x_color));
    t_float valueX  = baseX + gpointer_getFloatByDescriptor (gp, &x->x_positionX);
    t_float valueY  = baseY + gpointer_getFloatByDescriptor (gp, &x->x_positionY);
    int pixelX      = glist_valueToPixelX (glist, valueX);
    int pixelY      = glist_valueToPixelY (glist, valueY);
    
    drawtext_getContents (x, gp, t, PD_STRING);
    
    gui_vAdd ("%s.c create text %d %d"
                    " -anchor nw"
                    " -fill %s"
                    " -font [::getFont %d]"     // --
                    " -text {%s}"               // --
                    " -tags %lxNUMBER\n",
                    glist_getTagAsString (view),
                    pixelX,
                    pixelY,
                    color_toEncoded (color)->s_name,
                    glist_getFontSize (glist),
                    t, 
                    tag);
    //
    }
    //
    }
}

static int drawtext_behaviorMouse (t_gobj *z, t_gpointer *gp, t_float baseX, t_float baseY, t_mouse *m)
{
    t_drawtext *x = (t_drawtext *)z;
    
    int a = m->m_x;
    int b = m->m_y;
    
    t_rectangle t;
     
    drawtext_behaviorGetRectangle (z, gp, baseX, baseY, &t);

    if (!rectangle_isNothing (&t)) {
    //
    if (rectangle_contains (&t, a, b)) {
    //
    if (gpointer_fieldIsFloat (gp, x->x_fieldName)) {
    //
    if (m->m_clicked) {
    
        drawtext_cumulativeY = gpointer_getFloat (gp, x->x_fieldName);
        drawtext_factor      = drawtext_getFactor (x, gp, &t, m);
        
        gpointer_setByCopy (&drawtext_gpointer, gp);
        
        glist_setMotion (gpointer_getView (gp), z, (t_motionfn)drawtext_motion, a, b);
    }
    
    return (gpointer_isScalar (gp) ? CURSOR_OVER : CURSOR_ELEMENT_2);
    //
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_buffer *drawtext_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_drawtext *x = (t_drawtext *)z;
        
    if (field_isFloatConstant (&x->x_isVisible)) {
    //
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, &s_float);
    buffer_appendFloat (b, field_getFloatConstant (&x->x_isVisible));
    
    return b;
    //
    }
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *drawtext_new (t_symbol *s, int argc, t_atom *argv)
{
    t_drawtext *x = (t_drawtext *)pd_new (drawtext_class);

    field_setAsFloatConstant (&x->x_positionX,  0.0);
    field_setAsFloatConstant (&x->x_positionY,  0.0);
    field_setAsFloatConstant (&x->x_color,      0.0);
    field_setAsFloatConstant (&x->x_isVisible,  1.0);
    
    x->x_label = &s_;
    
    while (argc > 0) {

        t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
        #if PD_WITH_LEGACY
        
        if (t == sym___dash__v) { t = sym___dash__visible; }
        
        #endif
        
        if (argc > 1 && t == sym___dash__visible) {
            field_setAsFloat (&x->x_isVisible, 1, argv + 1);
            argc -= 2; argv += 2;
            
        } else {
            break;
        }
    }

    error__options (s, argc, argv);
    
    x->x_fieldName = atom_getSymbolAtIndex (0, argc, argv); 

    if (argc) { argc--; argv++; }
    
    if (argc) { field_setAsFloatExtended (&x->x_positionX, argc--, argv++); }
    if (argc) { field_setAsFloatExtended (&x->x_positionY, argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_color, argc--, argv++); }
    
    if (argc) { x->x_label = atom_getSymbolAtIndex (0, argc--, argv++); }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void drawtext_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_drawtext,
            (t_newmethod)drawtext_new,
            NULL,
            sizeof (t_drawtext),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, (t_method)drawtext_float);
    
    class_addCreator ((t_newmethod)drawtext_new, sym_drawsymbol, A_GIMME, A_NULL);
    class_addCreator ((t_newmethod)drawtext_new, sym_drawnumber, A_GIMME, A_NULL);
    
    class_setPainterBehavior (c, &drawtext_painterBehavior);
    
    class_setDataFunction (c, drawtext_functionData);

    drawtext_class = c;
}

void drawtext_destroy (void)
{
    class_free (drawtext_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
