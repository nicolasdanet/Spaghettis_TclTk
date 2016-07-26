
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
#pragma mark -

/* ---------------- drawnumber: draw a number (or symbol) ---------------- */

/*
    drawnumbers draw numeric fields at controllable locations, with
    controllable color and label.  invocation:
    (drawnumber|drawsymbol) [-v <visible>] variable x y color label
*/

t_class *drawnumber_class;

typedef struct _drawnumber
{
    t_object x_obj;
    t_symbol *x_fieldname;
    t_fielddescriptor x_xloc;
    t_fielddescriptor x_yloc;
    t_fielddescriptor x_color;
    t_fielddescriptor x_vis;
    t_symbol *x_label;
} t_drawnumber;

static void *drawnumber_new(t_symbol *classsym, int argc, t_atom *argv)
{
    t_drawnumber *x = (t_drawnumber *)pd_new(drawnumber_class);
    char *classname = classsym->s_name;

    field_setAsFloatConstant(&x->x_vis, 1);
    
    while (1)
    {
        t_symbol *firstarg = atom_getSymbolAtIndex(0, argc, argv);
        if (!strcmp(firstarg->s_name, "-v") && argc > 1)
        {
            field_setAsFloat(&x->x_vis, 1, argv+1);
            argc -= 2; argv += 2;
        }
        else break;
    }
        /* next argument is name of field to draw - we don't know its type yet
        but field_setAsFloat() will do fine here. */
    x->x_fieldname = atom_getSymbolAtIndex(0, argc, argv);
    if (argc)
        argc--, argv++;
    if (argc) field_setAsFloat(&x->x_xloc, argc--, argv++);
    else field_setAsFloatConstant(&x->x_xloc, 0);
    if (argc) field_setAsFloat(&x->x_yloc, argc--, argv++);
    else field_setAsFloatConstant(&x->x_yloc, 0);
    if (argc) field_setAsFloat(&x->x_color, argc--, argv++);
    else field_setAsFloatConstant(&x->x_color, 1);
    if (argc)
        x->x_label = atom_getSymbolAtIndex(0, argc, argv);
    else x->x_label = &s_;

    return (x);
}

void drawnumber_float(t_drawnumber *x, t_float f)
{
    int viswas;
    if (!field_isFloatConstant (&x->x_vis))
    {
        post_error ("global vis/invis for a template with variable visibility");
        return;
    }
    viswas = (field_getFloatConstant (&x->x_vis) != 0);
    
    if ((f != 0 && viswas) || (f == 0 && !viswas))
        return;
    paint_scalarsEraseAll();
    field_setAsFloatConstant(&x->x_vis, (f != 0));
    paint_scalarsDrawAll();
}

/* -------------------- widget behavior for drawnumber ------------ */

static int drawnumber_gettype(t_drawnumber *x, t_word *data,
    t_template *template, int *onsetp)
{
    int type;
    t_symbol *arraytype;
    if (template_findField(template, x->x_fieldname, onsetp, &type,
        &arraytype) && type != DATA_ARRAY)
            return (type);
    else return (-1);
}

#define DRAWNUMBER_BUFSIZE 1024
static void drawnumber_getbuf(t_drawnumber *x, t_word *data,
    t_template *template, char *buf)
{
    int nchars, onset, type = drawnumber_gettype(x, data, template, &onset);
    if (type < 0)
        buf[0] = 0;
    else
    {
        strncpy(buf, x->x_label->s_name, DRAWNUMBER_BUFSIZE);
        buf[DRAWNUMBER_BUFSIZE - 1] = 0;
        nchars = strlen(buf);
        if (type == DATA_TEXT)
        {
            char *buf2;
            int size2, ncopy;
            buffer_toStringUnzeroed(((t_word *)((char *)data + onset))->w_buffer,
                &buf2, &size2);
            ncopy = (size2 > DRAWNUMBER_BUFSIZE-1-nchars ? 
                DRAWNUMBER_BUFSIZE-1-nchars: size2);
            memcpy(buf+nchars, buf2, ncopy);
            buf[nchars+ncopy] = 0;
            if (nchars+ncopy == DRAWNUMBER_BUFSIZE-1)
                strcpy(buf+(DRAWNUMBER_BUFSIZE-4), "...");
            PD_MEMORY_FREE(buf2);
        }
        else
        {
            t_atom at;
            if (type == DATA_FLOAT)
                SET_FLOAT(&at, ((t_word *)((char *)data + onset))->w_float);
            else SET_SYMBOL(&at, ((t_word *)((char *)data + onset))->w_symbol);
            atom_toString(&at, buf + nchars, DRAWNUMBER_BUFSIZE - nchars);
        }
    }
}

static void drawnumber_getrect(t_gobj *z, t_glist *glist,
    t_word *data, t_template *template, t_float basex, t_float basey,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_drawnumber *x = (t_drawnumber *)z;
    t_atom at;
    int xloc, yloc, font, fontwidth, fontheight, bufsize, width, height;
    char buf[DRAWNUMBER_BUFSIZE], *startline, *newline;

    if (!word_getFloatByField(data, template, &x->x_vis))
    {
        *xp1 = *yp1 = PD_INT_MAX;
        *xp2 = *yp2 = -PD_INT_MAX;
        return;
    }
    xloc = canvas_valueToPositionX(glist,
        basex + word_getFloatByFieldAsPosition(data, template, &x->x_xloc));
    yloc = canvas_valueToPositionY(glist,
        basey + word_getFloatByFieldAsPosition(data, template, &x->x_yloc));
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

static void drawnumber_vis(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_float basex, t_float basey,
    int vis)
{
    t_drawnumber *x = (t_drawnumber *)z;
    
        /* see comment in plot_vis() */
    if (vis && !word_getFloatByField(data, template, &x->x_vis))
        return;
    if (vis)
    {
        t_atom at;
        int xloc = canvas_valueToPositionX(glist,
            basex + word_getFloatByFieldAsPosition(data, template, &x->x_xloc));
        int yloc = canvas_valueToPositionY(glist,
            basey + word_getFloatByFieldAsPosition(data, template, &x->x_yloc));
        char colorstring[20], buf[DRAWNUMBER_BUFSIZE];
        color_toEncodedString(colorstring, 20,
            color_withDigits (word_getFloatByField(data, template, &x->x_color)));
        drawnumber_getbuf(x, data, template, buf);
        sys_vGui(".x%lx.c create text %d %d -anchor nw -fill %s -text {%s}",
                canvas_getView(glist), xloc, yloc, colorstring, buf);
        sys_vGui(" -font [::getFont %d]",
                 font_getHostFontSize(canvas_getFontSize(glist)));
        sys_vGui(" -tags [list drawnumber%lx label]\n", data);
    }
    else sys_vGui(".x%lx.c delete drawnumber%lx\n", canvas_getView(glist), data);
}

static t_float drawnumber_motion_ycumulative;
static t_glist *drawnumber_motion_glist;
static t_scalar *drawnumber_motion_scalar;
static t_array *drawnumber_motion_array;
static t_word *drawnumber_motion_wp;
static t_template *drawnumber_motion_template;
static t_gpointer drawnumber_motion_gpointer;
static int drawnumber_motion_type;
static int drawnumber_motion_firstkey;

static void drawnumber_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    t_drawnumber *x = (t_drawnumber *)z;
    // t_atom at;
    if (!gpointer_isValid(&drawnumber_motion_gpointer))
    {
        post("drawnumber_motion: scalar disappeared");
        return;
    }
    if (drawnumber_motion_type != DATA_FLOAT)
        return;
    drawnumber_motion_ycumulative -= dy;
    word_setFloat(drawnumber_motion_wp, 
        drawnumber_motion_template,
        x->x_fieldname,
        drawnumber_motion_ycumulative);
    if (drawnumber_motion_scalar)
        template_notify(drawnumber_motion_template,
            drawnumber_motion_glist, drawnumber_motion_scalar,
                sym_change, 0, NULL);

    if (drawnumber_motion_scalar)
        scalar_redraw(drawnumber_motion_scalar, drawnumber_motion_glist);
    if (drawnumber_motion_array)
        array_redraw(drawnumber_motion_array, drawnumber_motion_glist);
}

static int drawnumber_click(t_gobj *z, t_glist *glist, 
    t_word *data, t_template *template, t_scalar *sc, t_array *ap,
    t_float basex, t_float basey,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_drawnumber *x = (t_drawnumber *)z;
    int x1, y1, x2, y2, type, onset;
    drawnumber_getrect(z, glist,
        data, template, basex, basey,
        &x1, &y1, &x2, &y2);
    if (xpix >= x1 && xpix <= x2 && ypix >= y1 && ypix <= y2 &&
        ((type = drawnumber_gettype(x, data, template, &onset)) == DATA_FLOAT ||
            type == DATA_SYMBOL))
    {
        if (doit)
        {
            drawnumber_motion_glist = glist;
            drawnumber_motion_wp = data;
            drawnumber_motion_template = template;
            drawnumber_motion_scalar = sc;
            drawnumber_motion_array = ap;
            drawnumber_motion_firstkey = 1;
            drawnumber_motion_ycumulative =
                word_getFloat(data, template, x->x_fieldname);
            drawnumber_motion_type = type;
            if (drawnumber_motion_scalar)
                gpointer_setAsScalar(&drawnumber_motion_gpointer, 
                    drawnumber_motion_glist, drawnumber_motion_scalar);
            else gpointer_setAsWord(&drawnumber_motion_gpointer,
                    drawnumber_motion_array, drawnumber_motion_wp);
            canvas_setMotionFunction(glist, z, (t_motionfn)drawnumber_motion, xpix, ypix);
        }
        return (1);
    }
    else return (0);
}

t_parentwidgetbehavior drawnumber_widgetbehavior =
{
    drawnumber_getrect,
    drawnumber_vis,
    drawnumber_click,
};

static void drawnumber_free(t_drawnumber *x)
{
}

void drawnumber_setup(void)
{
    drawnumber_class = class_new(sym_drawtext,
        (t_newmethod)drawnumber_new, (t_method)drawnumber_free,
        sizeof(t_drawnumber), 0, A_GIMME, 0);
    class_addFloat(drawnumber_class, drawnumber_float);
    class_addCreator((t_newmethod)drawnumber_new, sym_drawsymbol,
        A_GIMME, 0);
    class_addCreator((t_newmethod)drawnumber_new, sym_drawnumber,
        A_GIMME, 0);
    class_setParentWidgetBehavior(drawnumber_class, &drawnumber_widgetbehavior);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
