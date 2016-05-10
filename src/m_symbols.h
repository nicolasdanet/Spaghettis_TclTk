
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_symbols_h_
#define __m_symbols_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void symbols_initialize (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol s_pointer;
extern t_symbol s_float;
extern t_symbol s_symbol;
extern t_symbol s_bang;
extern t_symbol s_list;
extern t_symbol s_anything;
extern t_symbol s_signal;
extern t_symbol s__N;
extern t_symbol s__X;
extern t_symbol s__A;
extern t_symbol s_;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol *sym___ampersand____ampersand__;
extern t_symbol *sym___comma__;
extern t_symbol *sym___dash__;
extern t_symbol *sym___dot__;
extern t_symbol *sym___minus__;
extern t_symbol *sym___semicolon__;
extern t_symbol *sym__arraydialog;
extern t_symbol *sym__audiodialog;
extern t_symbol *sym__audioproperties;
extern t_symbol *sym__canvasdialog;
extern t_symbol *sym__cut;
extern t_symbol *sym__copy;
extern t_symbol *sym__data;
extern t_symbol *sym__duplicate;
extern t_symbol *sym__dummy;
extern t_symbol *sym__end;
extern t_symbol *sym__float_array_template;
extern t_symbol *sym__float_template;
extern t_symbol *sym__font;
extern t_symbol *sym__iemdialog;
extern t_symbol *sym__key;
extern t_symbol *sym__keyup;
extern t_symbol *sym__keyname;
extern t_symbol *sym__map;
extern t_symbol *sym__mididialog;
extern t_symbol *sym__midiproperties;
extern t_symbol *sym__paste;
extern t_symbol *sym__path;
extern t_symbol *sym__pop;
extern t_symbol *sym__popupdialog;
extern t_symbol *sym__quit;
extern t_symbol *sym__savepreferences;
extern t_symbol *sym__selectall;
extern t_symbol *sym__signoff;
extern t_symbol *sym__watchdog;
extern t_symbol *sym__A;
extern t_symbol *sym__N;
extern t_symbol *sym__X;
extern t_symbol *sym_add;
extern t_symbol *sym_addcomma;
extern t_symbol *sym_adddollar;
extern t_symbol *sym_append;
extern t_symbol *sym_array;
extern t_symbol *sym_atom;
extern t_symbol *sym_backgroundcolor;
extern t_symbol *sym_bindlist;
extern t_symbol *sym_bng;
extern t_symbol *sym_bounds;
extern t_symbol *sym_buttonsnumber;
extern t_symbol *sym_canvas;
extern t_symbol *sym_canvasmaker;
extern t_symbol *sym_change;
extern t_symbol *sym_clear;
extern t_symbol *sym_click;
extern t_symbol *sym_clip;
extern t_symbol *sym_close;
extern t_symbol *sym_cnv;
extern t_symbol *sym_color;
extern t_symbol *sym_comment;
extern t_symbol *sym_connect;
extern t_symbol *sym_coords;
extern t_symbol *sym_cosinesum;
extern t_symbol *sym_data;
extern t_symbol *sym_dbtopow;
extern t_symbol *sym_dbtorms;
extern t_symbol *sym_deselect;
extern t_symbol *sym_dirty;
extern t_symbol *sym_disconnect;
extern t_symbol *sym_displace;
extern t_symbol *sym_drawcurve;
extern t_symbol *sym_drawnumber;
extern t_symbol *sym_drawpolygon;
extern t_symbol *sym_drawtext;
extern t_symbol *sym_drawsymbol;
extern t_symbol *sym_dsp;
extern t_symbol *sym_editmode;
extern t_symbol *sym_element;
extern t_symbol *sym_empty;
extern t_symbol *sym_f;
extern t_symbol *sym_filledcurve;
extern t_symbol *sym_filledpolygon;
extern t_symbol *sym_ft1;
extern t_symbol *sym_flashtime;
extern t_symbol *sym_floatatom;
extern t_symbol *sym_foregroundcolor;
extern t_symbol *sym_ftom;
extern t_symbol *sym_gatom;
extern t_symbol *sym_get;
extern t_symbol *sym_getposition;
extern t_symbol *sym_getsize;
extern t_symbol *sym_guiconnect;
extern t_symbol *sym_guistub;
extern t_symbol *sym_graph;
extern t_symbol *sym_gripsize;
extern t_symbol *sym_hold;
extern t_symbol *sym_hradio;
extern t_symbol *sym_hslider;
extern t_symbol *sym_initialize;
extern t_symbol *sym_inlet;
extern t_symbol *sym_inlet__tilde__;
extern t_symbol *sym_intatom;
extern t_symbol *sym_key;
extern t_symbol *sym_label;
extern t_symbol *sym_labelcolor;
extern t_symbol *sym_labelfont;
extern t_symbol *sym_labelposition;
extern t_symbol *sym_linear;
extern t_symbol *sym_linewidth;
extern t_symbol *sym_loadbang;
extern t_symbol *sym_logarithmic;
extern t_symbol *sym_mergefile;
extern t_symbol *sym_message;
extern t_symbol *sym_messresponder;
extern t_symbol *sym_motion;
extern t_symbol *sym_mouse;
extern t_symbol *sym_mouseup;
extern t_symbol *sym_move;
extern t_symbol *sym_msg;
extern t_symbol *sym_mtof;
extern t_symbol *sym_nbx;
extern t_symbol *sym_new;
extern t_symbol *sym_next;
extern t_symbol *sym_nonzero;
extern t_symbol *sym_normalize;
extern t_symbol *sym_number;
extern t_symbol *sym_obj;
extern t_symbol *sym_objectmaker;
extern t_symbol *sym_open;
extern t_symbol *sym_outlet;
extern t_symbol *sym_outlet__tilde__;
extern t_symbol *sym_pad;
extern t_symbol *sym_panelsize;
extern t_symbol *sym_pd;
extern t_symbol *sym_pd__dash__float__dash__array;
extern t_symbol *sym_plot;
extern t_symbol *sym_pointer;
extern t_symbol *sym_position;
extern t_symbol *sym_pow;
extern t_symbol *sym_powtodb;
extern t_symbol *sym_print;
extern t_symbol *sym_quit;
extern t_symbol *sym_radio;
extern t_symbol *sym_range;
extern t_symbol *sym_read;
extern t_symbol *sym_receive;
extern t_symbol *sym_rename;
extern t_symbol *sym_resize;
extern t_symbol *sym_restore;
extern t_symbol *sym_rewind;
extern t_symbol *sym_rmstodb;
extern t_symbol *sym_saveto;
extern t_symbol *sym_savetofile;
extern t_symbol *sym_scalar;
extern t_symbol *sym_scale;
extern t_symbol *sym_select;
extern t_symbol *sym_send;
extern t_symbol *sym_set;
extern t_symbol *sym_setbounds;
extern t_symbol *sym_setsize;
extern t_symbol *sym_sinesum;
extern t_symbol *sym_size;
extern t_symbol *sym_slider;
extern t_symbol *sym_sort;
extern t_symbol *sym_sqrt;
extern t_symbol *sym_steady;
extern t_symbol *sym_steps;
extern t_symbol *sym_struct;
extern t_symbol *sym_style;
extern t_symbol *sym_subpatch;
extern t_symbol *sym_symbolatom;
extern t_symbol *sym_template;
extern t_symbol *sym_text;
extern t_symbol *sym_tgl;
extern t_symbol *sym_traverse;
extern t_symbol *sym_visible;
extern t_symbol *sym_vradio;
extern t_symbol *sym_vslider;
extern t_symbol *sym_vu;
extern t_symbol *sym_w;
extern t_symbol *sym_write;
extern t_symbol *sym_x;
extern t_symbol *sym_xticks;
extern t_symbol *sym_y;
extern t_symbol *sym_yticks;
extern t_symbol *sym_z;
extern t_symbol *sym_BackSpace;
extern t_symbol *sym_Delete;
extern t_symbol *sym_Down;
extern t_symbol *sym_Escape;
extern t_symbol *sym_Left;
extern t_symbol *sym_Return;
extern t_symbol *sym_Right;
extern t_symbol *sym_Space;
extern t_symbol *sym_Tab;
extern t_symbol *sym_Up;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_LEGACY

extern t_symbol *sym_add2;
extern t_symbol *sym_adddollsym;
extern t_symbol *sym_addsemi;
extern t_symbol *sym_const;
extern t_symbol *sym_delta;
extern t_symbol *sym_double_change;
extern t_symbol *sym_get_pos;
extern t_symbol *sym_hsl;
extern t_symbol *sym_init;
extern t_symbol *sym_label_font;
extern t_symbol *sym_label_pos;
extern t_symbol *sym_lin;
extern t_symbol *sym_log;
extern t_symbol *sym_log_height;
extern t_symbol *sym_menu__dash__open;
extern t_symbol *sym_menuarray;
extern t_symbol *sym_menuclose;
extern t_symbol *sym_menusave;
extern t_symbol *sym_menusaveas;
extern t_symbol *sym_my_canvas;
extern t_symbol *sym_my_numbox;
extern t_symbol *sym_mycnv;
extern t_symbol *sym_numbox;
extern t_symbol *sym_page;
extern t_symbol *sym_param;
extern t_symbol *sym_pos;
extern t_symbol *sym_send__dash__window;
extern t_symbol *sym_single_change;
extern t_symbol *sym_toggle;
extern t_symbol *sym_vis;
extern t_symbol *sym_vis_size;
extern t_symbol *sym_vnext;
extern t_symbol *sym_vsl;
extern t_symbol *sym_vumeter;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_symbols_h_
