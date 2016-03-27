/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_7_guis.c written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001 */
/* thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "g_canvas.h"

#include "g_iem.h"
#include <math.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define IEM_VUMETER_DEFAULT_HEIGHT       3
#define IEM_VUMETER_MINIMUM_HEIGHT       2
#define IEM_VUMETER_DEFAULT_WIDTH        15
#define IEM_VUMETER_MINIMUM_WIDTH        8
#define IEM_VUMETER_MINIMUM_DECIBELS     -99.9
#define IEM_VUMETER_MAXIMUM_DECIBELS     12.0
#define IEM_VUMETER_STEPS                40
#define IEM_VUMETER_OFFSET               100.0

static int vu_colors[30] =
    {
        0xfcfcfc, 0xa0a0a0, 0x404040, 0xfce0e0, 0xfce0c0, 
        0xfcfcc8, 0xd8fcd8, 0xd8fcfc, 0xdce4fc, 0xf8d8fc,
        0xe0e0e0, 0x7c7c7c, 0x202020, 0xfc2828, 0xfcac44,
        0xe8e828, 0x14e814, 0x28f4f4, 0x3c50fc, 0xf430f0,
        0xbcbcbc, 0x606060, 0x000000, 0x8c0808, 0x583000,
        0x782814, 0x285014, 0x004450, 0x001488, 0x580050
    };
    
/* ----- vu  gui-peak- & rms- vu-meter-display ---------- */

t_widgetbehavior vu_widgetbehavior;
static t_class *vu_class;

int iem_vu_db2i[]=
{
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    9, 9, 9, 9, 9,10,10,10,10,10,
    11,11,11,11,11,12,12,12,12,12,
    13,13,13,13,14,14,14,14,15,15,
    15,15,16,16,16,16,17,17,17,18,
    18,18,19,19,19,20,20,20,21,21,
    22,22,23,23,24,24,25,26,27,28,
    29,30,31,32,33,33,34,34,35,35,
    36,36,37,37,37,38,38,38,39,39,
    39,39,39,39,40,40
};

int iem_vu_col[]=
{
    0,17,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
    15,15,15,15,15,15,15,15,15,15,14,14,13,13,13,13,13,13,13,13,13,13,13,19,19,19
};

/* widget helper functions */

static void vu_update_rms(t_vu *x, t_glist *glist)
{
    if(glist_isvisible(glist))
    {
        int w4=x->x_gui.iem_width/4, off=text_ypix(&x->x_gui.iem_obj, glist)-1;
        int xpos=text_xpix(&x->x_gui.iem_obj, glist), quad1=xpos+w4+1, quad3=xpos+x->x_gui.iem_width-w4-1;

        sys_vGui(".x%lx.c coords %lxRCOVER %d %d %d %d\n",
                 glist_getcanvas(glist), x, quad1, off, quad3,
                 off + (x->x_led_size+1)*(IEM_VUMETER_STEPS-x->x_rms));
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
            int i=iem_vu_col[x->x_peak];
            int j=ypos + (x->x_led_size+1)*(IEM_VUMETER_STEPS+1-x->x_peak)
                - (x->x_led_size+1)/2;

            sys_vGui(".x%lx.c coords %lxPLED %d %d %d %d\n", canvas, x,
                     xpos, j,
                     xpos+x->x_gui.iem_width+1, j);
            sys_vGui(".x%lx.c itemconfigure %lxPLED -fill #%6.6x\n", canvas, x,
                     vu_colors[i]);
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
    if (x->x_updaterms)
    {
        vu_update_rms(x, glist);
        x->x_updaterms = 0;
    }
    if (x->x_updatepeak)
    {
        vu_update_peak(x, glist);
        x->x_updatepeak = 0;
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
    int k1=x->x_led_size+1, k2=IEM_VUMETER_STEPS+1, k3=k1/2;
    int led_col, yyy, i, k4=ypos-k3;

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xpos-1, ypos-2,
             xpos+x->x_gui.iem_width+1,
             ypos+x->x_gui.iem_height+2, x->x_gui.iem_colorBackground, x);
    for(i=1; i<=IEM_VUMETER_STEPS; i++)
    {
        led_col = iem_vu_col[i];
        yyy = k4 + k1*(k2-i);
        sys_vGui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxRLED%d\n",
                 canvas, quad1, yyy, quad3, yyy, x->x_led_size, vu_colors[led_col], x, i);
    }
    if(x->x_scale)
    {
        i=IEM_VUMETER_STEPS+1;
        yyy = k4 + k1*(k2-i);
    }
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxRCOVER\n",
             canvas, quad1, ypos-1, quad3-1,
             ypos-1 + k1*IEM_VUMETER_STEPS, x->x_gui.iem_colorBackground, x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxPLED\n",
             canvas, mid, ypos+10,
             mid, ypos+10, x->x_led_size, x->x_gui.iem_colorBackground, x);
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

    x->x_updaterms = x->x_updatepeak = 1;
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
    int k1=x->x_led_size+1, k2=IEM_VUMETER_STEPS+1, k3=k1/2;
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
    if(x->x_scale)
    {
        i=IEM_VUMETER_STEPS+1;
        yyy = k4 + k1*(k2-i);
        /*sys_vGui(".x%lx.c coords %lxSCALE%d %d %d\n",
                 canvas, x, i, end, yyy+k3);*/
    }
    x->x_updaterms = x->x_updatepeak = 1;
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

static void vu_draw_erase(t_vu* x,t_glist* glist)
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
    if(x->x_scale)
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

static void vu_draw_config(t_vu* x, t_glist* glist)
{
    int i;
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorBackground);
    for(i=1; i<=IEM_VUMETER_STEPS; i++)
    {
        sys_vGui(".x%lx.c itemconfigure %lxRLED%d -width %d\n", canvas, x, i,
                 x->x_led_size);
    }
    if(x->x_scale)
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
             x->x_led_size);
}

static void vu_draw_io(t_vu* x, t_glist* glist)
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

static void vu_draw_select(t_vu* x,t_glist* glist)
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
        if(x->x_scale)
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
        if(x->x_scale)
        {
            i=IEM_VUMETER_STEPS+1;
            /*sys_vGui(".x%lx.c itemconfigure %lxSCALE%d -fill #%6.6x\n",
                     canvas, x, i, x->x_gui.iem_colorLabel);*/
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, x->x_gui.iem_colorLabel);
    }
}

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
    else if(mode >= IEM_DRAW_IO)
        vu_draw_io(x, glist);
}

/* ------------------------ vu widgetbehaviour----------------------------- */


static void vu_getrect(t_gobj *z, t_glist *glist,
                       int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_vu* x = (t_vu*)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist) - 1;
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist) - 2;
    *xp2 = *xp1 + x->x_gui.iem_width + 2;
    *yp2 = *yp1 + x->x_gui.iem_height + 4;
}

static void vu_save(t_gobj *z, t_buffer *b)
{
    t_vu *x = (t_vu *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iemgui_serialize(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiissiiiiiiii", gensym("#X"),gensym("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate, (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym("vu"), x->x_gui.iem_width, x->x_gui.iem_height,
                srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_serializeFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[2], x->x_scale,
                iemgui_serializeLoadbang(&x->x_gui));
    buffer_vAppend(b, ";");
}

void vu_check_height(t_vu *x, int h)
{
    int n;

    n = h / IEM_VUMETER_STEPS;
    if(n < IEM_VUMETER_MINIMUM_HEIGHT)
        n = IEM_VUMETER_MINIMUM_HEIGHT;
    x->x_led_size = n-1;
    x->x_gui.iem_height = IEM_VUMETER_STEPS * n;
}

static void vu_scale(t_vu *x, t_float fscale)
{
    int i, scale = (int)fscale;

    if(scale != 0) scale = 1;
    if(x->x_scale && !scale)
    {
        t_glist *canvas=glist_getcanvas(x->x_gui.iem_glist);

        x->x_scale = (int)scale;
        if(glist_isvisible(x->x_gui.iem_glist))
        {
            for(i=1; i<=IEM_VUMETER_STEPS; i++)
            {
                /*if((i+2)&3)
                    sys_vGui(".x%lx.c delete %lxSCALE%d\n", canvas, x, i);*/
            }
            i=IEM_VUMETER_STEPS+1;
            /*sys_vGui(".x%lx.c delete %lxSCALE%d\n", canvas, x, i);*/
        }
    }
    if(!x->x_scale && scale)
    {
        int w4=x->x_gui.iem_width/4, end=text_xpix(&x->x_gui.iem_obj, x->x_gui.iem_glist)+x->x_gui.iem_width+4;
        int k1=x->x_led_size+1, k2=IEM_VUMETER_STEPS+1, k3=k1/2;
        int yyy, k4=text_ypix(&x->x_gui.iem_obj, x->x_gui.iem_glist)-k3;
        t_glist *canvas=glist_getcanvas(x->x_gui.iem_glist);

        x->x_scale = (int)scale;
        if(glist_isvisible(x->x_gui.iem_glist))
        {
            for(i=1; i<=IEM_VUMETER_STEPS; i++)
            {
                yyy = k4 + k1*(k2-i);
            }
            i=IEM_VUMETER_STEPS+1;
            yyy = k4 + k1*(k2-i);
        }
    }
}

static void vu_properties(t_gobj *z, t_glist *owner)
{
    t_vu *x = (t_vu *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_serializeNames(&x->x_gui, srl);
    sprintf(buf, "::ui_iem::create %%s VU \
            %d %d {Meter Width} %d %d {Meter Height} \
            0 empty 0 empty \
            %d empty empty \
            -1 \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.iem_width, IEM_VUMETER_MINIMUM_WIDTH, x->x_gui.iem_height, IEM_VUMETER_STEPS*IEM_VUMETER_MINIMUM_HEIGHT,
            x->x_scale, 
            "nosndno", srl[1]->s_name,/*no send*/
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, -1/*no front-color*/, 0xffffff & x->x_gui.iem_colorLabel);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

static void vu_dialog(t_vu *x, t_symbol *s, int argc, t_atom *argv)
{
    int w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
    int scale = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);

    //srl[0] = gensym("empty");
    iemgui_fromDialog(&x->x_gui, argc, argv);
    x->x_gui.iem_canSend = 0;
    x->x_gui.iem_loadbang = 0;
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    vu_check_height(x, h);
    if(scale != 0)
        scale = 1;
    vu_scale(x, (t_float)scale);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_IO);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, (t_object*)x);
}

static void vu_size(t_vu *x, t_symbol *s, int ac, t_atom *av)
{
    int w = atom_getFloatAtIndex(0, ac, av);
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    if(ac > 1)
        vu_check_height(x, (int)(t_int)atom_getFloatAtIndex(1, ac, av));
    if(glist_isvisible(x->x_gui.iem_glist))
    {
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
        canvas_fixlines(x->x_gui.iem_glist, (t_object*)x);
    }
}

static void vu_delta(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void vu_pos(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void vu_color(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setColor((void *)x, &x->x_gui, s, ac, av);}

static void vu_receive(t_vu *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void vu_label(t_vu *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

static void vu_label_pos(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void vu_label_font(t_vu *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void vu_float(t_vu *x, t_float rms)
{
    int i;
    int old = x->x_rms;
    if(rms <= IEM_VUMETER_MINIMUM_DECIBELS)
        x->x_rms = 0;
    else if(rms >= IEM_VUMETER_MAXIMUM_DECIBELS)
        x->x_rms = IEM_VUMETER_STEPS;
    else
    {
        int i = (int)(2.0*(rms + IEM_VUMETER_OFFSET));
        x->x_rms = iem_vu_db2i[i];
    }
    i = (int)(100.0*rms + 10000.5);
    rms = 0.01*(t_float)(i - 10000);
    x->x_fr = rms;
    outlet_float(x->x_out_rms, rms);
    x->x_updaterms = 1;
    if(x->x_rms != old)
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
}

static void vu_ft1(t_vu *x, t_float peak)
{
    int i;
    int old = x->x_peak;
    if(peak <= IEM_VUMETER_MINIMUM_DECIBELS)
        x->x_peak = 0;
    else if(peak >= IEM_VUMETER_MAXIMUM_DECIBELS)
        x->x_peak = IEM_VUMETER_STEPS;
    else
    {
        int i = (int)(2.0*(peak + IEM_VUMETER_OFFSET));
        x->x_peak = iem_vu_db2i[i];
    }
    i = (int)(100.0*peak + 10000.5);
    peak = 0.01*(t_float)(i - 10000);
    x->x_fp = peak;
    x->x_updatepeak = 1;
    if(x->x_peak != old)
        interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
    outlet_float(x->x_out_peak, peak);
}

static void vu_bang(t_vu *x)
{
    outlet_float(x->x_out_peak, x->x_fp);
    outlet_float(x->x_out_rms, x->x_fr);
    x->x_updaterms = x->x_updatepeak = 1;
    interface_guiQueueAddIfNotAlreadyThere(x, x->x_gui.iem_glist, vu_draw_update);
}

static void *vu_new(t_symbol *s, int argc, t_atom *argv)
{
    t_vu *x = (t_vu *)pd_new(vu_class);
    int bflcol[]={-66577, -1, -1};
    int w=IEM_VUMETER_DEFAULT_WIDTH, h=IEM_VUMETER_STEPS*IEM_VUMETER_DEFAULT_HEIGHT;
    int ldx=-1, ldy=-8, f=0, fs=10, scale=1;
    //int ftbreak=IEM_BANG_DEFAULT_BREAK, fthold=IEM_BANG_DEFAULT_HOLD;
    char str[144];

    iemgui_deserializeLoadbang(&x->x_gui, 0);
    iemgui_deserializeFontStyle(&x->x_gui, 0);

    if((argc >= 11)&&IS_FLOAT_AT(argv,0)&&IS_FLOAT_AT(argv,1)
       &&(IS_SYMBOL_AT(argv,2)||IS_FLOAT_AT(argv,2))
       &&(IS_SYMBOL_AT(argv,3)||IS_FLOAT_AT(argv,3))
       &&IS_FLOAT_AT(argv,4)&&IS_FLOAT_AT(argv,5)
       &&IS_FLOAT_AT(argv,6)&&IS_FLOAT_AT(argv,7)
       &&IS_FLOAT_AT(argv,8)&&IS_FLOAT_AT(argv,9)&&IS_FLOAT_AT(argv,10))
    {
        w = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        h = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        iemgui_deserializeNamesByIndex(&x->x_gui, 1, argv);
        ldx = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(5, argc, argv);
        iemgui_deserializeFontStyle(&x->x_gui, (t_int)atom_getFloatAtIndex(6, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(7, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(8, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(9, argc, argv);
        scale = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
    }
    else iemgui_deserializeNamesByIndex(&x->x_gui, 1, 0);
    if((argc == 12)&&IS_FLOAT_AT(argv,11))
        iemgui_deserializeLoadbang(&x->x_gui, (t_int)atom_getFloatAtIndex(11, argc, argv));
    x->x_gui.iem_draw = (t_iemfn)vu_draw;

    x->x_gui.iem_canSend = 0;
    x->x_gui.iem_canReceive = 1;
    x->x_gui.iem_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.iem_receive->s_name, "empty"))
        x->x_gui.iem_canReceive = 0;

    if(x->x_gui.iem_canReceive)
        pd_bind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    x->x_gui.iem_labelX = ldx;
    x->x_gui.iem_labelY = ldy;

    if(fs < 4)
        fs = 4;
    x->x_gui.iem_fontSize = fs;
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    vu_check_height(x, h);
    iemgui_deserializeColors(&x->x_gui, bflcol);
    if(scale != 0)
        scale = 1;
    x->x_scale = scale;
    x->x_peak = 0;
    x->x_rms = 0;
    x->x_fp = -101.0;
    x->x_fr = -101.0;
    iemgui_checkSendReceiveLoop(&x->x_gui);
    inlet_new(&x->x_gui.iem_obj, &x->x_gui.iem_obj.te_g.g_pd, &s_float, gensym("ft1"));
    x->x_out_rms = outlet_new(&x->x_gui.iem_obj, &s_float);
    x->x_out_peak = outlet_new(&x->x_gui.iem_obj, &s_float);
    return (x);
}

static void vu_free(t_vu *x)
{
    if(x->x_gui.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    gfxstub_deleteforkey(x);
}

void g_vumeter_setup(void)
{
    vu_class = class_new(gensym("vu"), (t_newmethod)vu_new, (t_method)vu_free,
                         sizeof(t_vu), 0, A_GIMME, 0);
    class_addBang(vu_class,vu_bang);
    class_addFloat(vu_class,vu_float);
    class_addMethod(vu_class, (t_method)vu_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addMethod(vu_class, (t_method)vu_dialog, gensym("dialog"),
                    A_GIMME, 0);
    class_addMethod(vu_class, (t_method)vu_size, gensym("size"), A_GIMME, 0);
    class_addMethod(vu_class, (t_method)vu_scale, gensym("scale"), A_DEFFLOAT, 0);
    class_addMethod(vu_class, (t_method)vu_delta, gensym("delta"), A_GIMME, 0);
    class_addMethod(vu_class, (t_method)vu_pos, gensym("pos"), A_GIMME, 0);
    class_addMethod(vu_class, (t_method)vu_color, gensym("color"), A_GIMME, 0);
    class_addMethod(vu_class, (t_method)vu_receive, gensym("receive"), A_DEFSYMBOL, 0);
    class_addMethod(vu_class, (t_method)vu_label, gensym("label"), A_DEFSYMBOL, 0);
    class_addMethod(vu_class, (t_method)vu_label_pos, gensym("label_pos"), A_GIMME, 0);
    class_addMethod(vu_class, (t_method)vu_label_font, gensym("label_font"), A_GIMME, 0);
    vu_widgetbehavior.w_getrectfn =    vu_getrect;
    vu_widgetbehavior.w_displacefn =   iemgui_behaviorDisplace;
    vu_widgetbehavior.w_selectfn =     iemgui_behaviorSelected;
    vu_widgetbehavior.w_activatefn =   NULL;
    vu_widgetbehavior.w_deletefn =     iemgui_behaviorDeleted;
    vu_widgetbehavior.w_visfn =        iemgui_behaviorVisible;
    vu_widgetbehavior.w_clickfn =      NULL;
    class_setWidgetBehavior(vu_class,&vu_widgetbehavior);
    class_setHelpName(vu_class, gensym("vu"));
    class_setSaveFunction(vu_class, vu_save);
    class_setPropertiesFunction(vu_class, vu_properties);
}
