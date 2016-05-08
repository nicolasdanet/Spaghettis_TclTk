
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

t_symbol *sym__X;
t_symbol *sym_canvasmaker;
t_symbol *sym_click;
t_symbol *sym_inlet;
t_symbol *sym_key;
t_symbol *sym_loadbang;
t_symbol *sym_motion;                                        
t_symbol *sym_mouse;
t_symbol *sym_mouseup;
t_symbol *sym_objectmaker;
t_symbol *sym_setbounds;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void symbols_initialize (void)
{ 
    sym__X                  = gensym ("#X");
    sym_canvasmaker         = gensym ("canvasmaker");
    sym_click               = gensym ("click");
    sym_inlet               = gensym ("inlet");
    sym_key                 = gensym ("key");
    sym_loadbang            = gensym ("loadbang");
    sym_motion              = gensym ("motion");
    sym_mouse               = gensym ("mouse");
    sym_mouseup             = gensym ("mouseup");
    sym_objectmaker         = gensym ("objectmaker");
    sym_setbounds           = gensym ("setbounds");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
