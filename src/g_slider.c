
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

#define IEM_HSLIDER_DEFAULT_WIDTH       128
#define IEM_HSLIDER_DEFAULT_HEIGHT      15

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_HSLIDER_MINIMUM_WIDTH       8
#define IEM_HSLIDER_MINIMUM_HEIGHT      8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_widgetbehavior slider_widgetbehavior;

static t_class *slider_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void slider_draw_update(t_gobj *client, t_glist *glist)
{
    t_slider *x = (t_slider *)client;
    if (glist_isvisible(glist))
    {
        int r = text_xpix(&x->x_gui.iem_obj, glist) + (x->x_value + 50)/100;
        int ypos=text_ypix(&x->x_gui.iem_obj, glist);
        t_glist *canvas=glist_getcanvas(glist);
        sys_vGui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
                 canvas, x, r, ypos+1,
                 r, ypos + x->x_gui.iem_height);
    }
}

static void slider_draw_new(t_slider *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    int r = xpos + (x->x_value + 50)/100;
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xpos-3, ypos,
             xpos + x->x_gui.iem_width+2, ypos + x->x_gui.iem_height,
             x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create line %d %d %d %d -width 3 -fill #%6.6x -tags %lxKNOB\n",
             canvas, r, ypos+1, r,
             ypos + x->x_gui.iem_height, x->x_gui.iem_colorForeground, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xpos+x->x_gui.iem_labelX,
             ypos+x->x_gui.iem_labelY,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
             x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);

        /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xpos-3, ypos + x->x_gui.iem_height-1,
             xpos+4, ypos + x->x_gui.iem_height, x, 0);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xpos-3, ypos,
             xpos+4, ypos+1, x, 0);*/
}

static void slider_draw_move(t_slider *x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    int r = xpos + (x->x_value + 50)/100;
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x,
             xpos-3, ypos,
             xpos + x->x_gui.iem_width+2, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxKNOB %d %d %d %d\n",
             canvas, x, r, ypos+1,
             r, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos-3, ypos + x->x_gui.iem_height-1,
             xpos+4, ypos + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos-3, ypos,
             xpos+4, ypos+1);*/
}

static void slider_draw_erase(t_slider* x,t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxKNOB\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

static void slider_draw_config(t_slider* x,t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    sys_vGui(".x%lx.c itemconfigure %lxKNOB -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorForeground);
    sys_vGui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorBackground);
}

static void slider_draw_io(t_slider* x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos-3, ypos + x->x_gui.iem_height-1,
        xpos+4, ypos + x->x_gui.iem_height, x, 0);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos-3, ypos,
        xpos+4, ypos+1, x, 0);*/
}

static void slider_draw_select(t_slider* x,t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    if(x->x_gui.iem_isSelected)
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_NORMAL);
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorLabel);
    }
}

void slider_draw(t_slider *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        interface_guiQueueAddIfNotAlreadyThere(x, glist, slider_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        slider_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        slider_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        slider_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        slider_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        slider_draw_config(x, glist);
}

/* ------------------------ hsl widgetbehaviour----------------------------- */


static void slider_getrect(t_gobj *z, t_glist *glist,
                            int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_slider* x = (t_slider*)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist) - 3;
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist);
    *xp2 = *xp1 + x->x_gui.iem_width + 5;
    *yp2 = *yp1 + x->x_gui.iem_height;
}

static void slider_save(t_gobj *z, t_buffer *b)
{
    t_slider *x = (t_slider *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iemgui_serialize(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiiffiisssiiiiiiiii", gensym("#X"),gensym("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate, (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym("hsl"), x->x_gui.iem_width, x->x_gui.iem_height,
                (t_float)x->x_minimum, (t_float)x->x_maximum,
                x->x_isLogarithmic, iemgui_serializeLoadbang(&x->x_gui),
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_serializeFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2],
                x->x_value, x->x_isSteadyOnClick);
    buffer_vAppend(b, ";");
}

void slider_check_width(t_slider *x, int w)
{
    if(w < IEM_HSLIDER_MINIMUM_WIDTH)
        w = IEM_HSLIDER_MINIMUM_WIDTH;
    x->x_gui.iem_width = w;
    if(x->x_value > (x->x_gui.iem_width*100 - 100))
    {
        x->x_position = x->x_gui.iem_width*100 - 100;
        x->x_value = x->x_position;
    }
    if(x->x_isLogarithmic)
        x->x_k = log(x->x_maximum/x->x_minimum)/(double)(x->x_gui.iem_width - 1);
    else
        x->x_k = (x->x_maximum - x->x_minimum)/(double)(x->x_gui.iem_width - 1);
}

void slider_check_minmax(t_slider *x, double min, double max)
{
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
    if(x->x_isLogarithmic)
        x->x_k = log(x->x_maximum/x->x_minimum)/(double)(x->x_gui.iem_width - 1);
    else
        x->x_k = (x->x_maximum - x->x_minimum)/(double)(x->x_gui.iem_width - 1);
}

static void slider_properties(t_gobj *z, t_glist *owner)
{
    t_slider *x = (t_slider *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_serializeNames(&x->x_gui, srl);
    sprintf(buf, "::ui_iem::create %%s Slider \
            %d %d {Slider Width} %d %d {Slider Height} \
            %g {Value Left} %g {Value Right} \
            %d Linear Logarithmic \
            %d \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            %d\n",
            x->x_gui.iem_width, IEM_HSLIDER_MINIMUM_WIDTH, x->x_gui.iem_height, IEM_HSLIDER_MINIMUM_HEIGHT,
            x->x_minimum, x->x_maximum,
            x->x_isLogarithmic, 
            x->x_gui.iem_loadbang,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, 0xffffff & x->x_gui.iem_colorForeground, 0xffffff & x->x_gui.iem_colorLabel,
            x->x_isSteadyOnClick);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

static void slider_set(t_slider *x, t_float f)    /* bugfix */
{
    int old = x->x_value;
    double g;

    x->x_floatValue = f;
    if (x->x_minimum > x->x_maximum)
    {
        if(f > x->x_minimum)
            f = x->x_minimum;
        if(f < x->x_maximum)
            f = x->x_maximum;
    }
    else
    {
        if(f > x->x_maximum)
            f = x->x_maximum;
        if(f < x->x_minimum)
            f = x->x_minimum;
    }
    if(x->x_isLogarithmic)
        g = log(f/x->x_minimum)/x->x_k;
    else
        g = (f - x->x_minimum) / x->x_k;
    x->x_value = (int)(100.0*g + 0.49999);
    x->x_position = x->x_value;
    if(x->x_value != old)
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

    /* compute numeric value (fval) from pixel location (val) and range */
static t_float slider_getfval(t_slider *x)
{
    t_float fval;
    if (x->x_isLogarithmic)
        fval = x->x_minimum*exp(x->x_k*(double)(x->x_value)*0.01);
    else fval = (double)(x->x_value)*0.01*x->x_k + x->x_minimum;
    if ((fval < 1.0e-10) && (fval > -1.0e-10))
        fval = 0.0;
    return (fval);
}

static void slider_bang(t_slider *x)
{
    double out;

    if (0)
        out = slider_getfval(x);
    else out = x->x_floatValue;
    outlet_float(x->x_gui.iem_obj.te_outlet, out);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_float(x->x_gui.iem_send->s_thing, out);
}

static void slider_dialog(t_slider *x, t_symbol *s, int argc, t_atom *argv)
{
    int w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
    double min = (double)atom_getFloatAtIndex(2, argc, argv);
    double max = (double)atom_getFloatAtIndex(3, argc, argv);
    int lilo = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
    int steady = (int)(t_int)atom_getFloatAtIndex(16, argc, argv);

    if(lilo != 0) lilo = 1;
    x->x_isLogarithmic = lilo;
    if(steady)
        x->x_isSteadyOnClick = 1;
    else
        x->x_isSteadyOnClick = 0;
    iemgui_fromDialog(&x->x_gui, argc, argv);
    x->x_gui.iem_height = PD_MAX (h, IEM_MINIMUM_HEIGHT);
    slider_check_width(x, w);
    slider_check_minmax(x, min, max);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, cast_object (x));
}

static void slider_motion(t_slider *x, t_float dx, t_float dy)
{
    int old = x->x_value;

    if(x->x_isAccurateMoving)
        x->x_position += (int)dx;
    else
        x->x_position += 100*(int)dx;
    x->x_value = x->x_position;
    if(x->x_value > (100*x->x_gui.iem_width - 100))
    {
        x->x_value = 100*x->x_gui.iem_width - 100;
        x->x_position += 50;
        x->x_position -= x->x_position%100;
    }
    if(x->x_value < 0)
    {
        x->x_value = 0;
        x->x_position -= 50;
        x->x_position -= x->x_position%100;
    }
    x->x_floatValue = slider_getfval(x);
    if (old != x->x_value)
    {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        slider_bang(x);
    }
}

static void slider_click(t_slider *x, t_float xpos, t_float ypos,
                          t_float shift, t_float ctrl, t_float alt)
{
    if(!x->x_isSteadyOnClick)
        x->x_value = (int)(100.0 * (xpos - text_xpix(&x->x_gui.iem_obj, x->x_gui.iem_glist)));
    if(x->x_value > (100*x->x_gui.iem_width - 100))
        x->x_value = 100*x->x_gui.iem_width - 100;
    if(x->x_value < 0)
        x->x_value = 0;
    x->x_floatValue = slider_getfval(x);
    x->x_position = x->x_value;
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    slider_bang(x);
    glist_grab(x->x_gui.iem_glist, &x->x_gui.iem_obj.te_g, (t_glistmotionfn)slider_motion,
               0, xpos, ypos);
}

static int slider_newclick(t_gobj *z, struct _glist *glist,
                            int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_slider* x = (t_slider *)z;

    if(doit)
    {
        slider_click( x, (t_float)xpix, (t_float)ypix, (t_float)shift,
                       0, (t_float)alt);
        if(shift)
            x->x_isAccurateMoving = 1;
        else
            x->x_isAccurateMoving = 0;
    }
    return (1);
}

static void slider_size(t_slider *x, t_symbol *s, int ac, t_atom *av)
{
    slider_check_width(x, (int)(t_int)atom_getFloatAtIndex(0, ac, av));
    if(ac > 1) {
        int h = atom_getFloatAtIndex(1, ac, av);
        x->x_gui.iem_height = PD_MAX (h, IEM_MINIMUM_HEIGHT);
    }
    iemgui_boxChanged((void *)x, &x->x_gui);
}

static void slider_delta(t_slider *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void slider_pos(t_slider *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void slider_range(t_slider *x, t_symbol *s, int ac, t_atom *av)
{
    slider_check_minmax(x, (double)atom_getFloatAtIndex(0, ac, av),
                         (double)atom_getFloatAtIndex(1, ac, av));
}

static void slider_color(t_slider *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setColor((void *)x, &x->x_gui, s, ac, av);}

static void slider_send(t_slider *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void slider_receive(t_slider *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void slider_label(t_slider *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

static void slider_label_pos(t_slider *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void slider_label_font(t_slider *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void slider_log(t_slider *x)
{
    x->x_isLogarithmic = 1;
    slider_check_minmax(x, x->x_minimum, x->x_maximum);
}

static void slider_lin(t_slider *x)
{
    x->x_isLogarithmic = 0;
    x->x_k = (x->x_maximum - x->x_minimum)/(double)(x->x_gui.iem_width - 1);
}

static void slider_init(t_slider *x, t_float f)
{
    x->x_gui.iem_loadbang = (f==0.0)?0:1;
}

static void slider_steady(t_slider *x, t_float f)
{
    x->x_isSteadyOnClick = (f==0.0)?0:1;
}

static void slider_float(t_slider *x, t_float f)
{
    double out;

    slider_set(x, f);
    if(x->x_gui.iem_goThrough)
        slider_bang(x);
}

static void slider_loadbang(t_slider *x)
{
    if(x->x_gui.iem_loadbang)
    {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        slider_bang(x);
    }
}

static void *slider_new(t_symbol *s, int argc, t_atom *argv)
{
    t_slider *x = (t_slider *)pd_new(slider_class);
    int bflcol[]={-262144, -1, -1};
    int w=IEM_HSLIDER_DEFAULT_WIDTH, h=IEM_HSLIDER_DEFAULT_HEIGHT;
    int lilo=0, ldx=-2, ldy=-8, f=0, steady=1;
    int fs=10;
    double min=0.0, max=(double)(IEM_HSLIDER_DEFAULT_WIDTH-1);
    char str[144];
    float v = 0;

    iemgui_deserializeLoadbang(&x->x_gui, 0);
    iemgui_deserializeFontStyle(&x->x_gui, 0);

    if(((argc == 17)||(argc == 18))&&IS_FLOAT(argv + 0)&&IS_FLOAT(argv + 1)
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
        steady = (int)(t_int)atom_getFloatAtIndex(17, argc, argv);

    x->x_gui.iem_draw = (t_iemfn)slider_draw;

    x->x_gui.iem_canSend = 1;
    x->x_gui.iem_canReceive = 1;

    x->x_gui.iem_glist = (t_glist *)canvas_getcurrent();
    if (x->x_gui.iem_loadbang)
        x->x_value = v;
    else x->x_value = 0;
    x->x_position = x->x_value;
    if(lilo != 0) lilo = 1;
    x->x_isLogarithmic = lilo;
    if(steady != 0) steady = 1;
    x->x_isSteadyOnClick = steady;
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
    x->x_gui.iem_height = PD_MAX (h, IEM_MINIMUM_HEIGHT);
    slider_check_width(x, w);
    slider_check_minmax(x, min, max);
    iemgui_deserializeColors(&x->x_gui, bflcol);
    iemgui_checkSendReceiveLoop(&x->x_gui);
    outlet_new(&x->x_gui.iem_obj, &s_float);
    x->x_floatValue = slider_getfval(x);
    return (x);
}

static void slider_free(t_slider *x)
{
    if(x->x_gui.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    gfxstub_deleteforkey(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void slider_setup(void)
{
    slider_class = class_new(gensym("hsl"), (t_newmethod)slider_new,
                              (t_method)slider_free, sizeof(t_slider), 0, A_GIMME, 0);
#ifndef GGEE_HSLIDER_COMPATIBLE
    class_addCreator((t_newmethod)slider_new, gensym("hslider"), A_GIMME, 0);
#endif
    class_addBang(slider_class,slider_bang);
    class_addFloat(slider_class,slider_float);
    class_addMethod(slider_class, (t_method)slider_click, gensym("click"),
                    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addMethod(slider_class, (t_method)slider_motion, gensym("motion"),
                    A_FLOAT, A_FLOAT, 0);
    class_addMethod(slider_class, (t_method)slider_dialog, gensym("dialog"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_loadbang, gensym("loadbang"), 0);
    class_addMethod(slider_class, (t_method)slider_set, gensym("set"), A_FLOAT, 0);
    class_addMethod(slider_class, (t_method)slider_size, gensym("size"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_delta, gensym("delta"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_pos, gensym("pos"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_range, gensym("range"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_color, gensym("color"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_send, gensym("send"), A_DEFSYMBOL, 0);
    class_addMethod(slider_class, (t_method)slider_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addMethod(slider_class, (t_method)slider_label, gensym("label"), A_DEFSYMBOL, 0);
    class_addMethod(slider_class, (t_method)slider_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_label_font, gensym("label_font"), A_GIMME, 0);
    class_addMethod(slider_class, (t_method)slider_log, gensym("log"), 0);
    class_addMethod(slider_class, (t_method)slider_lin, gensym("lin"), 0);
    class_addMethod(slider_class, (t_method)slider_init, gensym("init"), A_FLOAT, 0);
    class_addMethod(slider_class, (t_method)slider_steady, gensym("steady"), A_FLOAT, 0);
    slider_widgetbehavior.w_getrectfn =    slider_getrect;
    slider_widgetbehavior.w_displacefn =   iemgui_behaviorDisplace;
    slider_widgetbehavior.w_selectfn =     iemgui_behaviorSelected;
    slider_widgetbehavior.w_activatefn =   NULL;
    slider_widgetbehavior.w_deletefn =     iemgui_behaviorDeleted;
    slider_widgetbehavior.w_visfn =        iemgui_behaviorVisible;
    slider_widgetbehavior.w_clickfn =      slider_newclick;
    class_setWidgetBehavior(slider_class, &slider_widgetbehavior);
    class_setHelpName (slider_class, gensym ("hsl"));
    class_setSaveFunction(slider_class, slider_save);
    class_setPropertiesFunction(slider_class, slider_properties);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
