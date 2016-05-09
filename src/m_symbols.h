
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

extern t_symbol *sym___comma;
extern t_symbol *sym___semicolon;
extern t_symbol *sym__audiodialog;
extern t_symbol *sym__audioproperties;
extern t_symbol *sym__data;
extern t_symbol *sym__dummy;
extern t_symbol *sym__end;
extern t_symbol *sym__font;
extern t_symbol *sym__key;
extern t_symbol *sym__keyup;
extern t_symbol *sym__keyname;
extern t_symbol *sym__mididialog;
extern t_symbol *sym__midiproperties;
extern t_symbol *sym__path;
extern t_symbol *sym__pop;
extern t_symbol *sym__quit;
extern t_symbol *sym__savepreferences;
extern t_symbol *sym__signoff;
extern t_symbol *sym__watchdog;
extern t_symbol *sym__A;
extern t_symbol *sym__X;
extern t_symbol *sym_bindlist;
extern t_symbol *sym_canvas;
extern t_symbol *sym_canvasmaker;
extern t_symbol *sym_click;
extern t_symbol *sym_connect;
extern t_symbol *sym_coords;
extern t_symbol *sym_disconnect;
extern t_symbol *sym_dsp;
extern t_symbol *sym_f;
extern t_symbol *sym_floatatom;
extern t_symbol *sym_guiconnect;
extern t_symbol *sym_guistub;
extern t_symbol *sym_graph;
extern t_symbol *sym_inlet;
extern t_symbol *sym_key;
extern t_symbol *sym_loadbang;
extern t_symbol *sym_motion;
extern t_symbol *sym_mouse;
extern t_symbol *sym_mouseup;
extern t_symbol *sym_msg;
extern t_symbol *sym_new;
extern t_symbol *sym_open;
extern t_symbol *sym_obj;
extern t_symbol *sym_objectmaker;
extern t_symbol *sym_pd;
extern t_symbol *sym_quit;
extern t_symbol *sym_restore;
extern t_symbol *sym_scalar;
extern t_symbol *sym_setbounds;
extern t_symbol *sym_struct;
extern t_symbol *sym_subpatch;
extern t_symbol *sym_symbolatom;
extern t_symbol *sym_text;
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
#endif // __m_symbols_h_
