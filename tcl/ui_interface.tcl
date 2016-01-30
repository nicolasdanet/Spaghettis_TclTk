
# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Copyright (c) 1997-2015 Miller Puckette and others ( https://opensource.org/licenses/BSD-3-Clause ).

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Manage IPC with a TCP socket.

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

package provide ui_interface 1.0

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

namespace eval ::ui_interface:: {

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

variable tcpSocket
variable tcpBuffer ""

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _readSocket {} {
    
    variable tcpSocket
    variable tcpBuffer
    
    # Exit if socket lost.
    
    if {[eof $tcpSocket]} { close $tcpSocket; exit } 
    
    # Fetch data.
    
    append tcpBuffer [read $tcpSocket]
    
    # Execute the command (skip uncomplete) at global scope.
    
    if {[string index $tcpBuffer end] ne "\n" || ![info complete $tcpBuffer]} { 
        return 
    } else {
        set script $tcpBuffer
        set tcpBuffer ""
        if {[catch { uplevel "#0" $script }]} { 
            puts stderr $::errorInfo 
        }
    }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _configureSocket {sock} {
    
    # Non-blocking socket without buffer.
    
    fconfigure $sock -blocking 0 -buffering none -encoding utf-8
    
    # Set the callback executed while receiving data. 
    
    fileevent $sock readable { ::ui_interface::_readSocket }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc _configureClientSocket {} { 
    variable tcpSocket
    ::ui_interface::_configureSocket $tcpSocket 
}

proc _configureServerSocket {channel host port} {
    variable tcpSocket $channel
    ::ui_interface::_configureSocket $tcpSocket
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

proc clientSocket {port host} {

    variable tcpSocket
    
    if {[catch { set tcpSocket [socket $host $port] }]} { 
        error "Connection to the TCP server $host:$port failed."
    }

    ::ui_interface::_configureClientSocket
}

proc serverSocket {} {

    if {[catch { set sock [socket -server ::ui_interface::_configureServerSocket -myaddr localhost 0] }]} {
        error "Creation of the TCP server failed."
    }
    
    set ::var(tcpPort) [lindex [fconfigure $sock -sockname] 2]
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

# Send FUDI message to the pd executable ( https://en.wikipedia.org/wiki/FUDI ).

proc pdsend {args} {

    variable tcpSocket
    
    set message [join $args]
    
    append message \; 
    
    if {[catch { puts $tcpSocket $message }]} { error "Unable to post onto the TCP socket." }
}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------

}

# ------------------------------------------------------------------------------------------------------------
# ------------------------------------------------------------------------------------------------------------
