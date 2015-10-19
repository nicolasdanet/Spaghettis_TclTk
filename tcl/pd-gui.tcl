#! /usr/bin/env wish

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Withdraw the window first to avoid flashing.

if {[catch {wm withdraw .} fid]} {exit 2}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require Tcl 8.5
package require Tk

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Note that ALL the Tcl files MUST be kept in the same directory that this file.

set auto_path [linsert $auto_path 0 [file dirname [info script]]]

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Package specific to Mac OS X.

if {[tk windowingsystem] eq "aqua"} { package require pd_apple }

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package require dialog_array
package require dialog_audio
package require dialog_canvas
package require dialog_data
package require dialog_font
package require dialog_gatom
package require dialog_gui
package require dialog_message
package require dialog_midi
package require dialog_path
package require dialog_startup
package require pd_bindings
package require pd_commands
package require pd_connect
package require pd_console
package require pd_preferences
package require pd_menus
package require pd_miscellaneous
package require pd_parser
package require pdtk_canvas
package require pdtk_text
package require pdtk_textwindow

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Global variables that are used throughout the GUI.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set pd_gui(scriptname)                  [file normalize [info script]]
set pd_gui(modifier)                    ""
set pd_gui(host)                        ""
set pd_gui(port)                        0

set pd_gui(startup_flags)               {}
set pd_gui(startup_libraries)           {}

set pd_gui(api_audio_list)              {}
set pd_gui(api_midi_list)               {}
set pd_gui(api_audio)                   0
set pd_gui(api_midi)                    0

set pd_gui(directory_new)               [pwd]
set pd_gui(directory_open)              [pwd]
set pd_gui(directory_path)              {}

set pd_gui(file_pended)                 {}
set pd_gui(file_recent)                 {}
set pd_gui(file_recent_maximum)         5

set pd_gui(font_family)                 courier
set pd_gui(font_weight)                 normal
set pd_gui(font_fixed)                  "8 6 11 10 7 13 12 9 16 14 8 17 16 10 20 18 11 22 24 15 25 30 18 37"
set pd_gui(font_measured)               {}

set pd_gui(is_initialized)              0
set pd_gui(is_stderr)                   0
set pd_gui(is_dsp)                      0
set pd_gui(is_editmode)                 0

set pd_gui(window_focused)              .
set pd_gui(window_popup_x)              0
set pd_gui(window_popup_y)              0
set pd_gui(window_frame_x)              0
set pd_gui(window_frame_y)              0

set pd_gui(window_menubar)              ""
set pd_gui(window_menubar_height)       0
set pd_gui(window_minimum_width)        50
set pd_gui(window_minimum_height)       20

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

set audio_indev     {}
set audio_outdev    {}
set midi_indev      {}
set midi_outdev     {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

array set patch_is_editmode         {}
array set patch_is_editing          {}
array set patch_loaded              {}
array set patch_is_scrollable_x     {}
array set patch_is_scrollable_y     {}
array set patch_name                {}
array set patch_childs              {}
array set patch_parents             {}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc init_for_platform {} {
    # we are not using Tk scaling, so fix it to 1 on all platforms.  This
    # guarantees that patches will be pixel-exact on every platform
    # 2013.07.19 msp - trying without this to see what breaks - it's having
    # deleterious effects on dialog window font sizes.
    # tk scaling 1

    switch -- [tk windowingsystem] {
        "x11" {
            set ::pd_gui(modifier) "Control"
            option add *PatchWindow*Canvas.background "white" startupFile
            # add control to show/hide hidden files in the open panel (load
            # the tk_getOpenFile dialog once, otherwise it will not work)
            catch {tk_getOpenFile -with-invalid-argument} 
            set ::tk::dialog::file::showHiddenBtn 1
            set ::tk::dialog::file::showHiddenVar 0
            # set file types that open/save recognize
            set ::filetypes \
                [list \
                     [list [_ "Associated Files"]  {.pd .pat .mxt} ] \
                     [list [_ "Pd Files"]          {.pd}  ] \
                     [list [_ "Max Patch Files"]   {.pat} ] \
                     [list [_ "Max Text Files"]    {.mxt} ] \
                    ]
            # some platforms have a menubar on the top, so place below them
            set ::pd_gui(window_menubar_height) 0
            # Tk handles the window placement differently on each
            # platform. With X11, the x,y placement refers to the window
            # frame's upper left corner. http://wiki.tcl.tk/11502
            set ::pd_gui(window_frame_x) 3
            set ::pd_gui(window_frame_y) 53
			# TODO add wm iconphoto/iconbitmap here if it makes sense
            # mouse cursors for all the different modes
            set ::cursor_runmode_nothing "left_ptr"
            set ::cursor_runmode_clickme "arrow"
            set ::cursor_runmode_thicken "sb_v_double_arrow"
            set ::cursor_runmode_addpoint "plus"
            set ::cursor_editmode_nothing "hand2"
            set ::cursor_editmode_connect "circle"
            set ::cursor_editmode_disconnect "X_cursor"
            set ::cursor_editmode_resize "sb_h_double_arrow"
        }
        "aqua" {
            set ::pd_gui(modifier) "Mod1"
            option add *DialogWindow*background "#E8E8E8" startupFile
            option add *DialogWindow*Entry.highlightBackground "#E8E8E8" startupFile
            option add *DialogWindow*Button.highlightBackground "#E8E8E8" startupFile
            option add *DialogWindow*Entry.background "white" startupFile
            # Mac OS X needs a menubar all the time
            set ::pd_gui(window_menubar) ".menubar"
            # set file types that open/save recognize
            set ::filetypes \
                [list \
                     [list [_ "Associated Files"]       {.pd .pat .mxt} ] \
                     [list [_ "Pd Files"]               {.pd}  ] \
                     [list [_ "Max Patch Files (.pat)"] {.pat} ] \
                     [list [_ "Max Text Files (.mxt)"]  {.mxt} ] \
                ]
            # some platforms have a menubar on the top, so place below them
            set ::pd_gui(window_menubar_height) 22
            # Tk handles the window placement differently on each platform, on
            # Mac OS X, the x,y placement refers to the content window's upper
            # left corner (not of the window frame) http://wiki.tcl.tk/11502
            set ::pd_gui(window_frame_x) 0
            set ::pd_gui(window_frame_y) 0
            # mouse cursors for all the different modes
            set ::cursor_runmode_nothing "arrow"
            set ::cursor_runmode_clickme "center_ptr"
            set ::cursor_runmode_thicken "sb_v_double_arrow"
            set ::cursor_runmode_addpoint "plus"
            set ::cursor_editmode_nothing "hand2"
            set ::cursor_editmode_connect "circle"
            set ::cursor_editmode_disconnect "X_cursor"
            set ::cursor_editmode_resize "sb_h_double_arrow"
        }
        "win32" {
            set ::pd_gui(modifier) "Control"
            option add *PatchWindow*Canvas.background "white" startupFile
            # fix menu font size on Windows with tk scaling = 1
            font create menufont -family Tahoma -size -11
            option add *Menu.font menufont startupFile
            option add *DialogWindow*font menufont startupFile
            option add *PdWindow*font menufont startupFile
            option add *ErrorDialog*font menufont startupFile
            # set file types that open/save recognize
            set ::filetypes \
                [list \
                     [list [_ "Associated Files"]  {.pd .pat .mxt} ] \
                     [list [_ "Pd Files"]          {.pd}  ] \
                     [list [_ "Max Patch Files"]   {.pat} ] \
                     [list [_ "Max Text Files"]    {.mxt} ] \
                    ]
            # some platforms have a menubar on the top, so place below them
            set ::pd_gui(window_menubar_height) 0
            # Tk handles the window placement differently on each platform, on
            # Mac OS X, the x,y placement refers to the content window's upper
            # left corner. http://wiki.tcl.tk/11502 
            # TODO this probably needs a script layer: http://wiki.tcl.tk/11291
            set ::pd_gui(window_frame_x) 0
            set ::pd_gui(window_frame_y) 0
            # TODO use 'winico' package for full, hicolor icon support
            # mouse cursors for all the different modes
            set ::cursor_runmode_nothing "right_ptr"
            set ::cursor_runmode_clickme "arrow"
            set ::cursor_runmode_thicken "sb_v_double_arrow"
            set ::cursor_runmode_addpoint "plus"
            set ::cursor_editmode_nothing "hand2"
            set ::cursor_editmode_connect "circle"
            set ::cursor_editmode_disconnect "X_cursor"
            set ::cursor_editmode_resize "sb_h_double_arrow"
        }
    }
}

# ------------------------------------------------------------------------------
# locale handling

# official GNU gettext msgcat shortcut
proc _ {s} {return $s}

proc load_locale {} {
    ##--moo: force default system and stdio encoding to UTF-8
    encoding system utf-8
    fconfigure stderr -encoding utf-8
    fconfigure stdout -encoding utf-8
    ##--/moo
}

# ------------------------------------------------------------------------------
# font handling

# this proc gets the internal font name associated with each size
proc get_font_for_size {size} {
    return "::pd_font_${size}"
}

# searches for a font to use as the default.  Tk automatically assigns a
# monospace font to the name "Courier" (see Tk 'font' docs), but it doesn't
# always do a good job of choosing in respect to Pd's needs.  So this chooses
# from a list of fonts that are known to work well with Pd.
proc find_default_font {} {
    set testfonts {"DejaVu Sans Mono" "Bitstream Vera Sans Mono" \
        "Inconsolata" "Courier 10 Pitch" "Andale Mono" "Droid Sans Mono"}
    foreach family $testfonts {
        if {[lsearch -exact -nocase [font families] $family] > -1} {
            set ::pd_gui(font_family) $family
            break
        }
    }
    ::pd_console::verbose 0 "Default font: $::pd_gui(font_family)\n"
}

proc set_base_font {family weight} {
    if {[lsearch -exact [font families] $family] > -1} {
        set ::pd_gui(font_family) $family
    } else {
        ::pd_console::post [format \
            [_ "WARNING: Font family '%s' not found, using default (%s)\n"] \
                $family $::pd_gui(font_family)]
    }
    if {[lsearch -exact {bold normal} $weight] > -1} {
        set ::pd_gui(font_weight) $weight
        set using_defaults 0
    } else {
        ::pd_console::post [format \
            [_ "WARNING: Font weight '%s' not found, using default (%s)\n"] \
                $weight $::pd_gui(font_weight)]
    }
}

# creates all the base fonts (i.e. pd_font_8 thru pd_font_36) so that they fit
# into the metrics given by $::pd_gui(font_fixed) for any given font/weight
proc fit_font_into_metrics {} {
# TODO the fonts picked seem too small, probably on fixed width
    foreach {size width height} $::pd_gui(font_fixed) {
        set myfont [get_font_for_size $size]
        font create $myfont -family $::pd_gui(font_family) -weight $::pd_gui(font_weight) \
            -size [expr {-$height}]
        set height2 $height
        set giveup 0
        while {[font measure $myfont M] > $width || \
            [font metrics $myfont -linespace] > $height} {
            incr height2 -1
            font configure $myfont -size [expr {-$height2}]
            if {$height2 * 2 <= $height} {
                set giveup 1
                set ::pd_gui(font_measured) $::pd_gui(font_fixed)
                break
            }
        }
        set ::pd_gui(font_measured) \
            "$::pd_gui(font_measured)  $size\
                [font measure $myfont M] [font metrics $myfont -linespace]"
        if {$giveup} {
            ::pd_console::post [format \
    [_ "WARNING: %s failed to find font size (%s) that fits into %sx%s!\n"]\
               [lindex [info level 0] 0] $size $width $height]
            continue
        }
    }
}


# ------------------------------------------------------------------------------
# procs called directly by pd

proc pdtk_pd_startup {major minor bugfix test
                      audio_apis midi_apis sys_font sys_fontweight} {
    set oldtclversion 0
    set ::pd_gui(api_audio_list) $audio_apis
    set ::pd_gui(api_midi_list) $midi_apis
    if {$::tcl_version >= 8.5} {find_default_font}
    set_base_font $sys_font $sys_fontweight
    fit_font_into_metrics
    ::pd_preferences::init
    ::pd_connect::pdsend "pd init [enquote_path [pwd]] $oldtclversion $::pd_gui(font_measured)"
    ::pd_bindings::class_bindings
    ::pd_bindings::global_bindings
    ::pd_menus::create_menubar
    ::pdtk_canvas::create_popup
    ::pd_console::create_window
    ::pd_menus::configure_for_pdwindow
    open_filestoopen
    set ::pd_gui(is_initialized) 1
}

##### routine to ask user if OK and, if so, send a message on to Pd ######
proc pdtk_check {mytoplevel message reply_to_pd default} {
    wm deiconify $mytoplevel
    raise $mytoplevel
    if {[tk windowingsystem] eq "win32"} {
        set answer [tk_messageBox -message [_ $message] -type yesno -default $default \
                        -icon question -title [wm title $mytoplevel]]
    } else {
        set answer [tk_messageBox -message [_ $message] -type yesno \
                        -default $default -parent $mytoplevel -icon question]
    }
    if {$answer eq "yes"} {
        ::pd_connect::pdsend $reply_to_pd
    }
}

# store plugin callbacks for dispatched messages from running Pd patches
global plugin_dispatch_receivers

# dispatch a message from running Pd patches to the intended plugin receiver
proc pdtk_plugin_dispatch { args } {
    set receiver [ lindex $args 0 ]
    foreach callback $::pd_connect::plugin_dispatch_receivers($receiver) {
        $callback [ lrange $args 1 end ]
    }
}

proc parse_args {argc argv} {
    pd_parser::init {
        {-stderr    set {::pd_gui(is_stderr)}}
        {-open      lappend {- ::pd_gui(file_pended)}}
    }
    set unflagged_files [pd_parser::get_options $argv]
    # if we have a single arg that is not a file, its a port or host:port combo
    if {$argc == 1 && ! [file exists $argv]} {
        if { [string is int $argv] && $argv > 0} {
            # 'pd-gui' got the port number from 'pd'
            set ::pd_gui(host) "localhost"
            set ::pd_gui(port) $argv 
        } else {
            set hostport [split $argv ":"]
            set ::pd_gui(port) [lindex $hostport 1]
            if { [string is int $::pd_gui(port)] && $::pd_gui(port) > 0} {
                set ::pd_gui(host) [lindex $hostport 0]
            } else {
                set ::pd_gui(port) 0
            }

        }
    } elseif {$unflagged_files ne ""} {
        foreach filename $unflagged_files {
            lappend ::pd_gui(file_pended) $filename
        }
    }
}

proc open_filestoopen {} {
    foreach filename $::pd_gui(file_pended) {
        ::pd_miscellaneous::open_file $filename
    }
}

# ------------------------------------------------------------------------------
# X11 procs for handling singleton state and getting args from other instances

# first instance
proc singleton {key} {
    if {![catch { selection get -selection $key }]} {
        return 0
    }
    selection handle -selection $key . "singleton_request"
    selection own -command first_lost -selection $key .
    return 1
}

proc singleton_request {offset maxbytes} {
## the next 2 lines raise the focus to the given window (and change desktop)
#    wm deiconify .pdwindow
#    raise .pdwindow
    return [tk appname]
}

proc first_lost {} {
    receive_args [selection get -selection $::pd_gui(scriptname) ]
    selection own -command first_lost -selection $::pd_gui(scriptname) .
 }

proc others_lost {} {
    set ::singleton_state "exit"
    destroy .
    exit
}

# all other instances
proc send_args {offset maxChars} {
    set sendargs {}
    foreach filename $::pd_gui(file_pended) {
        lappend sendargs [file normalize $filename]
    }
    return [string range $sendargs $offset [expr {$offset+$maxChars}]]
}

# this command will open files received from a 2nd instance of Pd
proc receive_args {filelist} {
    raise .
    wm deiconify .pdwindow
    raise .pdwindow
    foreach filename $filelist {
        ::pd_miscellaneous::open_file $filename
    }
}

proc dde_open_handler {cmd} {
    ::pd_miscellaneous::open_file [file normalize $cmd]
}

proc check_for_running_instances { } {
    switch -- [tk windowingsystem] {
        "aqua" {
            # handled by ::tk::mac::OpenDocument in pd_apple.tcl
        } "x11" {
            # http://wiki.tcl.tk/1558
            # TODO replace PUREDATA name with path so this code is a singleton
            # based on install location rather than this hard-coded name
            if {![singleton ${::pd_gui(scriptname)}_MANAGER ]} {
                # if pd-gui gets called from pd ('pd-gui 5400') or is told otherwise
                # to connect to a running instance of Pd (by providing [<host>:]<port>)
                # then we don't want to connect to a running instance
                if { $::pd_gui(port) > 0 && $::pd_gui(host) ne "" } { return }
                selection handle -selection $::pd_gui(scriptname) . "send_args"
                selection own -command others_lost -selection $::pd_gui(scriptname) .
                after 5000 set ::singleton_state "timeout"
                vwait ::singleton_state
                exit
            } else {
                # first instance
                selection own -command first_lost -selection $::pd_gui(scriptname) .
            }
        } "win32" {
            ## http://wiki.tcl.tk/8940
            package require dde ;# 1.4 or later needed for full unicode support
            set topic "Pure_Data_DDE_Open"
            # if no DDE service is running, start one and claim the name
            if { [dde services TclEval $topic] == {} } {
                dde servername -handler dde_open_handler $topic
            }
        }
    }
}


# ------------------------------------------------------------------------------
# main
proc main {argc argv} {
    tk appname pd-gui
    load_locale
    parse_args $argc $argv
    check_for_running_instances
    init_for_platform

    # ::pd_gui(host) and ::pd_gui(port) are parsed from argv by parse_args
    if { $::pd_gui(port) > 0 && $::pd_gui(host) ne "" } {
        # 'pd' started first and launched us, so get the port to connect to
        ::pd_connect::to_pd $::pd_gui(port) $::pd_gui(host)
    } else {
        # the GUI is starting first, so create socket and exec 'pd'
        set ::pd_gui(port) [::pd_connect::create_socket]
        set pd_exec [file join [file dirname [info script]] ../bin/pd]
        exec -- $pd_exec -guiport $::pd_gui(port) &
        if {[tk windowingsystem] eq "aqua"} {
            # on Aqua, if 'pd-gui' first, then initial dir is home
            set ::pd_gui(directory_new) $::env(HOME)
            set ::pd_gui(directory_open) $::env(HOME)
        }
    }
    ::pd_console::verbose 0 "------------------ done with main ----------------------\n"
}

main $::argc $::argv
