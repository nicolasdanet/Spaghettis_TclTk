
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that a negative number of channels corresponds to a disabled device. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd global_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_deviceslist audio_devices;        /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void audio_vectorInitialize (t_float, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* The APIs provided detect devices only at startup thus it can be cached. */

static t_error audio_getDevicesList (t_deviceslist *l)
{
    static int cacheLoaded = 0;     /* Static. */
    static t_deviceslist cache;     /* Static. */
    
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void audio_getDevices (t_devices *p)
{
    deviceslist_getDevices (&audio_devices, p);
}

void audio_setDevices (t_devices *p)
{
    int m, n;
    
    deviceslist_setDevices (&audio_devices, p);
    
    m = deviceslist_getTotalOfChannelsIn (&audio_devices);
    n = deviceslist_getTotalOfChannelsOut (&audio_devices);
    
    audio_vectorInitialize (devices_getSampleRate (p), m, n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Check maximum device channels (if defined) before to open stream. */

t_error audio_check (t_devices *p)
{
    t_deviceslist l; t_error err = audio_getDevicesList (&l);
    
    devices_check (p);

    if (!err) {
    //
    int i;
    
    for (i = 0; i < devices_getInSize (p); i++) {
    //
    int m = devices_getInChannelsAtIndex (p, i);
    int n = deviceslist_getInChannelsAtIndex (&l, devices_getInAtIndex (p, i));
    if (n > 0 && m > n) {
        err = PD_ERROR; break;
    }
    //
    }
    //
    }
    
    if (!err) {
    //
    int i;
    
    for (i = 0; i < devices_getOutSize (p); i++) {
    //
    int m = devices_getOutChannelsAtIndex (p, i);
    int n = deviceslist_getOutChannelsAtIndex (&l, devices_getOutAtIndex (p, i));
    if (n > 0 && m > n) {
        err = PD_ERROR; break;
    }
    //
    }
    //
    }
    
    if (err) { error_mismatch (sym_audio, sym_channels); }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int audio_deviceAsNumber (int isOutput, t_symbol *name)
{
    t_deviceslist l;
    
    if (!audio_getDevicesList (&l)) {
        if (isOutput) { return deviceslist_containsOut (&l, name); }
        else { 
            return deviceslist_containsIn (&l, name);
        }
    }
    
    return -1;
}

t_error audio_deviceAsString (int isOutput, int k, char *dest, size_t size)
{
    t_error err = PD_ERROR;
    t_symbol *t = audio_deviceAsSymbol (isOutput, k);
    
    if (t) { err = string_copy (dest, size, t->s_name); }
    if (err) { *dest = 0; }
    
    return err;
}

t_symbol *audio_deviceAsSymbol (int isOutput, int k)
{
    t_deviceslist l;
    
    if (k >= 0 && !audio_getDevicesList (&l)) {
    //
    return isOutput ? deviceslist_getOutAtIndex (&l, k) : deviceslist_getInAtIndex (&l, k);
    //
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error audio_requireDialogInitialize (void)
{
    t_deviceslist l;
    
    t_error err = audio_getDevicesList (&l);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int i;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_audio::audioIn [list ");       // --
    err |= string_copy (t2, PD_STRING, "set ::ui_audio::audioOut [list ");      // --
    
    for (i = 0; i < deviceslist_getInSize (&l); i++) {
        t_symbol *t = deviceslist_getInAtIndex (&l, i);
        PD_ASSERT (t);
        err |= string_addSprintf (t1, PD_STRING, " {%s}", t->s_name);           // --
    }
    for (i = 0; i < deviceslist_getOutSize (&l); i++) {
        t_symbol *t = deviceslist_getOutAtIndex (&l, i);
        PD_ASSERT (t);
        err |= string_addSprintf (t2, PD_STRING, " {%s}", t->s_name);           // --
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    gui_add (t1);
    gui_add (t2);
    //
    }
    
    return err;
}

void audio_requireDialog (void)
{
    t_error err = audio_requireDialogInitialize();
    
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 4);
    
    if (!err) {
    //
    t_devices audio; devices_initAsAudio (&audio);
    
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
            "::ui_audio::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
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
            n[3]);
            
        if (!err) {
            stub_new (&global_class, (void *)audio_requireDialog, t);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void audio_fromDialog (int argc, t_atom *argv)
{
    t_devices audio; devices_initAsAudio (&audio);
    
    int i;
    
    PD_ASSERT (argc == 16);
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 4);
    
    for (i = 0; i < 4; i++) {
        int t = (int)atom_getFloatAtIndex (i + 4,  argc, argv);
        if (t != 0) {
            devices_appendAudioIn (&audio, (int)atom_getFloatAtIndex (i + 0,  argc, argv), t);
        }
    }
    
    for (i = 0; i < 4; i++) {
        int t = (int)atom_getFloatAtIndex (i + 12, argc, argv);
        if (t != 0) {
            devices_appendAudioOut (&audio, (int)atom_getFloatAtIndex (i + 8,  argc, argv), t);
        }
    }
    
    audio_close();
    audio_setDevices (&audio);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
