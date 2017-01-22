
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void inmidi_noteOn (int port, int channel, int pitch, int velocity)
{
    if (pd_isThingQuiet (sym__notein)) {
    //
    t_atom a[3];
    
    SET_FLOAT (a + 0, pitch);
    SET_FLOAT (a + 1, velocity);
    SET_FLOAT (a + 2, channel + 1 + (port << 4));
    
    pd_list (pd_getThing (sym__notein), 3, a);
    //
    }
}

void inmidi_controlChange (int port, int channel, int control, int value)
{
    if (pd_isThingQuiet (sym__ctlin)) {
    //
    t_atom a[3];
    
    SET_FLOAT (a + 0, control);
    SET_FLOAT (a + 1, value);
    SET_FLOAT (a + 2, channel + 1 + (port << 4));
    
    pd_list (pd_getThing (sym__ctlin), 3, a);
    //
    }
}

void inmidi_programChange (int port, int channel, int value)
{
    if (pd_isThingQuiet (sym__pgmin)) {
    //
    t_atom a[2];
    
    SET_FLOAT (a + 0, value + 1);
    SET_FLOAT (a + 1, channel + 1 + (port << 4));
    
    pd_list (pd_getThing (sym__pgmin), 2, a);
    //
    }
}

void inmidi_pitchBend (int port, int channel, int value)
{
    if (pd_isThingQuiet (sym__bendin)) {
    //
    t_atom a[2];
    
    SET_FLOAT (a + 0, value);
    SET_FLOAT (a + 1, channel + 1 + (port << 4));
    
    pd_list (pd_getThing (sym__bendin), 2, a);
    //
    }
}

void inmidi_afterTouch (int port, int channel, int value)
{
    if (pd_isThingQuiet (sym__touchin)) {
    //
    t_atom a[2];
    
    SET_FLOAT (a + 0, value);
    SET_FLOAT (a + 1, channel + 1 + (port << 4));
    
    pd_list (pd_getThing (sym__touchin), 2, a);
    //
    }
}

void inmidi_polyPressure (int port, int channel, int pitch, int value)
{
    if (pd_isThingQuiet (sym__polytouchin)) {
    //
    t_atom a[3];
    
    SET_FLOAT (a + 0, pitch);
    SET_FLOAT (a + 1, value);
    SET_FLOAT (a + 2, channel + 1 + (port << 4));
    
    pd_list (pd_getThing (sym__polytouchin), 3, a);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void inmidi_byte (int port, int byte)
{
    if (pd_isThingQuiet (sym__midiin)) {
    //
    t_atom a[2];
        
    SET_FLOAT (a + 0, byte);
    SET_FLOAT (a + 1, port);
    
    pd_list (pd_getThing (sym__midiin), 2, a);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void inmidi_sysex (int port, int byte)
{
    if (pd_isThingQuiet (sym__sysexin)) {
    //
    t_atom a[2];
            
    SET_FLOAT (a + 0, byte);
    SET_FLOAT (a + 1, port);
    
    pd_list (pd_getThing (sym__sysexin), 2, a);
    //
    }
}

void inmidi_realTime (int port, int byte)
{
    if (pd_isThingQuiet (sym__midirealtimein)) {
    //
    t_atom a[2];
    
    SET_FLOAT (a + 0, byte);
    SET_FLOAT (a + 1, port);
    
    pd_list (pd_getThing (sym__midirealtimein), 2, a);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
