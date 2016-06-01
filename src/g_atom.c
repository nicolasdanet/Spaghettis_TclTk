
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_utf8.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_widgetbehavior gatom_widgetbehavior;

t_class *text_class;
t_class *gatom_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


#define ATOM_LABELLEFT 0
#define ATOM_LABELRIGHT 1
#define ATOM_LABELUP 2
#define ATOM_LABELDOWN 3

    /* prepend "-" as necessary to avoid empty strings, so we can
    use them in Pd messages.  A more complete solution would be
    to introduce some quoting mechanism; but then we'd be much more
    complicated. */
t_symbol *gatom_escapit(t_symbol *s)
{
    if (!*s->s_name)
        return (sym___dash__);
    else if (*s->s_name == '-')
    {
        char shmo[100];
        shmo[0] = '-';
        strncpy(shmo+1, s->s_name, 99);
        shmo[99] = 0;
        return (gensym (shmo));
    }
    else return (dollar_toHash(s));
}

    /* undo previous operation: strip leading "-" if found. */
static t_symbol *gatom_unescapit(t_symbol *s)
{
    if (*s->s_name == '-')
        return (gensym (s->s_name+1));
    else return (dollar_fromHash(s));
}

static void gatom_redraw(t_gobj *client, t_glist *glist)
{
    t_gatom *x = (t_gatom *)client;
    glist_retext(x->a_owner, &x->a_obj);
}

static void gatom_retext(t_gatom *x, int senditup)
{
    buffer_reset(x->a_obj.te_buffer);
    buffer_append(x->a_obj.te_buffer, 1, &x->a_atom);
    if (senditup && canvas_isMapped(x->a_owner))
        interface_guiQueueAddIfNotAlreadyThere(x, x->a_owner, gatom_redraw);
}

static void gatom_set(t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom oldatom = x->a_atom;
    int changed = 0;
    if (!argc) return;

    /*
    if (x->a_atom.a_type == A_FLOAT)
    {
        x->a_atom.a_w.w_float = atom_getFloat(argv);
        changed = ((x->a_atom.a_w.w_float != oldatom.a_w.w_float));
        if (isnan(x->a_atom.a_w.w_float) != isnan(oldatom.a_w.w_float))
            changed = 1;
    }*/
    
    if (x->a_atom.a_type == A_FLOAT)
        x->a_atom.a_w.w_float = atom_getFloat(argv),
            changed = (x->a_atom.a_w.w_float != oldatom.a_w.w_float);
    else if (x->a_atom.a_type == A_SYMBOL)
        x->a_atom.a_w.w_symbol = atom_getSymbol(argv),
            changed = (x->a_atom.a_w.w_symbol != oldatom.a_w.w_symbol);
    if (changed)
        gatom_retext(x, 1);
    x->a_string[0] = 0;
}

static void gatom_bang(t_gatom *x)
{
    if (x->a_atom.a_type == A_FLOAT)
    {
        if (x->a_obj.te_outlet)
            outlet_float(x->a_obj.te_outlet, x->a_atom.a_w.w_float);
        if (*x->a_send->s_name && x->a_send->s_thing)
        {
            if (x->a_unexpandedSend == x->a_unexpandedReceive)
                post_error ("%s: atom with same send/receive name (infinite loop)",
                        x->a_unexpandedSend->s_name);
            else pd_float(x->a_send->s_thing, x->a_atom.a_w.w_float);
        }
    }
    else if (x->a_atom.a_type == A_SYMBOL)
    {
        if (x->a_obj.te_outlet)
            outlet_symbol(x->a_obj.te_outlet, x->a_atom.a_w.w_symbol);
        if (*x->a_unexpandedSend->s_name && x->a_send->s_thing)
        {
            if (x->a_unexpandedSend == x->a_unexpandedReceive)
                post_error ("%s: atom with same send/receive name (infinite loop)",
                        x->a_unexpandedSend->s_name);
            else pd_symbol(x->a_send->s_thing, x->a_atom.a_w.w_symbol);
        }
    }
}

static void gatom_float(t_gatom *x, t_float f)
{
    t_atom at;
    SET_FLOAT(&at, f);
    gatom_set(x, 0, 1, &at);
    gatom_bang(x);
}

static void gatom_clipfloat(t_gatom *x, t_float f)
{
    if (x->a_lowRange != 0 || x->a_highRange != 0)
    {
        if (f < x->a_lowRange)
            f = x->a_lowRange;
        if (f > x->a_highRange)
            f = x->a_highRange;
    }
    gatom_float(x, f);
}

static void gatom_symbol(t_gatom *x, t_symbol *s)
{
    t_atom at;
    SET_SYMBOL(&at, s);
    gatom_set(x, 0, 1, &at);
    gatom_bang(x);
}

    /* We need a list method because, since there's both an "inlet" and a
    "nofirstin" flag, the standard list behavior gets confused. */
static void gatom_list(t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc)
        gatom_bang(x);
    else if (argv->a_type == A_FLOAT)
        gatom_float(x, argv->a_w.w_float);
    else if (argv->a_type == A_SYMBOL)
        gatom_symbol(x, argv->a_w.w_symbol);
    else { post_error ("gatom_list: need float or symbol"); }
}

static void gatom_motion(void *z, t_float dx, t_float dy, t_float modifier)
{
    t_gatom *x = (t_gatom *)z;
    if (dy == 0) return;
    if (x->a_atom.a_type == A_FLOAT)
    {
        if ((int)modifier & MODIFIER_SHIFT)
        {
            double nval = x->a_atom.a_w.w_float - 0.01 * dy;
            double trunc = 0.01 * (floor(100. * nval + 0.5));
            if (trunc < nval + 0.0001 && trunc > nval - 0.0001) nval = trunc;
            gatom_clipfloat(x, nval);
        }
        else
        {
            double nval = x->a_atom.a_w.w_float - dy;
            double trunc = 0.01 * (floor(100. * nval + 0.5));
            if (trunc < nval + 0.0001 && trunc > nval - 0.0001) nval = trunc;
            trunc = floor(nval + 0.5);
            if (trunc < nval + 0.001 && trunc > nval - 0.001) nval = trunc;
            gatom_clipfloat(x, nval);
        }
    }
}

static void gatom_key(void *z, t_float f)
{
    t_gatom *x = (t_gatom *)z;
    int c = f;
    int len = strlen(x->a_string);
    t_atom at;
    char sbuf[ATOM_BUFFER_SIZE + 4];
    if (c == 0)
    {
        /* we're being notified that no more keys will come for this grab */
        if (x->a_string[0])
            gatom_retext(x, 1);
        return;
    }
    else if (c == '\b')
    {
        if (len > 0)
        x->a_string[len-1] = 0;
        goto redraw;
    }
    else if (c == '\n')
    {
        if (x->a_atom.a_type == A_FLOAT)
            x->a_atom.a_w.w_float = atof(x->a_string);
        else if (x->a_atom.a_type == A_SYMBOL)
            x->a_atom.a_w.w_symbol = gensym (x->a_string);
        else { PD_BUG; }
        gatom_bang(x);
        gatom_retext(x, 1);
        x->a_string[0] = 0;
    }
    else if (len < (ATOM_BUFFER_SIZE-1))
    {
            /* for numbers, only let reasonable characters through */
        if ((x->a_atom.a_type == A_SYMBOL) ||
            (c >= '0' && c <= '9' || c == '.' || c == '-'
                || c == 'e' || c == 'E'))
        {
            /* the wchar could expand to up to 4 bytes, which
             * which might overrun our a_string;
             * therefore we first expand into a temporary buffer, 
             * and only if the resulting utf8 string fits into a_string
             * we apply it
             */
            char utf8[UTF8_MAXIMUM_BYTES];
            int utf8len = u8_wc_toutf8(utf8, c);
            if((len+utf8len) < (ATOM_BUFFER_SIZE-1))
            {
                int j=0;
                for(j=0; j<utf8len; j++)
                    x->a_string[len+j] = utf8[j];
                 
                x->a_string[len+utf8len] = 0;
            }
            goto redraw;
        }
    }
    return;
redraw:
        /* LATER figure out how to avoid creating all these symbols! */
    sprintf(sbuf, "%s...", x->a_string);
    SET_SYMBOL(&at, gensym (sbuf));
    buffer_reset(x->a_obj.te_buffer);
    buffer_append(x->a_obj.te_buffer, 1, &at);
    glist_retext(x->a_owner, &x->a_obj);
}

void gatom_click(t_gatom *x, t_float xpos, t_float ypos, t_float shift, t_float ctrl, t_float alt)
{
    if (x->a_obj.te_width == 1)
    {
        if (x->a_atom.a_type == A_FLOAT)
            gatom_float(x, (x->a_atom.a_w.w_float == 0));
    }
    else
    {
        if (alt)
        {
            if (x->a_atom.a_type != A_FLOAT) return;
            if (x->a_atom.a_w.w_float != 0)
            {
                x->a_toggledValue = x->a_atom.a_w.w_float;
                gatom_float(x, 0);
                return;
            }
            else gatom_float(x, x->a_toggledValue);
        }
        x->a_string[0] = 0;
        glist_grab(x->a_owner, &x->a_obj.te_g, (t_motionfn)gatom_motion, gatom_key,
            xpos, ypos);
    }
}

    /* message back from dialog window */
static void gatom_param(t_gatom *x, t_symbol *sel, int argc, t_atom *argv)
{
    t_float width = atom_getFloatAtIndex(0, argc, argv);
    t_float draglo = atom_getFloatAtIndex(1, argc, argv);
    t_float draghi = atom_getFloatAtIndex(2, argc, argv);
    t_symbol *symto = gatom_unescapit(atom_getSymbolAtIndex(3, argc, argv));
    t_symbol *symfrom = gatom_unescapit(atom_getSymbolAtIndex(4, argc, argv));
    t_symbol *label = gatom_unescapit(atom_getSymbolAtIndex(5, argc, argv));
    t_float wherelabel = atom_getFloatAtIndex(6, argc, argv);

    gobj_visibilityChanged(&x->a_obj.te_g, x->a_owner, 0);
    if (!*symfrom->s_name && *x->a_unexpandedReceive->s_name)
        inlet_new(&x->a_obj, &x->a_obj.te_g.g_pd, 0, 0);
    else if (*symfrom->s_name && !*x->a_unexpandedReceive->s_name && x->a_obj.te_inlet)
    {
        canvas_deleteLinesByInlets(x->a_owner, &x->a_obj,
            x->a_obj.te_inlet, 0);
        inlet_free(x->a_obj.te_inlet);
    }
    if (!*symto->s_name && *x->a_unexpandedSend->s_name)
        outlet_new(&x->a_obj, 0);
    else if (*symto->s_name && !*x->a_unexpandedSend->s_name && x->a_obj.te_outlet)
    {
        canvas_deleteLinesByInlets(x->a_owner, &x->a_obj,
            0, x->a_obj.te_outlet);
        outlet_free(x->a_obj.te_outlet);
    }
    if (draglo >= draghi)
        draglo = draghi = 0;
    x->a_lowRange = draglo;
    x->a_highRange = draghi;
    if (width < 0)
        width = 4;
    else if (width > 80)
        width = 80;
    x->a_obj.te_width = width;
    x->a_position = ((int)wherelabel & 3);
    x->a_label = label;
    if (*x->a_unexpandedReceive->s_name)
        pd_unbind(&x->a_obj.te_g.g_pd,
            canvas_expandDollar(x->a_owner, x->a_unexpandedReceive));
    x->a_unexpandedReceive = symfrom;
    if (*x->a_unexpandedReceive->s_name)
        pd_bind(&x->a_obj.te_g.g_pd,
            canvas_expandDollar(x->a_owner, x->a_unexpandedReceive));
    x->a_unexpandedSend = symto;
    x->a_send = canvas_expandDollar(x->a_owner, x->a_unexpandedSend);
    gobj_visibilityChanged(&x->a_obj.te_g, x->a_owner, 1);
    canvas_dirty(x->a_owner, 1);

    /* glist_retext(x->a_owner, &x->a_obj); */
}

    /* ---------------- gatom-specific widget functions --------------- */
static void gatom_getwherelabel(t_gatom *x, t_glist *glist, int *xp, int *yp)
{
    int x1, y1, x2, y2, width, height;
    text_getrect(&x->a_obj.te_g, glist, &x1, &y1, &x2, &y2);
    width = x2 - x1;
    height = y2 - y1;
    if (x->a_position == ATOM_LABELLEFT)
    {
        *xp = x1 - 3 -
            strlen(canvas_expandDollar(x->a_owner, x->a_label)->s_name) *
            (int)font_getHostFontWidth(canvas_getFontSize(glist));
        *yp = y1 + 2;
    }
    else if (x->a_position == ATOM_LABELRIGHT)
    {
        *xp = x2 + 2;
        *yp = y1 + 2;
    }
    else if (x->a_position == ATOM_LABELUP)
    {
        *xp = x1 - 1;
        *yp = y1 - 1 - (int)font_getHostFontHeight(canvas_getFontSize(glist));;
    }
    else
    {
        *xp = x1 - 1;
        *yp = y2 + 3;
    }
}

void gatom_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_gatom *x = (t_gatom*)z;
    text_displace(z, glist, dx, dy);
    sys_vGui(".x%lx.c move %lx.l %d %d\n", canvas_getView(glist), 
        x, dx, dy);
}

void gatom_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_gatom *x = (t_gatom*)z;
    text_vis(z, glist, vis);
    if (*x->a_label->s_name)
    {
        if (vis)
        {
            int x1, y1;
            gatom_getwherelabel(x, glist, &x1, &y1);
            sys_vGui("::ui_box::newText .x%lx.c {%lx.l label text} %f %f {%s} %d %s\n",
                canvas_getView(glist), x,
                (double)x1, (double)y1,
                canvas_expandDollar(x->a_owner, x->a_label)->s_name,
                font_getHostFontSize(canvas_getFontSize(glist)),
                "black");
        }
        else sys_vGui(".x%lx.c delete %lx.l\n", canvas_getView(glist), x);
    }
    if (!vis)
        interface_guiQueueRemove(x);
}

void canvas_atom(t_glist *gl, t_atomtype type,
    t_symbol *s, int argc, t_atom *argv)
{
    t_gatom *x = (t_gatom *)pd_new(gatom_class);
    t_atom at;
    x->a_obj.te_width = 0;                        /* don't know it yet. */
    x->a_obj.te_type = TYPE_ATOM;
    x->a_obj.te_buffer = buffer_new();
    x->a_owner = gl;
    x->a_atom.a_type = type;
    x->a_toggledValue = 1;
    x->a_lowRange = 0;
    x->a_highRange = 0;
    x->a_position = 0;
    x->a_label = &s_;
    x->a_unexpandedReceive = &s_;
    x->a_unexpandedSend = x->a_send = &s_;
    if (type == A_FLOAT)
    {
        x->a_atom.a_w.w_float = 0;
        x->a_obj.te_width = 5;
        SET_FLOAT(&at, 0);
    }
    else
    {
        x->a_atom.a_w.w_symbol = &s_symbol;
        x->a_obj.te_width = 10;
        SET_SYMBOL(&at, &s_symbol);
    }
    buffer_append(x->a_obj.te_buffer, 1, &at);
    if (argc > 1)
        /* create from file. x, y, width, low-range, high-range, flags,
            label, receive-name, send-name */
    {
        x->a_obj.te_xCoordinate = atom_getFloatAtIndex(0, argc, argv);
        x->a_obj.te_yCoordinate = atom_getFloatAtIndex(1, argc, argv);
        x->a_obj.te_width = (t_int)atom_getFloatAtIndex(2, argc, argv);
            /* sanity check because some very old patches have trash in this
            field... remove this in 2003 or so: */
        if (x->a_obj.te_width < 0 || x->a_obj.te_width > 500)
            x->a_obj.te_width = 4;
        x->a_lowRange = atom_getFloatAtIndex(3, argc, argv);
        x->a_highRange = atom_getFloatAtIndex(4, argc, argv);
        x->a_position = (((int)atom_getFloatAtIndex(5, argc, argv)) & 3);
        x->a_label = gatom_unescapit(atom_getSymbolAtIndex(6, argc, argv));
        x->a_unexpandedReceive = gatom_unescapit(atom_getSymbolAtIndex(7, argc, argv));
        if (*x->a_unexpandedReceive->s_name)
            pd_bind(&x->a_obj.te_g.g_pd,
                canvas_expandDollar(x->a_owner, x->a_unexpandedReceive));

        x->a_unexpandedSend = gatom_unescapit(atom_getSymbolAtIndex(8, argc, argv));
        x->a_send = canvas_expandDollar(x->a_owner, x->a_unexpandedSend);
        if (x->a_unexpandedSend == &s_)
            outlet_new(&x->a_obj,
                x->a_atom.a_type == A_FLOAT ? &s_float: &s_symbol);
        if (x->a_unexpandedReceive == &s_)
            inlet_new(&x->a_obj, &x->a_obj.te_g.g_pd, 0, 0);
        glist_add(gl, &x->a_obj.te_g);
    }
    else
    {
        int connectme, xpix, ypix, indx, nobj;
        canvas_howputnew(gl, &connectme, &xpix, &ypix, &indx, &nobj);
        outlet_new(&x->a_obj,
            x->a_atom.a_type == A_FLOAT ? &s_float: &s_symbol);
        inlet_new(&x->a_obj, &x->a_obj.te_g.g_pd, 0, 0);
        pd_vMessage(&gl->gl_obj.te_g.g_pd, sym_editmode, "i", 1);
        x->a_obj.te_xCoordinate = xpix;
        x->a_obj.te_yCoordinate = ypix;
        glist_add(gl, &x->a_obj.te_g);
        canvas_deselectAll(gl);
        canvas_selectObject(gl, &x->a_obj.te_g);
        if (connectme) {
            canvas_connect(gl, indx, 0, nobj, 0);
        } else { 
           // canvas_startmotion(canvas_getView(gl));
        }
    }
}

void canvas_floatatom(t_glist *gl, t_symbol *s, int argc, t_atom *argv)
{
    canvas_atom(gl, A_FLOAT, s, argc, argv);
}

void canvas_symbolatom(t_glist *gl, t_symbol *s, int argc, t_atom *argv)
{
    canvas_atom(gl, A_SYMBOL, s, argc, argv);
}

static void gatom_free(t_gatom *x)
{
    if (*x->a_unexpandedReceive->s_name)
        pd_unbind(&x->a_obj.te_g.g_pd,
            canvas_expandDollar(x->a_owner, x->a_unexpandedReceive));
    guistub_destroyWithKey(x);
}

static void gatom_properties(t_gobj *z, t_glist *owner)
{
    t_gatom *x = (t_gatom *)z;
    char buf[200];
    sprintf(buf, "::ui_atom::show %%s %d %g %g {%s} {%s} {%s} %d\n",
        x->a_obj.te_width, x->a_lowRange, x->a_highRange,
                gatom_escapit(x->a_unexpandedSend)->s_name,
                gatom_escapit(x->a_unexpandedReceive)->s_name,
                gatom_escapit(x->a_label)->s_name, 
                x->a_position);
    guistub_new(&x->a_obj.te_g.g_pd, x, buf);
}

    /* this gets called when a message gets sent to an object whose creation
    failed, presumably because of loading a patch with a missing extern or
    abstraction */
static void text_anything(t_object *x, t_symbol *s, int argc, t_atom *argv)
{
}

void text_setup(void)
{
    text_class = class_new (sym_text, 0, 0, sizeof(t_object),
        CLASS_NOINLET | CLASS_BOX, 0);
    class_addAnything(text_class, text_anything);

    gatom_class = class_new(sym_gatom, 0, (t_method)gatom_free,
        sizeof(t_gatom), CLASS_NOINLET | CLASS_BOX, 0);
    class_addBang(gatom_class, gatom_bang);
    class_addFloat(gatom_class, gatom_float);
    class_addSymbol(gatom_class, gatom_symbol);
    class_addList(gatom_class, gatom_list);
    class_addMethod(gatom_class, (t_method)gatom_set, sym_set,
        A_GIMME, 0);
    class_addClick (gatom_class, gatom_click);
    
    /* class_addMethod(gatom_class, (t_method)gatom_click, sym_click,
        A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0); */
    class_addMethod(gatom_class, (t_method)gatom_param, sym_param,   /* LEGACY !!! */
        A_GIMME, 0);
    class_setWidgetBehavior(gatom_class, &gatom_widgetbehavior);
    class_setPropertiesFunction(gatom_class, gatom_properties);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
