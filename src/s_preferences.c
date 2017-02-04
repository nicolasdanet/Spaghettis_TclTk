
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

extern t_symbol     *main_directoryExtras;
extern t_pathlist   *path_search;

extern int main_directoryWriteRequirePrivileges;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if ! ( PD_WINDOWS )

static char *preferences_loadBuffer;                        /* Shared. */
static FILE *preferences_saveFile;                          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error preferences_loadBegin (void)
{
    char filepath[PD_STRING] = { 0 };
    
    t_error err = PD_ERROR_NONE;
    
    if (!main_directoryWriteRequirePrivileges) {
        err = string_sprintf (filepath, PD_STRING, "%s/preferences.txt", main_directoryExtras->s_name);
    } else {
        char *home = getenv ("HOME");
        err = string_sprintf (filepath, PD_STRING, "%s/."PD_NAME_LOWERCASE"rc", (home ? home : "."));
    }

    if (!err) { err |= !path_isFileExist (filepath); }
    if (!err) {
    //
    int f;
    
    err |= ((f = file_openRaw (filepath, O_RDONLY)) < 0);
    
    if (!err) {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (!err) {
    //
    preferences_loadBuffer = (char *)PD_MEMORY_GET ((size_t)length + 2);
    preferences_loadBuffer[0] = '\n';
    err |= (read (f, preferences_loadBuffer + 1, (size_t)length) < length);
    //
    }
    
    if (err) { preferences_loadBuffer[0] = 0; PD_BUG; }
    
    close (f);
    //
    }
    //
    }
    
    return err;
}

static void preferences_loadClose (void)
{
    if (preferences_loadBuffer) { 
        PD_MEMORY_FREE (preferences_loadBuffer); preferences_loadBuffer = NULL; 
    }
}

static t_error preferences_saveBegin (void)
{
    char filepath[PD_STRING] = { 0 };
    
    t_error err = PD_ERROR_NONE;
    
    if (!main_directoryWriteRequirePrivileges) {
        err = string_sprintf (filepath, PD_STRING, "%s/preferences.txt", main_directoryExtras->s_name);
    } else {
        char *home = getenv ("HOME");
        err = string_sprintf (filepath, PD_STRING, "%s/."PD_NAME_LOWERCASE"rc", (home ? home : "."));
    }
    
    if (!err) { err = ((preferences_saveFile = file_openWrite (filepath)) == NULL); }
    
    return err;
}

static void preferences_saveClose (void)
{
    if (preferences_saveFile) { 
        fclose (preferences_saveFile); preferences_saveFile = NULL; 
    }
}

static int preferences_getKey (const char *key, char *value, int size)
{
    char t[PD_STRING] = { 0 };
    char *p = NULL;
    char *pEnd = NULL;
    t_error err = string_sprintf (t, PD_STRING, "\n%s:", key);

    PD_ASSERT (preferences_loadBuffer != NULL);
    PD_ASSERT (!err);
    
    p = strstr (preferences_loadBuffer, t);
    
    if (p) {
    //
    *value = 0; p += strlen (t);
    
    while (*p == ' ' || *p == '\t') { p++; }
    for (pEnd = p; *pEnd && *pEnd != '\n'; pEnd++) { } 
    
    if (*pEnd == '\n') { 
        pEnd--; 
    }
    
    size_t length = pEnd + 1 - p;
    
    if (length > 0) { 
        if (!string_append (value, size, p, (int)length)) { return 1; }
    }
    //
    }
    
    return 0;
}

static void preferences_setKey (const char *key, const char *value)
{
    if (preferences_saveFile) { fprintf (preferences_saveFile, "%s: %s\n", key, value); }   // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#else

static t_error preferences_loadBegin (void)
{
    return PD_ERROR_NONE;
}

static void preferences_loadClose (void)
{
}

static t_error preferences_saveBegin (void)
{
    return PD_ERROR_NONE;
}

static void preferences_saveClose (void)
{
}

static int preferences_getKey (const char *key, char *value, int size)
{
    HKEY hkey;
    DWORD n = size;
    LONG err = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\" PD_NAME_SHORT, 0, KEY_QUERY_VALUE, &hkey);
    
    if (err != ERROR_SUCCESS) { return 0; }
    
    err = RegQueryValueEx (hkey, key, 0, 0, value, &n);
    
    if (err != ERROR_SUCCESS) { RegCloseKey (hkey); return 0; }
    
    RegCloseKey (hkey);
    
    return 1;
}

static void preferences_setKey (const char *key, const char *value)
{
    HKEY hkey;
    LONG err = RegCreateKeyEx (HKEY_LOCAL_MACHINE,
                                "Software\\" PD_NAME_SHORT,
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE,
                                NULL,
                                &hkey,
                                NULL);
                                
    if (err != ERROR_SUCCESS) { PD_BUG; return; }
    
    err = RegSetValueEx (hkey, key, 0, REG_EXPAND_SZ, value, strlen (value) + 1);
    
    if (err != ERROR_SUCCESS) { PD_BUG; }
    
    RegCloseKey (hkey);
}

#endif

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
    
    if (preferences_loadBegin() == PD_ERROR_NONE) {
    //
    int i;

    /* Properties. */
    
    if (preferences_getKey ("SampleRate",   value, PD_STRING)) { sscanf (value, "%d", &sampleRate);   }
    if (preferences_getKey ("BlockSize",    value, PD_STRING)) { sscanf (value, "%d", &blockSize);    }
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {
    //
    string_sprintf (key, PD_STRING, "Path%d", i + 1);
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
        path_search = pathlist_newAppend (path_search, value);
    }
    //
    }
    
    /* Audio devices. */
    
    for (i = 0; i < MAXIMUM_AUDIO_IN; i++) {
    //
    int done = 0;
    
    string_sprintf (key, PD_STRING, "AudioInDeviceChannels%d", i + 1);
    
    if (preferences_getKey (key, value, PD_STRING)) {
        if (sscanf (value, "%d", &channelIn[i]) == 1) {    
            string_sprintf (key, PD_STRING, "AudioInDeviceName%d", i + 1);
            if (preferences_getKey (key, value, PD_STRING)) {
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
    
    if (preferences_getKey (key, value, PD_STRING)) {
        if (sscanf (value, "%d", &channelOut[i]) == 1) {    
            string_sprintf (key, PD_STRING, "AudioOutDeviceName%d", i + 1);
            if (preferences_getKey (key, value, PD_STRING)) {
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
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
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
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
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
        
    preferences_loadClose();
}

void preferences_save (void *dummy)
{
    if (preferences_saveBegin() == PD_ERROR_NONE) {
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
    
    string_sprintf (value, PD_STRING, "%d", sampleRate);        preferences_setKey ("SampleRate", value);
    string_sprintf (value, PD_STRING, "%d", blockSize);         preferences_setKey ("BlockSize",  value);
    
    /* Search paths. */
    
    list = path_search;
    
    for (i = 0; 1; i++) {
    //
    char *path = pathlist_getPath (list);
    if (!path) { break; }
    else {
        string_sprintf (key, PD_STRING, "Path%d", i + 1); 
        preferences_setKey (key, path);
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
        preferences_setKey (key, value);
        string_sprintf (key, PD_STRING, "AudioInDeviceChannels%d", i + 1);
        string_sprintf (value, PD_STRING, "%d", channelIn[i]);
        preferences_setKey (key, value);
    }
    //
    }

    for (i = 0; i < numberOfAudioOut; i++) {
    //
    string_sprintf (key, PD_STRING, "AudioOutDeviceName%d", i + 1);
    if (audio_numberToName (1, audioOut[i], value, PD_STRING)) { break; }
    else {
        preferences_setKey (key, value);
        string_sprintf (key, PD_STRING, "AudioOutDeviceChannels%d", i + 1);
        string_sprintf (value, PD_STRING, "%d", channelOut[i]);
        preferences_setKey (key, value);
    }
    //
    }

    /* MIDI devices. */
    
    for (i = 0; i < numberOfMidiIn; i++) {
    //
    string_sprintf (key, PD_STRING, "MidiInDeviceName%d", i + 1);
    midi_numberToName (0, midiIn[i], value, PD_STRING);
    preferences_setKey (key, value);
    //
    }

    for (i = 0; i < numberOfMidiOut; i++) {
    //
    string_sprintf (key, PD_STRING, "MidiOutDeviceName%d", i + 1);
    midi_numberToName (1, midiOut[i], value, PD_STRING);
    preferences_setKey (key, value);
    //
    }
    //
    }
    
    preferences_saveClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
