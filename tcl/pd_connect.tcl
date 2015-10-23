
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Manage IPC with TCP socket.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide pd_connect 0.1

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::pd_connect:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable tcpSocket
variable tcpBuffer ""

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc configureSocket {sock} {
    
    # Non-blocking socket without buffer.
    
    fconfigure $sock -blocking 0 -buffering none -encoding utf-8
    
    # Set the callback executed while receiving data. 
    
    fileevent $sock readable {::pd_connect::pd_readsocket}
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc configureClientSocket {} { 
    variable tcpSocket
    ::pd_connect::configureSocket $tcpSocket 
}

proc configureServerSocket {channel host port} {
    variable tcpSocket $channel
    ::pd_connect::configureSocket $tcpSocket
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc clientSocket {port host} {

    variable tcpSocket
    
    if {[catch {set tcpSocket [socket $host $port]}]} { 
        error "Connection to the TCP server $host:$port failed."
    }

    ::pd_connect::configureClientSocket
}

proc serverSocket {} {

    if {[catch {set sock [socket -server ::pd_connect::configureServerSocket -myaddr localhost 0]}]} {
        error "Creation of the TCP server failed."
    }
    
    return [lindex [fconfigure $sock -sockname] 2]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# send a pd/FUDI message from Tcl to Pd. This function aims to behave like a
# [; message( in Pd or pdsend on the command line.  Basically, whatever is in
# quotes after the proc name will be sent as if it was sent from a message box
# with a leading semi-colon.
proc pdsend {message} {
    variable tcpSocket
    append message \;
    if {[catch {puts $tcpSocket $message} errorname]} {
        puts stderr "pdsend errorname: >>$errorname<<"
        error "Not connected to 'pd' process"
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc pd_readsocket {} {
     variable tcpSocket
     variable tcpBuffer
     if {[eof $tcpSocket]} {
         # if we lose the socket connection, that means pd quit, so we quit
         close $tcpSocket
         exit
     } 
     append tcpBuffer [read $tcpSocket]
     if {[string index $tcpBuffer end] ne "\n" || \
             ![info complete $tcpBuffer]} {
         # the block is incomplete, wait for the next block of data
         return
     } else {
         set docmds $tcpBuffer
         set tcpBuffer ""
         if {![catch {uplevel #0 $docmds} errorname]} {
             # we ran the command block without error, reset the buffer
         } else {
             # oops, error, alert the user:
             global errorInfo
             switch -regexp -- $errorname {
                 "missing close-brace" {
                     ::pd_console::fatal \
                         [concat [_ "(Tcl) MISSING CLOSE-BRACE '\}': "] $errorInfo "\n"]
                 } "^invalid command name" {
                     ::pd_console::fatal \
                         [concat [_ "(Tcl) INVALID COMMAND NAME: "] $errorInfo "\n"]
                 } default {
                     ::pd_console::fatal \
                         [concat [_ "(Tcl) UNHANDLED ERROR: "] $errorInfo "\n"]
                 }
             }
         }
     }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
