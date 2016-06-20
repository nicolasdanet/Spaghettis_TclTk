
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* This file defines the "scalar" object, which is not a text object, just a
"gobj".  Scalars have templates which describe their structures, which
can contain numbers, sublists, and arrays.

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>      /* for read/write to files */
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"

t_class *scalar_class; /* Shared. */

void word_init(t_word *wp, t_template *template, t_gpointer *gp)
{
    int i, nitems = template->t_n;
    t_dataslot *datatypes = template->t_vec;
    for (i = 0; i < nitems; i++, datatypes++, wp++)
    {
        int type = datatypes->ds_type;
        if (type == DATA_FLOAT)
            wp->w_float = 0; 
        else if (type == DATA_SYMBOL)
            wp->w_symbol = &s_symbol;
        else if (type == DATA_ARRAY)
            wp->w_array = array_new(datatypes->ds_arraytemplate, gp);
        else if (type == DATA_TEXT)
            wp->w_buffer = buffer_new();
    }
}

void word_restore(t_word *wp, t_template *template,
    int argc, t_atom *argv)
{
    int i, nitems = template->t_n;
    t_dataslot *datatypes = template->t_vec;
    for (i = 0; i < nitems; i++, datatypes++, wp++)
    {
        int type = datatypes->ds_type;
        if (type == DATA_FLOAT)
        {
            t_float f;
            if (argc)
            {
                f =  atom_getFloat(argv);
                argv++, argc--;
            }
            else f = 0;
            wp->w_float = f; 
        }
        else if (type == DATA_SYMBOL)
        {
            t_symbol *s;
            if (argc)
            {
                s = atom_getSymbol(argv);
                argv++, argc--;
            }
            else s = &s_;
            wp->w_symbol = s;
        }
    }
    if (argc)
        post("warning: word_restore: extra arguments");
}

void word_free(t_word *wp, t_template *template)
{
    int i;
    t_dataslot *dt;
    for (dt = template->t_vec, i = 0; i < template->t_n; i++, dt++)
    {
        if (dt->ds_type == DATA_ARRAY)
            array_free(wp[i].w_array);
        else if (dt->ds_type == DATA_TEXT)
            buffer_free(wp[i].w_buffer);
    }
}

static int template_cancreate(t_template *template)
{
    int i, type, nitems = template->t_n;
    t_dataslot *datatypes = template->t_vec;
    t_template *elemtemplate;
    for (i = 0; i < nitems; i++, datatypes++)
        if (datatypes->ds_type == DATA_ARRAY &&
            (!(elemtemplate = template_findbyname(datatypes->ds_arraytemplate))
                || !template_cancreate(elemtemplate)))
    {
        post_error ("%s: no such template", datatypes->ds_arraytemplate->s_name);
        return (0);
    }
    return (1);
}

    /* make a new scalar and add to the glist.  We create a "gp" here which
    will be used for array items to point back here.  This gp doesn't do
    reference counting or "validation" updates though; the parent won't go away
    without the contained arrays going away too.  The "gp" is copied out
    by value in the word_init() routine so we can throw our copy away. */

t_scalar *scalar_new(t_glist *owner, t_symbol *templatesym)
{
    t_scalar *x;
    t_template *template;
    t_gpointer gp;
    gpointer_init(&gp);
    template = template_findbyname(templatesym);
    if (!template)
    {
        post_error ("scalar: couldn't find template %s", templatesym->s_name);
        return (0);
    }
    if (!template_cancreate(template))
        return (0);
    x = (t_scalar *)PD_MEMORY_GET(sizeof(t_scalar) +
        (template->t_n - 1) * sizeof(*x->sc_vector));
    x->sc_g.g_pd = scalar_class;
    x->sc_template = templatesym;
    gpointer_setglist(&gp, owner, x);
    word_init(x->sc_vector, template, &gp);
    return (x);
}

    /* Pd method to create a new scalar, add it to a glist, and initialize
    it from the message arguments. */

void canvas_makeScalar(t_glist *glist, t_symbol *classname, int argc, t_atom *argv)
{
    t_symbol *templatesym =
        canvas_makeBindSymbol(atom_getSymbolAtIndex(0, argc, argv));
    t_buffer *b;
    int natoms, nextmsg = 0;
    t_atom *vec;
    if (!template_findbyname(templatesym))
    {
        post_error ("%s: no such template",
            atom_getSymbolAtIndex(0, argc, argv)->s_name);
        return;
    }

    b = buffer_new();
    buffer_deserialize(b, argc, argv);
    natoms = buffer_size(b);
    vec = buffer_atoms(b);
    canvas_readscalar(glist, natoms, vec, &nextmsg, 0);
    buffer_free(b);
}

/* -------------------- widget behavior for scalar ------------ */
void scalar_getbasexy(t_scalar *x, t_float *basex, t_float *basey)
{
    t_template *template = template_findbyname(x->sc_template);
    *basex = template_getfloat(template, sym_x, x->sc_vector, 0);
    *basey = template_getfloat(template, sym_y, x->sc_vector, 0);
}

static void scalar_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_scalar *x = (t_scalar *)z;
    t_template *template = template_findbyname(x->sc_template);
    t_glist *templatecanvas = template_findcanvas(template);
    int x1 = PD_INT_MAX, x2 = -PD_INT_MAX, y1 = PD_INT_MAX, y2 = -PD_INT_MAX;
    t_gobj *y;
    t_float basex, basey;
    scalar_getbasexy(x, &basex, &basey);
        /* if someone deleted the template canvas, we're just a point */
    if (!templatecanvas)
    {
        x1 = x2 = canvas_valueToPositionX(owner, basex);
        y1 = y2 = canvas_valueToPositionY(owner, basey);
    }
    else
    {
        x1 = y1 = PD_INT_MAX;
        x2 = y2 = -PD_INT_MAX;
        for (y = templatecanvas->gl_graphics; y; y = y->g_next)
        {
            t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
            int nx1, ny1, nx2, ny2;
            if (!wb) continue;
            (*wb->w_fnParentGetRectangle)(y, owner,
                x->sc_vector, template, basex, basey,
                &nx1, &ny1, &nx2, &ny2);
            if (nx1 < x1) x1 = nx1;
            if (ny1 < y1) y1 = ny1;
            if (nx2 > x2) x2 = nx2;
            if (ny2 > y2) y2 = ny2;
        }
        if (x2 < x1 || y2 < y1)
            x1 = y1 = x2 = y2 = 0;
    }
    /* post("scalar x1 %d y1 %d x2 %d y2 %d", x1, y1, x2, y2); */
    *xp1 = x1;
    *yp1 = y1;
    *xp2 = x2;
    *yp2 = y2; 
}

static void scalar_drawselectrect(t_scalar *x, t_glist *glist, int state)
{
    if (state)
    {
        int x1, y1, x2, y2;
       
        scalar_getrect(&x->sc_g, glist, &x1, &y1, &x2, &y2);
        x1--; x2++; y1--; y2++;
        sys_vGui(".x%lx.c create line %d %d %d %d %d %d %d %d %d %d \
            -width 0 -fill blue -tags select%lx\n",
                canvas_getView(glist), x1, y1, x1, y2, x2, y2, x2, y1, x1, y1,
                x);
    }
    else
    {
        sys_vGui(".x%lx.c delete select%lx\n", canvas_getView(glist), x);
    }
}

static void scalar_select(t_gobj *z, t_glist *owner, int state)
{
    t_scalar *x = (t_scalar *)z;
    t_template *tmpl;
    t_symbol *templatesym = x->sc_template;
    t_atom at;
    t_gpointer gp;
    gpointer_init(&gp);
    gpointer_setglist(&gp, owner, x);
    SET_POINTER(&at, &gp);
    if (tmpl = template_findbyname(templatesym))
        template_notify(tmpl, (state ? sym_select : sym_deselect),
            1, &at);
    gpointer_unset(&gp);
    scalar_drawselectrect(x, owner, state);
}

static void scalar_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_scalar *x = (t_scalar *)z;
    t_symbol *templatesym = x->sc_template;
    t_template *template = template_findbyname(templatesym);
    t_symbol *zz;
    t_atom at[3];
    t_gpointer gp;
    int xonset, yonset, xtype, ytype, gotx, goty;
    if (!template)
    {
        post_error ("scalar: couldn't find template %s", templatesym->s_name);
        return;
    }
    gotx = template_find_field(template, sym_x, &xonset, &xtype, &zz);
    if (gotx && (xtype != DATA_FLOAT))
        gotx = 0;
    goty = template_find_field(template, sym_y, &yonset, &ytype, &zz);
    if (goty && (ytype != DATA_FLOAT))
        goty = 0;
    if (gotx)
        *(t_float *)(((char *)(x->sc_vector)) + xonset) += canvas_deltaPositionToValueX (glist, dx);
            // dx * (canvas_positionToValueX(glist, 1) - canvas_positionToValueX(glist, 0));
    if (goty)
        *(t_float *)(((char *)(x->sc_vector)) + yonset) += canvas_deltaPositionToValueY (glist, dy);
            // dy * (canvas_positionToValueY(glist, 1) - canvas_positionToValueY(glist, 0));
    gpointer_init(&gp);
    gpointer_setglist(&gp, glist, x);
    SET_POINTER(&at[0], &gp);
    SET_FLOAT(&at[1], (t_float)dx);
    SET_FLOAT(&at[2], (t_float)dy);
    template_notify(template, sym_displace, 2, at);
    scalar_redraw(x, glist);
}

static void scalar_activate(t_gobj *z, t_glist *owner, int state)
{
    /* post("scalar_activate %d", state); */
    /* later */
}

static void scalar_delete(t_gobj *z, t_glist *glist)
{
    /* nothing to do */
}

static void scalar_vis(t_gobj *z, t_glist *owner, int vis)
{
    t_scalar *x = (t_scalar *)z;
    t_template *template = template_findbyname(x->sc_template);
    t_glist *templatecanvas = template_findcanvas(template);
    t_gobj *y;
    t_float basex, basey;
    scalar_getbasexy(x, &basex, &basey);
        /* if we don't know how to draw it, make a small rectangle */
    if (!templatecanvas)
    {
        if (vis)
        {
            int x1 = canvas_valueToPositionX(owner, basex);
            int y1 = canvas_valueToPositionY(owner, basey);
            sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags scalar%lx\n",
                canvas_getView(owner), x1-1, y1-1, x1+1, y1+1, x);
        }
        else sys_vGui(".x%lx.c delete scalar%lx\n", canvas_getView(owner), x);
        return;
    }

    for (y = templatecanvas->gl_graphics; y; y = y->g_next)
    {
        t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
        if (!wb) continue;
        (*wb->w_fnParentVisibilityChanged)(y, owner, x->sc_vector, template, basex, basey, vis);
    }
    if (canvas_isObjectSelected(owner, &x->sc_g))
    {
        scalar_drawselectrect(x, owner, 0);
        scalar_drawselectrect(x, owner, 1);
    }
    interface_guiQueueRemove(x);
}

static void scalar_drawJob(t_gobj *client, t_glist *glist)
{
    scalar_vis(client, glist, 0);
    scalar_vis(client, glist, 1);
}

void scalar_redraw(t_scalar *x, t_glist *glist)
{
    if (canvas_isMapped(glist))
        interface_guiQueueAddIfNotAlreadyThere(x, glist, scalar_drawJob);
}

extern void template_notifyforscalar(t_template *template, t_glist *owner,
    t_scalar *sc, t_symbol *s, int argc, t_atom *argv);

int scalar_doclick(t_word *data, t_template *template, t_scalar *sc,
    t_array *ap, struct _glist *owner,
    t_float xloc, t_float yloc, int xpix, int ypix,
    int shift, int alt, int dbl, int doit)
{
    int hit = 0;
    t_glist *templatecanvas = template_findcanvas(template);
    t_gobj *y;
    t_atom at[2];
    t_float basex = template_getfloat(template, sym_x, data, 0);
    t_float basey = template_getfloat(template, sym_y, data, 0);
    SET_FLOAT(at, basex + xloc);
    SET_FLOAT(at+1, basey + yloc);
    if (doit)
        template_notifyforscalar(template, owner, 
            sc, sym_click, 2, at);
    for (y = templatecanvas->gl_graphics; y; y = y->g_next)
    {
        t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
        if (!wb) continue;
        if (hit = (*wb->w_fnParentClicked)(y, owner,
            data, template, sc, ap, basex + xloc, basey + yloc,
            xpix, ypix, shift, alt, dbl, doit))
                return (hit);
    }
    return (0);
}

static int scalar_click(t_gobj *z, struct _glist *owner,
    int xpix, int ypix, int shift, int ctrl, int alt, int dbl, int doit)
{
    t_scalar *x = (t_scalar *)z;
    t_template *template = template_findbyname(x->sc_template);
    return (scalar_doclick(x->sc_vector, template, x, 0,
        owner, 0, 0, xpix, ypix, shift, alt, dbl, doit));
}

static void scalar_save(t_gobj *z, t_buffer *b)
{
    t_scalar *x = (t_scalar *)z;
    t_buffer *b2 = buffer_new();
    t_atom a, *argv;
    int i, argc;
    canvas_writescalar(x->sc_template, x->sc_vector, b2, 0);
    buffer_vAppend(b, "ss", sym___hash__X, sym_scalar);
    buffer_serialize(b, b2);
    buffer_appendSemicolon(b);
    buffer_free(b2);
}

static void scalar_properties(t_gobj *z, struct _glist *owner)
{
    t_scalar *x = (t_scalar *)z;
    char *buf, buf2[80];
    int bufsize;
    t_buffer *b;
    canvas_deselectAll(owner);
    canvas_selectObject(owner, z);
    b = glist_writetobinbuf(owner, 0);
    buffer_toStringUnzeroed(b, &buf, &bufsize);
    buffer_free(b);
    buf = PD_MEMORY_RESIZE(buf, bufsize, bufsize+1);
    buf[bufsize] = 0;
    sprintf(buf2, "::ui_data::show %%s {");
    guistub_new((t_pd *)owner, x, buf2);
    sys_gui(buf);
    sys_gui("}\n");
    PD_MEMORY_FREE(buf);
}

static t_widgetbehavior scalar_widgetbehavior =
{
    scalar_getrect,
    scalar_displace,
    scalar_select,
    scalar_activate,
    scalar_delete,
    scalar_vis,
    scalar_click,
};

static void scalar_free(t_scalar *x)
{
    int i;
    t_dataslot *datatypes, *dt;
    t_symbol *templatesym = x->sc_template;
    t_template *template = template_findbyname(templatesym);
    if (!template)
    {
        post_error ("scalar: couldn't find template %s", templatesym->s_name);
        return;
    }
    word_free(x->sc_vector, template);
    guistub_destroyWithKey(x);
        /* the "size" field in the class is zero, so Pd doesn't try to free
        us automatically (see pd_free()) */
    PD_MEMORY_FREE(x);
}

/* ----------------- setup function ------------------- */

void g_scalar_setup(void)
{
    scalar_class = class_new(sym_scalar, 0, (t_method)scalar_free, 0,
        CLASS_GRAPHIC, 0);
    class_setWidgetBehavior(scalar_class, &scalar_widgetbehavior);
    class_setSaveFunction(scalar_class, scalar_save);
    class_setPropertiesFunction(scalar_class, scalar_properties);
}
