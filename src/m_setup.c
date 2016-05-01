
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_initialize     (void);
void interface_initialize   (void);
void object_initialize      (void);
void bindlist_initialize    (void);
void global_initialize      (void);
void editor_initialize      (void);
void loader_initialize      (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void message_release        (void);
void interface_release      (void);
void object_release         (void);
void bindlist_release       (void);
void global_release         (void);
void editor_release         (void);
void loader_release         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void g_array_setup          (void);
void canvas_setup           (void);
void g_graph_setup          (void);
void g_readwrite_setup      (void);
void guiconnect_setup       (void);
void bng_setup              (void);
void toggle_setup           (void);
void radio_setup            (void);
void slider_setup           (void);
void dial_setup             (void);
void vu_setup               (void);
void panel_setup            (void);
void g_io_setup             (void);
void g_scalar_setup         (void);
void g_template_setup       (void);
void g_text_setup           (void);
void g_traversal_setup      (void);
void x_acoustics_setup      (void);
void x_interface_setup      (void);
void x_connective_setup     (void);
void x_time_setup           (void);
void x_arithmetic_setup     (void);
void x_array_setup          (void);
void x_midi_setup           (void);
void x_misc_setup           (void);
void x_net_setup            (void);
void x_qlist_setup          (void);
void guistub_setup          (void);
void x_gui_setup            (void);
void x_list_setup           (void);
void x_scalar_setup         (void);
void d_arithmetic_setup     (void);
void d_array_setup          (void);
void d_ctl_setup            (void);
void d_dac_setup            (void);
void d_delay_setup          (void);
void d_fft_setup            (void);
void d_filter_setup         (void);
void d_global_setup         (void);
void d_math_setup           (void);
void d_misc_setup           (void);
void d_osc_setup            (void);
void d_soundfile_setup      (void);
void d_ugen_setup           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void setup_initialize (void)
{
    message_initialize();
    interface_initialize();
    object_initialize();
    bindlist_initialize();
    global_initialize();
    editor_initialize();
    
    g_array_setup();
    canvas_setup();
    g_graph_setup();
    g_readwrite_setup();
    guiconnect_setup();
    bng_setup();
    toggle_setup();
    radio_setup();
    slider_setup();
    dial_setup();
    vu_setup();
    panel_setup();
    g_io_setup();
    g_scalar_setup();
    g_template_setup();
    g_text_setup();
    g_traversal_setup();
    
    x_acoustics_setup();
    x_interface_setup();
    x_connective_setup();
    x_time_setup();
    x_arithmetic_setup();
    x_array_setup();
    x_midi_setup();
    x_misc_setup();
    x_net_setup();
    x_qlist_setup();
    guistub_setup();
    x_gui_setup();
    x_list_setup();
    x_scalar_setup();
    
    d_arithmetic_setup();
    d_array_setup();
    d_ctl_setup();
    d_dac_setup();
    d_delay_setup();
    d_fft_setup();
    d_filter_setup();
    d_global_setup();
    d_math_setup();
    d_misc_setup();
    d_osc_setup();
    d_soundfile_setup();
    d_ugen_setup();
    
    garray_init();
    loader_initialize();
}

void setup_release (void)
{
    loader_release();
    
    editor_release();
    global_release();
    bindlist_release();
    object_release();
    interface_release();
    message_release();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
