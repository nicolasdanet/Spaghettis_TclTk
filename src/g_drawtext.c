
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class      *drawtext_class;                        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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

static t_painterbehavior drawtext_painterBehavior =
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

static t_error drawtext_getContents (t_drawtext *x, t_gpointer *gp, char *dest, int size, int *m, int *n)
{
    if (gpointer_hasField (gp, x->x_fieldName)) {
        if (!gpointer_fieldIsArray (gp, x->x_fieldName)) {
            t_error err = string_copy (dest, size, x->x_label->s_name);
            err |= gpointer_addFieldToString (gp, x->x_fieldName, dest, size);
            if (string_containsOccurrence (dest, "{}")) {               // --
                err |= string_escapeOccurrence (dest, size, "{}");      // --
            }
            if (m && n) { string_getNumberOfColumnsAndLines (dest, m, n); }
            return err;
        }
    }
    
    return PD_ERROR;
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
    drawtext_cumulativeY -= deltaY;
    
    gpointer_erase (&drawtext_gpointer);
    
    gpointer_setFloat (&drawtext_gpointer, x->x_fieldName, drawtext_cumulativeY);
    
    PD_ASSERT (gpointer_isScalar (&drawtext_gpointer));
    
    template_notify (gpointer_getTemplate (&drawtext_gpointer), 
        gpointer_getView (&drawtext_gpointer), 
        gpointer_getScalar (&drawtext_gpointer),
        sym_change,
        0,
        NULL);

    gpointer_draw (&drawtext_gpointer);
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
    int m, n;
        
    t_float valueX      = baseX + gpointer_getFloatByDescriptor (gp, &x->x_positionX);
    t_float valueY      = baseY + gpointer_getFloatByDescriptor (gp, &x->x_positionY);
    int pixelX          = glist_valueToPixelX (glist, valueX);
    int pixelY          = glist_valueToPixelY (glist, valueY);
    t_fontsize fontSize = glist_getFontSize (glist);
    
    if (!drawtext_getContents (x, gp, t, PD_STRING, &m, &n)) {
    
        int a = pixelX;
        int b = pixelY;
        int c = (int)(pixelX + (m * font_getHostFontWidth (fontSize)));
        int d = (int)(pixelY + (n * font_getHostFontHeight (fontSize)));
        
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
    
    drawtext_getContents (x, gp, t, PD_STRING, NULL, NULL);
    
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
                    font_getHostFontSize (glist_getFontSize (glist)),
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
    if (rectangle_containsPoint (&t, a, b)) {
    //
    if (gpointer_fieldIsFloat (gp, x->x_fieldName)) {
    //
    if (m->m_clicked) {
    
        drawtext_cumulativeY = gpointer_getFloat (gp, x->x_fieldName);

        gpointer_setByCopy (&drawtext_gpointer, gp);
        
        glist_setMotion (gpointer_getView (gp), z, (t_motionfn)drawtext_motion, a, b);
    }
    
    return CURSOR_OVER;
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

static void *drawtext_new (t_symbol *s, int argc, t_atom *argv)
{
    t_drawtext *x = (t_drawtext *)pd_new (drawtext_class);

    field_setAsFloatConstant (&x->x_positionX,  (t_float)0.0);
    field_setAsFloatConstant (&x->x_positionY,  (t_float)0.0);
    field_setAsFloatConstant (&x->x_color,      (t_float)0.0);
    field_setAsFloatConstant (&x->x_isVisible,  (t_float)1.0);
    
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

    error__options (s, argc, argv);
    
    x->x_fieldName = atom_getSymbolAtIndex (0, argc, argv); 

    if (argc) { argc--; argv++; }
    
    if (argc) { field_setAsFloat (&x->x_positionX,  argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_positionY,  argc--, argv++); }
    if (argc) { field_setAsFloat (&x->x_color,      argc--, argv++); }
    
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
    
    drawtext_class = c;
}

void drawtext_destroy (void)
{
    class_free (drawtext_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
