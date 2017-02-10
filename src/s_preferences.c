
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
    int sampleRate  = AUDIO_DEFAULT_SAMPLERATE;
    int blockSize   = AUDIO_DEFAULT_BLOCKSIZE;

    t_devicesproperties midi;    
    t_devicesproperties audio;   

    devices_init (&midi);
    devices_init (&audio);
    
    if (properties_loadBegin() == PD_ERROR_NONE) {
    //
    int i;

    char k[PD_STRING] = { 0 };
    char v[PD_STRING] = { 0 };
    
    /* Audio settings. */
    
    if (properties_getKey ("SampleRate", v, PD_STRING)) { sscanf (v, "%d", &sampleRate); }
    if (properties_getKey ("BlockSize",  v, PD_STRING)) { sscanf (v, "%d", &blockSize);  }
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {

        string_sprintf (k, PD_STRING, "Path%d", i + 1);
        
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else {
            path_appendToSearchPath (v);
        }
    }
    
    /* Audio devices. */
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        int channels;
        
        string_sprintf (k, PD_STRING, "AudioInDeviceChannels%d", i + 1);
        
        if (properties_getKey (k, v, PD_STRING)) {
            if (sscanf (v, "%d", &channels) == 1) {    
                string_sprintf (k, PD_STRING, "AudioInDeviceName%d", i + 1);
                if (properties_getKey (k, v, PD_STRING)) {
                    devices_appendAudioIn (&audio, v, channels);
                }
            }
        }
    }
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        int channels;
        
        string_sprintf (k, PD_STRING, "AudioOutDeviceChannels%d", i + 1);
        
        if (properties_getKey (k, v, PD_STRING)) {
            if (sscanf (v, "%d", &channels) == 1) {    
                string_sprintf (k, PD_STRING, "AudioOutDeviceName%d", i + 1);
                if (properties_getKey (k, v, PD_STRING)) {
                    devices_appendAudioOut (&audio, v, channels);
                }
            }
        }
    }
        
    /* MIDI devices. */
    
    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        string_sprintf (k, PD_STRING, "MidiInDeviceName%d", i + 1);
        
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else {
            devices_appendMidiIn (&midi, v);
        }
    }

    for (i = 0; i < DEVICES_MAXIMUM_IO; i++) {

        string_sprintf (k, PD_STRING, "MidiOutDeviceName%d", i + 1);
        
        if (!properties_getKey (k, v, PD_STRING)) { break; }
        else { 
            devices_appendMidiOut (&midi, v);
        }
    }
    //
    }
    
    devices_setDefaultsMidi (&midi);
    devices_setDefaultsAudio (&audio);
    
    midi_setDevices (&midi);
    audio_setDevices (&audio, sampleRate, blockSize);
    
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
    
    int audioIn[DEVICES_MAXIMUM_IO]       = { 0 };
    int audioOut[DEVICES_MAXIMUM_IO]     = { 0 };
    int midiIn[DEVICES_MAXIMUM_IO]         = { 0 };
    int midiOut[DEVICES_MAXIMUM_IO]       = { 0 };
    int channelIn[DEVICES_MAXIMUM_IO]     = { 0 };
    int channelOut[DEVICES_MAXIMUM_IO]   = { 0 };
    
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
    if (audio_deviceAsStringWithNumber (0, audioIn[i], value, PD_STRING)) { break; }
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
    if (audio_deviceAsStringWithNumber (1, audioOut[i], value, PD_STRING)) { break; }
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
    midi_deviceAsStringWithNumber (0, midiIn[i], value, PD_STRING);
    properties_setKey (key, value);
    //
    }

    for (i = 0; i < numberOfMidiOut; i++) {
    //
    string_sprintf (key, PD_STRING, "MidiOutDeviceName%d", i + 1);
    midi_deviceAsStringWithNumber (1, midiOut[i], value, PD_STRING);
    properties_setKey (key, value);
    //
    }
    //
    }
    
    properties_saveClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
