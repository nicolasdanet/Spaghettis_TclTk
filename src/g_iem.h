
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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

#define IEM_DEFAULT_SIZE                15
#define IEM_DEFAULT_FONTSIZE            10

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define IEM_DEFAULT_LABELX_TOP          0
#define IEM_DEFAULT_LABELY_TOP         -((IEM_DEFAULT_FONTSIZE / 2) + 2)
#define IEM_DEFAULT_LABELX_NEXT         (IEM_DEFAULT_SIZE + 2)
#define IEM_DEFAULT_LABELY_NEXT         ((IEM_DEFAULT_FONTSIZE / 2) + 2)

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

#define IEM_DIGITS_SIZE                 64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iemcolors {
    t_symbol    *c_symColorBackground;
    t_symbol    *c_symColorForeground;
    t_symbol    *c_symColorLabel;
    } t_iemcolors;

typedef struct _iemnames {
    t_symbol    *n_unexpandedSend;
    t_symbol    *n_unexpandedReceive;
    t_symbol    *n_unexpandedLabel;
    } t_iemnames;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_iemfn) (void *x, t_glist *glist, int mode);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _iem {
    t_object    iem_obj;                        /* MUST be the first. */
    t_glist     *iem_owner;
    t_iemfn     iem_fnDraw;
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
    t_fontsize  iem_fontSize;
    t_color     iem_colorBackground;
    t_color     iem_colorForeground;
    t_color     iem_colorLabel;
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
    t_iem       x_gui;                          /* MUST be the first. */
    int         x_flashed;
    int         x_flashTimeBreak;               /* Unused but kept for compatibility. */
    int         x_flashTimeHold;
    t_outlet    *x_outlet;
    t_clock     *x_clock;
    } t_bng;

typedef struct _toggle {
    t_iem       x_gui;
    t_float     x_state;
    t_float     x_nonZero;
    t_outlet    *x_outlet;
    } t_toggle;

typedef struct _radio {
    t_iem       x_gui;
    int         x_isVertical;
    int         x_changed;                      /* Unused but kept for compatibility. */
    int         x_numberOfButtons;
    int         x_state;
    int         x_stateDrawn;
    t_float     x_floatValue;
    t_outlet    *x_outlet;
    } t_radio;

typedef struct _slider {
    t_iem       x_gui;
    int         x_isVertical;
    int         x_position;
    int         x_isLogarithmic;
    int         x_isSteadyOnClick;
    double      x_minimum;
    double      x_maximum;
    t_float     x_floatValue;
    t_outlet    *x_outlet;
    } t_slider;

typedef struct _dial {
    t_iem       x_gui;
    char        x_t[IEM_DIGITS_SIZE];
    int         x_hasKnob;
    int         x_position;
    int         x_isLogarithmic;
    int         x_digitsNumber;
    int         x_digitsFontSize;
    int         x_steps;
    double      x_minimum;
    double      x_maximum;
    t_float     x_floatValue;
    t_outlet    *x_outlet;
    } t_dial;
    
typedef struct _vu {
    t_iem       x_gui;
    int         x_hasScale;                     /* Unused but kept for compatibility. */
    int         x_thickness;
    int         x_peak;
    int         x_rms;
    t_float     x_peakValue;
    t_float     x_rmsValue;
    t_outlet    *x_outletLeft;
    t_outlet    *x_outletRight;
    } t_vu;

typedef struct _panel {
    t_iem       x_gui;
    t_atom      x_t[2];
    int         x_panelWidth;
    int         x_panelHeight;
    } t_panel;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_deserializeColors            (t_iem *iem, t_atom *bgrd, t_atom *fgrd, t_atom *label);
void        iemgui_serializeColors              (t_iem *iem, t_iemcolors *c);
void        iemgui_deserializeFontStyle         (t_iem *iem, int n);
int         iemgui_serializeFontStyle           (t_iem *iem);
void        iemgui_deserializeLoadbang          (t_iem *iem, int n);
int         iemgui_serializeLoadbang            (t_iem *iem);
void        iemgui_deserializeNames             (t_iem *iem, int i, t_atom *argv);
void        iemgui_serializeNames               (t_iem *iem, t_iemnames *n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_checkSendReceiveLoop         (t_iem *iem);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_behaviorDisplaced            (t_gobj *z, t_glist *glist, int deltaX, int deltaY);
void        iemgui_behaviorSelected             (t_gobj *z, t_glist *glist, int isSelected);
void        iemgui_behaviorVisibilityChanged    (t_gobj *z, t_glist *glist, int isVisible);
void        iemgui_behaviorDeleted              (t_gobj *z, t_glist *glist);

void        iemgui_serialize                    (t_iem *iem, t_iemnames *n, t_iemcolors *c);
void        iemgui_fromDialog                   (t_iem *iem, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_setSend                      (void *x, t_symbol *s);
void        iemgui_setReceive                   (void *x, t_symbol *s);
void        iemgui_setLabel                     (void *x, t_symbol *s);
void        iemgui_setLabelPosition             (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setLabelFont                 (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setBackgroundColor           (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setForegroundColor           (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setLabelColor                (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_setPosition                  (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_movePosition                 (void *x, t_symbol *s, int argc, t_atom *argv);
void        iemgui_boxChanged                   (void *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        iemgui_dummy                        (void *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __g_iem_h_
