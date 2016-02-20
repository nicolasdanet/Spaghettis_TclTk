
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_midi_h_
#define __s_midi_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MIDI_NOTEOFF                    0x80
#define MIDI_NOTEON                     0x90
#define MIDI_POLYPRESSURE               0xa0
#define MIDI_CONTROLCHANGE              0xb0
#define MIDI_PROGRAMCHANGE              0xc0
#define MIDI_AFTERTOUCH                 0xd0
#define MIDI_PITCHBEND                  0xe0

#define MIDI_STARTSYSEX                 0xf0
#define MIDI_TIMECODE                   0xf1
#define MIDI_SONGPOS                    0xf2
#define MIDI_SONGSELECT                 0xf3
#define MIDI_RESERVED1                  0xf4
#define MIDI_RESERVED2                  0xf5
#define MIDI_TUNEREQUEST                0xf6
#define MIDI_ENDSYSEX                   0xf7
#define MIDI_CLOCK                      0xf8
#define MIDI_TICK                       0xf9
#define MIDI_START                      0xfa
#define MIDI_CONTINUE                   0xfb
#define MIDI_STOP                       0xfc
#define MIDI_ACTIVESENSE                0xfe
#define MIDI_RESET                      0xff

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_midi_h_
