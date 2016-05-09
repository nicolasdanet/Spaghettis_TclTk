
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

t_symbol *sym___comma__;
t_symbol *sym___dot__;
t_symbol *sym___semicolon__;
t_symbol *sym__arraydialog;
t_symbol *sym__audiodialog;
t_symbol *sym__audioproperties;
t_symbol *sym__canvasdialog;
t_symbol *sym__copy;
t_symbol *sym__cut;
t_symbol *sym__data;
t_symbol *sym__duplicate;
t_symbol *sym__dummy;
t_symbol *sym__end;
t_symbol *sym__float_array_template;
t_symbol *sym__float_template;
t_symbol *sym__font;
t_symbol *sym__key;
t_symbol *sym__keyup;
t_symbol *sym__keyname;
t_symbol *sym__map;
t_symbol *sym__mididialog;
t_symbol *sym__midiproperties;
t_symbol *sym__paste;
t_symbol *sym__path;
t_symbol *sym__pop;
t_symbol *sym__popupdialog;
t_symbol *sym__quit;
t_symbol *sym__savepreferences;
t_symbol *sym__selectall;
t_symbol *sym__signoff;
t_symbol *sym__watchdog;
t_symbol *sym__A;
t_symbol *sym__N;
t_symbol *sym__X;
t_symbol *sym_array;
t_symbol *sym_atom;
t_symbol *sym_bindlist;
t_symbol *sym_bng;
t_symbol *sym_bounds;
t_symbol *sym_canvas;
t_symbol *sym_canvasmaker;
t_symbol *sym_clear;
t_symbol *sym_click;
t_symbol *sym_close;
t_symbol *sym_cnv;
t_symbol *sym_connect;
t_symbol *sym_coords;
t_symbol *sym_cosinesum;
t_symbol *sym_data;
t_symbol *sym_dirty;
t_symbol *sym_disconnect;
t_symbol *sym_dsp;
t_symbol *sym_editmode;
t_symbol *sym_f;
t_symbol *sym_floatatom;
t_symbol *sym_guiconnect;
t_symbol *sym_guistub;
t_symbol *sym_graph;
t_symbol *sym_hold;
t_symbol *sym_hradio;
t_symbol *sym_hslider;
t_symbol *sym_inlet;
t_symbol *sym_inlet__tilde__;
t_symbol *sym_key;
t_symbol *sym_linear;
t_symbol *sym_linewidth;
t_symbol *sym_loadbang;
t_symbol *sym_mergefile;
t_symbol *sym_motion;                                        
t_symbol *sym_mouse;
t_symbol *sym_mouseup;
t_symbol *sym_msg;
t_symbol *sym_nbx;
t_symbol *sym_new;
t_symbol *sym_normalize;
t_symbol *sym_obj;
t_symbol *sym_objectmaker;
t_symbol *sym_open;
t_symbol *sym_outlet;
t_symbol *sym_outlet__tilde__;
t_symbol *sym_pad;
t_symbol *sym_pd;
t_symbol *sym_pd__dash__float__dash__array;
t_symbol *sym_print;
t_symbol *sym_quit;
t_symbol *sym_read;
t_symbol *sym_rename;
t_symbol *sym_resize;
t_symbol *sym_restore;
t_symbol *sym_saveto;
t_symbol *sym_savetofile;
t_symbol *sym_scalar;
t_symbol *sym_send;
t_symbol *sym_set;
t_symbol *sym_setbounds;
t_symbol *sym_sinesum;
t_symbol *sym_sort;
t_symbol *sym_struct;
t_symbol *sym_style;
t_symbol *sym_subpatch;
t_symbol *sym_symbolatom;
t_symbol *sym_template;
t_symbol *sym_text;
t_symbol *sym_tgl;
t_symbol *sym_visible;
t_symbol *sym_vradio;
t_symbol *sym_vslider;
t_symbol *sym_vu;
t_symbol *sym_write;
t_symbol *sym_x;
t_symbol *sym_xticks;
t_symbol *sym_y;
t_symbol *sym_yticks;
t_symbol *sym_z;
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

#if PD_WITH_LEGACY

t_symbol *sym_const;
t_symbol *sym_lin;
t_symbol *sym_menu__dash__open;
t_symbol *sym_menuarray;
t_symbol *sym_menuclose;
t_symbol *sym_menusave;
t_symbol *sym_menusaveas;
t_symbol *sym_mycnv;
t_symbol *sym_numbox;
t_symbol *sym_page;
t_symbol *sym_toggle;
t_symbol *sym_vis;
t_symbol *sym_vumeter;

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void symbols_initialize (void)
{ 
    sym___comma__                       = gensym (",");
    sym___dot__                         = gensym (".");
    sym___semicolon__                   = gensym (";");
    sym__arraydialog                    = gensym ("_arraydialog");
    sym__audiodialog                    = gensym ("_audiodialog");
    sym__audioproperties                = gensym ("_audioproperties");
    sym__canvasdialog                   = gensym ("_canvasdialog");
    sym__cut                            = gensym ("_cut");
    sym__copy                           = gensym ("_copy");
    sym__data                           = gensym ("_data");
    sym__duplicate                      = gensym ("_duplicate");
    sym__end                            = gensym ("_end");
    sym__dummy                          = gensym ("_dummy");
    sym__float_array_template           = gensym ("_float_array_template");
    sym__float_template                 = gensym ("_float_template");
    sym__font                           = gensym ("_font");
    sym__key                            = gensym ("_key");
    sym__keyup                          = gensym ("_keyup");
    sym__keyname                        = gensym ("_keyname");
    sym__map                            = gensym ("_map");
    sym__mididialog                     = gensym ("_mididialog");
    sym__midiproperties                 = gensym ("_midiproperties");
    sym__paste                          = gensym ("_paste");
    sym__path                           = gensym ("_path");
    sym__pop                            = gensym ("_pop");
    sym__popupdialog                    = gensym ("_popupdialog");
    sym__quit                           = gensym ("_quit");
    sym__savepreferences                = gensym ("_savepreferences");
    sym__selectall                      = gensym ("_selectall");
    sym__signoff                        = gensym ("_signoff");
    sym__watchdog                       = gensym ("_watchdog");
    sym__A                              = gensym ("#A");
    sym__N                              = gensym ("#N");
    sym__X                              = gensym ("#X");
    sym_array                           = gensym ("array");
    sym_atom                            = gensym ("atom");
    sym_bindlist                        = gensym ("bindlist");
    sym_bng                             = gensym ("bng");
    sym_bounds                          = gensym ("bounds");
    sym_canvas                          = gensym ("canvas");
    sym_canvasmaker                     = gensym ("canvasmaker");
    sym_clear                           = gensym ("clear");
    sym_click                           = gensym ("click");
    sym_close                           = gensym ("close");
    sym_cnv                             = gensym ("cnv");
    sym_connect                         = gensym ("connect");
    sym_coords                          = gensym ("coords");
    sym_cosinesum                       = gensym ("cosinesum");
    sym_data                            = gensym ("data");
    sym_dirty                           = gensym ("dirty");
    sym_disconnect                      = gensym ("disconnect");
    sym_dsp                             = gensym ("dsp");
    sym_editmode                        = gensym ("editmode");
    sym_f                               = gensym ("f");
    sym_floatatom                       = gensym ("floatatom");
    sym_guiconnect                      = gensym ("guiconnect");
    sym_guistub                         = gensym ("guistub");
    sym_graph                           = gensym ("graph");
    sym_hold                            = gensym ("hold");
    sym_hradio                          = gensym ("hradio");
    sym_hslider                         = gensym ("hslider");
    sym_inlet                           = gensym ("inlet");
    sym_inlet__tilde__                  = gensym ("inlet~");
    sym_key                             = gensym ("key");
    sym_linear                          = gensym ("linear");
    sym_linewidth                       = gensym ("linewidth");
    sym_loadbang                        = gensym ("loadbang");
    sym_mergefile                       = gensym ("mergefile");
    sym_motion                          = gensym ("motion");
    sym_mouse                           = gensym ("mouse");
    sym_mouseup                         = gensym ("mouseup");
    sym_msg                             = gensym ("msg");
    sym_nbx                             = gensym ("nbx");
    sym_new                             = gensym ("new");
    sym_normalize                       = gensym ("normalize");
    sym_obj                             = gensym ("obj");
    sym_objectmaker                     = gensym ("objectmaker");
    sym_open                            = gensym ("open");
    sym_outlet                          = gensym ("outlet");
    sym_outlet__tilde__                 = gensym ("outlet~");
    sym_pad                             = gensym ("pad");
    sym_pd                              = gensym ("pd");
    sym_pd__dash__float__dash__array    = gensym ("pd-float-array");
    sym_print                           = gensym ("print");
    sym_quit                            = gensym ("quit");
    sym_read                            = gensym ("read");
    sym_rename                          = gensym ("rename");
    sym_resize                          = gensym ("resize");
    sym_restore                         = gensym ("restore");
    sym_saveto                          = gensym ("saveto");
    sym_savetofile                      = gensym ("savetofile");
    sym_scalar                          = gensym ("scalar");
    sym_send                            = gensym ("send");
    sym_set                             = gensym ("set");
    sym_setbounds                       = gensym ("setbounds");
    sym_sinesum                         = gensym ("sinesum");
    sym_sort                            = gensym ("sort");
    sym_struct                          = gensym ("struct");
    sym_style                           = gensym ("style");
    sym_subpatch                        = gensym ("subpatch");
    sym_symbolatom                      = gensym ("symbolatom");
    sym_template                        = gensym ("template");
    sym_text                            = gensym ("text");
    sym_tgl                             = gensym ("tgl");
    sym_visible                         = gensym ("visible");
    sym_vradio                          = gensym ("vradio");
    sym_vslider                         = gensym ("vslider");
    sym_vu                              = gensym ("vu");
    sym_write                           = gensym ("write");
    sym_x                               = gensym ("x");
    sym_xticks                          = gensym ("xticks");
    sym_y                               = gensym ("y");
    sym_yticks                          = gensym ("yticks");
    sym_z                               = gensym ("z");
    sym_BackSpace                       = gensym ("BackSpace");
    sym_Delete                          = gensym ("Delete");
    sym_Down                            = gensym ("Down");
    sym_Escape                          = gensym ("Escape");
    sym_Left                            = gensym ("Left");
    sym_Return                          = gensym ("Return");
    sym_Right                           = gensym ("Right");
    sym_Space                           = gensym ("Space");
    sym_Tab                             = gensym ("Tab");
    sym_Up                              = gensym ("Up");
    
    #if PD_WITH_LEGACY

    sym_const                           = gensym ("const");
    sym_lin                             = gensym ("lin");
    sym_menu__dash__open                = gensym ("menu-open");
    sym_menuarray                       = gensym ("menuarray");
    sym_menuclose                       = gensym ("menuclose");
    sym_menusave                        = gensym ("menusave");
    sym_menusaveas                      = gensym ("menusaveas");
    sym_mycnv                           = gensym ("mycnv");
    sym_numbox                          = gensym ("numbox");
    sym_page                            = gensym ("page");
    sym_toggle                          = gensym ("toggle");
    sym_vis                             = gensym ("vis");
    sym_vumeter                         = gensym ("vumeter");
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
