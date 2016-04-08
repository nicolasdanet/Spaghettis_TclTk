
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"
#include "g_iem.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define IEM_DIAL_COLOR_EDITED   0xff0000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_key (void *z, t_float fkey);
static void dial_draw_update (t_gobj *client, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_widgetbehavior dial_widgetbehavior;

static t_class *dial_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void dial_clip(t_dial *x)
{
    if(x->x_value < x->x_minimum)
        x->x_value = x->x_minimum;
    if(x->x_value > x->x_maximum)
        x->x_value = x->x_maximum;
}

void dial_calc_fontwidth(t_dial *x)
{
    int w, f=31;

    if(x->x_gui.iem_fontStyle == 1)
        f = 27;
    else if(x->x_gui.iem_fontStyle == 2)
        f = 25;

    w = x->x_gui.iem_fontSize * f * x->x_gui.iem_width;
    w /= 36;
    x->x_digitsWidth = w + (x->x_gui.iem_height / 2) + 4;
}

void dial_ftoa(t_dial *x)
{
    double f=x->x_value;
    int bufsize, is_exp=0, i, idecimal;

    sprintf(x->x_t, "%g", f);
    bufsize = strlen(x->x_t);
    if(bufsize >= 5)/* if it is in exponential mode */
    {
        i = bufsize - 4;
        if((x->x_t[i] == 'e') || (x->x_t[i] == 'E'))
            is_exp = 1;
    }
    if(bufsize > x->x_gui.iem_width)/* if to reduce */
    {
        if(is_exp)
        {
            if(x->x_gui.iem_width <= 5)
            {
                x->x_t[0] = (f < 0.0 ? '-' : '+');
                x->x_t[1] = 0;
            }
            i = bufsize - 4;
            for(idecimal=0; idecimal < i; idecimal++)
                if(x->x_t[idecimal] == '.')
                    break;
            if(idecimal > (x->x_gui.iem_width - 4))
            {
                x->x_t[0] = (f < 0.0 ? '-' : '+');
                x->x_t[1] = 0;
            }
            else
            {
                int new_exp_index=x->x_gui.iem_width-4, old_exp_index=bufsize-4;

                for(i=0; i < 4; i++, new_exp_index++, old_exp_index++)
                    x->x_t[new_exp_index] = x->x_t[old_exp_index];
                x->x_t[x->x_gui.iem_width] = 0;
            }

        }
        else
        {
            for(idecimal=0; idecimal < bufsize; idecimal++)
                if(x->x_t[idecimal] == '.')
                    break;
            if(idecimal > x->x_gui.iem_width)
            {
                x->x_t[0] = (f < 0.0 ? '-' : '+');
                x->x_t[1] = 0;
            }
            else
                x->x_t[x->x_gui.iem_width] = 0;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_draw_update(t_gobj *client, t_glist *glist)
{
    t_dial *x = (t_dial *)client;
    if (glist_isvisible(glist))
    {
        if(x->x_hasChanged)
        {
            if(x->x_t[0])
            {
                char *cp=x->x_t;
                int sl = strlen(x->x_t);

                x->x_t[sl] = '>';
                x->x_t[sl+1] = 0;
                if(sl >= x->x_gui.iem_width)
                    cp += sl - x->x_gui.iem_width + 1;
                sys_vGui(
                    ".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s} \n",
                         glist_getcanvas(glist), x, IEM_DIAL_COLOR_EDITED, cp);
                x->x_t[sl] = 0;
            }
            else
            {
                dial_ftoa(x);
                sys_vGui(
                    ".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s} \n",
                    glist_getcanvas(glist), x, IEM_DIAL_COLOR_EDITED, x->x_t);
                x->x_t[0] = 0;
            }
        }
        else
        {
            dial_ftoa(x);
            sys_vGui(
                ".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x -text {%s} \n",
                glist_getcanvas(glist), x,
                x->x_gui.iem_isSelected?
                    IEM_COLOR_SELECTED:x->x_gui.iem_colorForeground,
                x->x_t);
            x->x_t[0] = 0;
        }
    }
}

static void dial_draw_new(t_dial *x, t_glist *glist)
{
    int half=x->x_gui.iem_height/2, d=1+x->x_gui.iem_height/34;
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(
".x%lx.c create polygon %d %d %d %d %d %d %d %d %d %d -outline #%6.6x \
-fill #%6.6x -tags %lxBASE1\n",
             canvas, xpos, ypos,
             xpos + x->x_digitsWidth-4, ypos,
             xpos + x->x_digitsWidth, ypos+4,
             xpos + x->x_digitsWidth, ypos + x->x_gui.iem_height,
             xpos, ypos + x->x_gui.iem_height,
             IEM_COLOR_NORMAL, x->x_gui.iem_colorBackground, x);
    sys_vGui(
        ".x%lx.c create line %d %d %d %d %d %d -fill #%6.6x -tags %lxBASE2\n",
        canvas, xpos, ypos,
        xpos + half, ypos + half,
        xpos, ypos + x->x_gui.iem_height,
        x->x_gui.iem_colorForeground, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
        -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
        canvas, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY,
        strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
        x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);
    dial_ftoa(x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
        -font [::getFont %d] -fill #%6.6x -tags %lxNUMBER\n",
        canvas, xpos+half+2, ypos+half+d,
        x->x_t, x->x_gui.iem_fontSize,
        x->x_gui.iem_colorForeground, x);

        /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas,
             xpos, ypos + x->x_gui.iem_height-1,
             xpos+INLETS_WIDTH, ypos + x->x_gui.iem_height,
             x, 0);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos, ypos,
             xpos+INLETS_WIDTH, ypos+1,
             x, 0);*/
}

static void dial_draw_move(t_dial *x, t_glist *glist)
{
    int half = x->x_gui.iem_height/2, d=1+x->x_gui.iem_height/34;
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c coords %lxBASE1 %d %d %d %d %d %d %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + x->x_digitsWidth-4, ypos,
             xpos + x->x_digitsWidth, ypos+4,
             xpos + x->x_digitsWidth, ypos + x->x_gui.iem_height,
             xpos, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxBASE2 %d %d %d %d %d %d\n",
             canvas, x, xpos, ypos,
             xpos + half, ypos + half,
             xpos, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY);
    sys_vGui(".x%lx.c coords %lxNUMBER %d %d\n",
             canvas, x, xpos+half+2, ypos+half+d);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos + x->x_gui.iem_height-1,
             xpos+INLETS_WIDTH, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos, ypos,
             xpos+INLETS_WIDTH, ypos+1);*/
}

static void dial_draw_erase(t_dial* x,t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE1\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxBASE2\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxNUMBER\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void dial_draw_config(t_dial* x,t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    sys_vGui(".x%lx.c itemconfigure %lxNUMBER -font [::getFont %d] -fill #%6.6x \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorForeground);
    sys_vGui(".x%lx.c itemconfigure %lxBASE1 -fill #%6.6x\n", canvas,
             x, x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxBASE2 -fill #%6.6x\n", canvas,
             x, x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorForeground);
}

static void dial_draw_io(t_dial* x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas,
        xpos, ypos + x->x_gui.iem_height-1,
        xpos+INLETS_WIDTH, ypos + x->x_gui.iem_height,
        x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas,
        xpos, ypos,
        xpos+INLETS_WIDTH, ypos+1,
        x, 0);*/
}

static void dial_draw_select(t_dial *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    if(x->x_gui.iem_isSelected)
    {
        if(x->x_hasChanged)
        {
            x->x_hasChanged = 0;
            x->x_t[0] = 0;
            interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        }
        sys_vGui(".x%lx.c itemconfigure %lxBASE1 -outline #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxBASE2 -fill #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x\n",
            canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE1 -outline #%6.6x\n",
            canvas, x, IEM_COLOR_NORMAL);
        sys_vGui(".x%lx.c itemconfigure %lxBASE2 -fill #%6.6x\n",
            canvas, x, x->x_gui.iem_colorForeground);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n",
            canvas, x, x->x_gui.iem_colorLabel);
        sys_vGui(".x%lx.c itemconfigure %lxNUMBER -fill #%6.6x\n",
            canvas, x, x->x_gui.iem_colorForeground);
    }
}

void dial_draw(t_dial *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        interface_guiQueueAddIfNotAlreadyThere(x, glist, dial_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        dial_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        dial_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        dial_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        dial_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        dial_draw_config(x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dial_getrect(t_gobj *z, t_glist *glist,
                              int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_dial* x = (t_dial*)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist);
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist);
    *xp2 = *xp1 + x->x_digitsWidth;
    *yp2 = *yp1 + x->x_gui.iem_height;
}

static void dial_save(t_gobj *z, t_buffer *b)
{
    t_dial *x = (t_dial *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iemgui_serialize(&x->x_gui, srl, bflcol);
    if(x->x_hasChanged)
    {
        x->x_hasChanged = 0;
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
    }
    buffer_vAppend(b, "ssiisiiffiisssiiiiiiifi", gensym ("#X"),gensym ("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate, (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym ("nbx"), x->x_gui.iem_width, x->x_gui.iem_height,
                (t_float)x->x_minimum, (t_float)x->x_maximum,
                x->x_isLogarithmic, iemgui_serializeLoadbang(&x->x_gui),
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_serializeFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2],
                x->x_value, x->x_logarithmSteps);
    buffer_vAppend(b, ";");
}

int dial_check_minmax(t_dial *x, double min, double max)
{
    int ret=0;

    if(x->x_isLogarithmic)
    {
        if((min == 0.0)&&(max == 0.0))
            max = 1.0;
        if(max > 0.0)
        {
            if(min <= 0.0)
                min = 0.01*max;
        }
        else
        {
            if(min > 0.0)
                max = 0.01*min;
        }
    }
    x->x_minimum = min;
    x->x_maximum = max;
    if(x->x_value < x->x_minimum)
    {
        x->x_value = x->x_minimum;
        ret = 1;
    }
    if(x->x_value > x->x_maximum)
    {
        x->x_value = x->x_maximum;
        ret = 1;
    }
    if(x->x_isLogarithmic)
        x->x_k = exp(log(x->x_maximum/x->x_minimum)/(double)(x->x_logarithmSteps));
    else
        x->x_k = 1.0;
    return(ret);
}

static void dial_properties(t_gobj *z, t_glist *owner)
{
    t_dial *x = (t_dial *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_serializeNames(&x->x_gui, srl);
    if(x->x_hasChanged)
    {
        x->x_hasChanged = 0;
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);

    }
    sprintf(buf, "::ui_iem::create %%s Dial \
            %d %d Digits %d %d Size \
            %g {Value Low} %g {Value High} \
            %d Linear Logarithmic \
            %d \
            %d 1024 {Logarithmic Steps} \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.iem_width, 1, x->x_gui.iem_height, 8,
            x->x_minimum, x->x_maximum,
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            x->x_logarithmSteps, /*no multi, but iem-characteristic*/
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, 0xffffff & x->x_gui.iem_colorForeground,
                0xffffff & x->x_gui.iem_colorLabel);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

static void dial_bang(t_dial *x)
{
    outlet_float(x->x_gui.iem_obj.te_outlet, x->x_value);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_float(x->x_gui.iem_send->s_thing, x->x_value);
}

static void dial_dialog(t_dial *x, t_symbol *s, int argc,
    t_atom *argv)
{
    int w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
    double min = (double)atom_getFloatAtIndex(2, argc, argv);
    double max = (double)atom_getFloatAtIndex(3, argc, argv);
    int lilo = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
    int log_height = (int)(t_int)atom_getFloatAtIndex(6, argc, argv);

    if(lilo != 0) lilo = 1;
    x->x_isLogarithmic = lilo;
    iemgui_fromDialog(&x->x_gui, argc, argv);
    if(w < 1)
        w = 1;
    x->x_gui.iem_width = w;
    if(h < 8)
        h = 8;
    x->x_gui.iem_height = h;
    if(log_height < 10)
        log_height = 10;
    x->x_logarithmSteps = log_height;
    dial_calc_fontwidth(x);
    /*if(dial_check_minmax(x, min, max))
     dial_bang(x);*/
    dial_check_minmax(x, min, max);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, cast_object (x));
}

static void dial_motion(t_dial *x, t_float dx, t_float dy)
{
    double k2=1.0;

    if(x->x_isAccurateMoving)
        k2 = 0.01;
    if(x->x_isLogarithmic)
        x->x_value *= pow(x->x_k, -k2*dy);
    else
        x->x_value -= k2*dy;
    dial_clip(x);
    interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
    dial_bang(x);
}

static void dial_click(t_dial *x, t_float xpos, t_float ypos,
                            t_float shift, t_float ctrl, t_float alt)
{
    glist_grab(x->x_gui.iem_glist, &x->x_gui.iem_obj.te_g,
        (t_glistmotionfn)dial_motion, dial_key, xpos, ypos);
}

static int dial_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_dial* x = (t_dial *)z;

    if(doit)
    {
        dial_click( x, (t_float)xpix, (t_float)ypix,
            (t_float)shift, 0, (t_float)alt);
        if(shift)
            x->x_isAccurateMoving = 1;
        else
            x->x_isAccurateMoving = 0;
        if(!x->x_hasChanged)
        {
            x->x_hasChanged = 1;
            x->x_t[0] = 0;
        }
        else
        {
            x->x_hasChanged = 0;
            x->x_t[0] = 0;
            interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        }
    }
    return (1);
}

static void dial_set(t_dial *x, t_float f)
{
    if(x->x_value != f)
    {
        x->x_value = f;
        dial_clip(x);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
    }
}

static void dial_log_height(t_dial *x, t_float lh)
{
    if(lh < 10.0)
        lh = 10.0;
    x->x_logarithmSteps = (int)lh;
    if(x->x_isLogarithmic)
        x->x_k = exp(log(x->x_maximum/x->x_minimum)/(double)(x->x_logarithmSteps));
    else
        x->x_k = 1.0;
    
}

static void dial_float(t_dial *x, t_float f)
{
    dial_set(x, f);
    if(x->x_gui.iem_goThrough)
        dial_bang(x);
}

static void dial_size(t_dial *x, t_symbol *s, int ac, t_atom *av)
{
    int h, w;

    w = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    if(w < 1)
        w = 1;
    x->x_gui.iem_width = w;
    if(ac > 1)
    {
        h = (int)(t_int)atom_getFloatAtIndex(1, ac, av);
        if(h < 8)
            h = 8;
        x->x_gui.iem_height = h;
    }
    dial_calc_fontwidth(x);
    iemgui_boxChanged((void *)x, &x->x_gui);
}

static void dial_delta(t_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void dial_pos(t_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void dial_range(t_dial *x, t_symbol *s, int ac, t_atom *av)
{
    if(dial_check_minmax(x, (double)atom_getFloatAtIndex(0, ac, av),
                              (double)atom_getFloatAtIndex(1, ac, av)))
    {
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        /*dial_bang(x);*/
    }
}

static void dial_color(t_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setColor((void *)x, &x->x_gui, s, ac, av);}

static void dial_send(t_dial *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void dial_receive(t_dial *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void dial_label(t_dial *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

static void dial_label_pos(t_dial *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void dial_label_font(t_dial *x,
    t_symbol *s, int ac, t_atom *av)
{
    int f = (int)(t_int)atom_getFloatAtIndex(1, ac, av);

    if(f < 4)
        f = 4;
    x->x_gui.iem_fontSize = f;
    f = (int)(t_int)atom_getFloatAtIndex(0, ac, av);
    if((f < 0) || (f > 2))
        f = 0;
    x->x_gui.iem_fontStyle = f;
    dial_calc_fontwidth(x);
    iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);
}

static void dial_log(t_dial *x)
{
    x->x_isLogarithmic = 1;
    if(dial_check_minmax(x, x->x_minimum, x->x_maximum))
    {
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        /*dial_bang(x);*/
    }
}

static void dial_lin(t_dial *x)
{
    x->x_isLogarithmic = 0;
}

static void dial_init(t_dial *x, t_float f)
{
    x->x_gui.iem_loadbang = (f==0.0)?0:1;
}

static void dial_loadbang(t_dial *x)
{
    if(x->x_gui.iem_loadbang)
    {
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        dial_bang(x);
    }
}

static void dial_key(void *z, t_float fkey)
{
    t_dial *x = z;
    char c=fkey;
    char buf[3];
    buf[1] = 0;

    if (c == 0)
    {
        x->x_hasChanged = 0;
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        return;
    }
    if(((c>='0')&&(c<='9'))||(c=='.')||(c=='-')||
        (c=='e')||(c=='+')||(c=='E'))
    {
        if(strlen(x->x_t) < (IEM_DIAL_BUFFER_LENGTH-2))
        {
            buf[0] = c;
            strcat(x->x_t, buf);
            interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
        }
    }
    else if((c=='\b')||(c==127))
    {
        int sl=strlen(x->x_t)-1;

        if(sl < 0)
            sl = 0;
        x->x_t[sl] = 0;
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
    }
    else if((c=='\n')||(c==13))
    {
        x->x_value = atof(x->x_t);
        x->x_t[0] = 0;
        x->x_hasChanged = 0;
        dial_clip(x);
        dial_bang(x);
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, dial_draw_update);
    }
}

static void dial_list(t_dial *x, t_symbol *s, int ac, t_atom *av)
{
    if (IS_FLOAT(av + 0))
    {
        dial_set(x, atom_getFloatAtIndex(0, ac, av));
        dial_bang(x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *dial_new(t_symbol *s, int argc, t_atom *argv)
{
    t_dial *x = (t_dial *)pd_new(dial_class);
    int bflcol[]={-262144, -1, -1};
    int w=5, h=14;
    int lilo=0, f=0, ldx=0, ldy=-8;
    int fs=10;
    int log_height=256;
    double min=-1.0e+37, max=1.0e+37,v=0.0;
    char str[144];

    if((argc >= 17)&&IS_FLOAT(argv + 0)&&IS_FLOAT(argv + 1)
       &&IS_FLOAT(argv + 2)&&IS_FLOAT(argv + 3)
       &&IS_FLOAT(argv + 4)&&IS_FLOAT(argv + 5)
       &&(IS_SYMBOL(argv + 6)||IS_FLOAT(argv + 6))
       &&(IS_SYMBOL(argv + 7)||IS_FLOAT(argv + 7))
       &&(IS_SYMBOL(argv + 8)||IS_FLOAT(argv + 8))
       &&IS_FLOAT(argv + 9)&&IS_FLOAT(argv + 10)
       &&IS_FLOAT(argv + 11)&&IS_FLOAT(argv + 12)&&IS_FLOAT(argv + 13)
       &&IS_FLOAT(argv + 14)&&IS_FLOAT(argv + 15)&&IS_FLOAT(argv + 16))
    {
        w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        min = (double)atom_getFloatAtIndex(2, argc, argv);
        max = (double)atom_getFloatAtIndex(3, argc, argv);
        lilo = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
        iemgui_deserializeLoadbang(&x->x_gui, (t_int)atom_getFloatAtIndex(5, argc, argv));
        iemgui_deserializeNamesByIndex(&x->x_gui, 6, argv);
        ldx = (int)(t_int)atom_getFloatAtIndex(9, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
        iemgui_deserializeFontStyle(&x->x_gui, (t_int)atom_getFloatAtIndex(11, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(12, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(13, argc, argv);
        bflcol[1] = (int)(t_int)atom_getFloatAtIndex(14, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(15, argc, argv);
        v = atom_getFloatAtIndex(16, argc, argv);
    }
    else iemgui_deserializeNamesByIndex(&x->x_gui, 6, 0);
    if((argc == 18)&&IS_FLOAT(argv + 17))
    {
        log_height = (int)(t_int)atom_getFloatAtIndex(17, argc, argv);
    }
    x->x_gui.iem_draw = (t_iemfn)dial_draw;
    x->x_gui.iem_canSend = 1;
    x->x_gui.iem_canReceive = 1;
    x->x_gui.iem_glist = (t_glist *)canvas_getcurrent();
    if(x->x_gui.iem_loadbang)
        x->x_value = v;
    else
        x->x_value = 0.0;
    if(lilo != 0) lilo = 1;
    x->x_isLogarithmic = lilo;
    if(log_height < 10)
        log_height = 10;
    x->x_logarithmSteps = log_height;
    if (!strcmp(x->x_gui.iem_send->s_name, "empty"))
        x->x_gui.iem_canSend = 0;
    if (!strcmp(x->x_gui.iem_receive->s_name, "empty"))
        x->x_gui.iem_canReceive = 0;

    if (x->x_gui.iem_canReceive)
        pd_bind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    x->x_gui.iem_labelX = ldx;
    x->x_gui.iem_labelY = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.iem_fontSize = fs;
    if(w < 1)
        w = 1;
    x->x_gui.iem_width = w;
    if(h < 8)
        h = 8;
    x->x_gui.iem_height = h;
    x->x_t[0] = 0;
    dial_calc_fontwidth(x);
    dial_check_minmax(x, min, max);
    iemgui_deserializeColors(&x->x_gui, bflcol);
    iemgui_checkSendReceiveLoop(&x->x_gui);
    x->x_hasChanged = 0;
    outlet_new(&x->x_gui.iem_obj, &s_float);
    return (x);
}

static void dial_free(t_dial *x)
{
    if(x->x_gui.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    gfxstub_deleteforkey(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dial_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("nbx"),
        (t_newmethod)dial_new,
        (t_method)dial_free,
        sizeof (t_dial),
        CLASS_DEFAULT,
        A_GIMME,
        A_NULL);
        
    class_addBang (c,dial_bang);
    class_addFloat (c,dial_float);
    class_addList (c, dial_list);
    class_addMethod(c, (t_method)dial_click,
        gensym ("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(c, (t_method)dial_motion,
        gensym ("motion"), A_FLOAT, A_FLOAT, 0);
    class_addMethod(c, (t_method)dial_dialog,
        gensym ("dialog"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_loadbang,
        gensym ("loadbang"), 0);
    class_addMethod(c, (t_method)dial_set,
        gensym ("set"), A_FLOAT, 0);
    class_addMethod(c, (t_method)dial_size,
        gensym ("size"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_delta,
        gensym ("delta"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_pos,
        gensym ("pos"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_range,
        gensym ("range"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_color,
        gensym ("color"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_send,
        gensym ("send"), A_DEFSYMBOL, 0);
    class_addMethod(c, (t_method)dial_receive,
        gensym ("receive"), A_DEFSYMBOL, 0);
    class_addMethod(c, (t_method)dial_label,
        gensym ("label"), A_DEFSYMBOL, 0);
    class_addMethod(c, (t_method)dial_label_pos,
        gensym ("label_pos"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_label_font,
        gensym ("label_font"), A_GIMME, 0);
    class_addMethod(c, (t_method)dial_log,
        gensym ("log"), 0);
    class_addMethod(c, (t_method)dial_lin,
        gensym ("lin"), 0);
    class_addMethod(c, (t_method)dial_init,
        gensym ("init"), A_FLOAT, 0);
    class_addMethod(c, (t_method)dial_log_height,
        gensym ("log_height"), A_FLOAT, 0);
    
    #if PD_WITH_LEGACY
    
    class_addCreator ((t_newmethod)dial_new, gensym ("my_numbox"), A_GIMME, A_NULL);
    
    #endif
    
    dial_widgetbehavior.w_getrectfn =    dial_getrect;
    dial_widgetbehavior.w_displacefn =   iemgui_behaviorDisplace;
    dial_widgetbehavior.w_selectfn =     iemgui_behaviorSelected;
    dial_widgetbehavior.w_activatefn =   NULL;
    dial_widgetbehavior.w_deletefn =     iemgui_behaviorDeleted;
    dial_widgetbehavior.w_visfn =        iemgui_behaviorVisible;
    dial_widgetbehavior.w_clickfn =      dial_newclick;
    class_setWidgetBehavior(c, &dial_widgetbehavior);
    class_setHelpName (c, gensym ("nbx"));
    class_setSaveFunction(c, dial_save);
    class_setPropertiesFunction(c, dial_properties);
    
    dial_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
