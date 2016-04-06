
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

#define IEM_VUMETER_THICKNESS               3
#define IEM_VUMETER_THICKNESS_MINIMUM       2

#define IEM_VUMETER_DECIBELS_TOP            12.0
#define IEM_VUMETER_DECIBELS_BOTTOM        -99.9

#define IEM_VUMETER_STEPS                   40
#define IEM_VUMETER_OFFSET                  100.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int vu_colors[43]=
    {
        0x000000,
        0x14e814,   // Green.
        0x14e814,       
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0x14e814,
        0xe8e828,   // Yellow.
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xe8e828,
        0xfcac44,   // Orange.
        0xfcac44,
        0xfcac44,
        0xfcac44,
        0xfcac44,
        0xfc2828,   // Red.
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xfc2828,
        0xf430f0,   // Violet.
        0xf430f0,
        0xf430f0
    };

static int vu_decibelToStep[226]=
    {
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
        3,  3,  3,  3,  3,  3,  3,  3,  3,  3,
        4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
        5,  5,  5,  5,  5,  5,  5,  5,  5,  5,
        6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
        7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
        8,  8,  8,  8,  8,  8,  8,  8,  8,  8,
        9,  9,  9,  9,  9,  10, 10, 10, 10, 10,
        11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
        13, 13, 13, 13, 14, 14, 14, 14, 15, 15,
        15, 15, 16, 16, 16, 16, 17, 17, 17, 18,
        18, 18, 19, 19, 19, 20, 20, 20, 21, 21,
        22, 22, 23, 23, 24, 24, 25, 26, 27, 28,
        29, 30, 31, 32, 33, 33, 34, 34, 35, 35,
        36, 36, 37, 37, 37, 38, 38, 38, 39, 39,
        39, 39, 39, 39, 40, 40
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior vu_widgetBehavior;

static t_class *vu_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* widget helper functions */

static void vu_update_rms(t_vu *x, t_glist *glist)
{
    if(glist_isvisible(glist))
    {
        int w4=x->x_gui.iem_width/4, off=text_ypix(&x->x_gui.iem_obj, glist)-1;
        int xpos=text_xpix(&x->x_gui.iem_obj, glist), quad1=xpos+w4+1, quad3=xpos+x->x_gui.iem_width-w4-1;

        sys_vGui(".x%lx.c coords %lxRCOVER %d %d %d %d\n",
                 glist_getcanvas(glist), x, quad1, off, quad3,
                 off + (x->x_thickness+1)*(IEM_VUMETER_STEPS-x->x_rms));
    }
}

static void vu_update_peak(t_vu *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    if(glist_isvisible(glist))
    {
        int xpos=text_xpix(&x->x_gui.iem_obj, glist);
        int ypos=text_ypix(&x->x_gui.iem_obj, glist);

        if(x->x_peak)
        {
            int i=vu_colors[x->x_peak];
            int j=ypos + (x->x_thickness+1)*(IEM_VUMETER_STEPS+1-x->x_peak)
                - (x->x_thickness+1)/2;

            sys_vGui(".x%lx.c coords %lxPLED %d %d %d %d\n", canvas, x,
                     xpos, j,
                     xpos+x->x_gui.iem_width+1, j);
            sys_vGui(".x%lx.c itemconfigure %lxPLED -fill #%6.6x\n", canvas, x,
                    i);
        }
        else
        {
            int mid=xpos+x->x_gui.iem_width/2;

            sys_vGui(".x%lx.c itemconfigure %lxPLED -fill #%6.6x\n",
                     canvas, x, x->x_gui.iem_colorBackground);
            sys_vGui(".x%lx.c coords %lxPLED %d %d %d %d\n",
                     canvas, x, mid, ypos+20,
                     mid, ypos+20);
        }
    }
}

static void vu_draw_update(t_gobj *client, t_glist *glist)
{
    t_vu *x = (t_vu *)client;
    if (x->x_needToUpdateRms)
    {
        vu_update_rms(x, glist);
        x->x_needToUpdateRms = 0;
    }
    if (x->x_needToUpdatePeak)
    {
        vu_update_peak(x, glist);
        x->x_needToUpdatePeak = 0;
    }
}
    
static void vu_draw_new(t_vu *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    int w4=x->x_gui.iem_width/4, mid=xpos+x->x_gui.iem_width/2,
        quad1=xpos+w4+1;
    int quad3=xpos+x->x_gui.iem_width-w4,
        end=xpos+x->x_gui.iem_width+4;
    int k1=x->x_thickness+1, k2=IEM_VUMETER_STEPS+1, k3=k1/2;
    int led_col, yyy, i, k4=ypos-k3;

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xpos-1, ypos-2,
             xpos+x->x_gui.iem_width+1,
             ypos+x->x_gui.iem_height+2, x->x_gui.iem_colorBackground, x);
    for(i=1; i<=IEM_VUMETER_STEPS; i++)
    {
        led_col = vu_colors[i];
        yyy = k4 + k1*(k2-i);
        sys_vGui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxRLED%d\n",
                 canvas, quad1, yyy, quad3, yyy, x->x_thickness, led_col, x, i);
    }
    if(x->x_hasScale)
    {
        i=IEM_VUMETER_STEPS+1;
        yyy = k4 + k1*(k2-i);
    }
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxRCOVER\n",
             canvas, quad1, ypos-1, quad3-1,
             ypos-1 + k1*IEM_VUMETER_STEPS, x->x_gui.iem_colorBackground, x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxPLED\n",
             canvas, mid, ypos+10,
             mid, ypos+10, x->x_thickness, x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xpos+x->x_gui.iem_labelX, ypos+x->x_gui.iem_labelY,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
             x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);

        /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas,
             xpos-1, ypos + x->x_gui.iem_height+1,
             xpos + INLETS_WIDTH-1, ypos + x->x_gui.iem_height+2,
             x, 0);
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]x\n",
             canvas,
             xpos+x->x_gui.iem_width+1-INLETS_WIDTH, ypos + x->x_gui.iem_height+1,
             xpos+x->x_gui.iem_width+1, ypos + x->x_gui.iem_height+2,
             x, 1);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos-1, ypos-2,
             xpos + INLETS_WIDTH-1, ypos-1,
             x, 0);
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas,
             xpos+x->x_gui.iem_width+1-INLETS_WIDTH, ypos-2,
             xpos+x->x_gui.iem_width+1, ypos-1,
             x, 1);*/

    x->x_needToUpdateRms = x->x_needToUpdatePeak = 1;
    interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
}


static void vu_draw_move(t_vu *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    int w4=x->x_gui.iem_width/4, quad1=xpos+w4+1;
    int quad3=xpos+x->x_gui.iem_width-w4,
        end=xpos+x->x_gui.iem_width+4;
    int k1=x->x_thickness+1, k2=IEM_VUMETER_STEPS+1, k3=k1/2;
    int yyy, i, k4=ypos-k3;

    sys_vGui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x, xpos-1, ypos-2,
             xpos+x->x_gui.iem_width+1,ypos+x->x_gui.iem_height+2);
    for(i=1; i<=IEM_VUMETER_STEPS; i++)
    {
        yyy = k4 + k1*(k2-i);
        sys_vGui(".x%lx.c coords %lxRLED%d %d %d %d %d\n",
                 canvas, x, i, quad1, yyy, quad3, yyy);
        /*if(((i+2)&3) && (x->iem_scale))*/
            /*sys_vGui(".x%lx.c coords %lxSCALE%d %d %d\n",
                     canvas, x, i, end, yyy+k3);*/
    }
    if(x->x_hasScale)
    {
        i=IEM_VUMETER_STEPS+1;
        yyy = k4 + k1*(k2-i);
        /*sys_vGui(".x%lx.c coords %lxSCALE%d %d %d\n",
                 canvas, x, i, end, yyy+k3);*/
    }
    x->x_needToUpdateRms = x->x_needToUpdatePeak = 1;
    interface_guiQueueAddIfNotAlreadyThere(x, glist, vu_draw_update);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xpos+x->x_gui.iem_labelX,
             ypos+x->x_gui.iem_labelY);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0,
             xpos-1, ypos + x->x_gui.iem_height+1,
             xpos + INLETS_WIDTH-1, ypos + x->x_gui.iem_height+2);
    sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 1,
             xpos+x->x_gui.iem_width+1-INLETS_WIDTH, ypos + x->x_gui.iem_height+1,
                 xpos+x->x_gui.iem_width+1, ypos + x->x_gui.iem_height+2);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0,
             xpos-1, ypos-2,
             xpos + INLETS_WIDTH-1, ypos-1);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 1,
             xpos+x->x_gui.iem_width+1-INLETS_WIDTH, ypos-2,
             xpos+x->x_gui.iem_width+1, ypos-1);*/
}

static void vu_draw_erase(t_vu* x,t_glist *glist)
{
    int i;
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE\n", canvas, x);
    for(i=1; i<=IEM_VUMETER_STEPS; i++)
    {
        sys_vGui(".x%lx.c delete %lxRLED%d\n", canvas, x, i);
        /*if(((i+2)&3) && (x->iem_scale))
            sys_vGui(".x%lx.c delete %lxSCALE%d\n", canvas, x, i);*/
    }
    if(x->x_hasScale)
    {
        i=IEM_VUMETER_STEPS+1;
        /*sys_vGui(".x%lx.c delete %lxSCALE%d\n", canvas, x, i);*/
    }
    sys_vGui(".x%lx.c delete %lxPLED\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxRCOVER\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 1);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 1);

}

static void vu_draw_config(t_vu* x, t_glist *glist)
{
    int i;
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorBackground);
    for(i=1; i<=IEM_VUMETER_STEPS; i++)
    {
        sys_vGui(".x%lx.c itemconfigure %lxRLED%d -width %d\n", canvas, x, i,
                 x->x_thickness);
    }
    if(x->x_hasScale)
    {
        i=IEM_VUMETER_STEPS+1;
    }
    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");

    sys_vGui(".x%lx.c itemconfigure %lxRCOVER -fill #%6.6x -outline #%6.6x\n", canvas,
             x, x->x_gui.iem_colorBackground, x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxPLED -width %d\n", canvas, x,
             x->x_thickness);
}

static void vu_draw_io(t_vu* x, t_glist *glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
         canvas,
         xpos-1, ypos + x->x_gui.iem_height+1,
         xpos + INLETS_WIDTH-1, ypos + x->x_gui.iem_height+2,
         x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
         canvas,
         xpos+x->x_gui.iem_width+1-INLETS_WIDTH, ypos + x->x_gui.iem_height+1,
         xpos+x->x_gui.iem_width+1, ypos + x->x_gui.iem_height+2,
         x, 1);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
         canvas,
         xpos-1, ypos-2,
         xpos + INLETS_WIDTH-1, ypos-1,
         x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
         canvas,
         xpos+x->x_gui.iem_width+1-INLETS_WIDTH, ypos-2,
         xpos+x->x_gui.iem_width+1, ypos-1,
         x, 1);*/
}

static void vu_draw_select(t_vu* x,t_glist *glist)
{
    int i;
    t_glist *canvas=glist_getcanvas(glist);

    if(x->x_gui.iem_isSelected)
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
        for(i=1; i<=IEM_VUMETER_STEPS; i++)
        {
            /*if(((i+2)&3) && (x->iem_scale))
                sys_vGui(".x%lx.c itemconfigure %lxSCALE%d -fill #%6.6x\n",
                         canvas, x, i, IEM_COLOR_SELECTED);*/
        }
        if(x->x_hasScale)
        {
            i=IEM_VUMETER_STEPS+1;
           /* sys_vGui(".x%lx.c itemconfigure %lxSCALE%d -fill #%6.6x\n",
                     canvas, x, i, IEM_COLOR_SELECTED);*/
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE -outline #%6.6x\n", canvas, x, IEM_COLOR_NORMAL);
        for(i=1; i<=IEM_VUMETER_STEPS; i++)
        {
            /*if(((i+2)&3) && (x->iem_scale))
                sys_vGui(".x%lx.c itemconfigure %lxSCALE%d -fill #%6.6x\n",
                         canvas, x, i, x->x_gui.iem_colorLabel);*/
        }
        if(x->x_hasScale)
        {
            i=IEM_VUMETER_STEPS+1;
            /*sys_vGui(".x%lx.c itemconfigure %lxSCALE%d -fill #%6.6x\n",
                     canvas, x, i, x->x_gui.iem_colorLabel);*/
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorLabel);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vu_draw(t_vu *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_MOVE)
        vu_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        vu_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        vu_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        vu_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        vu_draw_config(x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vu_check_height(t_vu *x, int h)
{
    int n;

    n = h / IEM_VUMETER_STEPS;
    if(n < IEM_VUMETER_THICKNESS_MINIMUM)
        n = IEM_VUMETER_THICKNESS_MINIMUM;
    x->x_thickness = n-1;
    x->x_gui.iem_height = IEM_VUMETER_STEPS * n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_bang(t_vu *x)
{
    outlet_float(x->x_outRight, x->x_peakValue);
    outlet_float(x->x_outLeft, x->x_rmsValue);
    x->x_needToUpdateRms = x->x_needToUpdatePeak = 1;
    interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
}

static void vu_float(t_vu *x, t_float rms)
{
    int i;
    int old = x->x_rms;
    if(rms <= IEM_VUMETER_DECIBELS_BOTTOM)
        x->x_rms = 0;
    else if(rms >= IEM_VUMETER_DECIBELS_TOP)
        x->x_rms = IEM_VUMETER_STEPS;
    else
    {
        int i = (int)(2.0*(rms + IEM_VUMETER_OFFSET));
        x->x_rms = vu_decibelToStep[i];
    }
    i = (int)(100.0*rms + 10000.5);
    rms = 0.01*(t_float)(i - 10000);
    x->x_rmsValue = rms;
    outlet_float(x->x_outLeft, rms);
    x->x_needToUpdateRms = 1;
    if(x->x_rms != old)
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
}

static void vu_ft1(t_vu *x, t_float peak)
{
    int i;
    int old = x->x_peak;
    if(peak <= IEM_VUMETER_DECIBELS_BOTTOM)
        x->x_peak = 0;
    else if(peak >= IEM_VUMETER_DECIBELS_TOP)
        x->x_peak = IEM_VUMETER_STEPS;
    else
    {
        int i = (int)(2.0*(peak + IEM_VUMETER_OFFSET));
        x->x_peak = vu_decibelToStep[i];
    }
    i = (int)(100.0*peak + 10000.5);
    peak = 0.01*(t_float)(i - 10000);
    x->x_peakValue = peak;
    x->x_needToUpdatePeak = 1;
    if(x->x_peak != old)
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
    outlet_float(x->x_outRight, peak);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_dialog(t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    int w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
    int scale = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);

    //srl[0] = gensym ("empty");
    iemgui_fromDialog(&x->x_gui, argc, argv);
    x->x_gui.iem_canSend = 0;
    x->x_gui.iem_loadbang = 0;
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    vu_check_height(x, h);
    if(scale != 0)
        scale = 1;
    //vu_scale(x, (t_float)scale);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, (t_object *)x);
}

static void vu_size(t_vu *x, t_symbol *s, int ac, t_atom *av)
{
    int w = atom_getFloatAtIndex(0, ac, av);
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    if(ac > 1)
        vu_check_height(x, (int)(t_int)atom_getFloatAtIndex(1, ac, av));
    if(glist_isvisible(x->x_gui.iem_glist))
    {
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
        (*x->x_gui.iem_draw) (x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
        canvas_fixlines(x->x_gui.iem_glist, (t_object *)x);
    }
}

static void vu_delta(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void vu_pos(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void vu_label_font(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void vu_label_pos(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void vu_receive(t_vu *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void vu_label(t_vu *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    *a = text_xpix (cast_object (z), glist);
    *b = text_ypix (cast_object (z), glist);
    *c = *a + cast_iem (z)->iem_width;
    *d = *b + cast_iem (z)->iem_height;
}

static void vu_behaviorSave (t_gobj *z, t_buffer *b)
{
    t_vu *x = (t_vu *)z;
    
    t_iemnames names;
    t_iemcolors colors;

    iemgui_serialize (&x->x_gui, &names, &colors);
    
    buffer_vAppend (b, "ssiisiissiiiiiiii",
        gensym ("#X"),
        gensym ("obj"),
        (int)cast_object (x)->te_xCoordinate,
        (int)cast_object (x)->te_yCoordinate,
        gensym ("vu"),
        x->x_gui.iem_width,                                         // Width.
        x->x_gui.iem_height,                                        // Height.
        names.n_unexpandedReceive,                                  // Receive.
        names.n_unexpandedLabel,                                    // Label.
        x->x_gui.iem_labelX,                                        // Label X.
        x->x_gui.iem_labelY,                                        // Label Y.
        iemgui_serializeFontStyle (&x->x_gui),                      // Label font.
        x->x_gui.iem_fontSize,                                      // Label font size.
        colors.c_colorBackground,                                   // Background color.
        colors.c_colorLabel,                                        // Label color.
        x->x_hasScale,                                              // Dummy.
        0);                                                         // Dummy.
        
    buffer_vAppend (b, ";");
}

static void vu_behaviorProperties (t_gobj *z, t_glist *owner)
{
    t_vu *x = (t_vu *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING];
    t_iemnames names;

    iemgui_serializeNames (&x->x_gui, &names);
    
    err = string_sprintf (t, PD_STRING, "::ui_iem::create %%s VU"
            " %d %d {Meter Width}"
            " %d %d {Meter Height}"
            " 0 empty 0 empty"
            " 0 empty empty"
            " -1"
            " -1 -1 empty"
            " %s %s"
            " %s %d %d"
            " %d"
            " %d %d %d"
            " -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_gui.iem_height, IEM_VUMETER_STEPS * IEM_VUMETER_THICKNESS_MINIMUM,
            "nosndno", names.n_unexpandedReceive->s_name,                               /* No send. */
            names.n_unexpandedLabel->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            x->x_gui.iem_colorBackground, -1, x->x_gui.iem_colorLabel);                 /* No foreground. */
            
    PD_ASSERT (!err);
    
    gfxstub_new (cast_pd (x), (void *)x, t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vu_dummy (t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vu_new (t_symbol *s, int argc, t_atom *argv)
{
    t_vu *x = (t_vu *)pd_new (vu_class);

    int width           = IEM_DEFAULT_SIZE;
    int height          = IEM_VUMETER_STEPS * IEM_VUMETER_THICKNESS;
    int labelX          = IEM_DEFAULT_LABELX_TOP;
    int labelY          = IEM_DEFAULT_LABELY_TOP;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    int hasScale        = 0;
    t_iemcolors colors  = IEM_COLORS_DEFAULT;

    if (argc >= 11                                                  // --
            && IS_FLOAT (argv + 0)                                  // Width.
            && IS_FLOAT (argv + 1)                                  // Height.
            && (IS_SYMBOL (argv + 2) || IS_FLOAT (argv + 2))        // Receive.
            && (IS_SYMBOL (argv + 3) || IS_FLOAT (argv + 3))        // Label.
            && IS_FLOAT (argv + 4)                                  // Label X.
            && IS_FLOAT (argv + 5)                                  // Label Y.
            && IS_FLOAT (argv + 6)                                  // Label font.
            && IS_FLOAT (argv + 7)                                  // Label font size.
            && IS_FLOAT (argv + 8)                                  // Background color.
            && IS_FLOAT (argv + 9)                                  // Label color.
            && IS_FLOAT (argv + 10))                                // Dummy.
    {
        width                       = (int)atom_getFloatAtIndex (0,  argc, argv);
        height                      = (int)atom_getFloatAtIndex (1,  argc, argv);
        labelX                      = (int)atom_getFloatAtIndex (4,  argc, argv);
        labelY                      = (int)atom_getFloatAtIndex (5,  argc, argv);
        labelFontSize               = (int)atom_getFloatAtIndex (7,  argc, argv);
        colors.c_colorBackground    = (int)atom_getFloatAtIndex (8,  argc, argv);
        colors.c_colorLabel         = (int)atom_getFloatAtIndex (9,  argc, argv);
        hasScale                    = (int)atom_getFloatAtIndex (10, argc, argv);
        
        /* Note that due to the overlap a fake float value is pitiably attribute to the send symbol. */
        
        iemgui_deserializeNamesByIndex (&x->x_gui, 1, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (6, argc, argv));
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 1, NULL);
    }

    x->x_gui.iem_glist      = (t_glist *)canvas_getcurrent();
    x->x_gui.iem_draw       = (t_iemfn)vu_draw;
    x->x_gui.iem_canSend    = 0;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_width      = PD_MAX (width, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
        
    vu_check_height (x, height);
    
    iemgui_checkSendReceiveLoop(&x->x_gui);
    iemgui_deserializeColors(&x->x_gui, &colors);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_hasScale   = (hasScale != 0);
    x->x_peakValue  = -101.0;
    x->x_rmsValue   = -101.0;
    
    inlet_new (cast_object (x), cast_pd (x), &s_float, gensym ("ft1"));
    
    x->x_outLeft  = outlet_new (cast_object (x), &s_float);
    x->x_outRight = outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void vu_free (t_vu *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_object (x), x->x_gui.iem_receive); }
        
    gfxstub_deleteforkey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vu_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("vu"),
        (t_newmethod)vu_new,
        (t_method)vu_free,
        sizeof (t_vu),
        CLASS_DEFAULT,
        A_GIMME,
        A_NULL);
        
    class_addBang (c, vu_bang);
    class_addFloat (c, vu_float);
    
    class_addMethod (c, (t_method)vu_ft1,           gensym ("ft1"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vu_dialog,        gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_size,          gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_delta,         gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_pos,           gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_label_font,    gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_label_pos,     gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_receive,       gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)vu_label,         gensym ("label"),           A_DEFSYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)vu_delta,         gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_pos,           gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_label_pos,     gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_label_font,    gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_dummy,         gensym ("scale"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vu_dummy,         gensym ("color"),           A_GIMME, A_NULL);

    #endif
    
    vu_widgetBehavior.w_getrectfn   = vu_behaviorGetRectangle;
    vu_widgetBehavior.w_displacefn  = iemgui_behaviorDisplace;
    vu_widgetBehavior.w_selectfn    = iemgui_behaviorSelected;
    vu_widgetBehavior.w_activatefn  = NULL;
    vu_widgetBehavior.w_deletefn    = iemgui_behaviorDeleted;
    vu_widgetBehavior.w_visfn       = iemgui_behaviorVisible;
    vu_widgetBehavior.w_clickfn     = NULL;
    
    class_setWidgetBehavior (c,&vu_widgetBehavior);
    class_setHelpName (c, gensym ("vu"));
    class_setSaveFunction (c, vu_behaviorSave);
    class_setPropertiesFunction (c, vu_behaviorProperties);
    
    vu_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
