
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_iem_h_
#define __g_iem_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_COLOR_NORMAL                0               /* Black. */
#define IEM_COLOR_SELECTED              255             /* Blue.  */

#define IEM_MINIMUM_WIDTH               8
#define IEM_MINIMUM_HEIGHT              8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DRAW_UPDATE                 0
#define IEM_DRAW_MOVE                   1
#define IEM_DRAW_NEW                    2
#define IEM_DRAW_SELECT                 3
#define IEM_DRAW_ERASE                  4
#define IEM_DRAW_CONFIG                 5
#define IEM_DRAW_IO                     6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_NUMBER_BUFFER_LENGTH        32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iemflags {
    char iem_font;                      /* Unused but kept for compatibility. */
    char iem_scale;                     /* Unused but kept for compatibility. */
    char iem_canReceive;
    char iem_canSend;
    char iem_loadOnStart;
    char iem_isSelected;
    char iem_accurateMoving;
    char iem_goThrough;
    char iem_hasChanged;
    char iem_isLogarithmic;
    char iem_isSteadyOnClick;
    char iem_isLocked;
    } t_iemflags;

typedef struct _iemarguments {
    char iem_loadOnStart;
    char iem_scale;                     /* Unused but kept for compatibility. */
    char iem_flash;                     /* Unused. */
    char iem_isLocked;
    } t_iemarguments;

typedef struct _iemcolors {
    int colorBackground;
    int colorForeground;
    int colorLabel;
    } t_iemcolors;

typedef struct _iemnames {
    t_symbol *unexpendedSend;
    t_symbol *unexpendedReceive;
    t_symbol *unexpendedLabel;
    } t_iemnames;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_iemfn)(void *x, t_glist *glist, int mode);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_COLORS_DEFAULT      { -262144, -1, -1 }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iem {
    t_object            iem_obj;        /* MUST be the first. */
    t_glist             *iem_glist;
    t_iemfn             iem_draw;
    t_iemflags          iem_flags;
    t_iemarguments      x_isa;
    int                 iem_height;
    int                 iem_width;
    int                 iem_labelX;
    int                 iem_labelY;
    int                 iem_fontSize;
    int                 iem_colorForeground;
    int                 iem_colorBackground;
    int                 iem_colorLabel;
    int                 iem_cacheIndex;
    t_symbol            *iem_send;
    t_symbol            *iem_receive;
    t_symbol            *iem_label;
    t_symbol            *iem_unexpandedSend;
    t_symbol            *iem_unexpandedReceive;
    t_symbol            *iem_unexpandedLabel;
    } t_iem;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _bng {
    t_iem       x_gui;
    int         x_flashed;
    int         x_flashtime_break;
    int         x_flashtime_hold;
    t_clock     *x_clock_hld;
    t_clock     *x_clock_brk;
    t_clock     *x_clock_lck;
    } t_bng;

typedef struct _hslider {
    t_iem       x_gui;
    int         x_pos;
    int         x_val;
    int         x_isLogarithmic;
    int         x_isSteadyOnClick;
    double      x_min;
    double      x_max;
    double      x_k;
    t_float     x_fval;
    } t_hslider;

typedef struct _hradio {
    t_iem       x_gui;
    int         x_on;
    int         x_changed;
    int         x_number;
    int         x_drawn;
    t_float     x_fval;
    t_atom      x_at[2];
    } t_hradio;

typedef struct _toggle {
    t_iem       x_gui;
    t_float     x_on;
    t_float     x_nonzero;
    } t_toggle;

typedef struct _my_canvas {
    t_iem       x_gui;
    t_atom      x_at[3];
    int         x_vis_w;
    int         x_vis_h;
    } t_my_canvas;

typedef struct _vslider {
    t_iem       x_gui;
    int         x_pos;
    int         x_val;
    int         x_isLogarithmic;
    int         x_isSteadyOnClick;
    double      x_min;
    double      x_max;
    double      x_k;
    t_float     x_fval;
    } t_vslider;

typedef struct _vu {
    t_iem       x_gui;
    int         x_led_size;
    int         x_peak;
    int         x_rms;
    t_float     x_fp;
    t_float     x_fr;
    int         x_scale;
    void        *x_out_rms;
    void        *x_out_peak;
    char        x_updaterms;
    char        x_updatepeak;
    } t_vu;

typedef struct _my_numbox {
    t_iem       x_gui;
    t_clock     *x_clock_reset;
    t_clock     *x_clock_wait;
    double      x_val;
    double      x_min;
    double      x_max;
    double      x_k;
    int         x_hasChanged;
    int         x_isLogarithmic;
    char        x_buf[IEM_NUMBER_BUFFER_LENGTH];
    int         x_numwidth;
    int         x_log_height;
    } t_my_numbox;

typedef struct _vradio {
    t_iem       x_gui;
    int         x_on;
    int         x_changed;
    int         x_number;
    int         x_drawn;
    t_float     x_fval;
    t_atom      x_at[2];
    } t_vradio;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *iem_empty (void);    

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iem_getColors               (t_iem *iem, t_iemcolors *c);
void        iem_setColors               (t_iem *iem, t_iemcolors *c);

void        iem_setNamesByIndex        (t_iem *iem, int i, t_atom *argv);

void        iem_checkSendReceiveLoop    (t_iem *iem);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *iem_unique2dollarzero      (t_symbol *s, int unique_num, int and_unique_flag);
t_symbol    *iem_sym2dollararg          (t_symbol *s, int nth_arg, int tail_len);
t_symbol    *iem_dollarzero2unique      (t_symbol *s, int unique_num);
t_symbol    *iem_dollararg2sym          (t_symbol *s, int nth_arg, int tail_len, int pargc, t_atom *pargv);
int         iem_is_dollarzero           (t_symbol *s);
int         iem_is_dollararg            (t_symbol *s, int *tail_len);
void        iem_fetch_unique            (t_iem *iem);
void        iem_fetch_parent_args       (t_iem *iem, int *pargc, t_atom **pargv);
void        iem_all_unique2dollarzero   (t_iem *iem, t_symbol **srlsym);
void        iem_all_dollarzero2unique   (t_iem *iem, t_symbol **srlsym);

void        iem_send                    (void *x, t_iem *iem, t_symbol *s);
void        iem_receive                 (void *x, t_iem *iem, t_symbol *s);
void        iem_label                   (void *x, t_iem *iem, t_symbol *s);
void        iem_label_pos               (void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av);
void        iem_label_font              (void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av);
void        iem_size                    (void *x, t_iem *iem);
void        iem_delta                   (void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av);
void        iem_pos                     (void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av);
void        iem_color                   (void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av);
int         iem_list                    (void *x, t_iem *iem, t_symbol *s, int ac, t_atom *av);
void        iem_displace                (t_gobj *z, t_glist *glist, int dx, int dy);
void        iem_select                  (t_gobj *z, t_glist *glist, int selected);
void        iem_delete                  (t_gobj *z, t_glist *glist);
void        iem_vis                     (t_gobj *z, t_glist *glist, int vis);
void        iem_save                    (t_iem *iem, t_symbol **srl, t_iemcolors *c);
void        iem_properties              (t_iem *iem, t_symbol **srl);
void        iem_dialog                  (t_iem *iem, t_symbol **srl, int argc, t_atom *argv);
void        iem_inttosymargs            (t_iemarguments *symargp, int n);
int         iem_symargstoint            (t_iemarguments *symargp);
void        iem_inttofstyle             (t_iemflags *fstylep, int n);
int         iem_fstyletoint             (t_iemflags *fstylep);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __g_iem_h_
