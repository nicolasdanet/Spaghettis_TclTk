
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_midi_h_
#define __s_midi_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define MIDI_NOTEOFF            0x80    // 128
#define MIDI_NOTEON             0x90    // 144
#define MIDI_POLYPRESSURE       0xa0    // 160
#define MIDI_CONTROLCHANGE      0xb0    // 176
#define MIDI_PROGRAMCHANGE      0xc0    // 192
#define MIDI_AFTERTOUCH         0xd0    // 208
#define MIDI_PITCHBEND          0xe0    // 224

#define MIDI_STARTSYSEX         0xf0    // 240
#define MIDI_TIMECODE           0xf1    // 241
#define MIDI_SONGPOS            0xf2    // 242
#define MIDI_SONGSELECT         0xf3    // 243
#define MIDI_RESERVED1          0xf4    // 244
#define MIDI_RESERVED2          0xf5    // 245
#define MIDI_TUNEREQUEST        0xf6    // 246
#define MIDI_ENDSYSEX           0xf7    // 247

#define MIDI_CLOCK              0xf8    // 248
#define MIDI_TICK               0xf9    // 249
#define MIDI_START              0xfa    // 250
#define MIDI_CONTINUE           0xfb    // 251
#define MIDI_STOP               0xfc    // 252
#define MIDI_ACTIVESENSE        0xfe    // 254
#define MIDI_RESET              0xff    // 255

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define MIDI_BYTES(x)           (((x) < MIDI_PROGRAMCHANGE) ? 2 : \
                                    ((x) < MIDI_PITCHBEND)  ? 1 : \
                                    ((x) < MIDI_STARTSYSEX) ? 2 : \
                                    ((x) == MIDI_SONGPOS)   ? 2 : \
                                    ((x) < MIDI_RESERVED1)  ? 1 : \
                                    0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void inmidi_noteOn              (int port, int channel, int pitch, int velocity);
void inmidi_controlChange       (int port, int channel, int control, int value);
void inmidi_programChange       (int port, int channel, int value);
void inmidi_pitchBend           (int port, int channel, int value);
void inmidi_afterTouch          (int port, int channel, int value);
void inmidi_polyPressure        (int port, int channel, int pitch, int value);
void inmidi_byte                (int port, int byte);
void inmidi_sysex               (int port, int byte);
void inmidi_realTime            (int port, int byte);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void outmidi_noteOn             (int channel, int pitch, int velocity);
void outmidi_controlChange      (int channel, int control, int value);
void outmidi_programChange      (int channel, int value);
void outmidi_pitchBend          (int channel, int value);
void outmidi_afterTouch         (int channel, int value);
void outmidi_polyPressure       (int channel, int pitch, int value);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_midi_h_
