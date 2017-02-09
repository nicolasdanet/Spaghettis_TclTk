
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

void preferences_load (void)
{
    int sampleRate                      = AUDIO_DEFAULT_SAMPLERATE;
    int blockSize                       = AUDIO_DEFAULT_BLOCKSIZE;

    int numberOfAudioIn                 = 0;
    int numberOfAudioOut                = 0;
    int numberOfMidiIn                  = 0;
    int numberOfMidiOut                 = 0;

    int audioIn[MAXIMUM_AUDIO_IN]       = { 0 };
    int audioOut[MAXIMUM_AUDIO_OUT]     = { 0 };
    int channelIn[MAXIMUM_AUDIO_IN]     = { 0 };
    int channelOut[MAXIMUM_AUDIO_OUT]   = { 0 };
    int midiIn[MAXIMUM_MIDI_IN]         = { 0 };
    int midiOut[MAXIMUM_MIDI_OUT]       = { 0 };
    
    char key[PD_STRING]                 = { 0 };
    char value[PD_STRING]               = { 0 };
    
    if (properties_loadBegin() == PD_ERROR_NONE) {
    //
    int i;

    /* Properties. */
    
    if (properties_getKey ("SampleRate",   value, PD_STRING)) { sscanf (value, "%d", &sampleRate);   }
    if (properties_getKey ("BlockSize",    value, PD_STRING)) { sscanf (value, "%d", &blockSize);    }
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {
    //
    string_sprintf (key, PD_STRING, "Path%d", i + 1);
    if (!properties_getKey (key, value, PD_STRING)) { break; }
    else {
        path_appendToSearchPath (value);
    }
    //
    }
    
    /* Audio devices. */
    
    for (i = 0; i < MAXIMUM_AUDIO_IN; i++) {
    //
    int done = 0;
    
    string_sprintf (key, PD_STRING, "AudioInDeviceChannels%d", i + 1);
    
    if (properties_getKey (key, value, PD_STRING)) {
        if (sscanf (value, "%d", &channelIn[i]) == 1) {    
            string_sprintf (key, PD_STRING, "AudioInDeviceName%d", i + 1);
            if (properties_getKey (key, value, PD_STRING)) {
                int device = audio_numberWithName (0, value); 
                if (device >= 0) { audioIn[i] = device; numberOfAudioIn++; done = 1; }
            }
        }
    }
    
    if (!done) { break; }
    //
    }
    
    for (i = 0; i < MAXIMUM_AUDIO_OUT; i++) {
    //
    int done = 0;
    
    string_sprintf (key, PD_STRING, "AudioOutDeviceChannels%d", i + 1);
    
    if (properties_getKey (key, value, PD_STRING)) {
        if (sscanf (value, "%d", &channelOut[i]) == 1) {    
            string_sprintf (key, PD_STRING, "AudioOutDeviceName%d", i + 1);
            if (properties_getKey (key, value, PD_STRING)) {
                int device = audio_numberWithName (1, value); 
                if (device >= 0) { audioOut[i] = device; numberOfAudioOut++; done = 1; }
            }
        }
    }
    
    if (!done) { break; }
    //
    }
        
    /* MIDI devices. */
    
    for (i = 0; i < MAXIMUM_MIDI_IN; i++) {
    //
    int device;
    
    string_sprintf (key, PD_STRING, "MidiInDeviceName%d", i + 1);
    
    if (!properties_getKey (key, value, PD_STRING)) { break; }
    else {
        if ((device = midi_numberWithName (0, value)) >= 0) { 
            midiIn[i] = device;
            numberOfMidiIn++;
        }
    }
    //
    }

    for (i = 0; i < MAXIMUM_MIDI_OUT; i++) {
    //
    int device;
    
    string_sprintf (key, PD_STRING, "MidiOutDeviceName%d", i + 1);
    
    if (!properties_getKey (key, value, PD_STRING)) { break; }
    else { 
        if ((device = midi_numberWithName (1, value)) >= 0) {
            midiOut[i] = device;
            numberOfMidiOut++;
        }
    }
    //
    }
    //
    }
    
    audio_setDevicesWithDefault (numberOfAudioIn,
        audioIn, 
        channelIn,
        numberOfAudioOut,
        audioOut,
        channelOut, 
        sampleRate,
        blockSize);
                            
    midi_setDefaultDevices (numberOfMidiIn, midiIn, numberOfMidiOut, midiOut);
        
    properties_loadClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

void preferences_save (void *dummy)
{
    if (properties_saveBegin() == PD_ERROR_NONE) {
    //
    int i;
    t_pathlist *list;
    
    int sampleRate                      = AUDIO_DEFAULT_SAMPLERATE;
    int blockSize                       = AUDIO_DEFAULT_BLOCKSIZE;
    
    int numberOfAudioIn                 = 0;
    int numberOfAudioOut                = 0;
    int numberOfMidiIn                  = 0;
    int numberOfMidiOut                 = 0;
    
    int audioIn[MAXIMUM_AUDIO_IN]       = { 0 };
    int audioOut[MAXIMUM_AUDIO_OUT]     = { 0 };
    int midiIn[MAXIMUM_MIDI_IN]         = { 0 };
    int midiOut[MAXIMUM_MIDI_OUT]       = { 0 };
    int channelIn[MAXIMUM_AUDIO_IN]     = { 0 };
    int channelOut[MAXIMUM_AUDIO_OUT]   = { 0 };
    
    char key[PD_STRING]                 = { 0 };
    char value[PD_STRING]               = { 0 };
    
    audio_getDevices (&numberOfAudioIn,
                            audioIn, 
                            channelIn,
                            &numberOfAudioOut, 
                            audioOut, 
                            channelOut, 
                            &sampleRate,
                            &blockSize);
    
    midi_getDevices (&numberOfMidiIn, midiIn, &numberOfMidiOut, midiOut);
    
    /* Properties. */
    
    string_sprintf (value, PD_STRING, "%d", sampleRate);        properties_setKey ("SampleRate", value);
    string_sprintf (value, PD_STRING, "%d", blockSize);         properties_setKey ("BlockSize",  value);
    
    /* Search paths. */
    
    list = path_getSearchPath();
    
    for (i = 0; 1; i++) {
    //
    char *path = pathlist_getPath (list);
    if (!path) { break; }
    else {
        string_sprintf (key, PD_STRING, "Path%d", i + 1); 
        properties_setKey (key, path);
        list = pathlist_getNext (list);
    }
    //
    }
    
    /* Audio devices. */
    
    for (i = 0; i < numberOfAudioIn; i++) {
    //
    string_sprintf (key, PD_STRING, "AudioInDeviceName%d", i + 1);
    if (audio_numberToName (0, audioIn[i], value, PD_STRING)) { break; }
    else {
        properties_setKey (key, value);
        string_sprintf (key, PD_STRING, "AudioInDeviceChannels%d", i + 1);
        string_sprintf (value, PD_STRING, "%d", channelIn[i]);
        properties_setKey (key, value);
    }
    //
    }

    for (i = 0; i < numberOfAudioOut; i++) {
    //
    string_sprintf (key, PD_STRING, "AudioOutDeviceName%d", i + 1);
    if (audio_numberToName (1, audioOut[i], value, PD_STRING)) { break; }
    else {
        properties_setKey (key, value);
        string_sprintf (key, PD_STRING, "AudioOutDeviceChannels%d", i + 1);
        string_sprintf (value, PD_STRING, "%d", channelOut[i]);
        properties_setKey (key, value);
    }
    //
    }

    /* MIDI devices. */
    
    for (i = 0; i < numberOfMidiIn; i++) {
    //
    string_sprintf (key, PD_STRING, "MidiInDeviceName%d", i + 1);
    midi_numberToName (0, midiIn[i], value, PD_STRING);
    properties_setKey (key, value);
    //
    }

    for (i = 0; i < numberOfMidiOut; i++) {
    //
    string_sprintf (key, PD_STRING, "MidiOutDeviceName%d", i + 1);
    midi_numberToName (1, midiOut[i], value, PD_STRING);
    properties_setKey (key, value);
    //
    }
    //
    }
    
    properties_saveClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
