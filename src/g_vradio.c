
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

#define IEM_VRADIO_DEFAULT_LABELX    0
#define IEM_VRADIO_DEFAULT_LABELY   -8
#define IEM_VRADIO_DEFAULT_BUTTONS   8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior vradio_widgetBehavior;

static t_class *vradio_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void vradio_draw_update(t_gobj *client, t_glist *glist)
{
    t_hradio *x = (t_hradio *)client;
    if(glist_isvisible(glist))
    {
        t_glist *canvas=glist_getcanvas(glist);

        sys_vGui(".x%lx.c itemconfigure %lxBUTTON%d -fill #%6.6x -outline #%6.6x\n",
                 canvas, x, x->x_stateDrawn,
                 x->x_gui.iem_colorBackground, x->x_gui.iem_colorBackground);
        sys_vGui(".x%lx.c itemconfigure %lxBUTTON%d -fill #%6.6x -outline #%6.6x\n",
                 canvas, x, x->x_state,
                 x->x_gui.iem_colorForeground, x->x_gui.iem_colorForeground);
        x->x_stateDrawn = x->x_state;
    }
}

void vradio_draw_new(t_vradio *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i, dy=x->x_gui.iem_height, s4=dy/4;
    int yy11b=text_ypix(&x->x_gui.iem_obj, glist); 
    int yy11=yy11b, yy12=yy11+dy;
    int yy21=yy11+s4, yy22=yy12-s4;
    int xx11=text_xpix(&x->x_gui.iem_obj, glist), xx12=xx11+dy;
    int xx21=xx11+s4, xx22=xx12-s4;

    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE%d\n",
                 canvas, xx11, yy11, xx12, yy12,
                 x->x_gui.iem_colorBackground, x, i);
        sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -outline #%6.6x -tags %lxBUTTON%d\n",
                 canvas, xx21, yy21, xx22, yy22,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground, x, i);
        yy11 += dy;
        yy12 += dy;
        yy21 += dy;
        yy22 += dy;
        x->x_stateDrawn = x->x_state;
    }
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xx11+x->x_gui.iem_labelX, yy11b+x->x_gui.iem_labelY,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
             x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);

       /* sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xx11, yy11-1, xx11 + INLETS_WIDTH, yy11, x, 0);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xx11, yy11b, xx11 + INLETS_WIDTH, yy11b+1, x, 0);*/
}

void vradio_draw_move(t_vradio *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i, dy=x->x_gui.iem_height, s4=dy/4;
    int yy11b=text_ypix(&x->x_gui.iem_obj, glist);
    int yy11=yy11b, yy12=yy11+dy;
    int yy21=yy11+s4, yy22=yy12-s4;
    int xx11=text_xpix(&x->x_gui.iem_obj, glist), xx12=xx11+dy;
    int xx21=xx11+s4, xx22=xx12-s4;

    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c coords %lxBASE%d %d %d %d %d\n",
                 canvas, x, i, xx11, yy11, xx12, yy12);
        sys_vGui(".x%lx.c coords %lxBUTTON%d %d %d %d %d\n",
                 canvas, x, i, xx21, yy21, xx22, yy22);
        yy11 += dy;
        yy12 += dy;
        yy21 += dy;
        yy22 += dy;
    }
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xx11+x->x_gui.iem_labelX, yy11b+x->x_gui.iem_labelY);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0, xx11, yy11-1, xx11 + INLETS_WIDTH, yy11);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0, xx11, yy11b, xx11 + INLETS_WIDTH, yy11b+1);*/
}

void vradio_draw_erase(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i;

    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c delete %lxBASE%d\n", canvas, x, i);
        sys_vGui(".x%lx.c delete %lxBUTTON%d\n", canvas, x, i);
    }
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void vradio_draw_config(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i;

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    for(i=0; i<n; i++)
    {
        sys_vGui(".x%lx.c itemconfigure %lxBASE%d -fill #%6.6x\n", canvas, x, i,
                 x->x_gui.iem_colorBackground);
        sys_vGui(".x%lx.c itemconfigure %lxBUTTON%d -fill #%6.6x -outline #%6.6x\n", canvas, x, i,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground,
                 (x->x_state==i)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
    }
}

void vradio_draw_io(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos,
        ypos+(x->x_numberOfButtons*x->x_gui.iem_height)-1,
        xpos+ INLETS_WIDTH,
        ypos+(x->x_numberOfButtons*x->x_gui.iem_height), x, 0);

    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos, ypos,
        xpos+ INLETS_WIDTH, ypos+1,
        x, 0);*/
}

void vradio_draw_select(t_vradio* x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int n=x->x_numberOfButtons, i;

    if(x->x_gui.iem_isSelected)
    {
        for(i=0; i<n; i++)
        {
            sys_vGui(".x%lx.c itemconfigure %lxBASE%d -outline #%6.6x\n", canvas, x, i,
                     IEM_COLOR_SELECTED);
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x, IEM_COLOR_SELECTED);
    }
    else
    {
        for(i=0; i<n; i++)
        {
            sys_vGui(".x%lx.c itemconfigure %lxBASE%d -outline #%6.6x\n", canvas, x, i,
                     IEM_COLOR_NORMAL);
        }
        sys_vGui(".x%lx.c itemconfigure %lxLABEL -fill #%6.6x\n", canvas, x,
                 x->x_gui.iem_colorLabel);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vradio_draw(t_vradio *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        interface_guiQueueAddIfNotAlreadyThere(x, glist, vradio_draw_update);
    else if(mode == IEM_DRAW_MOVE)
        vradio_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        vradio_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        vradio_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        vradio_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        vradio_draw_config(x, glist);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_fout(t_vradio *x, t_float f)
{
    int i=(int)f;

    x->x_floatValue = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_numberOfButtons)
        i = x->x_numberOfButtons-1;

        float outval = (0 ? i : x->x_floatValue);
        x->x_state = i;
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        outlet_float(x->x_gui.iem_obj.te_outlet, outval);
        if (x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
            pd_float(x->x_gui.iem_send->s_thing, outval);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_bang(t_vradio *x)
{
        float outval = (0 ? x->x_state : x->x_floatValue);
        outlet_float(x->x_gui.iem_obj.te_outlet, outval);
        if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
            pd_float(x->x_gui.iem_send->s_thing, outval);
}

static void vradio_float(t_vradio *x, t_float f)
{
    int i=(int)f;

    x->x_floatValue = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_numberOfButtons)
        i = x->x_numberOfButtons-1;

        float outval = (0 ? i : x->x_floatValue);
        x->x_state = i;
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
        if (x->x_gui.iem_goThrough)
        {
            outlet_float(x->x_gui.iem_obj.te_outlet, outval);
            if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
                pd_float(x->x_gui.iem_send->s_thing, outval);
        }
}

static void vradio_click(t_vradio *x, t_float xpos, t_float ypos,
    t_float shift, t_float ctrl, t_float alt)
{
    int yy =  (int)ypos - text_ypix(&x->x_gui.iem_obj, x->x_gui.iem_glist);

    vradio_fout(x, (t_float)(yy / x->x_gui.iem_height));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_loadbang(t_vradio *x)
{
    if(x->x_gui.iem_loadbang)
        vradio_bang(x);
}

static void vradio_init(t_vradio *x, t_float f)
{
    x->x_gui.iem_loadbang = (f==0.0)?0:1;
}

static void vradio_dialog(t_vradio *x, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    int chg = (int)(t_int)atom_getFloatAtIndex(4, argc, argv);
    int num = (int)(t_int)atom_getFloatAtIndex(6, argc, argv);

    if(chg != 0) chg = 1;
    x->x_changed = chg;
    iemgui_fromDialog(&x->x_gui, argc, argv);
    x->x_gui.iem_width = PD_MAX (a, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    if(x->x_numberOfButtons != num)
    {
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_ERASE);
        x->x_numberOfButtons = num;
        if(x->x_state >= x->x_numberOfButtons)
        {
            x->x_state = x->x_numberOfButtons - 1;
        }
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_NEW);
    }
    else
    {
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
        canvas_fixlines(x->x_gui.iem_glist, (t_object*)x);
    }
}

static void vradio_size(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{
    int w = atom_getFloatAtIndex(0, ac, av);
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    iemgui_boxChanged((void *)x, &x->x_gui);
}

static void vradio_delta(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void vradio_pos(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void vradio_label_font(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void vradio_label_pos(t_vradio *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void vradio_set(t_vradio *x, t_float f)
{
    int i=(int)f;
    int old;

    x->x_floatValue = f;
    if(i < 0)
        i = 0;
    if(i >= x->x_numberOfButtons)
        i = x->x_numberOfButtons-1;

    x->x_state = i;
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

static void vradio_number(t_vradio *x, t_float num)
{
    int n=(int)num;

    if(n < 1)
        n = 1;
    if(n > IEM_MAXIMUM_BUTTONS)
        n = IEM_MAXIMUM_BUTTONS;
    if(n != x->x_numberOfButtons)
    {
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_ERASE);
        x->x_numberOfButtons = n;
        if(x->x_state >= x->x_numberOfButtons)
            x->x_state = x->x_numberOfButtons - 1;
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_NEW);
    }
}

static void vradio_send(t_vradio *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void vradio_receive(t_vradio *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void vradio_label(t_vradio *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_vradio *x = (t_vradio *)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist);
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist);
    *xp2 = *xp1 + x->x_gui.iem_width;
    *yp2 = *yp1 + x->x_gui.iem_height*x->x_numberOfButtons;
}

static int vradio_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        vradio_click((t_vradio *)z, (t_float)xpix, (t_float)ypix,
            (t_float)shift, 0, (t_float)alt);
    return (1);
}

static void vradio_save(t_gobj *z, t_buffer *b)
{
    t_vradio *x = (t_vradio *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iemgui_serialize(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiiiisssiiiiiiif", gensym ("#X"),gensym ("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate,
                (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym ("vradio"),
                x->x_gui.iem_width,
                x->x_changed, iemgui_serializeLoadbang(&x->x_gui), x->x_numberOfButtons,
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_serializeFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2], x->x_floatValue);
    buffer_vAppend(b, ";");
}

static void vradio_properties(t_gobj *z, t_glist *owner)
{
    t_vradio *x = (t_vradio *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_serializeNames(&x->x_gui, srl);

    sprintf(buf, "::ui_iem::create %%s {Radio Button} \
            %d %d Size 0 0 empty \
            0 empty 0 empty \
            -1 empty empty \
            %d \
            %d 256 {Number Of Buttons} \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_gui.iem_loadbang, 
            x->x_numberOfButtons,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, 0xffffff & x->x_gui.iem_colorForeground, 0xffffff & x->x_gui.iem_colorLabel);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void vradio_dummy (t_vradio *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *vradio_donew(t_symbol *s, int argc, t_atom *argv)
{
    t_vradio *x = (t_vradio *)pd_new(vradio_class);
    int bflcol[]={-262144, -1, -1};
    int a=IEM_DEFAULT_SIZE, on = 0, f=0;
    int ldx=0, ldy=-8, chg=1, num=8;
    int fs=10;
    //int ftbreak=IEM_BANG_DEFAULT_BREAK, fthold=IEM_BANG_DEFAULT_HOLD;
    char str[144];
    float fval = 0;

    if((argc == 15)&&IS_FLOAT(argv + 0)&&IS_FLOAT(argv + 1)&&IS_FLOAT(argv + 2)
       &&IS_FLOAT(argv + 3)
       &&(IS_SYMBOL(argv + 4)||IS_FLOAT(argv + 4))
       &&(IS_SYMBOL(argv + 5)||IS_FLOAT(argv + 5))
       &&(IS_SYMBOL(argv + 6)||IS_FLOAT(argv + 6))
       &&IS_FLOAT(argv + 7)&&IS_FLOAT(argv + 8)
       &&IS_FLOAT(argv + 9)&&IS_FLOAT(argv + 10)&&IS_FLOAT(argv + 11)
       &&IS_FLOAT(argv + 12)&&IS_FLOAT(argv + 13)&&IS_FLOAT(argv + 14))
    {
        a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
        chg = (int)(t_int)atom_getFloatAtIndex(1, argc, argv);
        iemgui_deserializeLoadbang(&x->x_gui, (t_int)atom_getFloatAtIndex(2, argc, argv));
        num = (int)(t_int)atom_getFloatAtIndex(3, argc, argv);
        iemgui_deserializeNamesByIndex(&x->x_gui, 4, argv);
        ldx = (int)(t_int)atom_getFloatAtIndex(7, argc, argv);
        ldy = (int)(t_int)atom_getFloatAtIndex(8, argc, argv);
        iemgui_deserializeFontStyle(&x->x_gui, (t_int)atom_getFloatAtIndex(9, argc, argv));
        fs = (int)(t_int)atom_getFloatAtIndex(10, argc, argv);
        bflcol[0] = (int)(t_int)atom_getFloatAtIndex(11, argc, argv);
        bflcol[1] = (int)(t_int)atom_getFloatAtIndex(12, argc, argv);
        bflcol[2] = (int)(t_int)atom_getFloatAtIndex(13, argc, argv);
        fval = (t_int)atom_getFloatAtIndex(14, argc, argv);
    }
    else iemgui_deserializeNamesByIndex(&x->x_gui, 4, 0);
    x->x_gui.iem_draw = (t_iemfn)vradio_draw;
    x->x_gui.iem_canSend = 1;
    x->x_gui.iem_canReceive = 1;
    x->x_gui.iem_glist = (t_glist *)canvas_getcurrent();
    if (!strcmp(x->x_gui.iem_send->s_name, "empty"))
        x->x_gui.iem_canSend = 0;
    if (!strcmp(x->x_gui.iem_receive->s_name, "empty"))
        x->x_gui.iem_canReceive = 0;

    if(num < 1)
        num = 1;
    if(num > IEM_MAXIMUM_BUTTONS)
        num = IEM_MAXIMUM_BUTTONS;
    x->x_numberOfButtons = num;
    x->x_floatValue = fval;
    on = fval;
    if(on < 0)
        on = 0;
    if(on >= x->x_numberOfButtons)
        on = x->x_numberOfButtons - 1;
    if(x->x_gui.iem_loadbang)
        x->x_state = on;
    else
        x->x_state = 0;
    x->x_changed = (chg==0)?0:1;
    if (x->x_gui.iem_canReceive)
        pd_bind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    x->x_gui.iem_labelX = ldx;
    x->x_gui.iem_labelY = ldy;
    if(fs < 4)
        fs = 4;
    x->x_gui.iem_fontSize = fs;
    x->x_gui.iem_width = PD_MAX (a, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    iemgui_checkSendReceiveLoop(&x->x_gui);
    iemgui_deserializeColors(&x->x_gui, bflcol);
    outlet_new(&x->x_gui.iem_obj, &s_list);
    return (x);
}

static void *vradio_new(t_symbol *s, int argc, t_atom *argv)
{
    return (vradio_donew(s, argc, argv));
}

static void vradio_ff(t_vradio *x)
{
    if(x->x_gui.iem_canReceive)
        pd_unbind(&x->x_gui.iem_obj.te_g.g_pd, x->x_gui.iem_receive);
    gfxstub_deleteforkey(x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void vradio_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("vradio"),
            (t_newmethod)vradio_new,
            (t_method)vradio_ff,
            sizeof (t_vradio),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, vradio_bang);
    class_addFloat (c, vradio_float);
    class_addClick (c, vradio_click);
    
    class_addMethod (c, (t_method)vradio_loadbang,      gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)vradio_init,          gensym ("init"),            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_dialog,        gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_size,          gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_delta,         gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_pos,           gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_label_font,    gensym ("label_font"),      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_label_pos,     gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_set,           gensym ("set"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_number,        gensym ("number"),          A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)vradio_send,          gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)vradio_receive,       gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)vradio_label,         gensym ("label"),           A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)vradio_dummy,         gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_dummy,         gensym ("single_change"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)vradio_dummy,         gensym ("double_change"),   A_GIMME, A_NULL);
    
    #endif
    
    vradio_widgetBehavior.w_getrectfn   = vradio_getrect;
    vradio_widgetBehavior.w_displacefn  = iemgui_behaviorDisplace;
    vradio_widgetBehavior.w_selectfn    = iemgui_behaviorSelected;
    vradio_widgetBehavior.w_activatefn  = NULL;
    vradio_widgetBehavior.w_deletefn    = iemgui_behaviorDeleted;
    vradio_widgetBehavior.w_visfn       = iemgui_behaviorVisible;
    vradio_widgetBehavior.w_clickfn     = vradio_newclick;
    
    class_setWidgetBehavior (c, &vradio_widgetBehavior);
    class_setHelpName (c, gensym ("vradio"));
    class_setSaveFunction (c, vradio_save);
    class_setPropertiesFunction (c, vradio_properties);
    
    vradio_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
