
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

static void preferences_loadInitialize (void)
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

static void sys_initsavepreferences( void)
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

static void sys_putpreference(const char *key, const char *value)
{
    if (sys_prefsavefp)
        fprintf(sys_prefsavefp, "%s: %s\n",
            key, value);
}

static void sys_donesavepreferences( void)
{
    if (sys_prefsavefp)
    {
        fclose(sys_prefsavefp);
        sys_prefsavefp = 0;
    }
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if PD_WINDOWS

static void preferences_loadInitialize (void)
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

static void sys_initsavepreferences( void)
{
}

static void sys_putpreference(const char *key, const char *value)
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

static void sys_donesavepreferences( void)
{
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

#if PD_APPLE

static void preferences_loadInitialize (void)
{
}

static void preferences_loadClose (void)
{
}

static int preferences_getKey(const char *key, char *value, int size)
{
    char cmdbuf[256];
    int nread = 0, nleft = size;
    char embedded_prefs[PD_STRING];
    char user_prefs[PD_STRING];
    char *homedir = getenv("HOME");
    struct stat statbuf;
   /* the 'defaults' command expects the filename without .plist at the
        end */
    snprintf(embedded_prefs, PD_STRING, "%s/../org.puredata.puredata",
        main_rootDirectory->s_name);
    snprintf(user_prefs, PD_STRING,
        "%s/Library/Preferences/org.puredata.puredata.plist", homedir);
    if (stat(user_prefs, &statbuf) == 0)
        snprintf(cmdbuf, 256, "defaults read org.puredata.puredata %s 2> /dev/null\n",
            key);
    else snprintf(cmdbuf, 256, "defaults read %s %s 2> /dev/null\n",
            embedded_prefs, key);
    FILE *fp = popen(cmdbuf, "r");
    while (nread < size)
    {
        int newread = fread(value+nread, 1, size-nread, fp);
        if (newread <= 0)
            break;
        nread += newread;
    }
    pclose(fp);
    if (nread < 1)
        return (0);
    if (nread >= size)
        nread = size-1;
    value[nread] = 0;
    if (value[nread-1] == '\n')     /* remove newline character at end */
        value[nread-1] = 0;
    return(1);
}

static void sys_initsavepreferences( void)
{
}

static void sys_putpreference(const char *key, const char *value)
{
    char cmdbuf[PD_STRING];
    snprintf(cmdbuf, PD_STRING, 
        "defaults write org.puredata.puredata %s \"%s\" 2> /dev/null\n", key, value);
    system(cmdbuf);
}

static void sys_donesavepreferences( void)
{
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark-

void preferences_load (void)
{
    int i;
    int numberOfPath                    = 0;
    int callback                        = 0;
    int sampleRate                      = AUDIO_DEFAULT_SAMPLING;
    int advance                         = AUDIO_DEFAULT_ADVANCE;
    int blockSize                       = AUDIO_DEFAULT_BLOCK;
    int audioApi                        = API_DEFAULT;
    
    int noAudioIn                       = 0;
    int noAudioOut                      = 0;
    int noMidiIn                        = 0;
    int noMidiOut                       = 0;
    
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

    preferences_loadInitialize();
    
    if (preferences_getKey ("npath",     value, PD_STRING)) { sscanf (value, "%d", &numberOfPath); }
    if (preferences_getKey ("callback",  value, PD_STRING)) { sscanf (value, "%d", &callback);     }
    if (preferences_getKey ("rate",      value, PD_STRING)) { sscanf (value, "%d", &sampleRate);   }
    if (preferences_getKey ("audiobuf",  value, PD_STRING)) { sscanf (value, "%d", &advance);      }
    if (preferences_getKey ("blocksize", value, PD_STRING)) { sscanf (value, "%d", &blockSize);    }
    if (preferences_getKey ("audioapi",  value, PD_STRING)) { sscanf (value, "%d", &audioApi);     }
    
    noAudioIn   = (preferences_getKey ("noaudioin",  value, PD_STRING) && !strcmp (value, "True"));
    noAudioOut  = (preferences_getKey ("noaudioout", value, PD_STRING) && !strcmp (value, "True"));
    noMidiIn    = (preferences_getKey ("nomidiin",   value, PD_STRING) && !strcmp (value, "True"));
    noMidiOut   = (preferences_getKey ("nomidiout",  value, PD_STRING) && !strcmp (value, "True"));
    
    for (i = 0; i < numberOfPath; i++) {
    //
    utils_snprintf (key, PD_STRING, "path%d", i + 1);
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
        sys_searchpath = pathlist_newAppend (sys_searchpath, value);
    }
    //
    }
    
    if (!noAudioIn) {
    //
    for (i = 0; i < AUDIO_MAXIMUM_IN; i++) {
    //
    utils_snprintf (key, PD_STRING, "audioindev%d", i + 1);
    
    if (!preferences_getKey (key, value, PD_STRING)) { break; }
    else {
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
    }
        
    if (numberOfAudioOut == 0) { numberOfAudioOut = -1; }
    //
    }
    
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
    int numberOfAudioIn;
    int audioindev[AUDIO_MAXIMUM_IN], chindev[AUDIO_MAXIMUM_IN];
    int numberOfAudioOut, audiooutdev[AUDIO_MAXIMUM_OUT], choutdev[AUDIO_MAXIMUM_OUT];
    int i, rate, advance, callback, blocksize;
    char buf1[PD_STRING], buf2[PD_STRING];
    int numberOfMidiIn, midiindev[MIDI_MAXIMUM_IN];
    int numberOfMidiOut, midioutdev[MIDI_MAXIMUM_OUT];

    sys_initsavepreferences();

        /* audio settings */
    sprintf(buf1, "%d", sys_audioapi);
    sys_putpreference("audioapi", buf1);

    sys_get_audio_params(&numberOfAudioIn, audioindev, chindev,
        &numberOfAudioOut, audiooutdev, choutdev, &rate, &advance, &callback,
            &blocksize);

    sys_putpreference("noaudioin", (numberOfAudioIn <= 0 ? "True" : "False"));
    for (i = 0; i < numberOfAudioIn; i++)
    {
        sprintf(buf1, "audioindev%d", i+1);
        sprintf(buf2, "%d %d", audioindev[i], chindev[i]);
        sys_putpreference(buf1, buf2);
        sprintf(buf1, "audioindevname%d", i+1);
        sys_audiodevnumbertoname(0, audioindev[i], buf2, PD_STRING);
        if (! *buf2)
            strcat(buf2, "?");
        sys_putpreference(buf1, buf2);
    }
    sys_putpreference("noaudioout", (numberOfAudioOut <= 0 ? "True" : "False"));
    for (i = 0; i < numberOfAudioOut; i++)
    {
        sprintf(buf1, "audiooutdev%d", i+1);
        sprintf(buf2, "%d %d", audiooutdev[i], choutdev[i]);
        sys_putpreference(buf1, buf2);
        sprintf(buf1, "audiooutdevname%d", i+1);
        sys_audiodevnumbertoname(1, audiooutdev[i], buf2, PD_STRING);
        if (! *buf2)
            strcat(buf2, "?");
        sys_putpreference(buf1, buf2);
   }

    sprintf(buf1, "%d", advance);
    sys_putpreference("audiobuf", buf1);

    sprintf(buf1, "%d", rate);
    sys_putpreference("rate", buf1);

    sprintf(buf1, "%d", callback);
    sys_putpreference("callback", buf1);

    sprintf(buf1, "%d", blocksize);
    sys_putpreference("blocksize", buf1);

        /* MIDI settings */
    sys_get_midi_params(&numberOfMidiIn, midiindev, &numberOfMidiOut, midioutdev);
    sys_putpreference("nomidiin", (numberOfMidiIn <= 0 ? "True" : "False"));
    for (i = 0; i < numberOfMidiIn; i++)
    {
        sprintf(buf1, "midiindev%d", i+1);
        sprintf(buf2, "%d", midiindev[i]);
        sys_putpreference(buf1, buf2);
        sprintf(buf1, "midiindevname%d", i+1);
        sys_mididevnumbertoname(0, midiindev[i], buf2, PD_STRING);
        if (! *buf2)
            strcat(buf2, "?");
        sys_putpreference(buf1, buf2);
    }
    sys_putpreference("nomidiout", (numberOfMidiOut <= 0 ? "True" : "False"));
    for (i = 0; i < numberOfMidiOut; i++)
    {
        sprintf(buf1, "midioutdev%d", i+1);
        sprintf(buf2, "%d", midioutdev[i]);
        sys_putpreference(buf1, buf2);
        sprintf(buf1, "midioutdevname%d", i+1);
        sys_mididevnumbertoname(1, midioutdev[i], buf2, PD_STRING);
        if (! *buf2)
            strcat(buf2, "?");
        sys_putpreference(buf1, buf2);
    }
        /* file search path */

    for (i = 0; 1; i++)
    {
        char *pathelem = pathlist_getFileAtIndex(sys_searchpath, i);
        if (!pathelem)
            break;
        sprintf(buf1, "path%d", i+1);
        sys_putpreference(buf1, pathelem);
    }
    sprintf(buf1, "%d", i);
    sys_putpreference("npath", buf1);
    //sprintf(buf1, "%d", sys_usestdpath);
    //sys_putpreference("standardpath", buf1);
    //sprintf(buf1, "%d", sys_verbose);
    //sys_putpreference("verbose", buf1);
    
        /* startup */
    /*for (i = 0; 1; i++)
    {
        char *pathelem = pathlist_getFileAtIndex(sys_externlist, i);
        if (!pathelem)
            break;
        sprintf(buf1, "loadlib%d", i+1);
        sys_putpreference(buf1, pathelem);
    }*/
    //sprintf(buf1, "%d", i);
    //sys_putpreference("nloadlib", buf1);
    //sprintf(buf1, "%d", sys_defeatrt);
    //sys_putpreference("defeatrt", buf1);
    //sys_putpreference("flags", 
      //  (sys_flags ? sys_flags->s_name : ""));
    sys_donesavepreferences();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
