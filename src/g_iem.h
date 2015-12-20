
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

#define IEM_COLOR_NORMAL            0
#define IEM_COLOR_SELECTED          255

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DRAW_UPDATE             0
#define IEM_DRAW_MOVE               1
#define IEM_DRAW_NEW                2
#define IEM_DRAW_SELECT             3
#define IEM_DRAW_ERASE              4
#define IEM_DRAW_CONFIG             5
#define IEM_DRAW_IO                 6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IS_FLOAT(atom, index)       ((atom + index)->a_type == A_FLOAT)
#define IS_SYMBOL(atom, index)      ((atom + index)->a_type == A_SYMBOL)
#define IS_DOLLAR(atom, index)      ((atom + index)->a_type == A_DOLLAR)
#define IS_DOLLSYM(atom, index)     ((atom + index)->a_type == A_DOLLARSYMBOL)
#define IS_POINTER(atom, index)     ((atom + index)->a_type == A_POINTER)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_NUMBER_BUFFER_LENGTH    32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iem_fstyle_flags {
    char x_font_style;                  /* Unused but kept for compatibility. */
    char x_rcv_able;
    char x_snd_able;
    char x_selected;
    char x_finemoved;
    char x_put_in2out;
    char x_change;
    char x_lin0_log1;
    char x_steady;
    } t_iem_fstyle_flags;

typedef struct _iem_init_symargs {
    char x_loadinit;
    char x_scale;
    char x_flashed;
    char x_locked;
    } t_iem_init_symargs;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_iemfunptr)(void *x, t_glist *glist, int mode);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _iem {
    t_object            x_obj;
    t_glist             *x_glist;
    t_iemfunptr         x_draw;
    int                 x_h;
    int                 x_w;
    int                 x_ldx;
    int                 x_ldy;
    t_iem_fstyle_flags  x_fsf;
    int                 x_fontsize;
    t_iem_init_symargs  x_isa;
    int                 x_fcol;
    int                 x_bcol;
    int                 x_lcol;
    t_symbol            *x_snd;
    t_symbol            *x_rcv;
    t_symbol            *x_lab;
    t_symbol            *x_snd_unexpanded;
    t_symbol            *x_rcv_unexpanded;
    t_symbol            *x_lab_unexpanded;
    int                 x_binbufindex;
    int                 x_labelbindex;
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
    int         x_lin0_log1;
    int         x_steady;
    double      x_min;
    double      x_max;
    double      x_k;
    t_float     x_fval;
    } t_hslider;

typedef struct _hradio {
    t_iem       x_gui;
    int         x_on;
    int         x_change;
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
    int         x_lin0_log1;
    int         x_steady;
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
    int         x_lin0_log1;
    char        x_buf[IEM_NUMBER_BUFFER_LENGTH];
    int         x_numwidth;
    int         x_log_height;
    } t_my_numbox;

typedef struct _vradio {
    t_iem       x_gui;
    int         x_on;
    int         x_change;
    int         x_number;
    int         x_drawn;
    t_float     x_fval;
    t_atom      x_at[2];
    } t_vradio;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         canvas_getdollarzero        (void);
void        canvas_getargs              (int *argcp, t_atom **argvp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         iem_clip_size               (int size);
int         iem_clip_font               (int size);
int         iem_modulo_color            (int col);
t_symbol    *iem_unique2dollarzero      (t_symbol *s, int unique_num, int and_unique_flag);
t_symbol    *iem_sym2dollararg          (t_symbol *s, int nth_arg, int tail_len);
t_symbol    *iem_dollarzero2unique      (t_symbol *s, int unique_num);
t_symbol    *iem_dollararg2sym          (t_symbol *s, int nth_arg, int tail_len, int pargc, t_atom *pargv);
int         iem_is_dollarzero           (t_symbol *s);
int         iem_is_dollararg            (t_symbol *s, int *tail_len);
void        iem_fetch_unique            (t_iem *iem);
void        iem_fetch_parent_args       (t_iem *iem, int *pargc, t_atom **pargv);
void        iem_verify_snd_ne_rcv       (t_iem *iem);
void        iem_all_unique2dollarzero   (t_iem *iem, t_symbol **srlsym);
void        iem_all_sym2dollararg       (t_iem *iem, t_symbol **srlsym);
void        iem_all_dollarzero2unique   (t_iem *iem, t_symbol **srlsym);
t_symbol    *iem_new_dogetname          (t_iem *iem, int indx, t_atom *argv);
void        iem_new_getnames            (t_iem *iem, int indx, t_atom *argv);
void        iem_all_dollararg2sym       (t_iem *iem, t_symbol **srlsym);
void        iem_all_col2save            (t_iem *iem, int *bflcol);
void        iem_all_colfromload         (t_iem *iem, int *bflcol);
int         iem_compatible_col          (int i);
void        iem_all_dollar2raute        (t_symbol **srlsym);
void        iem_all_raute2dollar        (t_symbol **srlsym);
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
void        iem_save                    (t_iem *iem, t_symbol **srl, int *bflcol);
void        iem_properties              (t_iem *iem, t_symbol **srl);
void        iem_dialog                  (t_iem *iem, t_symbol **srl, int argc, t_atom *argv);
void        iem_inttosymargs            (t_iem_init_symargs *symargp, int n);
int         iem_symargstoint            (t_iem_init_symargs *symargp);
void        iem_inttofstyle             (t_iem_fstyle_flags *fstylep, int n);
int         iem_fstyletoint             (t_iem_fstyle_flags *fstylep);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __g_iem_h_
