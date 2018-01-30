
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Core. */

#include "m_symbols.c"
#include "m_environment.c"
#include "m_autorelease.c"
#include "m_instance.c"
#include "m_stack.c"
#include "m_bind.c"
#include "m_message.c"
#include "m_pd.c"
#include "m_method.c"
#include "m_class.c"
#include "m_object.c"
#include "m_inlet.c"
#include "m_outlet.c"
#include "m_atom.c"
#include "m_buffer.c"
#include "m_parse.c"
#include "m_file.c"
#include "m_eval.c"
#include "m_global.c"
#include "m_clipboard.c"
#include "m_snap.c"
#include "m_snippet.c"
#include "m_setup.c"
#include "m_dollar.c"
#include "m_error.c"
#include "m_rectangle.c"
#include "m_bounds.c"
#include "m_iterator.c"
#include "m_pathlist.c"
#include "m_heapstring.c"
#include "m_utils.c"
#include "m_symbol.c"
#include "m_color.c"
#include "m_math.c"
#include "m_string.c"

/* System. */

#include "s_entry.c"
#include "s_main.c"
#include "s_scheduler.c"
#include "s_priority.c"
#include "s_handlers.c"
#include "s_clock.c"
#include "s_time.c"
#include "s_atomicMac.c"
#include "s_atomicPosix.c"
#include "s_receiver.c"
#include "s_monitor.c"
#include "s_defer.c"
#include "s_gui.c"
#include "s_interface.c"
#include "s_midi.c"
#include "s_inmidi.c"
#include "s_outmidi.c"
#include "s_audio.c"
#include "s_file.c"
#include "s_searchpath.c"
#include "s_path.c"
#include "s_loader.c"
#include "s_properties.c"
#include "s_preferences.c"
#include "s_memory.c"
#include "s_leak.c"
#include "s_font.c"
#include "s_post.c"
#include "s_ringbuffer.c"
#include "s_logger.c"
#include "s_utf8.c"
#include "s_MT.c"
#include "s_MT32.c"
#include "s_MT64.c"
#include "s_devicesproperties.c"
#include "s_deviceslist.c"
#include "s_midiAPI.c"
#include "s_audioAPI.c"

/* Graphics. */

#include "g_stub.c"
#include "g_proxy.c"
#include "g_object.c"
#include "g_typeset.c"
#include "g_box.c"
#include "g_traverser.c"
#include "g_cord.c"
#include "g_editor.c"
#include "g_glist.c"
#include "g_draw.c"
#include "g_select.c"
#include "g_edit.c"
#include "g_arrange.c"
#include "g_interface.c"
#include "g_file.c"
#include "g_make.c"
#include "g_geometry.c"
#include "g_behavior.c"
#include "g_canvas.c"
#include "g_message.c"
#include "g_text.c"
#include "g_gatom.c"
#include "g_garray.c"
#include "g_vinlet.c"
#include "g_voutlet.c"
#include "g_iem.c"
#include "g_bang.c"
#include "g_toggle.c"
#include "g_radio.c"
#include "g_slider.c"
#include "g_dial.c"
#include "g_vu.c"
#include "g_panel.c"
#include "g_word.c"
#include "g_field.c"
#include "g_array.c"
#include "g_gpointer.c"
#include "g_template.c"
#include "g_struct.c"
#include "g_scalar.c"
#include "g_paint.c"
#include "g_serialize.c"
#include "g_pointer.c"
#include "g_get.c"
#include "g_set.c"
#include "g_element.c"
#include "g_getsize.c"
#include "g_setsize.c"
#include "g_append.c"
#include "g_drawpolygon.c"
#include "g_plot.c"
#include "g_drawtext.c"

/* Control. */

#include "x_atomoutlet.c"
#include "x_int.c"
#include "x_float.c"
#include "x_symbol.c"
#include "x_bang.c"
#include "x_list.c"
#include "x_listinlet.c"
#include "x_listappend.c"
#include "x_listprepend.c"
#include "x_listsplit.c"
#include "x_listtrim.c"
#include "x_listlength.c"
#include "x_liststore.c"
#include "x_listiterate.c"
#include "x_listgroup.c"
#include "x_listfromsymbol.c"
#include "x_listtosymbol.c"
#include "x_text.c"
#include "x_textbuffer.c"
#include "x_textclient.c"
#include "x_textget.c"
#include "x_textset.c"
#include "x_textinsert.c"
#include "x_textdelete.c"
#include "x_textsize.c"
#include "x_textlist.c"
#include "x_textsearch.c"
#include "x_textsequence.c"
#include "x_qlist.c"
#include "x_textfile.c"
#include "x_array.c"
#include "x_arrayclient.c"
#include "x_arrayrange.c"
#include "x_arraysize.c"
#include "x_arraysum.c"
#include "x_arrayget.c"
#include "x_arrayset.c"
#include "x_arrayquantile.c"
#include "x_arrayrandom.c"
#include "x_arraymax.c"
#include "x_arraymin.c"
#include "x_tabwrite.c"
#include "x_tabread.c"
#include "x_tabread4.c"
#include "x_tabreceive.c"
#include "x_acoustic.c"
#include "x_math.c"
#include "x_atan2.c"
#include "x_binop1.c"
#include "x_binop2.c"
#include "x_binop3.c"
#include "x_random.c"
#include "x_clip.c"
#include "x_expr.c"
#include "x_metro.c"
#include "x_delay.c"
#include "x_line.c"
#include "x_timer.c"
#include "x_pipe.c"
#include "x_send.c"
#include "x_receive.c"
#include "x_select.c"
#include "x_route.c"
#include "x_pack.c"
#include "x_unpack.c"
#include "x_trigger.c"
#include "x_prepend.c"
#include "x_spigot.c"
#include "x_moses.c"
#include "x_until.c"
#include "x_uzi.c"
#include "x_swap.c"
#include "x_change.c"
#include "x_value.c"
#include "x_counter.c"
#include "x_print.c"
#include "x_key.c"
#include "x_keyup.c"
#include "x_keyname.c"
#include "x_openpanel.c"
#include "x_savepanel.c"
#include "x_makefilename.c"
#include "x_loadbang.c"
#include "x_samplerate.c"
#include "x_blocksize.c"
#include "x_dspstatus.c"
#include "x_arguments.c"
#include "x_title.c"
#include "x_freeze.c"
#include "x_namecanvas.c"
#include "x_serial.c"
#include "x_realtime.c"
#include "x_netsend.c"
#include "x_netreceive.c"
#include "x_oscparse.c"
#include "x_oscformat.c"
#include "x_makenote.c"
#include "x_stripnote.c"
#include "x_bag.c"
#include "x_poly.c"
#include "x_midiin.c"
#include "x_midiout.c"
#include "x_notein.c"
#include "x_noteout.c"
#include "x_ctlin.c"
#include "x_ctlout.c"
#include "x_pgmin.c"
#include "x_pgmout.c"
#include "x_bendin.c"
#include "x_bendout.c"
#include "x_touchin.c"
#include "x_touchout.c"
#include "x_polytouchin.c"
#include "x_polytouchout.c"
#include "x_sysexin.c"
#include "x_midirealtimein.c"

/* DSP. */

#include "d_dsp.c"
#include "d_signal.c"
#include "d_ugen.c"
#include "d_canvas.c"
#include "d_vinlet.c"
#include "d_voutlet.c"
#include "d_adc.c"
#include "d_dac.c"
#include "d_resample.c"
#include "d_block.c"
#include "d_vperform.c"
#include "d_perform.c"
#include "d_functions.c"
#include "d_throw.c"
#include "d_catch.c"
#include "d_send.c"
#include "d_receive.c"
#include "d_osc.c"
#include "d_phasor.c"
#include "d_cos.c"
#include "d_sig.c"
#include "d_line.c"
#include "d_vline.c"
#include "d_snapshot.c"
#include "d_env.c"
#include "d_threshold.c"
#include "d_samphold.c"
#include "d_delwrite.c"
#include "d_delread.c"
#include "d_vd.c"
#include "d_tabwrite.c"
#include "d_tabread.c"
#include "d_tabread4.c"
#include "d_tabplay.c"
#include "d_tabosc4.c"
#include "d_tabsend.c"
#include "d_tabreceive.c"
#include "d_add.c"
#include "d_subtract.c"
#include "d_multiply.c"
#include "d_divide.c"
#include "d_greater.c"
#include "d_less.c"
#include "d_max.c"
#include "d_min.c"
#include "d_clip.c"
#include "d_abs.c"
#include "d_wrap.c"
#include "d_sqrt.c"
#include "d_rsqrt.c"
#include "d_pow.c"
#include "d_exp.c"
#include "d_log.c"
#include "d_mtof.c"
#include "d_ftom.c"
#include "d_dbtorms.c"
#include "d_rmstodb.c"
#include "d_dbtopow.c"
#include "d_powtodb.c"
#include "d_print.c"
#include "d_bang.c"
#include "d_noise.c"
#include "d_lrshift.c"
#include "d_soundfile.c"
#include "d_subchunk.c"
#include "d_codec.c"
#include "d_soundfiler.c"
#include "d_sfthread.c"
#include "d_readsf.c"
#include "d_writesf.c"
#include "d_vcf.c"
#include "d_hip.c"
#include "d_lop.c"
#include "d_bp.c"
#include "d_biquad.c"
#include "d_rpole.c"
#include "d_rzero.c"
#include "d_rzeroreverse.c"
#include "d_cpole.c"
#include "d_czero.c"
#include "d_czeroreverse.c"
#include "d_rfft.c"
#include "d_rifft.c"
#include "d_fft.c"
#include "d_ifft.c"
#include "d_framp.c"
#include "d_mag.c"
#include "d_rmag.c"
#include "d_fftOOURA.c"

/* Libraries. */

#include "fftsg.c"

#if PD_WITH_DUMMY

    #include "s_midi_dummy.c"
    #include "s_audio_dummy.c"

#else

#if PD_APPLE
    #include "s_midi_pm.c"
    #include "s_audio_pa.c"
    #include "pa_mac_hostapis.c"
#endif

#if PD_LINUX
    #include "s_midi_alsa.c"
    #include "s_audio_jack.c"
#endif

#endif // PD_WITH_DUMMY

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
