
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

#define DRAWNUMBER_BUFFER_SIZE      1024

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int drawnumber_gettype(t_drawnumber *x, t_word *data,
    t_template *template, int *onsetp)
{
    int type;
    t_symbol *arraytype;
    if (template_findField(template, x->x_fieldName, onsetp, &type,
        &arraytype) && type != DATA_ARRAY)
            return (type);
    else return (-1);
}

static void drawnumber_getbuf(t_drawnumber *x, t_word *data,
    t_template *template, char *buf)
{
    int nchars, onset, type = drawnumber_gettype(x, data, template, &onset);
    if (type < 0)
        buf[0] = 0;
    else
    {
        strncpy(buf, x->x_label->s_name, DRAWNUMBER_BUFFER_SIZE);
        buf[DRAWNUMBER_BUFFER_SIZE - 1] = 0;
        nchars = strlen(buf);
        if (type == DATA_TEXT)
        {
            char *buf2;
            int size2, ncopy;
            buffer_toStringUnzeroed(((t_word *)((char *)data + onset))->w_buffer,
                &buf2, &size2);
            ncopy = (size2 > DRAWNUMBER_BUFFER_SIZE-1-nchars ? 
                DRAWNUMBER_BUFFER_SIZE-1-nchars: size2);
            memcpy(buf+nchars, buf2, ncopy);
            buf[nchars+ncopy] = 0;
            if (nchars+ncopy == DRAWNUMBER_BUFFER_SIZE-1)
                strcpy(buf+(DRAWNUMBER_BUFFER_SIZE-4), "...");
            PD_MEMORY_FREE(buf2);
        }
        else
        {
            t_atom at;
            if (type == DATA_FLOAT)
                SET_FLOAT(&at, ((t_word *)((char *)data + onset))->w_float);
            else SET_SYMBOL(&at, ((t_word *)((char *)data + onset))->w_symbol);
            atom_toString(&at, buf + nchars, DRAWNUMBER_BUFFER_SIZE - nchars);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void drawnumber_float(t_drawnumber *x, t_float f)
{
    int viswas;
    if (!field_isFloatConstant (&x->x_isVisible))
    {
        post_error ("global vis/invis for a template with variable visibility");
        return;
    }
    viswas = (field_getFloatConstant (&x->x_isVisible) != 0);
    
    if ((f != 0 && viswas) || (f == 0 && !viswas))
        return;
    paint_scalarsEraseAll();
    field_setAsFloatConstant(&x->x_isVisible, (f != 0));
    paint_scalarsDrawAll();
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

static void drawnumber_behaviorGetRectangle(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_drawnumber *x = (t_drawnumber *)z;
    t_atom at;
    int xloc, yloc, font, fontwidth, fontheight, bufsize, width, height;
    char buf[DRAWNUMBER_BUFFER_SIZE], *startline, *newline;

    if (!word_getFloatByDescriptor(data, template, &x->x_isVisible))
    {
        *xp1 = *yp1 = PD_INT_MAX;
        *xp2 = *yp2 = -PD_INT_MAX;
        return;
    }
    xloc = canvas_valueToPixelX(glist,
        basex + word_getFloatByDescriptorAsPosition(data, template, &x->x_positionX));
    yloc = canvas_valueToPixelY(glist,
        basey + word_getFloatByDescriptorAsPosition(data, template, &x->x_positionY));
    font = canvas_getFontSize(glist);
    fontwidth = font_getHostFontWidth(font);
        fontheight = font_getHostFontHeight(font);
    drawnumber_getbuf(x, data, template, buf);
    width = 0;
    height = 1;
    for (startline = buf; newline = strchr(startline, '\n');
        startline = newline+1)
    {
        if (newline - startline > width)
            width = newline - startline;
        height++;
    }
    if (strlen(startline) > (unsigned)width)
        width = strlen(startline);
    *xp1 = xloc;
    *yp1 = yloc;
    *xp2 = xloc + fontwidth * width;
    *yp2 = yloc + fontheight * height;
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
        char colorstring[20], buf[DRAWNUMBER_BUFFER_SIZE];
        color_toEncodedString(colorstring, 20,
            color_withDigits (word_getFloatByDescriptor(data, template, &x->x_color)));
        drawnumber_getbuf(x, data, template, buf);
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

static void *drawnumber_new(t_symbol *classsym, int argc, t_atom *argv)
{
    t_drawnumber *x = (t_drawnumber *)pd_new(drawnumber_class);
    char *classname = classsym->s_name;

    field_setAsFloatConstant(&x->x_isVisible, 1);
    
    while (1)
    {
        t_symbol *firstarg = atom_getSymbolAtIndex(0, argc, argv);
        if (!strcmp(firstarg->s_name, "-v") && argc > 1)
        {
            field_setAsFloat(&x->x_isVisible, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else break;
    }
        /* next argument is name of field to draw - we don't know its type yet
        but field_setAsFloat() will do fine here. */
    x->x_fieldName = atom_getSymbolAtIndex(0, argc, argv);
    if (argc)
        argc--, argv++;
    if (argc) field_setAsFloat(&x->x_positionX, argc--, argv++);
    else field_setAsFloatConstant(&x->x_positionX, 0);
    if (argc) field_setAsFloat(&x->x_positionY, argc--, argv++);
    else field_setAsFloatConstant(&x->x_positionY, 0);
    if (argc) field_setAsFloat(&x->x_color, argc--, argv++);
    else field_setAsFloatConstant(&x->x_color, 1);
    if (argc)
        x->x_label = atom_getSymbolAtIndex(0, argc, argv);
    else x->x_label = &s_;

    return (x);
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
