
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
static int          drawnumber_type;                        /* Shared. */
static t_gpointer   drawnumber_gpointer;                    /* Shared. */

static t_glist      *drawnumber_glist;                      /* Shared. */
static t_scalar     *drawnumber_asScalar;                   /* Shared. */
static t_array      *drawnumber_asArray;                    /* Shared. */
static t_word       *drawnumber_data;                       /* Shared. */
static t_template   *drawnumber_template;                   /* Shared. */

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

static int drawnumber_gettype (t_drawnumber *x, t_word *data, t_template *template, int *onsetp)
{
    int type;
    t_symbol *arraytype;
    if (template_findField(template, x->x_fieldName, onsetp, &type,
        &arraytype) && type != DATA_ARRAY)
            return (type);
    else return (-1);
}

static t_error drawnumber_getContents (t_drawnumber *x,
    t_word *w,
    t_template *tmpl,
    char *dest,
    size_t size,
    int *m,
    int *n)
{
    if (!template_fieldIsArray (tmpl, x->x_fieldName)) {
    //
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (dest, size, x->x_label->s_name);
    
    if (template_fieldIsText (tmpl, x->x_fieldName)) {
        char *t = NULL;
        buffer_toString (word_getBuffer (w, tmpl, x->x_fieldName), &t);
        err |= string_add (dest, size, t);
        PD_MEMORY_FREE (t);
        
    } else {
        t_atom a;
        if (template_fieldIsFloat (tmpl, x->x_fieldName)) {
            SET_FLOAT (&a, word_getFloat (w, tmpl, x->x_fieldName));
        } else {
            SET_SYMBOL (&a, word_getSymbol (w, tmpl, x->x_fieldName));
        }
        err |= string_addAtom (dest, size, &a);
    }
    
    if (m && n) { string_getColumnsAndLines (dest, m, n); }
    
    return err;
    //
    }
    
    return PD_ERROR;
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

static void drawnumber_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    t_drawnumber *x = (t_drawnumber *)z;
    // t_atom at;
    if (!gpointer_isValid(&drawnumber_gpointer))
    {
        post("drawnumber_motion: scalar disappeared");
        return;
    }
    if (drawnumber_type != DATA_FLOAT)
        return;
    drawnumber_cumulativeY -= dy;
    word_setFloat(drawnumber_data, 
        drawnumber_template,
        x->x_fieldName,
        drawnumber_cumulativeY);
    if (drawnumber_asScalar)
        template_notify(drawnumber_template,
            drawnumber_glist, drawnumber_asScalar,
                sym_change, 0, NULL);

    if (drawnumber_asScalar)
        scalar_redraw(drawnumber_asScalar, drawnumber_glist);
    if (drawnumber_asArray)
        array_redraw(drawnumber_asArray, drawnumber_glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawnumber_behaviorGetRectangle (t_gobj *z,
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
    t_drawnumber *x = (t_drawnumber *)z;
    
    if (word_getFloatByDescriptor (w, tmpl, &x->x_isVisible)) {
    //
    char t[PD_STRING] = { 0 };
    int m, n;
    
    t_float valueX      = baseX + word_getFloatByDescriptorAsPosition (w, tmpl, &x->x_positionX);
    t_float valueY      = baseY + word_getFloatByDescriptorAsPosition (w, tmpl, &x->x_positionY);
    int pixelX          = canvas_valueToPixelX (glist, valueX);
    int pixelY          = canvas_valueToPixelY (glist, valueY);
    t_fontsize fontSize = canvas_getFontSize (glist);
    
    if (!drawnumber_getContents (x, w, tmpl, t, PD_STRING, &m, &n)) {
        *a = pixelX;
        *b = pixelY;
        *c = pixelX + (m * font_getHostFontWidth (fontSize));
        *d = pixelY + (n * font_getHostFontHeight (fontSize));
        return;
    }
    //
    }
    
    *a = PD_INT_MAX; *b = PD_INT_MAX; *c = -PD_INT_MAX; *d = -PD_INT_MAX;
}

static void drawnumber_behaviorVisibilityChanged(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_float basex, t_float basey,
    int vis)
{
    t_drawnumber *x = (t_drawnumber *)z;
    
        /* see comment in plot_vis() */
    if (vis && !word_getFloatByDescriptor(data, template, &x->x_isVisible))
        return;
    if (vis)
    {
        t_atom at;
        int xloc = canvas_valueToPixelX(glist,
            basex + word_getFloatByDescriptorAsPosition(data, template, &x->x_positionX));
        int yloc = canvas_valueToPixelY(glist,
            basey + word_getFloatByDescriptorAsPosition(data, template, &x->x_positionY));
        char colorstring[20], buf[PD_STRING];
        color_toEncodedString(colorstring, 20,
            color_withDigits (word_getFloatByDescriptor(data, template, &x->x_color)));
        drawnumber_getContents(x, data, template, buf, PD_STRING, NULL, NULL);
        sys_vGui(".x%lx.c create text %d %d -anchor nw -fill %s -text {%s}",
                canvas_getView(glist), xloc, yloc, colorstring, buf);
        sys_vGui(" -font [::getFont %d]",
                 font_getHostFontSize(canvas_getFontSize(glist)));
        sys_vGui(" -tags [list drawnumber%lx label]\n", data);
    }
    else sys_vGui(".x%lx.c delete drawnumber%lx\n", canvas_getView(glist), data);
}

static int drawnumber_behaviorClicked(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_drawnumber *x = (t_drawnumber *)z;
    int x1, y1, x2, y2, type, onset;
    drawnumber_behaviorGetRectangle(z, glist,
        data, template, basex, basey,
        &x1, &y1, &x2, &y2);
    if (xpix >= x1 && xpix <= x2 && ypix >= y1 && ypix <= y2 &&
        ((type = drawnumber_gettype(x, data, template, &onset)) == DATA_FLOAT ||
            type == DATA_SYMBOL))
    {
        if (doit)
        {
            drawnumber_glist = glist;
            drawnumber_data = data;
            drawnumber_template = template;
            drawnumber_asScalar = sc;
            drawnumber_asArray = ap;
            drawnumber_cumulativeY =
                word_getFloat(data, template, x->x_fieldName);
            drawnumber_type = type;
            if (drawnumber_asScalar)
                gpointer_setAsScalar(&drawnumber_gpointer, 
                    drawnumber_glist, drawnumber_asScalar);
            else gpointer_setAsWord(&drawnumber_gpointer,
                    drawnumber_asArray, drawnumber_data);
            canvas_setMotionFunction(glist, z, (t_motionfn)drawnumber_motion, xpix, ypix);
        }
        return (1);
    }
    else return (0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_parentwidgetbehavior drawnumber_widgetBehavior =
    {
        drawnumber_behaviorGetRectangle,
        drawnumber_behaviorVisibilityChanged,
        drawnumber_behaviorClicked,
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
    
    class_setParentWidgetBehavior (c, &drawnumber_widgetBehavior);
    
    drawnumber_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
