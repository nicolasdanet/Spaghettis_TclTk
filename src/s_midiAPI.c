
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
#pragma mark -

#define MIDI_SOMETHING      1               /* First item is always "none". */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd             global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_deviceslist    midi_devices;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if 0

static t_error midi_requireDialogInitialize (void)
{
    int  m = 0;
    int  n = 0;
    char i[DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION] = { 0 };
    char o[DEVICES_MAXIMUM_DEVICES * DEVICES_DESCRIPTION] = { 0 };
    
    t_error err = midi_getListsNative (i, &m, o, &n);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int k;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_midi::midiIn [list {none}");   // --
    err |= string_copy (t2, PD_STRING, "set ::ui_midi::midiOut [list {none}");  // --
    
    for (k = 0; k < m; k++) {
        err |= string_addSprintf (t1, PD_STRING, " {%s}", i + (k * DEVICES_DESCRIPTION));   // --
    }
    for (k = 0; k < n; k++) {
        err |= string_addSprintf (t2, PD_STRING, " {%s}", o + (k * DEVICES_DESCRIPTION));   // --
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    sys_gui (t1);
    sys_gui (t2);
    //
    }
    
    return err;
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
        devices_appendMidiIn (p, deviceslist_getInAtIndex (&midi_devices, i));
    }
        
    for (i = 0; i < deviceslist_getOutSize (&midi_devices); i++) {
        devices_appendMidiOut (p, deviceslist_getOutAtIndex (&midi_devices, i));
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
    t_deviceslist l; deviceslist_init (&l);
    
    if (!midi_getListsNative (&l)) {
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
    
    t_deviceslist l; deviceslist_init (&l);
    
    if (k >= 0 && !midi_getListsNative (&l)) {
        char *t = isOutput ? deviceslist_getOutAtIndex (&l, k) : deviceslist_getInAtIndex (&l, k);
        if (t) { err = string_copy (dest, size, t); }
    }
    
    if (err) { *dest = 0; }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_requireDialog (void *dummy)
{
    #if 0
    
    int m = 0;
    int n = 0;
    int i[DEVICES_MAXIMUM_IO]  = { 0 };
    int o[DEVICES_MAXIMUM_IO] = { 0 };
    
    t_error err = midi_requireDialogInitialize();
    
    midi_getDevices (&m, i, &n, o);
    
    if (!err) {
    //
    char t[PD_STRING] = { 0 };
    
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 8);
    
    int i1 = (m > 0 && i[0] >= 0 ? i[0] + MIDI_SOMETHING : 0);
    int i2 = (m > 1 && i[1] >= 0 ? i[1] + MIDI_SOMETHING : 0);
    int i3 = (m > 2 && i[2] >= 0 ? i[2] + MIDI_SOMETHING : 0);
    int i4 = (m > 3 && i[3] >= 0 ? i[3] + MIDI_SOMETHING : 0);
    int i5 = (m > 4 && i[4] >= 0 ? i[4] + MIDI_SOMETHING : 0);
    int i6 = (m > 5 && i[5] >= 0 ? i[5] + MIDI_SOMETHING : 0);
    int i7 = (m > 6 && i[6] >= 0 ? i[6] + MIDI_SOMETHING : 0);
    int i8 = (m > 7 && i[7] >= 0 ? i[7] + MIDI_SOMETHING : 0);
    
    PD_ASSERT (DEVICES_MAXIMUM_IO >= 8);
    
    int o1 = (n > 0 && o[0] >= 0 ? o[0] + MIDI_SOMETHING : 0);
    int o2 = (n > 1 && o[1] >= 0 ? o[1] + MIDI_SOMETHING : 0);
    int o3 = (n > 2 && o[2] >= 0 ? o[2] + MIDI_SOMETHING : 0);
    int o4 = (n > 3 && o[3] >= 0 ? o[3] + MIDI_SOMETHING : 0);
    int o5 = (n > 4 && o[4] >= 0 ? o[4] + MIDI_SOMETHING : 0);
    int o6 = (n > 5 && o[5] >= 0 ? o[5] + MIDI_SOMETHING : 0);
    int o7 = (n > 6 && o[6] >= 0 ? o[6] + MIDI_SOMETHING : 0);
    int o8 = (n > 7 && o[7] >= 0 ? o[7] + MIDI_SOMETHING : 0);

    err = string_sprintf (t, PD_STRING,
        "::ui_midi::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            i1,
            i2,
            i3,
            i4,
            i5,
            i6,
            i7,
            i8,
            o1,
            o2,
            o3,
            o4,
            o5,
            o6,
            o7,
            o8);
                
    if (!err) {
        guistub_new (&global_object, (void *)midi_requireDialog, t);
    }
    //
    }
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void midi_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    #if 0
    
    int m = 0;
    int n = 0;
    int i[DEVICES_MAXIMUM_IO]  = { 0 };
    int o[DEVICES_MAXIMUM_IO] = { 0 };

    int t = argc / 2;
    int k;

    PD_ASSERT (t == DEVICES_MAXIMUM_IO);
    PD_ASSERT (t == DEVICES_MAXIMUM_IO);
    
    for (k = 0; k < t; k++) {
        i[k] = (int)atom_getFloatAtIndex (k, argc, argv);
        o[k] = (int)atom_getFloatAtIndex (k + t, argc, argv);
    }

    for (k = 0; k < t; k++) { if (i[k] > 0) { i[m] = i[k] - MIDI_SOMETHING; m++; } }
    for (k = 0; k < t; k++) { if (o[k] > 0) { o[n] = o[k] - MIDI_SOMETHING; n++; } }

    //midi_close();
    //midi_setDevices (m, i, n, o);
    //midi_open();
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
