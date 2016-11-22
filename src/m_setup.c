
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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

void interface_initialize   (void);
void object_initialize      (void);
void bindlist_initialize    (void);
void global_initialize      (void);
void editor_initialize      (void);
void drawpolygon_initialize (void);
void plot_initialize        (void);
void drawnumber_initialize  (void);
void canvas_initialize      (void);
void garray_initialize      (void);
void textdefine_initialize  (void);
void loader_initialize      (void);


// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void interface_release      (void);
void object_release         (void);
void bindlist_release       (void);
void global_release         (void);
void editor_release         (void);
void drawpolygon_release    (void);
void plot_release           (void);
void drawnumber_release     (void);
void canvas_release         (void);
void garray_release         (void);
void textdefine_release     (void);
void loader_release         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void garray_setup           (void);
void canvas_setup           (void);
void guistub_setup          (void);
void guiconnect_setup       (void);
void bng_setup              (void);
void toggle_setup           (void);
void radio_setup            (void);
void slider_setup           (void);
void dial_setup             (void);
void vu_setup               (void);
void panel_setup            (void);
void vinlet_setup           (void);
void voutlet_setup          (void);
void scalar_setup           (void);
void template_setup         (void);
void struct_setup           (void);
void drawpolygon_setup      (void);
void plot_setup             (void);
void drawnumber_setup       (void);
void message_setup          (void);
void gatom_setup            (void);
void text_setup             (void);
void pointer_setup          (void);
void get_setup              (void);
void set_setup              (void);
void element_setup          (void);
void getsize_setup          (void);
void setsize_setup          (void);
void append_setup           (void);
void scalardefine_setup     (void);
void acoustic_setup         (void);
void print_setup            (void);
void int_setup              (void);
void float_setup            (void);
void symbol_setup           (void);
void bang_setup             (void);
void send_setup             (void);
void receive_setup          (void);
void select_setup           (void);
void route_setup            (void);
void pack_setup             (void);
void unpack_setup           (void);
void trigger_setup          (void);
void spigot_setup           (void);
void moses_setup            (void);
void until_setup            (void);
void makefilename_setup     (void);
void swap_setup             (void);
void change_setup           (void);
void value_setup            (void);
void metro_setup            (void);
void delay_setup            (void);
void line_setup             (void);
void timer_setup            (void);
void pipe_setup             (void);
void math_setup             (void);
void clip_setup             (void);
void atan2_setup            (void);
void binop1_setup           (void);
void binop2_setup           (void);
void binop3_setup           (void);        
void arraydefine_setup      (void);
void table_setup            (void);
void arraysize_setup        (void);
void arraysum_setup         (void);
void arrayget_setup         (void);
void arrayset_setup         (void);
void arrayquantile_setup    (void);
void arrayrandom_setup      (void);
void arraymax_setup         (void);
void arraymin_setup         (void);
void midiin_setup           (void);
void sysexin_setup          (void);
void midirealtimein_setup   (void);
void notein_setup           (void);
void ctlin_setup            (void);
void pgmin_setup            (void);
void bendin_setup           (void);
void touchin_setup          (void);
void polytouchin_setup      (void);
void midiout_setup          (void);
void noteout_setup          (void);
void ctlout_setup           (void);
void pgmout_setup           (void);
void bendout_setup          (void);
void touchout_setup         (void);
void polytouchout_setup     (void);
void makenote_setup         (void);
void stripnote_setup        (void);
void poly_setup             (void);
void bag_setup              (void);
void random_setup           (void);
void loadbang_setup         (void);
void namecanvas_setup       (void);
void serial_setup           (void);
void realtime_setup         (void);
void oscparse_setup         (void);
void oscformat_setup        (void);
void netsend_setup          (void);
void netreceive_setup       (void);
void textdefine_setup       (void);
void textget_setup          (void);
void textset_setup          (void);
void textsize_setup         (void);
void textlist_setup         (void);
void textsearch_setup       (void);
void textsequence_setup     (void);
void qlist_setup            (void);
void textfile_setup         (void);
void key_setup              (void);
void keyup_setup            (void);
void keyname_setup          (void);
void openpanel_setup        (void);
void savepanel_setup        (void);
void listinlet_setup        (void);
void list_setup             (void);
void listappend_setup       (void);
void listprepend_setup      (void);
void listsplit_setup        (void);
void listtrim_setup         (void);
void listlength_setup       (void);
void listfromsymbol_setup   (void);
void listtosymbol_setup     (void);
void d_arithmetic_setup     (void);
void d_array_setup          (void);
void d_ctl_setup            (void);
void dac_setup              (void);
void adc_setup              (void);
void d_delay_setup          (void);
void d_fft_setup            (void);
void d_filter_setup         (void);
void sigsend_setup          (void);
void sigreceive_setup       (void);
void catch_tilde_setup      (void);
void throw_tilde_setup      (void);
void d_math_setup           (void);
void print_tilde_setup      (void);
void bang_tilde_setup       (void);
void d_osc_setup            (void);
void d_soundfile_setup      (void);
void block_tilde_setup      (void);
void samplerate_tilde_setup (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void setup_initialize (void)        /* Note that order of calls below may be critical. */
{
    interface_initialize();
    object_initialize();
    bindlist_initialize();
    global_initialize();
    editor_initialize();
    drawpolygon_initialize();
    plot_initialize();
    drawnumber_initialize();
    
    garray_setup();
    canvas_setup();
    guistub_setup();
    guiconnect_setup();
    bng_setup();
    toggle_setup();
    radio_setup();
    slider_setup();
    dial_setup();
    vu_setup();
    panel_setup();
    vinlet_setup();
    voutlet_setup();
    scalar_setup();
    template_setup();
    struct_setup();
    drawpolygon_setup();
    plot_setup();
    drawnumber_setup();
    message_setup();
    gatom_setup();
    text_setup();
    pointer_setup();
    get_setup();
    set_setup();
    element_setup();
    getsize_setup();
    setsize_setup();
    append_setup();
    scalardefine_setup();
    acoustic_setup();
    print_setup();
    int_setup();
    float_setup();
    symbol_setup();
    bang_setup();
    send_setup();
    receive_setup();
    select_setup();
    route_setup();
    pack_setup();
    unpack_setup();
    trigger_setup();
    spigot_setup();
    moses_setup();
    until_setup();
    swap_setup();
    change_setup();
    value_setup();
    makefilename_setup();
    metro_setup();
    delay_setup();
    line_setup();
    timer_setup();
    pipe_setup();
    math_setup();
    clip_setup();
    atan2_setup();
    binop1_setup();
    binop2_setup();
    binop3_setup();
    arraydefine_setup();
    table_setup();
    arraysize_setup();
    arraysum_setup();
    arrayget_setup();
    arrayset_setup();
    arrayquantile_setup();
    arrayrandom_setup();
    arraymax_setup();
    arraymin_setup();
    midiin_setup();
    sysexin_setup();
    midirealtimein_setup();
    notein_setup();
    ctlin_setup();
    pgmin_setup();
    bendin_setup();
    touchin_setup();
    polytouchin_setup();
    midiout_setup();
    noteout_setup();
    ctlout_setup();
    pgmout_setup();
    bendout_setup();
    touchout_setup();
    polytouchout_setup();
    makenote_setup();
    stripnote_setup();
    poly_setup();
    bag_setup();
    random_setup();
    loadbang_setup();
    namecanvas_setup();
    serial_setup();
    realtime_setup();
    oscparse_setup();
    oscformat_setup();
    netsend_setup();
    netreceive_setup();
    textdefine_setup();
    textget_setup();
    textset_setup();
    textsize_setup();
    textlist_setup();
    textsearch_setup();
    textsequence_setup();
    qlist_setup();
    textfile_setup();
    key_setup();
    keyup_setup();
    keyname_setup();
    openpanel_setup();
    savepanel_setup();
    listinlet_setup();
    list_setup();
    listappend_setup();
    listprepend_setup();
    listsplit_setup();
    listtrim_setup();
    listlength_setup();
    listfromsymbol_setup();
    listtosymbol_setup();

    d_arithmetic_setup();
    d_array_setup();
    d_ctl_setup();
    dac_setup();
    adc_setup();
    d_delay_setup();
    d_fft_setup();
    d_filter_setup();
    sigsend_setup();
    sigreceive_setup();
    catch_tilde_setup();
    throw_tilde_setup();
    d_math_setup();
    print_tilde_setup();
    bang_tilde_setup();
    d_osc_setup();
    d_soundfile_setup();
    block_tilde_setup();
    samplerate_tilde_setup();
    
    canvas_initialize();
    garray_initialize();
    textdefine_initialize();
    loader_initialize();
}

void setup_release (void)
{
    loader_release();
    textdefine_release();
    garray_release();
    canvas_release();
    
    drawnumber_release();
    plot_release();
    drawpolygon_release();
    editor_release();
    global_release();
    bindlist_release();
    object_release();
    interface_release();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
