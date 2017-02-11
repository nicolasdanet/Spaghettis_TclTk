
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

extern t_pd             global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_deviceslist    midi_devices;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define MIDI_SOMETHING  1                   /* First item is always "none". */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

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
#pragma mark -

void midi_getDevices (t_devicesproperties *p)
{
    int i;
    
    for (i = 0; i < deviceslist_getInSize (&midi_devices); i++) {
        devices_appendMidiIn (p, deviceslist_getInAtIndexAsString (&midi_devices, i));
    }
        
    for (i = 0; i < deviceslist_getOutSize (&midi_devices); i++) {
        devices_appendMidiOut (p, deviceslist_getOutAtIndexAsString (&midi_devices, i));
    }
}

void midi_setDevices (t_devicesproperties *p)
{
    int i;
    
    deviceslist_init (&midi_devices);
    
    for (i = 0; i < devices_getInSize (p); i++) {
        deviceslist_appendMidiInAsNumber (&midi_devices, devices_getInAtIndex (p, i));
    }
    
    for (i = 0; i < devices_getOutSize (p); i++) {
        deviceslist_appendMidiOutAsNumber (&midi_devices, devices_getOutAtIndex (p, i));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

static t_error midi_requireDialogInitialize (void)
{
    t_deviceslist l;
    
    t_error err = midi_getLists (&l);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int k;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_midi::midiIn [list {none}");                           // --
    err |= string_copy (t2, PD_STRING, "set ::ui_midi::midiOut [list {none}");                          // --
    
    for (k = 0; k < deviceslist_getInSize (&l); k++) {
        err |= string_addSprintf (t1, PD_STRING, " {%s}", deviceslist_getInAtIndexAsString (&l, k));    // --
    }
    for (k = 0; k < deviceslist_getOutSize (&l); k++) {
        err |= string_addSprintf (t2, PD_STRING, " {%s}", deviceslist_getOutAtIndexAsString (&l, k));   // --
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    sys_gui (t1);
    sys_gui (t2);
    //
    }
    
    return err;
}

void midi_requireDialog (void *dummy)
{
    t_error err = midi_requireDialogInitialize();
    
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 8);
    
    if (!err) {
    //
    t_devicesproperties midi; devices_initAsMidi (&midi);
    
    midi_getDevices (&midi);
    
    {
        char t[PD_STRING] = { 0 };
        
        int i[8];
        int o[8];
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
            guistub_new (&global_object, (void *)midi_requireDialog, t);
        }
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
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
