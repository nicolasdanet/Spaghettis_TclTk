
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
t_symbol *sym__data;
t_symbol *sym__end;
t_symbol *sym__dummy;
t_symbol *sym__font;
t_symbol *sym__key;
t_symbol *sym__keyup;
t_symbol *sym__keyname;
t_symbol *sym__mididialog;
t_symbol *sym__midiproperties;
t_symbol *sym__path;
t_symbol *sym__pop;
t_symbol *sym__quit;
t_symbol *sym__savepreferences;
t_symbol *sym__signoff;
t_symbol *sym__watchdog;
t_symbol *sym__A;
t_symbol *sym__X;
t_symbol *sym_bindlist;
t_symbol *sym_canvas;
t_symbol *sym_canvasmaker;
t_symbol *sym_click;
t_symbol *sym_connect;
t_symbol *sym_coords;
t_symbol *sym_disconnect;
t_symbol *sym_dsp;
t_symbol *sym_f;
t_symbol *sym_floatatom;
t_symbol *sym_guiconnect;
t_symbol *sym_guistub;
t_symbol *sym_graph;
t_symbol *sym_inlet;
t_symbol *sym_key;
t_symbol *sym_loadbang;
t_symbol *sym_motion;                                        
t_symbol *sym_mouse;
t_symbol *sym_mouseup;
t_symbol *sym_msg;
t_symbol *sym_new;
t_symbol *sym_obj;
t_symbol *sym_objectmaker;
t_symbol *sym_open;
t_symbol *sym_pd;
t_symbol *sym_quit;
t_symbol *sym_restore;
t_symbol *sym_scalar;
t_symbol *sym_setbounds;
t_symbol *sym_struct;
t_symbol *sym_subpatch;
t_symbol *sym_symbolatom;
t_symbol *sym_text;
t_symbol *sym_BackSpace;
t_symbol *sym_Delete;
t_symbol *sym_Down;
t_symbol *sym_Escape;
t_symbol *sym_Left;
t_symbol *sym_Space;
t_symbol *sym_Return;
t_symbol *sym_Right;
t_symbol *sym_Tab;
t_symbol *sym_Up;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void symbols_initialize (void)
{ 
    sym___comma             = gensym (",");
    sym___semicolon         = gensym (";");
    sym__audiodialog        = gensym ("_audiodialog");
    sym__audioproperties    = gensym ("_audioproperties");
    sym__data               = gensym ("_data");
    sym__end                = gensym ("_end");
    sym__dummy              = gensym ("_dummy");
    sym__font               = gensym ("_font");
    sym__key                = gensym ("_key");
    sym__keyup              = gensym ("_keyup");
    sym__keyname            = gensym ("_keyname");
    sym__mididialog         = gensym ("_mididialog");
    sym__midiproperties     = gensym ("_midiproperties");
    sym__path               = gensym ("_path");
    sym__pop                = gensym ("_pop");
    sym__quit               = gensym ("_quit");
    sym__savepreferences    = gensym ("_savepreferences");
    sym__signoff            = gensym ("_signoff");
    sym__watchdog           = gensym ("_watchdog");
    sym__A                  = gensym ("#A");
    sym__X                  = gensym ("#X");
    sym_bindlist            = gensym ("bindlist");
    sym_canvas              = gensym ("canvas");
    sym_canvasmaker         = gensym ("canvasmaker");
    sym_click               = gensym ("click");
    sym_connect             = gensym ("connect");
    sym_coords              = gensym ("coords");
    sym_disconnect          = gensym ("disconnect");
    sym_dsp                 = gensym ("dsp");
    sym_f                   = gensym ("f");
    sym_floatatom           = gensym ("floatatom");
    sym_guiconnect          = gensym ("guiconnect");
    sym_guistub             = gensym ("guistub");
    sym_graph               = gensym ("graph");
    sym_inlet               = gensym ("inlet");
    sym_key                 = gensym ("key");
    sym_loadbang            = gensym ("loadbang");
    sym_motion              = gensym ("motion");
    sym_mouse               = gensym ("mouse");
    sym_mouseup             = gensym ("mouseup");
    sym_msg                 = gensym ("msg");
    sym_new                 = gensym ("new");
    sym_obj                 = gensym ("obj");
    sym_objectmaker         = gensym ("objectmaker");
    sym_open                = gensym ("open");
    sym_pd                  = gensym ("pd");
    sym_quit                = gensym ("quit");
    sym_restore             = gensym ("restore");
    sym_scalar              = gensym ("scalar");
    sym_setbounds           = gensym ("setbounds");
    sym_struct              = gensym ("struct");
    sym_subpatch            = gensym ("subpatch");
    sym_symbolatom          = gensym ("symbolatom");
    sym_text                = gensym ("text");
    sym_BackSpace           = gensym ("BackSpace");
    sym_Delete              = gensym ("Delete");
    sym_Down                = gensym ("Down");
    sym_Escape              = gensym ("Escape");
    sym_Left                = gensym ("Left");
    sym_Return              = gensym ("Return");
    sym_Right               = gensym ("Right");
    sym_Space               = gensym ("Space");
    sym_Tab                 = gensym ("Tab");
    sym_Up                  = gensym ("Up");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
