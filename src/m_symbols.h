
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
extern t_symbol *sym__pop;
extern t_symbol *sym__X;

extern t_symbol *sym_canvasmaker;
extern t_symbol *sym_click;
extern t_symbol *sym_dsp;
extern t_symbol *sym_f;
extern t_symbol *sym_inlet;
extern t_symbol *sym_key;
extern t_symbol *sym_loadbang;
extern t_symbol *sym_motion;
extern t_symbol *sym_mouse;
extern t_symbol *sym_mouseup;
extern t_symbol *sym_new;
extern t_symbol *sym_open;
extern t_symbol *sym_objectmaker;
extern t_symbol *sym_pd;
extern t_symbol *sym_quit;
extern t_symbol *sym_setbounds;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_symbols_h_
