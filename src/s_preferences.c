
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
#pragma mark -

#if PD_MSVC
    #define snprintf sprintf_s
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_symbol     *main_rootDirectory;
extern t_pathlist   *sys_searchpath;

extern int sys_audioapi;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if ( PD_LINUX || PD_CYGWIN || PD_BSD || PD_HURD || PD_ANDROID )

static char *sys_prefbuf;

static void preferences_loadBegin (void)
{
    char filenamebuf[PD_STRING], *homedir = getenv("HOME");
    int fd, length;
    char user_prefs_file[PD_STRING]; /* user prefs file */
        /* default prefs embedded in the package */
    char default_prefs_file[PD_STRING];
    struct stat statbuf;

    snprintf(default_prefs_file, PD_STRING, "%s/default.puredata", 
        main_rootDirectory->s_name);
    snprintf(user_prefs_file, PD_STRING, "%s/.puredata", 
        (homedir ? homedir : "."));
    if (stat(user_prefs_file, &statbuf) == 0) 
        strncpy(filenamebuf, user_prefs_file, PD_STRING);
    else if (stat(default_prefs_file, &statbuf) == 0)
        strncpy(filenamebuf, default_prefs_file, PD_STRING);
    else return;
    filenamebuf[PD_STRING-1] = 0;
    if ((fd = open(filenamebuf, 0)) < 0)
    {
        if (0)
            perror(filenamebuf);
        return;
    }
    length = lseek(fd, 0, 2);
    if (length < 0)
    {
        if (0)
            perror(filenamebuf);
        close(fd);
        return;
    }
    lseek(fd, 0, 0);
    if (!(sys_prefbuf = malloc(length + 2)))
    {
        post_error ("couldn't allocate memory for preferences buffer");
        close(fd);
        return;
    }
    sys_prefbuf[0] = '\n';
    if (read(fd, sys_prefbuf+1, length) < length)
    {
        perror(filenamebuf);
        sys_prefbuf[0] = 0;
        close(fd);
        return;
    }
    sys_prefbuf[length+1] = 0;
    close(fd);
    if (0)
        post("success reading preferences from: %s", filenamebuf);
}

static void preferences_loadClose (void)
{
    if (sys_prefbuf)
        free(sys_prefbuf);
}

static int preferences_getKey(const char *key, char *value, int size)
{
    char searchfor[80], *where, *whereend;
    if (!sys_prefbuf)
        return (0);
    sprintf(searchfor, "\n%s:", key);
    where = strstr(sys_prefbuf, searchfor);
    if (!where)
        return (0);
    where += strlen(searchfor);
    while (*where == ' ' || *where == '\t')
        where++;
    for (whereend = where; *whereend && *whereend != '\n'; whereend++)
        ;
    if (*whereend == '\n')
        whereend--;
    if (whereend > where + size - 1)
        whereend = where + size - 1;
    strncpy(value, where, whereend+1-where);
    value[whereend+1-where] = 0;
    return (1);
}

static FILE *sys_prefsavefp;

static void preferences_saveBegin (void)
{
    char filenamebuf[PD_STRING],
        *homedir = getenv("HOME");
    FILE *fp;

    if (!homedir)
        return;
    snprintf(filenamebuf, PD_STRING, "%s/.puredata", homedir);
    filenamebuf[PD_STRING-1] = 0;
    if ((sys_prefsavefp = fopen(filenamebuf, "w")) == NULL)
    {
        post_error ("%s: %s", filenamebuf, strerror(errno));
    }
}

static void preferences_saveClose (void)
{
    if (sys_prefsavefp)
    {
        fclose(sys_prefsavefp);
        sys_prefsavefp = 0;
    }
}

static void preferences_setKey(const char *key, const char *value)
{
    if (sys_prefsavefp)
        fprintf(sys_prefsavefp, "%s: %s\n",
            key, value);
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if PD_WINDOWS

static void preferences_loadBegin (void)
{
}

static void preferences_loadClose (void)
{
}

static int preferences_getKey(const char *key, char *value, int size)
{
    HKEY hkey;
    DWORD bigsize = size;
    LONG err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "Software\\Pd", 0,  KEY_QUERY_VALUE, &hkey);
    if (err != ERROR_SUCCESS)
    {
        return (0);
    }
    err = RegQueryValueEx(hkey, key, 0, 0, value, &bigsize);
    if (err != ERROR_SUCCESS)
    {
        RegCloseKey(hkey);
        return (0);
    }
    RegCloseKey(hkey);
    return (1);
}

static void preferences_saveBegin (void)
{
}

static void preferences_saveClose (void)
{
}

static void preferences_setKey(const char *key, const char *value)
{
    HKEY hkey;
    LONG err = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
        "Software\\Pd", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE,
        NULL, &hkey, NULL);
    if (err != ERROR_SUCCESS)
    {
        post_error ("unable to create registry entry: %s\n", key);
        return;
    }
    err = RegSetValueEx(hkey, key, 0, REG_EXPAND_SZ, value, strlen(value)+1);
    if (err != ERROR_SUCCESS)
        post_error ("unable to set registry entry: %s\n", key);
    RegCloseKey(hkey);
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if PD_APPLE

static void preferences_loadBegin (void)
{
    ;
}

static void preferences_loadClose (void)
{
    ;
}

static int preferences_getKey (const char *k, char *v, int length)
{
    char t[PD_STRING] = { 0 };
    t_error err = utils_snprintf (t, PD_STRING, "defaults read org.puredata.puredata %s 2> /dev/null\n", k);
    
    if (err) { PD_BUG; }
    else {
    //
    FILE *f = popen (t, "r");
    int i = 0;
        
    while (i < length) {
        int n = fread (v + i, 1, length - i, f);
        if (n <= 0) { break; }
        i += n;
    }
    
    pclose (f);
    
    if (i >= 1) {
        if (i >= length) { i = length - 1; }
        v[i] = 0; if (v[i - 1] == '\n') { v[i - 1] = 0; }
        return 1;
    }
    //
    }
    
    return 0;
}

static void preferences_saveBegin (void)
{
    ;
}

static void preferences_saveClose (void)
{
    ;
}

static void preferences_setKey (const char *k, const char *v)
{
    t_error err = PD_ERROR_NONE;
    
    char t[PD_STRING] = { 0 };
    
    err = utils_snprintf (t, PD_STRING, "defaults write org.puredata.puredata %s \"%s\"", k, v);
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
    int i;
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

    int audioIn[AUDIO_MAXIMUM_IN]       = { 0 };
    int audioOut[AUDIO_MAXIMUM_OUT]     = { 0 };
    int midiIn[MIDI_MAXIMUM_IN]         = { 0 };
    int midiOut[MIDI_MAXIMUM_OUT]       = { 0 };
    int channelIn[AUDIO_MAXIMUM_IN]     = { 0 };
    int channelOut[AUDIO_MAXIMUM_OUT]   = { 0 };
    
    char key[PD_STRING]                 = { 0 };
    char value[PD_STRING]               = { 0 };

    preferences_loadBegin();
    
    /* Properties. */
    
    noAudioIn   = (preferences_getKey ("noaudioin",  value, PD_STRING) && !strcmp (value, "True"));
    noAudioOut  = (preferences_getKey ("noaudioout", value, PD_STRING) && !strcmp (value, "True"));
    noMidiIn    = (preferences_getKey ("nomidiin",   value, PD_STRING) && !strcmp (value, "True"));
    noMidiOut   = (preferences_getKey ("nomidiout",  value, PD_STRING) && !strcmp (value, "True"));
    
    if (preferences_getKey ("audioapi",  value, PD_STRING)) { sscanf (value, "%d", &audioApi);     }
    if (preferences_getKey ("callback",  value, PD_STRING)) { sscanf (value, "%d", &callback);     }
    if (preferences_getKey ("rate",      value, PD_STRING)) { sscanf (value, "%d", &sampleRate);   }
    if (preferences_getKey ("audiobuf",  value, PD_STRING)) { sscanf (value, "%d", &advance);      }
    if (preferences_getKey ("blocksize", value, PD_STRING)) { sscanf (value, "%d", &blockSize);    }
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {
    //
    utils_snprintf (key, PD_STRING, "path%d", i + 1);
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
        sys_searchpath = pathlist_newAppend (sys_searchpath, value);
    }
    //
    }
    
    /* Audio devices. */
    
    if (!noAudioIn) {
    //
    for (i = 0; i < AUDIO_MAXIMUM_IN; i++) {
    //
    utils_snprintf (key, PD_STRING, "audioindev%d", i + 1);
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
    //
    if (sscanf (value, "%d %d", &audioIn[i], &channelIn[i]) < 2) { break; }
    else {       
        utils_snprintf (key, PD_STRING, "audioindevname%d", i + 1);
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
    for (i = 0; i < AUDIO_MAXIMUM_OUT; i++) {
    //
    utils_snprintf (key, PD_STRING, "audiooutdev%d", i + 1);
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
    //
    if (sscanf (value, "%d %d", &audioOut[i], &channelOut[i]) < 2) { break; }
    else {
        utils_snprintf (key, PD_STRING, "audiooutdevname%d", i + 1);
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
    for (i = 0; i < MIDI_MAXIMUM_IN; i++) {
    //
    int device;
    
    utils_snprintf (key, PD_STRING, "midiindevname%d", i + 1);
    
    if (preferences_getKey (key, value, PD_STRING) && (device = sys_mididevnametonumber (0, value)) >= 0) {
        midiIn[i] = device;
        
    } else {
        utils_snprintf (key, PD_STRING, "midiindev%d", i + 1);
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
    for (i = 0; i < MIDI_MAXIMUM_OUT; i++) {
    //
    int device;
    
    utils_snprintf (key, PD_STRING, "midioutdevname%d", i + 1);
    
    if (preferences_getKey (key, value, PD_STRING) && (device = sys_mididevnametonumber (1, value)) >= 0) {
        midiOut[i] = device;
        
    } else {
        utils_snprintf (key, PD_STRING, "midioutdev%d", i + 1);
        if (!preferences_getKey (key, value, PD_STRING)) { break; }
        else if (sscanf (value, "%d", &midiOut[i]) < 1)  { break; }
    }
    numberOfMidiOut++;
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
    int i;
    int callback                        = 0;
    int sampleRate                      = AUDIO_DEFAULT_SAMPLING;
    int advance                         = AUDIO_DEFAULT_ADVANCE;
    int blockSize                       = AUDIO_DEFAULT_BLOCK;
    
    int numberOfAudioIn                 = 0;
    int numberOfAudioOut                = 0;
    int numberOfMidiIn                  = 0;
    int numberOfMidiOut                 = 0;
    
    int audioIn[AUDIO_MAXIMUM_IN]       = { 0 };
    int audioOut[AUDIO_MAXIMUM_OUT]     = { 0 };
    int midiIn[MIDI_MAXIMUM_IN]         = { 0 };
    int midiOut[MIDI_MAXIMUM_OUT]       = { 0 };
    int channelIn[AUDIO_MAXIMUM_IN]     = { 0 };
    int channelOut[AUDIO_MAXIMUM_OUT]   = { 0 };
    
    char key[PD_STRING]                 = { 0 };
    char value[PD_STRING]               = { 0 };
    
    preferences_saveBegin();

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
    
    preferences_setKey ("noaudioin",    (numberOfAudioIn <= 0 ?  "True" : "False"));
    preferences_setKey ("noaudioout",   (numberOfAudioOut <= 0 ? "True" : "False"));
    preferences_setKey ("nomidiin",     (numberOfMidiIn <= 0 ?   "True" : "False"));
    preferences_setKey ("nomidiout",    (numberOfMidiOut <= 0 ?  "True" : "False"));
    
    utils_snprintf (value, PD_STRING, "%d", sys_audioapi);  preferences_setKey ("audioapi",  value);
    utils_snprintf (value, PD_STRING, "%d", callback);      preferences_setKey ("callback",  value);
    utils_snprintf (value, PD_STRING, "%d", sampleRate);    preferences_setKey ("rate",      value);
    utils_snprintf (value, PD_STRING, "%d", advance);       preferences_setKey ("audiobuf",  value);
    utils_snprintf (value, PD_STRING, "%d", blockSize);     preferences_setKey ("blocksize", value);
    
    /* Search paths. */
    
    for (i = 0; 1; i++) {
    //
    char *path = pathlist_getFileAtIndex (sys_searchpath, i);
    if (!path) { break; }
    else {
        utils_snprintf (key, PD_STRING, "path%d", i + 1);
        preferences_setKey (key, path);
    }
    //
    }
    
    /* Audio devices. */
    
    for (i = 0; i < numberOfAudioIn; i++) {
    //
    utils_snprintf (key, PD_STRING, "audioindev%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d %d", audioIn[i], channelIn[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "audioindevname%d", i + 1);
    sys_audiodevnumbertoname (0, audioIn[i], value, PD_STRING);
    if (*value == 0) { utils_strnadd (value, PD_STRING, "?"); }
    preferences_setKey (key, value);
    //
    }

    for (i = 0; i < numberOfAudioOut; i++) {
    //
    utils_snprintf (key, PD_STRING, "audiooutdev%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d %d", audioOut[i], channelOut[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "audiooutdevname%d", i + 1);
    sys_audiodevnumbertoname (1, audioOut[i], value, PD_STRING);
    if (*value == 0) { utils_strnadd (value, PD_STRING, "?"); }
    preferences_setKey (key, value);
    //
    }

    /* MIDI devices. */
    
    for (i = 0; i < numberOfMidiIn; i++) {
    //
    utils_snprintf (key, PD_STRING, "midiindev%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d", midiIn[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "midiindevname%d", i + 1);
    sys_mididevnumbertoname (0, midiIn[i], value, PD_STRING);
    if (*value == 0) { utils_strnadd (value, PD_STRING, "?"); }
    preferences_setKey (key, value);
    //
    }

    for (i = 0; i < numberOfMidiOut; i++) {
    //
    utils_snprintf (key, PD_STRING, "midioutdev%d", i + 1);
    utils_snprintf (value, PD_STRING, "%d", midiOut[i]);
    preferences_setKey (key, value);
    utils_snprintf (key, PD_STRING, "midioutdevname%d", i + 1);
    sys_mididevnumbertoname (1, midiOut[i], value, PD_STRING);
    if (*value == 0) { utils_strnadd (value, PD_STRING, "?"); }
    preferences_setKey (key, value);
    //
    }

    preferences_saveClose();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
