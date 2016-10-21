
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void outmidi_noteOn (int port, int channel, int pitch, int velocity)
{
    pitch    = PD_CLAMP (pitch, 0, 127);
    velocity = PD_CLAMP (velocity, 0, 127);

    midi_broadcast (port, 0, MIDI_NOTEON + (channel & 0xf), pitch, velocity);
}

void outmidi_controlChange (int port, int channel, int control, int value)
{
    control = PD_CLAMP (control, 0, 127);
    value   = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_CONTROLCHANGE + (channel & 0xf), control, value);
}

void outmidi_programChange (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_PROGRAMCHANGE + (channel & 0xf), value, 0);
}

void outmidi_pitchBend (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 16383);     // 0x3fff 
    
    midi_broadcast (port, 0, MIDI_PITCHBEND + (channel & 0xf), (value & 127), ((value >> 7) & 127));
}

void outmidi_afterTouch (int port, int channel, int value)
{
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_AFTERTOUCH + (channel & 0xf), value, 0);
}

void outmidi_polyPressure (int port, int channel, int pitch, int value)
{
    pitch = PD_CLAMP (pitch, 0, 127);
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_POLYPRESSURE + (channel & 0xf), pitch, value);
}

void outmidi_clock (int port)
{
    midi_broadcast (port, 1, MIDI_CLOCK, 0, 0);
}
