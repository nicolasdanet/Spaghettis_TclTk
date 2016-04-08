
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

#define IEM_DIALOG_SIZE                 17

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_COLOR_NORMAL                0               // Black.
#define IEM_COLOR_SELECTED              255             // Blue.

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DEFAULT_SIZE                15
#define IEM_DEFAULT_FONTSIZE            10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DEFAULT_LABELX_TOP         -1
#define IEM_DEFAULT_LABELY_TOP         -8
#define IEM_DEFAULT_LABELX_NEXT         17
#define IEM_DEFAULT_LABELY_NEXT         7

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DEFAULT_COLORS              { -262144, -1, -1 }

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_MINIMUM_WIDTH               8
#define IEM_MINIMUM_HEIGHT              8
#define IEM_MINIMUM_FONTSIZE            4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_MAXIMUM_BUTTONS             128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DRAW_UPDATE                 0
#define IEM_DRAW_MOVE                   1
#define IEM_DRAW_NEW                    2
#define IEM_DRAW_SELECT                 3
#define IEM_DRAW_ERASE                  4
#define IEM_DRAW_CONFIG                 5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DIAL_BUFFER_LENGTH          32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iemcolors {
    int c_colorBackground;
    int c_colorForeground;
    int c_colorLabel;
    } t_iemcolors;

typedef struct _iemnames {
    t_symbol *n_unexpandedSend;
    t_symbol *n_unexpandedReceive;
    t_symbol *n_unexpandedLabel;
    } t_iemnames;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_iemfn)(void *x, t_glist *glist, int mode);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iem {
    t_object    iem_obj;                        /* MUST be the first. */
    t_glist     *iem_glist;
    t_iemfn     iem_draw;
    char        iem_fontStyle;                  /* Unused but kept for compatibility. */
    char        iem_canSend;
    char        iem_canReceive;
    char        iem_loadbang;
    char        iem_isSelected;
    char        iem_goThrough;
    int         iem_width;
    int         iem_height;
    int         iem_labelX;
    int         iem_labelY;
    int         iem_fontSize;
    int         iem_colorForeground;
    int         iem_colorBackground;
    int         iem_colorLabel;
    int         iem_cacheIndex;
    t_symbol    *iem_send;
    t_symbol    *iem_receive;
    t_symbol    *iem_label;
    t_symbol    *iem_unexpandedSend;
    t_symbol    *iem_unexpandedReceive;
    t_symbol    *iem_unexpandedLabel;
    } t_iem;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _bng {
    t_iem       x_gui;
    int         x_flashed;
    int         x_flashTimeBreak;
    int         x_flashTimeHold;
    t_clock     *x_clockHold;
    t_clock     *x_clockBreak;
    } t_bng;

typedef struct _toggle {
    t_iem       x_gui;
    t_float     x_state;
    t_float     x_nonZero;
    } t_toggle;

typedef struct _radio {
    t_iem       x_gui;
    int         x_isVertical;
    int         x_changed;                      /* Unused but kept for compatibility. */
    int         x_numberOfButtons;
    int         x_state;
    int         x_stateDrawn;
    t_float     x_floatValue;
    } t_radio;

typedef struct _slider {
    t_iem       x_gui;
    int         x_isVertical;
    int         x_position;
    int         x_isLogarithmic;
    int         x_isSteadyOnClick;
    int         x_isAccurateMoving;
    double      x_minimum;
    double      x_maximum;
    t_float     x_floatValue;
    } t_slider;

typedef struct _vu {
    t_iem       x_gui;
    int         x_hasScale;                     /* Unused but kept for compatibility. */
    int         x_thickness;
    int         x_peak;
    int         x_rms;
    t_float     x_peakValue;
    t_float     x_rmsValue;
    void        *x_outLeft;
    void        *x_outRight;
    } t_vu;

typedef struct _dial {
    t_iem       x_gui;
    int         x_hasChanged;
    int         x_isLogarithmic;
    int         x_isAccurateMoving;
    int         x_digitsNumber;
    int         x_digitsFontSize;
    int         x_logarithmSteps;
    double      x_value;
    double      x_minimum;
    double      x_maximum;
    double      x_k;
    char        x_t[IEM_DIAL_BUFFER_LENGTH];
    } t_dial;

typedef struct _my_canvas {
    t_iem       x_gui;
    t_atom      x_at[3];
    int         x_vis_w;
    int         x_vis_h;
    } t_my_canvas;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *iemgui_empty (void);    

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_deserializeColors            (t_iem *iem, t_iemcolors *c);
void        iemgui_serializeColors              (t_iem *iem, t_iemcolors *c);
void        iemgui_deserializeFontStyle         (t_iem *iem, int n);
int         iemgui_serializeFontStyle           (t_iem *iem);
void        iemgui_deserializeLoadbang          (t_iem *iem, int n);
int         iemgui_serializeLoadbang            (t_iem *iem);
void        iemgui_deserializeNamesByIndex      (t_iem *iem, int i, t_atom *argv);
void        iemgui_serializeNames               (t_iem *iem, t_iemnames *n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_checkSendReceiveLoop         (t_iem *iem);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_setSend                      (void *x, t_iem *iem, t_symbol *s);
void        iemgui_setReceive                   (void *x, t_iem *iem, t_symbol *s);
void        iemgui_setLabel                     (void *x, t_iem *iem, t_symbol *s);
void        iemgui_setLabelPosition             (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setLabelFont                 (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setColor                     (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setPosition                  (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv);
void        iemgui_movePosition                 (void *x, t_iem *iem, t_symbol *s, int argc, t_atom *argv);
void        iemgui_boxChanged                   (void *x, t_iem *iem);

void        iemgui_behaviorDisplace             (t_gobj *z, t_glist *glist, int deltaX, int deltaY);
void        iemgui_behaviorSelected             (t_gobj *z, t_glist *glist, int isSelected);
void        iemgui_behaviorVisible              (t_gobj *z, t_glist *glist, int isVisible);
void        iemgui_behaviorDeleted              (t_gobj *z, t_glist *glist);

void        iemgui_serialize                    (t_iem *iem, t_iemnames *n, t_iemcolors *c);
void        iemgui_fromDialog                   (t_iem *iem, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __g_iem_h_
