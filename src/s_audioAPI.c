
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

/* Note that a negative number of channels corresponds to a disabled device. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_deviceslist   audio_devices;              /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      audio_state;                /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if 1

/* The APIs provided detect devices only at startup thus it can be cached. */

static t_error audio_getLists (t_deviceslist *l)
{
    static int cacheLoaded = 0;
    static t_deviceslist cache;
    
    t_error err = PD_ERROR_NONE;
    
    if (!cacheLoaded) {
    //
    deviceslist_init (&cache);
    err = audio_getListsNative (&cache);
    if (!err) { cacheLoaded = 1; }
    //
    }
    
    deviceslist_copy (l, &cache);
    
    return err;
}

#endif

#if 0

static t_error audio_getLists (t_deviceslist *l)
{
    deviceslist_init (l); return audio_getListsNative (l);
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error audio_open (void)
{
    t_error err = PD_ERROR;
    
    t_devicesproperties audio; devices_initAsAudio (&audio);
    
    audio_getDevices (&audio);
    
    if (devices_getInSize (&audio) || devices_getOutSize (&audio)) {
    
        devices_checkDisabled (&audio);
        
        audio_initializeMemory (devices_getSampleRate (&audio),
            audio_getChannelsIn(),
            audio_getChannelsOut());
        
        err = audio_openNative (&audio); 
    }
    
    if (err) {
        error_canNotOpen (sym_audio);
        audio_state = 0;
        scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
        
    } else {
        audio_state = 1;
        scheduler_setAudioMode (SCHEDULER_AUDIO_POLL);
    }
    
    return err;
}

void audio_close (void)
{
    if (audio_isOpened()) { audio_closeNative(); }
    audio_state = 0;
    scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_isOpened (void)
{
    return (audio_state != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_getDevices (t_devicesproperties *p)
{
    deviceslist_getDevices (&audio_devices, p);
}

void audio_setDevices (t_devicesproperties *p)
{
    int m, n;
    
    deviceslist_setDevices (&audio_devices, p);
    
    m = deviceslist_getTotalOfChannelsIn (&audio_devices);
    n = deviceslist_getTotalOfChannelsOut (&audio_devices);
    
    audio_initializeMemory (devices_getSampleRate (p), m, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int audio_deviceAsNumberWithString (int isOutput, char *name)
{
    t_deviceslist l;
    
    if (!audio_getLists (&l)) {
        if (isOutput) { return deviceslist_containsOut (&l, name); }
        else { 
            return deviceslist_containsIn (&l, name);
        }
    }
    
    return -1;
}

t_error audio_deviceAsStringWithNumber (int isOutput, int k, char *dest, size_t size)
{
    t_error err = PD_ERROR;
    
    t_deviceslist l;
    
    if (k >= 0 && !audio_getLists (&l)) {
    //
    char *t = isOutput ? deviceslist_getOutAtIndexAsString (&l, k) : deviceslist_getInAtIndexAsString (&l, k);
    if (t) { err = string_copy (dest, size, t); }
    //
    }
    
    if (err) { *dest = 0; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error audio_requireDialogInitialize (void)
{
    t_deviceslist l;
    
    t_error err = audio_getLists (&l);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int i;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_audio::audioIn [list ");                               // --
    err |= string_copy (t2, PD_STRING, "set ::ui_audio::audioOut [list ");                              // --
    
    for (i = 0; i < deviceslist_getInSize (&l); i++) {
        err |= string_addSprintf (t1, PD_STRING, " {%s}", deviceslist_getInAtIndexAsString (&l, i));    // --
    }
    for (i = 0; i < deviceslist_getOutSize (&l); i++) {
        err |= string_addSprintf (t2, PD_STRING, " {%s}", deviceslist_getOutAtIndexAsString (&l, i));   // --
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    sys_gui (t1);
    sys_gui (t2);
    //
    }
    
    return err;
}

void audio_requireDialog (void *dummy)
{
    t_error err = audio_requireDialogInitialize();
    
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 4);
    
    if (!err) {
    //
    t_devicesproperties audio; devices_initAsAudio (&audio);
    
    audio_getDevices (&audio);
    
    {
        char t[PD_STRING] = { 0 };
        
        int i[4] = { 0 };
        int m[4] = { 0 };
        int o[4] = { 0 };
        int n[4] = { 0 };
        int j;
        int k;
        
        for (j = 0; j < 4; j++) {
            k = devices_getInAtIndex (&audio, j); 
            i[j] = (k >= 0) ? k : 0;
            m[j] = (k >= 0) ? devices_getInChannelsAtIndex (&audio, j) : 0;
        } 
        
        for (j = 0; j < 4; j++) {
            k = devices_getOutAtIndex (&audio, j);
            o[j] = (k >= 0) ? k : 0;
            n[j] = (k >= 0) ? devices_getOutChannelsAtIndex (&audio, j) : 0;
        } 
        
        err |= string_sprintf (t, PD_STRING,
            "::ui_audio::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            i[0],
            i[1],
            i[2],
            i[3], 
            m[0],
            m[1],
            m[2],
            m[3], 
            o[0],
            o[1],
            o[2],
            o[3],
            n[0],
            n[1],
            n[2],
            n[3], 
            devices_getSampleRate (&audio),
            devices_getBlockSize (&audio));
            
        if (!err) {
            guistub_new (&global_object, (void *)audio_requireDialog, t);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void audio_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    t_devicesproperties audio; devices_initAsAudio (&audio);
    
    int i;
    
    PD_ASSERT (argc == 18);
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 4);
    
    for (i = 0; i < 4; i++) {
        int t = (int)atom_getFloatAtIndex (i + 4,  argc, argv);
        if (t != 0) {
            devices_appendAudioInAsNumber (&audio, (int)atom_getFloatAtIndex (i + 0,  argc, argv), t);
        }
    }
    
    for (i = 0; i < 4; i++) {
        int t = (int)atom_getFloatAtIndex (i + 12, argc, argv);
        if (t != 0) {
            devices_appendAudioOutAsNumber (&audio, (int)atom_getFloatAtIndex (i + 8,  argc, argv), t);
        }
    }
    
    devices_setSampleRate (&audio, (int)atom_getFloatAtIndex (16, argc, argv));
    devices_setBlockSize (&audio,  (int)atom_getFloatAtIndex (17, argc, argv));
    
    audio_close();
    audio_setDevices (&audio);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
