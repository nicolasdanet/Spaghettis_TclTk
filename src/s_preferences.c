/* Copyright (c) 1997-2004 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*
 * this file implements a mechanism for storing and retrieving preferences.
 * Should later be renamed "preferences.c" or something.
 *
 * In unix this is handled by the "~/.puredata" file, in windows by
 * the registry, and in MacOS by the Preferences system.
 */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifdef _WIN32
    #include <windows.h>
    #include <tchar.h>
#else
    #include <sys/stat.h>
#endif

#ifdef _MSC_VER  /* This is only for Microsoft's compiler, not cygwin, e.g. */
#define snprintf sprintf_s
#endif

extern int sys_usestdpath;
extern t_pathlist *sys_searchpath;
extern t_symbol *main_libDirectory;
extern int sys_audioapi;

//t_symbol *sys_flags = &s_;      /* Shared. */
//void sys_doflags( void);

    /* Hmm... maybe better would be to #if on not-apple-or-windows  */
#if defined(__linux__) || defined(__CYGWIN__) || defined(__FreeBSD_kernel__) \
|| defined(__GNU__) || defined(ANDROID)

/*****  linux/android/BSD etc: read and write to ~/.puredata file ******/

static char *sys_prefbuf;
static int sys_prefbufsize;

static void sys_initloadpreferences( void)
{
    char filenamebuf[PD_STRING], *homedir = getenv("HOME");
    int fd, length;
    char user_prefs_file[PD_STRING]; /* user prefs file */
        /* default prefs embedded in the package */
    char default_prefs_file[PD_STRING];
    struct stat statbuf;

    snprintf(default_prefs_file, PD_STRING, "%s/default.puredata", 
        main_libDirectory->s_name);
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

static int sys_getpreference(const char *key, char *value, int size)
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

static void sys_doneloadpreferences( void)
{
    if (sys_prefbuf)
        free(sys_prefbuf);
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

#endif /* __linux__ || __CYGWIN__ || __FreeBSD_kernel__ || __GNU__ */

#ifdef _WIN32

static void sys_initloadpreferences( void)
{
}

static int sys_getpreference(const char *key, char *value, int size)
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

static void sys_doneloadpreferences( void)
{
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

#endif /* _WIN32 */

#ifdef __APPLE__

static void sys_initloadpreferences( void)
{
}

static int sys_getpreference(const char *key, char *value, int size)
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
        main_libDirectory->s_name);
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

static void sys_doneloadpreferences( void)
{
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

#endif /* __APPLE__ */


void sys_loadpreferences( void)
{
    int naudioindev, audioindev[AUDIO_MAXIMUM_IN], chindev[AUDIO_MAXIMUM_IN];
    int naudiooutdev, audiooutdev[AUDIO_MAXIMUM_OUT], choutdev[AUDIO_MAXIMUM_OUT];
    int nmidiindev, midiindev[MIDI_MAXIMUM_IN];
    int nmidioutdev, midioutdev[MIDI_MAXIMUM_OUT];
    int i, rate = 0, advance = -1, callback = 0, blocksize = 0,
        api, nolib, maxi;
    char prefbuf[PD_STRING], keybuf[80];

    sys_initloadpreferences();
        /* load audio preferences */
    if (sys_getpreference("audioapi", prefbuf, PD_STRING)
        && sscanf(prefbuf, "%d", &api) > 0)
            sys_set_audio_api(api);
            /* JMZ/MB: brackets for initializing */
    if (sys_getpreference("noaudioin", prefbuf, PD_STRING) &&
        (!strcmp(prefbuf, ".") || !strcmp(prefbuf, "True")))
            naudioindev = 0;
    else
    {
        for (i = 0, naudioindev = 0; i < AUDIO_MAXIMUM_IN; i++)
        {
                /* first try to find a name - if that matches an existing
                device use it.  Otherwise fall back to device number. */
            int devn;
                /* read in device number and channel count */
            sprintf(keybuf, "audioindev%d", i+1);
            if (!sys_getpreference(keybuf, prefbuf, PD_STRING))
                break;
            if (sscanf(prefbuf, "%d %d", &audioindev[i], &chindev[i]) < 2)
                break;
                /* possibly override device number if the device name was
                also saved and if it matches one we have now */        
            sprintf(keybuf, "audioindevname%d", i+1);
            if (sys_getpreference(keybuf, prefbuf, PD_STRING)
                && (devn = sys_audiodevnametonumber(0, prefbuf)) >= 0)
                    audioindev[i] = devn;
            naudioindev++;
        }
            /* if no preferences at all, set -1 for default behavior */
        if (naudioindev == 0)
            naudioindev = -1;
    }
        /* JMZ/MB: brackets for initializing */
    if (sys_getpreference("noaudioout", prefbuf, PD_STRING) &&
        (!strcmp(prefbuf, ".") || !strcmp(prefbuf, "True")))
            naudiooutdev = 0;
    else
    {
        for (i = 0, naudiooutdev = 0; i < AUDIO_MAXIMUM_OUT; i++)
        {
            int devn;
            sprintf(keybuf, "audiooutdev%d", i+1);
            if (!sys_getpreference(keybuf, prefbuf, PD_STRING))
                break;
            if (sscanf(prefbuf, "%d %d", &audiooutdev[i], &choutdev[i]) < 2)
                break;
            sprintf(keybuf, "audiooutdevname%d", i+1);
            if (sys_getpreference(keybuf, prefbuf, PD_STRING)
                && (devn = sys_audiodevnametonumber(1, prefbuf)) >= 0)
                    audiooutdev[i] = devn;
            naudiooutdev++;
        }
        if (naudiooutdev == 0)
            naudiooutdev = -1;
    }
    if (sys_getpreference("rate", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &rate);
    if (sys_getpreference("audiobuf", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &advance);
    if (sys_getpreference("callback", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &callback);
    if (sys_getpreference("blocksize", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &blocksize);
    sys_set_audio_settings(naudioindev, audioindev, naudioindev, chindev,
        naudiooutdev, audiooutdev, naudiooutdev, choutdev, rate, advance,
        callback, blocksize);
        
        /* load MIDI preferences */
        /* JMZ/MB: brackets for initializing */
    if (sys_getpreference("nomidiin", prefbuf, PD_STRING) &&
        (!strcmp(prefbuf, ".") || !strcmp(prefbuf, "True")))
            nmidiindev = 0;
    else for (i = 0, nmidiindev = 0; i < MIDI_MAXIMUM_IN; i++)
    {
            /* first try to find a name - if that matches an existing device
            use it.  Otherwise fall back to device number. */
        int devn;
        sprintf(keybuf, "midiindevname%d", i+1);
        if (sys_getpreference(keybuf, prefbuf, PD_STRING)
            && (devn = sys_mididevnametonumber(0, prefbuf)) >= 0)
                midiindev[i] = devn;
        else
        {
            sprintf(keybuf, "midiindev%d", i+1);
            if (!sys_getpreference(keybuf, prefbuf, PD_STRING))
                break;
            if (sscanf(prefbuf, "%d", &midiindev[i]) < 1)
                break;
        }
        nmidiindev++;
    }
        /* JMZ/MB: brackets for initializing */
    if (sys_getpreference("nomidiout", prefbuf, PD_STRING) &&
        (!strcmp(prefbuf, ".") || !strcmp(prefbuf, "True")))
            nmidioutdev = 0;
    else for (i = 0, nmidioutdev = 0; i < MIDI_MAXIMUM_OUT; i++)
    {
        int devn;
        sprintf(keybuf, "midioutdevname%d", i+1);
        if (sys_getpreference(keybuf, prefbuf, PD_STRING)
            && (devn = sys_mididevnametonumber(1, prefbuf)) >= 0)
                midioutdev[i] = devn;
        else
        {
            sprintf(keybuf, "midioutdev%d", i+1);
            if (!sys_getpreference(keybuf, prefbuf, PD_STRING))
                break;
            if (sscanf(prefbuf, "%d", &midioutdev[i]) < 1)
                break;
        }
        nmidioutdev++;
    }
    sys_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev, 0);

        /* search path */
    if (sys_getpreference("npath", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &maxi);
    else maxi = 0x7fffffff;
    for (i = 0; i<maxi; i++)
    {
        sprintf(keybuf, "path%d", i+1);
        if (!sys_getpreference(keybuf, prefbuf, PD_STRING))
            break;
        sys_searchpath = pathlist_newAppendFiles(sys_searchpath, prefbuf, PATHLIST_SEPARATOR);
    }
    if (sys_getpreference("standardpath", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &sys_usestdpath);
        
    /*if (sys_getpreference("verbose", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &sys_verbose); */

    /*
    if (sys_getpreference("nloadlib", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &maxi);
    else maxi = 0x7fffffff;
    for (i = 0; i<maxi; i++)
    {
        sprintf(keybuf, "loadlib%d", i+1);
        if (!sys_getpreference(keybuf, prefbuf, PD_STRING))
            break;
        sys_externlist = pathlist_newAppendFiles(sys_externlist, prefbuf, PATHLIST_SEPARATOR);
    }
    */
    /* if (sys_getpreference("defeatrt", prefbuf, PD_STRING))
        sscanf(prefbuf, "%d", &sys_defeatrt);*/
    /*if (sys_getpreference("flags", prefbuf, PD_STRING))
    {
        if (strcmp(prefbuf, "."))
            sys_flags = gensym(prefbuf);
    }*/
    //sys_doflags();
}

void global_savePreferences(void *dummy)
{
    int naudioindev, audioindev[AUDIO_MAXIMUM_IN], chindev[AUDIO_MAXIMUM_IN];
    int naudiooutdev, audiooutdev[AUDIO_MAXIMUM_OUT], choutdev[AUDIO_MAXIMUM_OUT];
    int i, rate, advance, callback, blocksize;
    char buf1[PD_STRING], buf2[PD_STRING];
    int nmidiindev, midiindev[MIDI_MAXIMUM_IN];
    int nmidioutdev, midioutdev[MIDI_MAXIMUM_OUT];

    sys_initsavepreferences();


        /* audio settings */
    sprintf(buf1, "%d", sys_audioapi);
    sys_putpreference("audioapi", buf1);

    sys_get_audio_params(&naudioindev, audioindev, chindev,
        &naudiooutdev, audiooutdev, choutdev, &rate, &advance, &callback,
            &blocksize);

    sys_putpreference("noaudioin", (naudioindev <= 0 ? "True" : "False"));
    for (i = 0; i < naudioindev; i++)
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
    sys_putpreference("noaudioout", (naudiooutdev <= 0 ? "True" : "False"));
    for (i = 0; i < naudiooutdev; i++)
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
    sys_get_midi_params(&nmidiindev, midiindev, &nmidioutdev, midioutdev);
    sys_putpreference("nomidiin", (nmidiindev <= 0 ? "True" : "False"));
    for (i = 0; i < nmidiindev; i++)
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
    sys_putpreference("nomidiout", (nmidioutdev <= 0 ? "True" : "False"));
    for (i = 0; i < nmidioutdev; i++)
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
    sprintf(buf1, "%d", sys_usestdpath);
    sys_putpreference("standardpath", buf1);
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
