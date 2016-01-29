
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WINDOWS
    #include <tchar.h>
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pathlist   *sys_searchpath;

extern int sys_audioapi;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if ( PD_LINUX || PD_CYGWIN || PD_BSD || PD_HURD || PD_ANDROID )

static char *preferences_loadBuffer;                        /* Shared. */
static FILE *preferences_saveFile;                          /* Shared. */

static t_error preferences_loadBegin (void)
{
    char *home = getenv ("HOME");
    char filepath[PD_STRING] = { 0 };
    t_error err = utils_snprintf (filepath, PD_STRING, "%s/.puredatarc", (home ? home : "."));
    
    if (!err) { err |= !path_isFileExist (filepath); }
    if (!err) {
    //
    int f;
    
    err |= ((f = open (filepath, 0)) < 0);
    
    if (!err) {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (!err) {
    //
    preferences_loadBuffer = PD_MEMORY_GET (length + 2);
    preferences_loadBuffer[0] = '\n';
    err |= (read (f, preferences_loadBuffer + 1, length) < length);
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
    char *home = getenv ("HOME");
    char filepath[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    err = (!home);

    if (!err) {
        err = utils_snprintf (filepath, PD_STRING, "%s/.puredatarc", home);
        if (!err) { 
            err = ((preferences_saveFile = fopen (filepath, "w")) == NULL); 
        }
    }
    
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
    t_error err = utils_snprintf (t, PD_STRING, "\n%s:", key);

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
        if (!utils_strncat (value, size, p, length)) { return 1; }
    }
    //
    }
    
    return 0;
}

static void preferences_setKey (const char *key, const char *value)
{
    if (preferences_saveFile) { fprintf (preferences_saveFile, "%s: %s\n", key, value); }   // --
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if PD_WINDOWS

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
    LONG err = RegOpenKeyEx (HKEY_LOCAL_MACHINE, "Software\\Pd", 0, KEY_QUERY_VALUE, &hkey);
    
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
                                "Software\\Pd",
                                0,
                                NULL,
                                REG_OPTION_NON_VOLATILE,
                                KEY_SET_VALUE,
                                NULL,
                                &hkey,
                                NULL);
                                
    if (err != ERROR_SUCCESS) {
        post_error (PD_TRANSLATE ("preferences: unable to create %s entry\n"), key);    // --
        return;
    }
    
    err = RegSetValueEx (hkey, key, 0, REG_EXPAND_SZ, value, strlen (value) + 1);
    
    if (err != ERROR_SUCCESS) {
        post_error (PD_TRANSLATE ("preferences: unable to set %s entry\n"), key);       // --
    }
    
    RegCloseKey (hkey);
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if PD_APPLE

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
    char t[PD_STRING] = { 0 };
    t_error err = utils_snprintf (t, PD_STRING, "defaults read org.puredata.puredata %s 2> /dev/null\n", key);
    
    PD_ASSERT (size > 2);
    
    if (err) { PD_BUG; }
    else {
    //
    FILE *f = popen (t, "r");
    int i = 0;
        
    while (i < size) {
        int n = fread (value + i, 1, size - 1 - i, f);
        if (n <= 0) { break; }
        else { 
            i += n; 
        }
    }
    
    pclose (f);
    
    PD_ASSERT (i < size);
    
    if (i > 1) {                                                        /* Values are ended by a newline. */
        value[i] = 0; if (value[i - 1] == '\n') { value[i - 1] = 0; }   /* Remove it. */
        return 1;
    }
    //
    }
    
    return 0;
}

static void preferences_setKey (const char *key, const char *value)
{
    t_error err = PD_ERROR_NONE;
    
    char t[PD_STRING] = { 0 };
    
    err = utils_snprintf (t, PD_STRING, "defaults write org.puredata.puredata %s \"%s\"", key, value);
    err |= utils_strnadd (t, PD_STRING, " 2> /dev/null\n");
    
    if (!err) { system (t); }
    else {
        PD_BUG;
    }
}

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

void preferences_load (void)
{
    int noAudioIn                       = 0;
    int noAudioOut                      = 0;
    int noMidiIn                        = 0;
    int noMidiOut                       = 0;
    
    int audioApi                        = API_DEFAULT;
    int callback                        = 0;
    int sampleRate                      = AUDIO_DEFAULT_SAMPLING;
    int advance                         = AUDIO_DEFAULT_ADVANCE;
    int blockSize                       = AUDIO_DEFAULT_BLOCK;

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
    
    if (preferences_loadBegin() == PD_ERROR_NONE) {
    //
    int i;

    /* Properties. */
    
    noAudioIn   = (preferences_getKey ("NoAudioIn",  value, PD_STRING) && !strcmp (value, "True"));
    noAudioOut  = (preferences_getKey ("NoAudioOut", value, PD_STRING) && !strcmp (value, "True"));
    noMidiIn    = (preferences_getKey ("NoMidiIn",   value, PD_STRING) && !strcmp (value, "True"));
    noMidiOut   = (preferences_getKey ("NoMidiOut",  value, PD_STRING) && !strcmp (value, "True"));
    
    if (preferences_getKey ("AudioApi",     value, PD_STRING)) { sscanf (value, "%d", &audioApi);     }
    if (preferences_getKey ("Callback",     value, PD_STRING)) { sscanf (value, "%d", &callback);     }
    if (preferences_getKey ("SampleRate",   value, PD_STRING)) { sscanf (value, "%d", &sampleRate);   }
    if (preferences_getKey ("Advance",      value, PD_STRING)) { sscanf (value, "%d", &advance);      }
    if (preferences_getKey ("BlockSize",    value, PD_STRING)) { sscanf (value, "%d", &blockSize);    }
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {
    //
    utils_snprintf (key, PD_STRING, "Path%d", i + 1);
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
        sys_searchpath = pathlist_newAppend (sys_searchpath, value);
    }
    //
    }
    
    /* Audio devices. */
    
    if (!noAudioIn) {
    //
    for (i = 0; i < MAXIMUM_AUDIO_IN; i++) {
    //
    utils_snprintf (key, PD_STRING, "AudioInDevice%d", i + 1);
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
    //
    if (sscanf (value, "%d %d", &audioIn[i], &channelIn[i]) < 2) { break; }
    else {       
        utils_snprintf (key, PD_STRING, "AudioInDeviceName%d", i + 1);
        if (preferences_getKey (key, value, PD_STRING)) {
            int device; 
            if ((device = sys_audiodevnametonumber (0, value)) >= 0) { audioIn[i] = device; }
        }
        numberOfAudioIn++;
        }
    }
    //
    //
    }
    
    if (numberOfAudioIn == 0) { numberOfAudioIn = -1; }
    //
    }
    
    if (!noAudioOut) {
    //
    for (i = 0; i < MAXIMUM_AUDIO_OUT; i++) {
    //
    utils_snprintf (key, PD_STRING, "AudioOutDevice%d", i + 1);
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
    //
    if (sscanf (value, "%d %d", &audioOut[i], &channelOut[i]) < 2) { break; }
    else {
        utils_snprintf (key, PD_STRING, "AudioOutDeviceName%d", i + 1);
        if (preferences_getKey (key, value, PD_STRING)) {
            int device;
            if ((device = sys_audiodevnametonumber (1, value)) >= 0) { audioOut[i] = device; }
        }
        numberOfAudioOut++;
        }
    }
    //
    //
    }
        
    if (numberOfAudioOut == 0) { numberOfAudioOut = -1; }
    //
    }
    
    /* MIDI devices. */
    
    if (!noMidiIn) {
    //
    for (i = 0; i < MAXIMUM_MIDI_IN; i++) {
    //
    int device;
    
    utils_snprintf (key, PD_STRING, "MidiInDeviceName%d", i + 1);
    
    if (preferences_getKey (key, value, PD_STRING) && (device = sys_mididevnametonumber (0, value)) >= 0) {
        midiIn[i] = device;
        
    } else {
        utils_snprintf (key, PD_STRING, "MidiInDevice%d", i + 1);
        if (!preferences_getKey (key, value, PD_STRING)) { break; }
        else if (sscanf (value, "%d", &midiIn[i]) < 1)   { break; }
    }
    numberOfMidiIn++;
    //
    }
    //
    }

    if (!noMidiOut) {
    //
    for (i = 0; i < MAXIMUM_MIDI_OUT; i++) {
    //
    int device;
    
    utils_snprintf (key, PD_STRING, "MidiOutDeviceName%d", i + 1);
    
    if (preferences_getKey (key, value, PD_STRING) && (device = sys_mididevnametonumber (1, value)) >= 0) {
        midiOut[i] = device;
        
    } else {
        utils_snprintf (key, PD_STRING, "MidiOutDevice%d", i + 1);
        if (!preferences_getKey (key, value, PD_STRING)) { break; }
        else if (sscanf (value, "%d", &midiOut[i]) < 1)  { break; }
    }
    numberOfMidiOut++;
    //
    }
    //
    }
    //
    }
    
    sys_set_audio_api (audioApi);
    
    sys_set_audio_settings (numberOfAudioIn,
                            audioIn, 
                            numberOfAudioIn,
                            channelIn,
                            numberOfAudioOut,
                            audioOut,
                            numberOfAudioOut,
                            channelOut, 
                            sampleRate,
                            advance,
                            callback,
                            blockSize);
                            
    sys_open_midi (numberOfMidiIn, midiIn, numberOfMidiOut, midiOut, 0);
    
    preferences_loadClose();
}

void preferences_save (void *dummy)
{
    if (preferences_saveBegin() == PD_ERROR_NONE) {
    //
    int i;
    t_pathlist *list;
    
    int callback                        = 0;
    int sampleRate                      = AUDIO_DEFAULT_SAMPLING;
    int advance                         = AUDIO_DEFAULT_ADVANCE;
    int blockSize                       = AUDIO_DEFAULT_BLOCK;
    
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
    
    sys_get_audio_params (&numberOfAudioIn,
                            audioIn, 
                            channelIn,
                            &numberOfAudioOut, 
                            audioOut, 
                            channelOut, 
                            &sampleRate, 
                            &advance, 
                            &callback,
                            &blockSize);
    
    sys_get_midi_params (&numberOfMidiIn, midiIn, &numberOfMidiOut, midiOut);
    
    /* Properties. */
    
    preferences_setKey ("NoAudioIn",    (numberOfAudioIn <= 0 ?  "True" : "False"));
    preferences_setKey ("NoAudioOut",   (numberOfAudioOut <= 0 ? "True" : "False"));
    preferences_setKey ("NoMidiIn",     (numberOfMidiIn <= 0 ?   "True" : "False"));
    preferences_setKey ("NoMidiOut",    (numberOfMidiOut <= 0 ?  "True" : "False"));
    
    utils_snprintf (value, PD_STRING, "%d", sys_audioapi);      preferences_setKey ("AudioApi",     value);
    utils_snprintf (value, PD_STRING, "%d", callback);          preferences_setKey ("Callback",     value);
    utils_snprintf (value, PD_STRING, "%d", sampleRate);        preferences_setKey ("SampleRate",   value);
    utils_snprintf (value, PD_STRING, "%d", advance);           preferences_setKey ("Advance",      value);
    utils_snprintf (value, PD_STRING, "%d", blockSize);         preferences_setKey ("BlockSize",    value);
    
    /* Search paths. */
    
    list = sys_searchpath;
    
    for (i = 0; 1; i++) {
    //
    char *path = pathlist_getFile (list);
    if (!path) { break; }
    else {
        utils_snprintf (key, PD_STRING, "Path%d", i + 1);       preferences_setKey (key, path);
        list = pathlist_getNext (list);
    }
    //
    }

    /* Audio devices. */
    
    for (i = 0; i < numberOfAudioIn; i++) {
    //
    utils_snprintf (key, PD_STRING, "AudioInDevice%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d %d", audioIn[i], channelIn[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "AudioInDeviceName%d", i + 1);
    sys_audiodevnumbertoname (0, audioIn[i], value, PD_STRING);
    preferences_setKey (key, value);
    //
    }

    for (i = 0; i < numberOfAudioOut; i++) {
    //
    utils_snprintf (key, PD_STRING, "AudioOutDevice%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d %d", audioOut[i], channelOut[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "AudioOutDeviceName%d", i + 1);
    sys_audiodevnumbertoname (1, audioOut[i], value, PD_STRING);
    preferences_setKey (key, value);
    //
    }

    /* MIDI devices. */
    
    for (i = 0; i < numberOfMidiIn; i++) {
    //
    utils_snprintf (key, PD_STRING, "MidiInDevice%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d", midiIn[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "MidiInDeviceName%d", i + 1);
    sys_mididevnumbertoname (0, midiIn[i], value, PD_STRING);
    preferences_setKey (key, value);
    //
    }
    
    for (i = 0; i < numberOfMidiOut; i++) {
    //
    utils_snprintf (key, PD_STRING, "MidiOutDevice%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d", midiOut[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "MidiOutDeviceName%d", i + 1);
    sys_mididevnumbertoname (1, midiOut[i], value, PD_STRING);
    preferences_setKey (key, value);
    //
    }
    //
    }
    
    preferences_saveClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
