
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd             global_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_deviceslist    midi_devices;       /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define MIDI_SOMETHING  1                   /* First item is always "none". */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if 1

/* The APIs provided detect devices only at startup thus it can be cached. */

static t_error midi_getLists (t_deviceslist *l)
{
    static int cacheLoaded = 0;
    static t_deviceslist cache;
    
    t_error err = PD_ERROR_NONE;
    
    if (!cacheLoaded) {
    //
    deviceslist_init (&cache);
    err = midi_getListsNative (&cache);
    if (!err) { cacheLoaded = 1; }
    //
    }
    
    deviceslist_copy (l, &cache);
    
    return err;
}

#endif

#if 0

static t_error midi_getLists (t_deviceslist *l)
{
    deviceslist_init (l); return midi_getListsNative (l);
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_open (void)
{
    t_devicesproperties midi; devices_initAsMidi (&midi);
     
    midi_getDevices (&midi);
    midi_openNative (&midi);
}

void midi_close (void)
{
    midi_closeNative();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_getDevices (t_devicesproperties *p)
{
    deviceslist_getDevices (&midi_devices, p);
}

void midi_setDevices (t_devicesproperties *p)
{
    deviceslist_setDevices (&midi_devices, p);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int midi_deviceAsNumberWithString (int isOutput, char *name)
{
    t_deviceslist l;
    
    if (!midi_getLists (&l)) {
        if (isOutput) { return deviceslist_containsOut (&l, name); }
        else { 
            return deviceslist_containsIn (&l, name);
        }
    }
    
    return -1;
}

t_error midi_deviceAsStringWithNumber (int isOutput, int k, char *dest, size_t size)
{
    t_error err = PD_ERROR;
    
    t_deviceslist l;
    
    if (k >= 0 && !midi_getLists (&l)) {
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
// MARK: -

static t_error midi_requireDialogInitialize (void)
{
    t_deviceslist l;
    
    t_error err = midi_getLists (&l);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int i;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_midi::midiIn [list {none}");                           // --
    err |= string_copy (t2, PD_STRING, "set ::ui_midi::midiOut [list {none}");                          // --
    
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

void midi_requireDialog (void)
{
    t_error err = midi_requireDialogInitialize();
    
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 8);
    
    if (!err) {
    //
    t_devicesproperties midi; devices_initAsMidi (&midi);
    
    midi_getDevices (&midi);
    
    {
        char t[PD_STRING] = { 0 };
        
        int i[8] = { 0 };
        int o[8] = { 0 };
        int j;
        
        for (j = 0; j < 8; j++) {
            int k = devices_getInAtIndex (&midi, j);  i[j] = (k >= 0) ? k + MIDI_SOMETHING : 0;
        } 
        
        for (j = 0; j < 8; j++) {
            int k = devices_getOutAtIndex (&midi, j); o[j] = (k >= 0) ? k + MIDI_SOMETHING : 0;
        } 

        err = string_sprintf (t, PD_STRING,
            "::ui_midi::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                i[0],
                i[1],
                i[2],
                i[3],
                i[4],
                i[5],
                i[6],
                i[7],
                o[0],
                o[1],
                o[2],
                o[3],
                o[4],
                o[5],
                o[6],
                o[7]);
                    
        if (!err) {
            stub_new (&global_class, (void *)midi_requireDialog, t);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void midi_fromDialog (int argc, t_atom *argv)
{
    t_devicesproperties midi; devices_initAsMidi (&midi);
    
    int t = argc / 2;
    int j;

    PD_ASSERT (t == 8);
    
    for (j = 0; j < t; j++) {
    //
    int i = (int)atom_getFloatAtIndex (j, argc, argv);
    int o = (int)atom_getFloatAtIndex (j + t, argc, argv);
    
    if (i > 0) { devices_appendMidiInAsNumber (&midi,  i - MIDI_SOMETHING); }
    if (o > 0) { devices_appendMidiOutAsNumber (&midi, o - MIDI_SOMETHING); }
    //
    }

    midi_close();
    midi_setDevices (&midi);
    midi_open();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
