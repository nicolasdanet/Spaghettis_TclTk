
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

static t_widgetbehavior toggle_widgetBehavior;

static t_class *toggle_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void toggle_draw_update(t_toggle *x, t_glist *glist)
{
    if(glist_isvisible(glist))
    {
        t_glist *canvas=glist_getcanvas(glist);

        sys_vGui(".x%lx.c itemconfigure %lxX1 -fill #%6.6x\n", canvas, x,
                 (x->x_state!=0.0)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
        sys_vGui(".x%lx.c itemconfigure %lxX2 -fill #%6.6x\n", canvas, x,
                 (x->x_state!=0.0)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
    }
}

void toggle_draw_new(t_toggle *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int w=1, xx=text_xpix(&x->x_gui.iem_obj, glist), yy=text_ypix(&x->x_gui.iem_obj, glist);

    if(x->x_gui.iem_width >= 30)
        w = 2;
    if(x->x_gui.iem_width >= 60)
        w = 3;
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -fill #%6.6x -tags %lxBASE\n",
             canvas, xx, yy, xx + x->x_gui.iem_width, yy + x->x_gui.iem_height,
             x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxX1\n",
             canvas, xx+w+1, yy+w+1, xx + x->x_gui.iem_width-w, yy + x->x_gui.iem_height-w, w,
             (x->x_state!=0.0)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create line %d %d %d %d -width %d -fill #%6.6x -tags %lxX2\n",
             canvas, xx+w+1, yy + x->x_gui.iem_height-w-1, xx + x->x_gui.iem_width-w, yy+w, w,
             (x->x_state!=0.0)?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground, x);
    sys_vGui(".x%lx.c create text %d %d -text {%s} -anchor w \
             -font [::getFont %d] -fill #%6.6x -tags [list %lxLABEL label text]\n",
             canvas, xx+x->x_gui.iem_labelX,
             yy+x->x_gui.iem_labelY,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"",
             x->x_gui.iem_fontSize,
             x->x_gui.iem_colorLabel, x);

        /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxOUT%d outlet]\n",
             canvas, xx, yy + x->x_gui.iem_height-1, xx + INLETS_WIDTH, yy + x->x_gui.iem_height, x, 0);

        sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags [list %lxIN%d inlet]\n",
             canvas, xx, yy, xx + INLETS_WIDTH, yy+1, x, 0);*/
}

void toggle_draw_move(t_toggle *x, t_glist *glist)
{
    t_glist *canvas=glist_getcanvas(glist);
    int w=1, xx=text_xpix(&x->x_gui.iem_obj, glist), yy=text_ypix(&x->x_gui.iem_obj, glist);

    if(x->x_gui.iem_width >= 30)
        w = 2;

    if(x->x_gui.iem_width >= 60)
        w = 3;
    sys_vGui(".x%lx.c coords %lxBASE %d %d %d %d\n",
             canvas, x, xx, yy, xx + x->x_gui.iem_width, yy + x->x_gui.iem_height);
    sys_vGui(".x%lx.c itemconfigure %lxX1 -width %d\n", canvas, x, w);
    sys_vGui(".x%lx.c coords %lxX1 %d %d %d %d\n",
             canvas, x, xx+w+1, yy+w+1, xx + x->x_gui.iem_width-w, yy + x->x_gui.iem_height-w);
    sys_vGui(".x%lx.c itemconfigure %lxX2 -width %d\n", canvas, x, w);
    sys_vGui(".x%lx.c coords %lxX2 %d %d %d %d\n",
             canvas, x, xx+w+1, yy + x->x_gui.iem_height-w-1, xx + x->x_gui.iem_width-w, yy+w);
    sys_vGui(".x%lx.c coords %lxLABEL %d %d\n",
             canvas, x, xx+x->x_gui.iem_labelX, yy+x->x_gui.iem_labelY);
    /*sys_vGui(".x%lx.c coords %lxOUT%d %d %d %d %d\n",
             canvas, x, 0, xx, yy + x->x_gui.iem_height-1, xx + INLETS_WIDTH, yy + x->x_gui.iem_height);
    sys_vGui(".x%lx.c coords %lxIN%d %d %d %d %d\n",
             canvas, x, 0, xx, yy, xx + INLETS_WIDTH, yy+1);*/
}

void toggle_draw_erase(t_toggle* x, t_glist* glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c delete %lxBASE\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxX1\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxX2\n", canvas, x);
    sys_vGui(".x%lx.c delete %lxLABEL\n", canvas, x);
    //sys_vGui(".x%lx.c delete %lxOUT%d\n", canvas, x, 0);
    //sys_vGui(".x%lx.c delete %lxIN%d\n", canvas, x, 0);
}

void toggle_draw_config(t_toggle* x, t_glist* glist)
{
    t_glist *canvas=glist_getcanvas(glist);

    sys_vGui(".x%lx.c itemconfigure %lxLABEL -font [::getFont %d] -fill #%6.6x -text {%s} \n",
             canvas, x, x->x_gui.iem_fontSize,
             x->x_gui.iem_isSelected?IEM_COLOR_SELECTED:x->x_gui.iem_colorLabel,
             strcmp(x->x_gui.iem_label->s_name, "empty")?x->x_gui.iem_label->s_name:"");
    sys_vGui(".x%lx.c itemconfigure %lxBASE -fill #%6.6x\n", canvas, x,
             x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxX1 -fill #%6.6x\n", canvas, x,
             x->x_state?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
    sys_vGui(".x%lx.c itemconfigure %lxX2 -fill #%6.6x\n", canvas, x,
             x->x_state?x->x_gui.iem_colorForeground:x->x_gui.iem_colorBackground);
}

void toggle_draw_io(t_toggle* x, t_glist* glist)
{
    int xpos=text_xpix(&x->x_gui.iem_obj, glist);
    int ypos=text_ypix(&x->x_gui.iem_obj, glist);
    t_glist *canvas=glist_getcanvas(glist);

    /*sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxOUT%d\n",
        canvas, xpos,
        ypos + x->x_gui.iem_height-1, xpos + INLETS_WIDTH,
        ypos + x->x_gui.iem_height, x, 0);
    sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags %lxIN%d\n",
        canvas, xpos, ypos,
        xpos + INLETS_WIDTH, ypos+1, x, 0);*/
}

void toggle_draw_select(t_toggle* x, t_glist* glist)
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

void toggle_draw(t_toggle *x, t_glist *glist, int mode)
{
    if(mode == IEM_DRAW_UPDATE)
        toggle_draw_update(x, glist);
    else if(mode == IEM_DRAW_MOVE)
        toggle_draw_move(x, glist);
    else if(mode == IEM_DRAW_NEW)
        toggle_draw_new(x, glist);
    else if(mode == IEM_DRAW_SELECT)
        toggle_draw_select(x, glist);
    else if(mode == IEM_DRAW_ERASE)
        toggle_draw_erase(x, glist);
    else if(mode == IEM_DRAW_CONFIG)
        toggle_draw_config(x, glist);
}

/* ------------------------ tgl widgetbehaviour----------------------------- */

static void toggle_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_toggle *x = (t_toggle *)z;

    *xp1 = text_xpix(&x->x_gui.iem_obj, glist);
    *yp1 = text_ypix(&x->x_gui.iem_obj, glist);
    *xp2 = *xp1 + x->x_gui.iem_width;
    *yp2 = *yp1 + x->x_gui.iem_height;
}

static void toggle_save(t_gobj *z, t_buffer *b)
{
    t_toggle *x = (t_toggle *)z;
    int bflcol[3];
    t_symbol *srl[3];

    iemgui_serialize(&x->x_gui, srl, bflcol);
    buffer_vAppend(b, "ssiisiisssiiiiiiiff", gensym ("#X"),gensym ("obj"),
                (int)x->x_gui.iem_obj.te_xCoordinate,
                (int)x->x_gui.iem_obj.te_yCoordinate,
                gensym ("tgl"), x->x_gui.iem_width,
                iemgui_serializeLoadbang(&x->x_gui),
                srl[0], srl[1], srl[2],
                x->x_gui.iem_labelX, x->x_gui.iem_labelY,
                iemgui_serializeFontStyle(&x->x_gui), x->x_gui.iem_fontSize,
                bflcol[0], bflcol[1], bflcol[2], x->x_state, x->x_nonZero);
    buffer_vAppend(b, ";");
}

static void toggle_properties(t_gobj *z, t_glist *owner)
{
    t_toggle *x = (t_toggle *)z;
    char buf[800];
    t_symbol *srl[3];

    iemgui_serializeNames(&x->x_gui, srl);
    sprintf(buf, "::ui_iem::create %%s Toggle \
            %d %d Size 0 0 empty \
            %g {Non-Zero Value} 0 empty \
            -1 empty empty \
            %d \
            -1 -1 empty \
            %s %s \
            %s %d %d \
            %d \
            %d %d %d \
            -1\n",
            x->x_gui.iem_width, IEM_MINIMUM_WIDTH,
            x->x_nonZero,
            x->x_gui.iem_loadbang,
            srl[0]->s_name, srl[1]->s_name,
            srl[2]->s_name, x->x_gui.iem_labelX, x->x_gui.iem_labelY,
            x->x_gui.iem_fontSize,
            0xffffff & x->x_gui.iem_colorBackground, 0xffffff & x->x_gui.iem_colorForeground, 0xffffff & x->x_gui.iem_colorLabel);
    gfxstub_new(&x->x_gui.iem_obj.te_g.g_pd, x, buf);
}

static void toggle_bang(t_toggle *x)
{
    x->x_state = (x->x_state==0.0)?x->x_nonZero:0.0;
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
    outlet_float(x->x_gui.iem_obj.te_outlet, x->x_state);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_float(x->x_gui.iem_send->s_thing, x->x_state);
}

static void toggle_dialog(t_toggle *x, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)(t_int)atom_getFloatAtIndex(0, argc, argv);
    t_float nonzero = (t_float)atom_getFloatAtIndex(2, argc, argv);

    if(nonzero == 0.0)
        nonzero = 1.0;
    x->x_nonZero = nonzero;
    if(x->x_state != 0.0)
        x->x_state = x->x_nonZero;
    iemgui_fromDialog(&x->x_gui, argc, argv);
    x->x_gui.iem_width = PD_MAX (a, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_CONFIG);
    (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_MOVE);
    canvas_fixlines(x->x_gui.iem_glist, (t_object*)x);
}

static void toggle_click(t_toggle *x, t_float xpos, t_float ypos, t_float shift, t_float ctrl, t_float alt)
{toggle_bang(x);}

static int toggle_newclick(t_gobj *z, struct _glist *glist, int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    if(doit)
        toggle_click((t_toggle *)z, (t_float)xpix, (t_float)ypix, (t_float)shift, 0, (t_float)alt);
    return (1);
}

static void toggle_set(t_toggle *x, t_float f)
{
    int old = (x->x_state != 0);
    x->x_state = f;
    if (f != 0.0 && 0)
        x->x_nonZero = f;
    if ((x->x_state != 0) != old)
        (*x->x_gui.iem_draw)(x, x->x_gui.iem_glist, IEM_DRAW_UPDATE);
}

static void toggle_float(t_toggle *x, t_float f)
{
    toggle_set(x, f);
    if(x->x_gui.iem_goThrough)
    {
        outlet_float(x->x_gui.iem_obj.te_outlet, x->x_state);
        if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
            pd_float(x->x_gui.iem_send->s_thing, x->x_state);
    }
}

static void toggle_fout(t_toggle *x, t_float f)
{
    toggle_set(x, f);
    outlet_float(x->x_gui.iem_obj.te_outlet, x->x_state);
    if(x->x_gui.iem_canSend && x->x_gui.iem_send->s_thing)
        pd_float(x->x_gui.iem_send->s_thing, x->x_state);
}

static void toggle_loadbang(t_toggle *x)
{
    if(x->x_gui.iem_loadbang)
        toggle_fout(x, (t_float)x->x_state);
}

static void toggle_size(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{
    int w = atom_getFloatAtIndex(0, ac, av);
    x->x_gui.iem_width = PD_MAX (w, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_height = x->x_gui.iem_width;
    iemgui_boxChanged((void *)x, &x->x_gui);
}

static void toggle_delta(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_movePosition((void *)x, &x->x_gui, s, ac, av);}

static void toggle_pos(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setPosition((void *)x, &x->x_gui, s, ac, av);}

static void toggle_color(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setColor((void *)x, &x->x_gui, s, ac, av);}

static void toggle_send(t_toggle *x, t_symbol *s)
{iemgui_setSend(x, &x->x_gui, s);}

static void toggle_receive(t_toggle *x, t_symbol *s)
{iemgui_setReceive(x, &x->x_gui, s);}

static void toggle_label(t_toggle *x, t_symbol *s)
{iemgui_setLabel((void *)x, &x->x_gui, s);}

static void toggle_label_font(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelFont((void *)x, &x->x_gui, s, ac, av);}

static void toggle_label_pos(t_toggle *x, t_symbol *s, int ac, t_atom *av)
{iemgui_setLabelPosition((void *)x, &x->x_gui, s, ac, av);}

static void toggle_init(t_toggle *x, t_float f)
{
    x->x_gui.iem_loadbang = (f==0.0)?0:1;
}

static void toggle_nonzero(t_toggle *x, t_float f)
{
    if(f != 0.0)
        x->x_nonZero = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *toggle_new (t_symbol *s, int argc, t_atom *argv)
{
    t_toggle *x = (t_toggle *)pd_new (toggle_class);
    
    int size            = IEM_DEFAULT_SIZE;
    int labelX          = IEM_DEFAULT_LABELX;
    int labelY          = IEM_DEFAULT_LABELY;
    int labelFontSize   = IEM_DEFAULT_FONTSIZE;
    t_float state       = 0.0;
    t_float nonZero     = 1.0;
    t_iemcolors colors  = IEM_COLORS_DEFAULT;

    if (argc >= 13                                              // --
            && IS_FLOAT (argv)                                  // Size.
            && IS_FLOAT (argv + 1)                              // Loadbang.
            && (IS_SYMBOL (argv + 2) || IS_FLOAT (argv + 2))    // Send.
            && (IS_SYMBOL (argv + 3) || IS_FLOAT (argv + 3))    // Receive.
            && (IS_SYMBOL (argv + 4) || IS_FLOAT (argv + 4))    // Label.
            && IS_FLOAT (argv + 5)                              // Label X.
            && IS_FLOAT (argv + 6)                              // Label Y.
            && IS_FLOAT (argv + 7)                              // Label font.
            && IS_FLOAT (argv + 8)                              // Label font size.
            && IS_FLOAT (argv + 9)                              // Background color.
            && IS_FLOAT (argv + 10)                             // Foreground color.
            && IS_FLOAT (argv + 11)                             // Label color.
            && IS_FLOAT (argv + 12))                            // Toggle state.
    {
        size                        = (int)atom_getFloatAtIndex (0, argc,  argv);
        labelX                      = (int)atom_getFloatAtIndex (5, argc,  argv);
        labelY                      = (int)atom_getFloatAtIndex (6, argc,  argv);
        labelFontSize               = (int)atom_getFloatAtIndex (8, argc,  argv);
        colors.c_colorBackground    = (int)atom_getFloatAtIndex (9, argc,  argv);
        colors.c_colorForeground    = (int)atom_getFloatAtIndex (10, argc, argv);
        colors.c_colorLabel         = (int)atom_getFloatAtIndex (11, argc, argv);
        state                       = (t_float)atom_getFloatAtIndex (12, argc, argv);
        nonZero                     = (argc == 14) ? atom_getFloatAtIndex (13, argc, argv) : 1.0;
        
        iemgui_deserializeLoadbang (&x->x_gui, (int)atom_getFloatAtIndex (1, argc, argv));
        iemgui_deserializeNamesByIndex (&x->x_gui, 2, argv);
        iemgui_deserializeFontStyle (&x->x_gui, (int)atom_getFloatAtIndex (7, argc, argv));
        
    } else {
        iemgui_deserializeNamesByIndex (&x->x_gui, 2, NULL);
    }
    
    x->x_gui.iem_glist      = (t_glist *)canvas_getcurrent();
    x->x_gui.iem_draw       = (t_iemfn)toggle_draw;
    x->x_gui.iem_canSend    = (x->x_gui.iem_send == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_canReceive = (x->x_gui.iem_receive == iemgui_empty()) ? 0 : 1;
    x->x_gui.iem_height     = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_width      = PD_MAX (size, IEM_MINIMUM_WIDTH);
    x->x_gui.iem_labelX     = labelX;
    x->x_gui.iem_labelY     = labelY;
    x->x_gui.iem_fontSize   = PD_MAX (labelFontSize, IEM_MINIMUM_FONTSIZE);
    
    iemgui_deserializeColors (&x->x_gui, &colors);
    iemgui_checkSendReceiveLoop (&x->x_gui);
    
    if (x->x_gui.iem_canReceive) { pd_bind (cast_pd (x), x->x_gui.iem_receive); }
        
    x->x_nonZero = (nonZero != 0.0) ? nonZero : 1.0;
    
    if (x->x_gui.iem_loadbang) { x->x_state = (state != 0.0) ? nonZero : 0.0; }
    else {
        x->x_state = 0.0;
    }

    outlet_new (cast_object (x), &s_float);
    
    return x;
}

static void toggle_free (t_toggle *x)
{
    if (x->x_gui.iem_canReceive) { pd_unbind (cast_pd (x), x->x_gui.iem_receive); }
    
    gfxstub_deleteforkey (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void toggle_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (gensym ("tgl"),
            (t_newmethod)toggle_new,
            (t_method)toggle_free,
            sizeof (t_toggle),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addCreator ((t_newmethod)toggle_new, gensym ("toggle"), A_GIMME, A_NULL);
    
    class_addBang (c, toggle_bang);
    class_addFloat (c, toggle_float);
    class_addClick (c, toggle_click);
    
    class_addMethod (c, (t_method)toggle_loadbang,      gensym ("loadbang"),        A_NULL);
    class_addMethod (c, (t_method)toggle_init,          gensym ("initialize"),      A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_dialog,        gensym ("dialog"),          A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_size,          gensym ("size"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_delta,         gensym ("move"),            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_pos,           gensym ("position"),        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_label_pos,     gensym ("labelposition"),   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_label_font,    gensym ("labelfont"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_set,           gensym ("set"),             A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_nonzero,       gensym ("nonzero"),         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_send,          gensym ("send"),            A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)toggle_receive,       gensym ("receive"),         A_DEFSYMBOL, A_NULL);
    class_addMethod (c, (t_method)toggle_label,         gensym ("label"),           A_DEFSYMBOL, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)toggle_init,          gensym ("init"),            A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)toggle_delta,         gensym ("delta"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_pos,           gensym ("pos"),             A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_color,         gensym ("color"),           A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_label_pos,     gensym ("label_pos"),       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)toggle_label_font,    gensym ("label_font"),      A_GIMME, A_NULL);
    
    #endif
    
    toggle_widgetBehavior.w_getrectfn   = toggle_getrect;
    toggle_widgetBehavior.w_displacefn  = iemgui_behaviorDisplace;
    toggle_widgetBehavior.w_selectfn    = iemgui_behaviorSelected;
    toggle_widgetBehavior.w_activatefn  = NULL;
    toggle_widgetBehavior.w_deletefn    = iemgui_behaviorDeleted;
    toggle_widgetBehavior.w_visfn       = iemgui_behaviorVisible;
    toggle_widgetBehavior.w_clickfn     = toggle_newclick;
    
    class_setWidgetBehavior (c, &toggle_widgetBehavior);
    class_setHelpName (c, gensym ("tgl"));
    class_setSaveFunction (c, toggle_save);
    class_setPropertiesFunction (c, toggle_properties);
    
    toggle_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
