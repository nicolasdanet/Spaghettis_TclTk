
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

void outmidi_noteOn (int channel, int pitch, int velocity)
{
    int t    = PD_MAX (0, channel - 1);
    pitch    = PD_CLAMP (pitch, 0, 127);
    velocity = PD_CLAMP (velocity, 0, 127);

    midi_broadcast ((t >> 4), 0, MIDI_NOTEON + (t & 0xf), pitch, velocity);
}

void outmidi_controlChange (int channel, int control, int value)
{
    int t   = PD_MAX (0, channel - 1);
    control = PD_CLAMP (control, 0, 127);
    value   = PD_CLAMP (value, 0, 127);
    
    midi_broadcast ((t >> 4), 0, MIDI_CONTROLCHANGE + (t & 0xf), control, value);
}

void outmidi_programChange (int channel, int value)
{
    int t = PD_MAX (0, channel - 1);
    value = PD_CLAMP (value - 1, 0, 127);
    
    midi_broadcast ((t >> 4), 0, MIDI_PROGRAMCHANGE + (t & 0xf), value, 0);
}

void outmidi_pitchBend (int channel, int value)
{
    int t = PD_MAX (0, channel - 1);
    value = PD_CLAMP (value, 0, 16383);     // 0x3fff 
    
    midi_broadcast ((t >> 4), 0, MIDI_PITCHBEND + (t & 0xf), (value & 127), ((value >> 7) & 127));
}

void outmidi_afterTouch (int channel, int value)
{
    int t = PD_MAX (0, channel - 1);
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast ((t >> 4), 0, MIDI_AFTERTOUCH + (t & 0xf), value, 0);
}

void outmidi_polyPressure (int port, int channel, int pitch, int value)
{
    pitch = PD_CLAMP (pitch, 0, 127);
    value = PD_CLAMP (value, 0, 127);
    
    midi_broadcast (port, 0, MIDI_POLYPRESSURE + (channel & 0xf), pitch, value);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void outmidi_clock (int port)
{
    midi_broadcast (port, 1, MIDI_CLOCK, 0, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
