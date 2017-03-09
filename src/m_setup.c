
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void cos_tilde_initialize   (void);
void editor_initialize      (void);
void fft_initialize         (void);
void garray_initialize      (void);
void gui_initialize         (void);
void instance_initialize    (void);
void interface_initialize   (void);
void monitor_initialize     (void);
void rsqrt_tilde_initialize (void);
void soundfile_initialize   (void);
void textdefine_initialize  (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void cos_tilde_release      (void);
void defer_release          (void);
void drawnumber_release     (void);
void drawpolygon_release    (void);
void editor_release         (void);
void environment_release    (void);
void fft_release            (void);
void gui_release            (void);
void interface_release      (void);
void instance_release       (void);
void loader_release         (void);
void monitor_release        (void);
void path_release           (void);
void plot_release           (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void acoustic_setup         (void);
void append_setup           (void);
void array_setup            (void);
void arrayget_setup         (void);
void arraymax_setup         (void);
void arraymin_setup         (void);
void arrayquantile_setup    (void);
void arrayrandom_setup      (void);
void arrayset_setup         (void);
void arraysize_setup        (void);
void arraysum_setup         (void);
void atan2_setup            (void);
void bag_setup              (void);
void bang_setup             (void);
void bendin_setup           (void);
void bendout_setup          (void);
void binop1_setup           (void);
void binop2_setup           (void);
void binop3_setup           (void); 
void bng_setup              (void);
void canvas_setup           (void);
void change_setup           (void);
void clip_setup             (void);
void ctlin_setup            (void);
void ctlout_setup           (void);
void delay_setup            (void);
void dial_setup             (void);
void drawnumber_setup       (void);
void drawpolygon_setup      (void);
void element_setup          (void);
void expr_setup             (void);
void float_setup            (void);
void garray_setup           (void);
void gatom_setup            (void);
void get_setup              (void);
void getsize_setup          (void);
void global_setup           (void);
void guiconnect_setup       (void);
void guistub_setup          (void);
void inlet_setup            (void);
void int_setup              (void);
void key_setup              (void);
void keyname_setup          (void);
void keyup_setup            (void);
void line_setup             (void);
void list_setup             (void);
void listappend_setup       (void);
void listfromsymbol_setup   (void);
void listinlet_setup        (void);
void listlength_setup       (void);
void listprepend_setup      (void);
void listsplit_setup        (void);
void listtosymbol_setup     (void);
void listtrim_setup         (void);
void loadbang_setup         (void);
void makefilename_setup     (void);
void makenote_setup         (void);
void math_setup             (void);
void message_setup          (void);
void metro_setup            (void);
void midiin_setup           (void);
void midiout_setup          (void);
void midirealtimein_setup   (void);
void moses_setup            (void);
void namecanvas_setup       (void);
void netreceive_setup       (void);
void netsend_setup          (void);
void notein_setup           (void);
void noteout_setup          (void);
void openpanel_setup        (void);
void oscformat_setup        (void);
void oscparse_setup         (void);
void pack_setup             (void);
void panel_setup            (void);
void pgmin_setup            (void);
void pgmout_setup           (void);
void pipe_setup             (void);
void plot_setup             (void);
void pointer_setup          (void);
void poly_setup             (void);
void polytouchin_setup      (void);
void polytouchout_setup     (void);
void print_setup            (void);
void qlist_setup            (void);
void radio_setup            (void);
void random_setup           (void);
void realtime_setup         (void);
void receive_setup          (void);
void route_setup            (void);
void savepanel_setup        (void);
void scalar_setup           (void);
void select_setup           (void);
void send_setup             (void);
void serial_setup           (void);
void set_setup              (void);
void setsize_setup          (void);
void slider_setup           (void);
void spigot_setup           (void);
void stripnote_setup        (void);
void struct_setup           (void);
void swap_setup             (void);
void symbol_setup           (void);
void sysexin_setup          (void);
void tabread_setup          (void);
void tabread4_setup         (void);
void tabwrite_setup         (void);
void template_setup         (void);
void text_setup             (void);
void textdefine_setup       (void);
void textfile_setup         (void);
void textget_setup          (void);
void textlist_setup         (void);
void textsearch_setup       (void);
void textsequence_setup     (void);
void textset_setup          (void);
void textsize_setup         (void);
void timer_setup            (void);
void toggle_setup           (void);
void touchin_setup          (void);
void touchout_setup         (void);
void trigger_setup          (void);
void unpack_setup           (void);
void until_setup            (void);
void value_setup            (void);
void vinlet_setup           (void);
void voutlet_setup          (void);
void vu_setup               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void abs_tilde_setup        (void);
void adc_tilde_setup        (void);
void add_tilde_setup        (void);
void bang_tilde_setup       (void);
void biquad_tilde_setup     (void);
void block_tilde_setup      (void);
void bp_tilde_setup         (void);
void catch_tilde_setup      (void);
void clip_tilde_setup       (void);
void cos_tilde_setup        (void);
void cpole_tilde_setup      (void);
void czero_rev_tilde_setup  (void);
void czero_tilde_setup      (void);
void dac_tilde_setup        (void);
void dbtopow_tilde_setup    (void);
void dbtorms_tilde_setup    (void);
void delread_tilde_setup    (void);
void delwrite_tilde_setup   (void);
void divide_tilde_setup     (void);
void env_tilde_setup        (void);
void exp_tilde_setup        (void);
void fft_tilde_setup        (void);
void framp_tilde_setup      (void);
void ftom_tilde_setup       (void);
void hip_tilde_setup        (void);
void ifft_tilde_setup       (void);
void line_tilde_setup       (void);
void log_tilde_setup        (void);
void lop_tilde_setup        (void);
void lrshift_tilde_setup    (void);
void max_tilde_setup        (void);
void min_tilde_setup        (void);
void mtof_tilde_setup       (void);
void multiply_tilde_setup   (void);
void noise_tilde_setup      (void);
void osc_tilde_setup        (void);
void phasor_tilde_setup     (void);
void pow_tilde_setup        (void);
void powtodb_tilde_setup    (void);
void print_tilde_setup      (void);
void readsf_tilde_setup     (void);
void receive_tilde_setup    (void);
void rmstodb_tilde_setup    (void);
void rpole_tilde_setup      (void);
void rfft_tilde_setup       (void);
void rifft_tilde_setup      (void);
void rsqrt_tilde_setup      (void);
void rzero_rev_tilde_setup  (void);
void samphold_tilde_setup   (void);
void samplerate_tilde_setup (void);
void send_tilde_setup       (void);
void sig_tilde_setup        (void);
void snapshot_tilde_setup   (void);
void soundfiler_setup       (void);
void sqrt_tilde_setup       (void);
void subtract_tilde_setup   (void);
void tabosc4_tilde_setup    (void);
void tabplay_tilde_setup    (void);
void tabread_tilde_setup    (void);
void tabread4_tilde_setup   (void);
void tabreceive_tilde_setup (void);
void tabsend_tilde_setup    (void);
void tabwrite_tilde_setup   (void);
void threshold_tilde_setup  (void);
void throw_tilde_setup      (void);
void vcf_tilde_setup        (void);
void vd_tilde_setup         (void);
void vline_tilde_setup      (void);
void wrap_tilde_setup       (void);
void writesf_tilde_setup    (void);
void zero_tilde_setup       (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void acoustic_destroy           (void);
void append_destroy             (void);
void arrayget_destroy           (void);
void arraymax_destroy           (void);
void arraymin_destroy           (void);
void arrayquantile_destroy      (void);
void arrayrandom_destroy        (void);
void arrayset_destroy           (void);
void arraysize_destroy          (void);
void arraysum_destroy           (void);
void atan2_destroy              (void);
void bag_destroy                (void);
void bang_destroy               (void);
void bendin_destroy             (void);
void bendout_destroy            (void);
void binop1_destroy             (void);
void binop2_destroy             (void);
void binop3_destroy             (void); 
void bng_destroy                (void);
void canvas_destroy             (void);
void change_destroy             (void);
void clip_destroy               (void);
void ctlin_destroy              (void);
void ctlout_destroy             (void);
void delay_destroy              (void);
void dial_destroy               (void);
void drawnumber_destroy         (void);
void drawpolygon_destroy        (void);
void element_destroy            (void);
void expr_destroy               (void);
void float_destroy              (void);
void garray_destroy             (void);
void gatom_destroy              (void);
void get_destroy                (void);
void getsize_destroy            (void);
void global_destroy             (void);
void guiconnect_destroy         (void);
void guistub_destroy            (void);
void inlet_destroy              (void);
void int_destroy                (void);
void key_destroy                (void);
void keyname_destroy            (void);
void keyup_destroy              (void);
void line_destroy               (void);
void listappend_destroy         (void);
void listfromsymbol_destroy     (void);
void listinlet_destroy          (void);
void listlength_destroy         (void);
void listprepend_destroy        (void);
void listsplit_destroy          (void);
void listtosymbol_destroy       (void);
void listtrim_destroy           (void);
void loadbang_destroy           (void);
void makefilename_destroy       (void);
void makenote_destroy           (void);
void math_destroy               (void);
void message_destroy            (void);
void metro_destroy              (void);
void midiin_destroy             (void);
void midiout_destroy            (void);
void midirealtimein_destroy     (void);
void moses_destroy              (void);
void namecanvas_destroy         (void);
void netreceive_destroy         (void);
void netsend_destroy            (void);
void notein_destroy             (void);
void noteout_destroy            (void);
void openpanel_destroy          (void);
void oscformat_destroy          (void);
void oscparse_destroy           (void);
void pack_destroy               (void);
void panel_destroy              (void);
void pgmin_destroy              (void);
void pgmout_destroy             (void);
void pipe_destroy               (void);
void plot_destroy               (void);
void pointer_destroy            (void);
void poly_destroy               (void);
void polytouchin_destroy        (void);
void polytouchout_destroy       (void);
void print_destroy              (void);
void qlist_destroy              (void);
void radio_destroy              (void);
void random_destroy             (void);
void realtime_destroy           (void);
void receive_destroy            (void);
void route_destroy              (void);
void savepanel_destroy          (void);
void scalar_destroy             (void);
void select_destroy             (void);
void send_destroy               (void);
void serial_destroy             (void);
void set_destroy                (void);
void setsize_destroy            (void);
void slider_destroy             (void);
void spigot_destroy             (void);
void stripnote_destroy          (void);
void struct_destroy             (void);
void swap_destroy               (void);
void symbol_destroy             (void);
void sysexin_destroy            (void);
void tabread_destroy            (void);
void tabread4_destroy           (void);
void tabwrite_destroy           (void);
void template_destroy           (void);
void text_destroy               (void);
void textdefine_destroy         (void);
void textfile_destroy           (void);
void textget_destroy            (void);
void textlist_destroy           (void);
void textsearch_destroy         (void);
void textsequence_destroy       (void);
void textset_destroy            (void);
void textsize_destroy           (void);
void timer_destroy              (void);
void toggle_destroy             (void);
void touchin_destroy            (void);
void touchout_destroy           (void);
void trigger_destroy            (void);
void unpack_destroy             (void);
void until_destroy              (void);
void value_destroy              (void);
void vinlet_destroy             (void);
void voutlet_destroy            (void);
void vu_destroy                 (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void abs_tilde_destroy          (void);
void adc_tilde_destroy          (void);
void add_tilde_destroy          (void);
void bang_tilde_destroy         (void);
void biquad_tilde_destroy       (void);
void block_tilde_destroy        (void);
void bp_tilde_destroy           (void);
void catch_tilde_destroy        (void);
void clip_tilde_destroy         (void);
void cos_tilde_destroy          (void);
void cpole_tilde_destroy        (void);
void czero_rev_tilde_destroy    (void);
void czero_tilde_destroy        (void);
void dac_tilde_destroy          (void);
void dbtopow_tilde_destroy      (void);
void dbtorms_tilde_destroy      (void);
void delread_tilde_destroy      (void);
void delwrite_tilde_destroy     (void);
void divide_tilde_destroy       (void);
void env_tilde_destroy          (void);
void exp_tilde_destroy          (void);
void fft_tilde_destroy          (void);
void framp_tilde_destroy        (void);
void ftom_tilde_destroy         (void);
void hip_tilde_destroy          (void);
void ifft_tilde_destroy         (void);
void line_tilde_destroy         (void);
void log_tilde_destroy          (void);
void lop_tilde_destroy          (void);
void lrshift_tilde_destroy      (void);
void max_tilde_destroy          (void);
void min_tilde_destroy          (void);
void mtof_tilde_destroy         (void);
void multiply_tilde_destroy     (void);
void noise_tilde_destroy        (void);
void osc_tilde_destroy          (void);
void phasor_tilde_destroy       (void);
void pow_tilde_destroy          (void);
void powtodb_tilde_destroy      (void);
void print_tilde_destroy        (void);
void readsf_tilde_destroy       (void);
void receive_tilde_destroy      (void);
void rmstodb_tilde_destroy      (void);
void rpole_tilde_destroy        (void);
void rfft_tilde_destroy         (void);
void rifft_tilde_destroy        (void);
void rsqrt_tilde_destroy        (void);
void rzero_rev_tilde_destroy    (void);
void samphold_tilde_destroy     (void);
void samplerate_tilde_destroy   (void);
void send_tilde_destroy         (void);
void sig_tilde_destroy          (void);
void snapshot_tilde_destroy     (void);
void soundfiler_destroy         (void);
void sqrt_tilde_destroy         (void);
void subtract_tilde_destroy     (void);
void tabosc4_tilde_destroy      (void);
void tabplay_tilde_destroy      (void);
void tabread_tilde_destroy      (void);
void tabread4_tilde_destroy     (void);
void tabreceive_tilde_destroy   (void);
void tabsend_tilde_destroy      (void);
void tabwrite_tilde_destroy     (void);
void threshold_tilde_destroy    (void);
void throw_tilde_destroy        (void);
void vcf_tilde_destroy          (void);
void vd_tilde_destroy           (void);
void vline_tilde_destroy        (void);
void wrap_tilde_destroy         (void);
void writesf_tilde_destroy      (void);
void zero_tilde_destroy         (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void setup_setup (void)
{
    acoustic_setup();
    append_setup();
    array_setup();
    arrayget_setup();
    arraymax_setup();
    arraymin_setup();
    arrayquantile_setup();
    arrayrandom_setup();
    arrayset_setup();
    arraysize_setup();
    arraysum_setup();
    atan2_setup();
    bag_setup();
    bang_setup();
    bendin_setup();
    bendout_setup();
    binop1_setup();
    binop2_setup();
    binop3_setup();
    bng_setup();
    canvas_setup();
    change_setup();
    clip_setup();
    ctlin_setup();
    ctlout_setup();
    delay_setup();
    dial_setup();
    drawnumber_setup();
    drawpolygon_setup();
    element_setup();
    expr_setup();
    float_setup();
    garray_setup();
    gatom_setup();
    get_setup();
    getsize_setup();
    guiconnect_setup();
    guistub_setup();
    int_setup();
    key_setup();
    keyname_setup();
    keyup_setup();
    line_setup();
    list_setup();
    listappend_setup();
    listfromsymbol_setup();
    listinlet_setup();
    listlength_setup();
    listprepend_setup();
    listsplit_setup();
    listtosymbol_setup();
    listtrim_setup();
    loadbang_setup();
    makefilename_setup();
    makenote_setup();
    math_setup();
    message_setup();
    metro_setup();
    midiin_setup();
    midiout_setup();
    midirealtimein_setup();
    moses_setup();
    namecanvas_setup();
    netreceive_setup();
    netsend_setup();
    notein_setup();
    noteout_setup();
    openpanel_setup();
    oscformat_setup();
    oscparse_setup();
    pack_setup();
    panel_setup();
    pgmin_setup();
    pgmout_setup();
    pipe_setup();
    plot_setup();
    pointer_setup();
    poly_setup();
    polytouchin_setup();
    polytouchout_setup();
    print_setup();
    qlist_setup();
    radio_setup();
    random_setup();
    realtime_setup();
    receive_setup();
    route_setup();
    savepanel_setup();
    scalar_setup();
    select_setup();
    send_setup();
    serial_setup();
    set_setup();
    setsize_setup();
    slider_setup();
    spigot_setup();
    stripnote_setup();
    struct_setup();
    swap_setup();
    symbol_setup();
    sysexin_setup();
    tabread_setup();
    tabread4_setup();
    tabwrite_setup();
    template_setup();
    text_setup();
    textdefine_setup();
    textfile_setup();
    textget_setup();
    textlist_setup();
    textsearch_setup();
    textsequence_setup();
    textset_setup();
    textsize_setup();
    timer_setup();
    toggle_setup();
    touchin_setup();
    touchout_setup();
    trigger_setup();
    unpack_setup();
    until_setup();
    value_setup();
    vinlet_setup();
    voutlet_setup();
    vu_setup();
    
    abs_tilde_setup();
    adc_tilde_setup();
    add_tilde_setup();
    bang_tilde_setup();
    biquad_tilde_setup();
    block_tilde_setup();
    bp_tilde_setup();
    catch_tilde_setup();
    clip_tilde_setup();
    cos_tilde_setup();
    cpole_tilde_setup();
    czero_tilde_setup();
    czero_rev_tilde_setup();
    dac_tilde_setup();
    dbtopow_tilde_setup();
    delread_tilde_setup();
    delwrite_tilde_setup();
    divide_tilde_setup();
    env_tilde_setup();
    exp_tilde_setup();
    fft_tilde_setup();
    framp_tilde_setup();
    ftom_tilde_setup();
    hip_tilde_setup();
    ifft_tilde_setup();
    line_tilde_setup();
    log_tilde_setup();
    lop_tilde_setup();
    lrshift_tilde_setup();
    max_tilde_setup();
    min_tilde_setup();
    mtof_tilde_setup();
    multiply_tilde_setup();
    noise_tilde_setup();
    osc_tilde_setup();
    phasor_tilde_setup();
    pow_tilde_setup();
    powtodb_tilde_setup();
    print_tilde_setup();
    readsf_tilde_setup();
    receive_tilde_setup();
    rfft_tilde_setup();
    rifft_tilde_setup();
    rmstodb_tilde_setup();
    rpole_tilde_setup();
    rsqrt_tilde_setup();
    rzero_rev_tilde_setup();
    samphold_tilde_setup();
    samplerate_tilde_setup();
    send_tilde_setup();
    sig_tilde_setup();
    snapshot_tilde_setup();
    soundfiler_setup();
    sqrt_tilde_setup();
    subtract_tilde_setup();
    tabosc4_tilde_setup();
    tabplay_tilde_setup();
    tabread_tilde_setup();
    tabread4_tilde_setup();
    tabreceive_tilde_setup();
    tabsend_tilde_setup();
    tabwrite_tilde_setup();
    threshold_tilde_setup();
    throw_tilde_setup();
    vcf_tilde_setup();
    vd_tilde_setup();
    vline_tilde_setup();
    wrap_tilde_setup();
    writesf_tilde_setup();
    zero_tilde_setup();
}

void setup_destroy (void)        
{
    acoustic_destroy();
    append_destroy();
    arrayget_destroy();
    arraymax_destroy();
    arraymin_destroy();
    arrayquantile_destroy();
    arrayrandom_destroy();
    arrayset_destroy();
    arraysize_destroy();
    arraysum_destroy();
    atan2_destroy();
    bag_destroy();
    bang_destroy();
    bendin_destroy();
    bendout_destroy();
    binop1_destroy();
    binop2_destroy();
    binop3_destroy();
    bng_destroy();
    canvas_destroy();
    change_destroy();
    clip_destroy();
    ctlin_destroy();
    ctlout_destroy();
    delay_destroy();
    dial_destroy();
    drawnumber_destroy();
    drawpolygon_destroy();
    element_destroy();
    expr_destroy();
    float_destroy();
    garray_destroy();
    gatom_destroy();
    get_destroy();
    getsize_destroy();
    guiconnect_destroy();
    guistub_destroy();
    int_destroy();
    key_destroy();
    keyname_destroy();
    keyup_destroy();
    line_destroy();
    listappend_destroy();
    listfromsymbol_destroy();
    listinlet_destroy();
    listlength_destroy();
    listprepend_destroy();
    listsplit_destroy();
    listtosymbol_destroy();
    listtrim_destroy();
    loadbang_destroy();
    makefilename_destroy();
    makenote_destroy();
    math_destroy();
    message_destroy();
    metro_destroy();
    midiin_destroy();
    midiout_destroy();
    midirealtimein_destroy();
    moses_destroy();
    namecanvas_destroy();
    netreceive_destroy();
    netsend_destroy();
    notein_destroy();
    noteout_destroy();
    openpanel_destroy();
    oscformat_destroy();
    oscparse_destroy();
    pack_destroy();
    panel_destroy();
    pgmin_destroy();
    pgmout_destroy();
    pipe_destroy();
    plot_destroy();
    pointer_destroy();
    poly_destroy();
    polytouchin_destroy();
    polytouchout_destroy();
    print_destroy();
    qlist_destroy();
    radio_destroy();
    random_destroy();
    realtime_destroy();
    receive_destroy();
    route_destroy();
    savepanel_destroy();
    scalar_destroy();
    select_destroy();
    send_destroy();
    serial_destroy();
    set_destroy();
    setsize_destroy();
    slider_destroy();
    spigot_destroy();
    stripnote_destroy();
    struct_destroy();
    swap_destroy();
    symbol_destroy();
    sysexin_destroy();
    tabread_destroy();
    tabread4_destroy();
    tabwrite_destroy();
    template_destroy();
    text_destroy();
    textdefine_destroy();
    textfile_destroy();
    textget_destroy();
    textlist_destroy();
    textsearch_destroy();
    textsequence_destroy();
    textset_destroy();
    textsize_destroy();
    timer_destroy();
    toggle_destroy();
    touchin_destroy();
    touchout_destroy();
    trigger_destroy();
    unpack_destroy();
    until_destroy();
    value_destroy();
    vinlet_destroy();
    voutlet_destroy();
    vu_destroy();
    
    abs_tilde_destroy();
    adc_tilde_destroy();
    add_tilde_destroy();
    bang_tilde_destroy();
    biquad_tilde_destroy();
    block_tilde_destroy();
    bp_tilde_destroy();
    catch_tilde_destroy();
    clip_tilde_destroy();
    cos_tilde_destroy();
    cpole_tilde_destroy();
    czero_tilde_destroy();
    czero_rev_tilde_destroy();
    dac_tilde_destroy();
    dbtopow_tilde_destroy();
    delread_tilde_destroy();
    delwrite_tilde_destroy();
    divide_tilde_destroy();
    env_tilde_destroy();
    exp_tilde_destroy();
    fft_tilde_destroy();
    framp_tilde_destroy();
    ftom_tilde_destroy();
    hip_tilde_destroy();
    ifft_tilde_destroy();
    line_tilde_destroy();
    log_tilde_destroy();
    lop_tilde_destroy();
    lrshift_tilde_destroy();
    max_tilde_destroy();
    min_tilde_destroy();
    mtof_tilde_destroy();
    multiply_tilde_destroy();
    noise_tilde_destroy();
    osc_tilde_destroy();
    phasor_tilde_destroy();
    pow_tilde_destroy();
    powtodb_tilde_destroy();
    print_tilde_destroy();
    readsf_tilde_destroy();
    receive_tilde_destroy();
    rfft_tilde_destroy();
    rifft_tilde_destroy();
    rmstodb_tilde_destroy();
    rpole_tilde_destroy();
    rsqrt_tilde_destroy();
    rzero_rev_tilde_destroy();
    samphold_tilde_destroy();
    samplerate_tilde_destroy();
    send_tilde_destroy();
    sig_tilde_destroy();
    snapshot_tilde_destroy();
    soundfiler_destroy();
    sqrt_tilde_destroy();
    subtract_tilde_destroy();
    tabosc4_tilde_destroy();
    tabplay_tilde_destroy();
    tabread_tilde_destroy();
    tabread4_tilde_destroy();
    tabreceive_tilde_destroy();
    tabsend_tilde_destroy();
    tabwrite_tilde_destroy();
    threshold_tilde_destroy();
    throw_tilde_destroy();
    vcf_tilde_destroy();
    vd_tilde_destroy();
    vline_tilde_destroy();
    wrap_tilde_destroy();
    writesf_tilde_destroy();
    zero_tilde_destroy();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Note that order of calls below may be critical. */

void setup_initialize (void)
{
    /* First. */
    
    instance_initialize();
        
    /* Second various initializations (alphabetically sorted there). */
    
    cos_tilde_initialize();
    editor_initialize();
    fft_initialize();
    gui_initialize();
    interface_initialize();
    monitor_initialize();
    rsqrt_tilde_initialize();
    soundfile_initialize();
    
    /* Then setup native classes. */
    
    global_setup();
    inlet_setup();
    setup_setup();
    
    /* At last instantiate the invisible patches required. */
    
    garray_initialize();
    textdefine_initialize();
}

void setup_release (void)
{
    /* Stop listening and close GUI socket. */
    
    interface_release();
    
    /* Close all remaining patches (included invisible ones). */
    
    instance_rootsFreeAll();
    
    /* Destroy all the third-party externals. */
    
    loader_release();
    
    /* Destroy the native classes. */
    
    setup_destroy(); 
    inlet_destroy();
    global_destroy();
    
    /* Various cleaning (reverse order). */
    
    plot_release();
    path_release();
    monitor_release();
    gui_release();
    fft_release();
    environment_release();
    editor_release();
    drawpolygon_release();
    drawnumber_release();
    defer_release();
    cos_tilde_release();
    
    /* At last. */
    
    instance_release();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
