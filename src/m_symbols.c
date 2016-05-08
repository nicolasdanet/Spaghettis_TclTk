
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_symbol s_pointer  = { "pointer"   , NULL, NULL };         /* Shared. */
t_symbol s_float    = { "float"     , NULL, NULL };         /* Shared. */
t_symbol s_symbol   = { "symbol"    , NULL, NULL };         /* Shared. */
t_symbol s_bang     = { "bang"      , NULL, NULL };         /* Shared. */
t_symbol s_list     = { "list"      , NULL, NULL };         /* Shared. */
t_symbol s_anything = { "anything"  , NULL, NULL };         /* Shared. */
t_symbol s_signal   = { "signal"    , NULL, NULL };         /* Shared. */
t_symbol s__N       = { "#N"        , NULL, NULL };         /* Shared. */
t_symbol s__X       = { "#X"        , NULL, NULL };         /* Shared. */
t_symbol s__A       = { "#A"        , NULL, NULL };         /* Shared. */
t_symbol s_         = { ""          , NULL, NULL };         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Shared. */

t_symbol *sym___comma;
t_symbol *sym___semicolon;

t_symbol *sym__audiodialog;
t_symbol *sym__audioproperties;
t_symbol *sym__dummy;
t_symbol *sym__font;
t_symbol *sym__mididialog;
t_symbol *sym__midiproperties;
t_symbol *sym__path;
t_symbol *sym__pop;
t_symbol *sym__quit;
t_symbol *sym__savepreferences;
t_symbol *sym__watchdog;

t_symbol *sym__X;

t_symbol *sym_canvasmaker;
t_symbol *sym_click;
t_symbol *sym_dsp;
t_symbol *sym_f;
t_symbol *sym_inlet;
t_symbol *sym_key;
t_symbol *sym_loadbang;
t_symbol *sym_motion;                                        
t_symbol *sym_mouse;
t_symbol *sym_mouseup;
t_symbol *sym_new;
t_symbol *sym_objectmaker;
t_symbol *sym_open;
t_symbol *sym_pd;
t_symbol *sym_quit;
t_symbol *sym_setbounds;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void symbols_initialize (void)
{ 
    sym___comma             = gensym (",");
    sym___semicolon         = gensym (";");

    sym__audiodialog        = gensym ("_audiodialog");
    sym__audioproperties    = gensym ("_audioproperties");
    sym__dummy              = gensym ("_dummy");
    sym__font               = gensym ("_font");
    sym__mididialog         = gensym ("_mididialog");
    sym__midiproperties     = gensym ("_midiproperties");
    sym__path               = gensym ("_path");
    sym__pop                = gensym ("_pop");
    sym__quit               = gensym ("_quit");
    sym__savepreferences    = gensym ("_savepreferences");
    sym__watchdog           = gensym ("_watchdog");
    
    sym__X                  = gensym ("#X");
    
    sym_canvasmaker         = gensym ("canvasmaker");
    sym_click               = gensym ("click");
    sym_dsp                 = gensym ("dsp");
    sym_f                   = gensym ("f");
    sym_inlet               = gensym ("inlet");
    sym_key                 = gensym ("key");
    sym_loadbang            = gensym ("loadbang");
    sym_motion              = gensym ("motion");
    sym_mouse               = gensym ("mouse");
    sym_mouseup             = gensym ("mouseup");
    sym_new                 = gensym ("new");
    sym_objectmaker         = gensym ("objectmaker");
    sym_open                = gensym ("open");
    sym_pd                  = gensym ("pd");
    sym_quit                = gensym ("quit");
    sym_setbounds           = gensym ("setbounds");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
