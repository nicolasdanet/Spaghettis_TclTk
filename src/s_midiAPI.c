
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

extern t_class *global_object;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int midi_api = API_DEFAULT_MIDI;                                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  midi_numberOfDevicesIn;                                             /* Shared. */
static int  midi_numberOfDevicesOut;                                            /* Shared. */
static int  midi_devicesIn[MAXIMUM_MIDI_IN];                                    /* Shared. */
static int  midi_devicesOut[MAXIMUM_MIDI_OUT];                                  /* Shared. */
static char midi_devicesInNames[MAXIMUM_MIDI_IN * MAXIMUM_DESCRIPTION];         /* Shared. */
static char midi_devicesOutNames[MAXIMUM_MIDI_OUT * MAXIMUM_DESCRIPTION];       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_setAPI (void *dummy, t_float f)
{
    if ((int)f != midi_api) {
        if (API_WITH_ALSA && midi_api == API_ALSA) { sys_alsa_close_midi(); }
        else {
            sys_close_midi();
        }
        
        midi_api = (int)f; midi_openAgain();
    }
    
    #ifdef USEAPI_ALSA
        midi_alsa_setndevs (midi_numberOfDevicesIn, midi_numberOfDevicesOut);
    #endif
}

t_error midi_getAPIAvailables (char *dest, size_t size)
{
    t_error err = PD_ERROR_NONE;

    #if defined ( USEAPI_OSS ) && defined ( USEAPI_ALSA )
    
    err |= string_sprintf (dest, size, "{ {OSS-MIDI %d} {ALSA-MIDI %d} }", API_DEFAULT_MIDI, API_ALSA);
    
    #else
    
    err |= string_copy (dest, size, "{}");
    
    #endif
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_open (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut, int enable)
{
    if (enable) {
    //
    #ifdef USEAPI_ALSA
        midi_initializeALSA();
    #endif
    
    #ifdef USEAPI_OSS
        midi_initializeOSS();
    #endif
    
    if (API_WITH_ALSA && midi_api == API_ALSA) {
        sys_alsa_do_open_midi (numberOfDevicesIn, devicesIn, numberOfDevicesOut, devicesOut);
    } else {
        sys_do_open_midi (numberOfDevicesIn, devicesIn, numberOfDevicesOut, devicesOut);
    }
    //
    }
    
    midi_setDevices (numberOfDevicesIn, devicesIn, numberOfDevicesOut, devicesOut);

    sys_vGui ("set ::var(apiMidi) %d\n", midi_api);
}

void midi_openAgain (void)
{
    int m, n;
    int i[MAXIMUM_MIDI_IN]  = { 0 };
    int o[MAXIMUM_MIDI_OUT] = { 0 };
    
    midi_getDevices (&m, i, &n, o); midi_open (m, i, n, o, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_getDevices (int *numberOfDevicesIn, int *devicesIn, int *numberOfDevicesOut, int *devicesOut)
{
    int i, n;
    
    *numberOfDevicesIn  = midi_numberOfDevicesIn;
    *numberOfDevicesOut = midi_numberOfDevicesOut;
    
    for (i = 0; i < midi_numberOfDevicesIn; i++) {
        char *s = &midi_devicesInNames[i * MAXIMUM_DESCRIPTION];
        if ((n = midi_inNumberWithName (s)) >= 0) {
            devicesIn[i] = n;
        } else {
            devicesIn[i] = midi_devicesIn[i]; 
        }
    }
        
    for (i = 0; i < midi_numberOfDevicesOut; i++) {
        char *s = &midi_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        if ((n = midi_outNumberWithName (s)) >= 0) {
            devicesOut[i] = n;
        } else {
            devicesOut[i] = midi_devicesOut[i];
        }
    }
}

void midi_setDevices (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut)
{
    int i;
    
    PD_ASSERT (numberOfDevicesIn <= MAXIMUM_MIDI_IN);
    PD_ASSERT (numberOfDevicesOut <= MAXIMUM_MIDI_OUT);
    
    midi_numberOfDevicesIn  = numberOfDevicesIn;
    midi_numberOfDevicesOut = numberOfDevicesOut;
    
    for (i = 0; i < numberOfDevicesIn; i++) {
        char *s = &midi_devicesInNames[i * MAXIMUM_DESCRIPTION];
        midi_devicesIn[i] = devicesIn[i];
        midi_inNumberToName (devicesIn[i], s, MAXIMUM_DESCRIPTION);
    }
    
    for (i = 0; i < numberOfDevicesOut; i++) {
        char *s = &midi_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        midi_devicesOut[i] = devicesOut[i]; 
        midi_outNumberToName (devicesOut[i], s, MAXIMUM_DESCRIPTION);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void midi_getLists (char *i, int *m, char *o, int *n)
{
    if (API_WITH_ALSA && midi_api == API_ALSA) { midi_alsa_getdevs (i, m, o, n); } 
    else {
        midi_getdevs (i, m, o, n);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int midi_numberWithName (int isOutput, const char *name)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };

    int k;
    
    midi_getLists (i, &m, o, &n);

    if (isOutput) {
        for (k = 0; k < n; k++) { 
            if (!strcmp (name, o + (k * MAXIMUM_DESCRIPTION))) { return k; }
        }
    } else {
        for (k = 0; k < m; k++) {
            if (!strcmp (name, i + (k * MAXIMUM_DESCRIPTION))) { return k; }
        }
    }
    
    return -1;
}

static void midi_numberToName (int isOutput, int k, char *dest, size_t size)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    
    t_error err = PD_ERROR;
    
    if (k >= 0) { 
    //
    midi_getLists (i, &m, o, &n);
    
    if (isOutput && (k < n))        { err = string_copy (dest, size, o + (k * MAXIMUM_DESCRIPTION)); }
    else if (!isOutput && (k < m))  { err = string_copy (dest, size, i + (k * MAXIMUM_DESCRIPTION)); }
    //
    }
    
    if (err) { *dest = 0; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int midi_inNumberWithName (const char *name)
{
    return midi_numberWithName (0, name);
}

int midi_outNumberWithName (const char *name)
{
    return midi_numberWithName (1, name);
}

void midi_inNumberToName (int n, char *dest, size_t size)
{
    midi_numberToName (0, n, dest, size);
}

void midi_outNumberToName (int n, char *dest, size_t size)
{
    midi_numberToName (1, n, dest, size);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void midi_requireDialogInitialize (void)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION] = { 0 };
    
    int k;

    midi_getLists (i, &m, o, &n);

    sys_gui("set ::ui_midi::midiIn {none}\n");
    sys_gui("set ::ui_midi::midiOut {none}\n");
    
    for (k = 0; k < m; k++) { sys_vGui ("lappend ::ui_midi::midiIn {%s}\n",  i + (k * MAXIMUM_DESCRIPTION)); }
    for (k = 0; k < n; k++) { sys_vGui ("lappend ::ui_midi::midiOut {%s}\n", o + (k * MAXIMUM_DESCRIPTION)); }
}

void midi_requireDialog (void *dummy)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_MIDI_IN]  = { 0 };
    int o[MAXIMUM_MIDI_OUT] = { 0 };
    
    midi_requireDialogInitialize();
    
    midi_getDevices (&m, i, &n, o);
    
    {
        t_error err = PD_ERROR_NONE;
        
        char t[PD_STRING] = { 0 };
            
        int i1 = (m > 0 && i[0]>= 0 ? i[0] + 1 : 0);
        int i2 = (m > 1 && i[1]>= 0 ? i[1] + 1 : 0);
        int i3 = (m > 2 && i[2]>= 0 ? i[2] + 1 : 0);
        int i4 = (m > 3 && i[3]>= 0 ? i[3] + 1 : 0);
        int i5 = (m > 4 && i[4]>= 0 ? i[4] + 1 : 0);
        int i6 = (m > 5 && i[5]>= 0 ? i[5] + 1 : 0);
        int i7 = (m > 6 && i[6]>= 0 ? i[6] + 1 : 0);
        int i8 = (m > 7 && i[7]>= 0 ? i[7] + 1 : 0);
        int i9 = (m > 8 && i[8]>= 0 ? i[8] + 1 : 0);
        
        int o1 = (n > 0 && o[0]>= 0 ? o[0] + 1 : 0); 
        int o2 = (n > 1 && o[1]>= 0 ? o[1] + 1 : 0); 
        int o3 = (n > 2 && o[2]>= 0 ? o[2] + 1 : 0); 
        int o4 = (n > 3 && o[3]>= 0 ? o[3] + 1 : 0); 
        int o5 = (n > 4 && o[4]>= 0 ? o[4] + 1 : 0);
        int o6 = (n > 5 && o[5]>= 0 ? o[5] + 1 : 0);
        int o7 = (n > 6 && o[6]>= 0 ? o[6] + 1 : 0);
        int o8 = (n > 7 && o[7]>= 0 ? o[7] + 1 : 0);
        int o9 = (n > 8 && o[8]>= 0 ? o[8] + 1 : 0);

        if (API_WITH_ALSA && midi_api == API_ALSA) {

            err = string_sprintf (t, PD_STRING, "::ui_midi::show %%s \
                %d %d %d %d 0 0 0 0 0 \
                %d %d %d %d 0 0 0 0 0 \
                1\n",
                i1, i2, i3, i4, 
                o1, o2, o3, o4);

        } else {
        
            err = string_sprintf (t, PD_STRING, "::ui_midi::show %%s \
                %d %d %d %d %d %d %d %d %d \
                %d %d %d %d %d %d %d %d %d \
                0\n",
                i1, i2, i3, i4, 
                i5, i6, i7, i8, i9, 
                o1, o2, o3, o4, 
                o5, o6, o7, o8, o9);
                                        
        }
        
        if (!err) {
            gfxstub_deleteforkey (NULL);
            gfxstub_new (&global_object, (void *)midi_requireDialog, t);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int nmidiindev, midiindev[MAXIMUM_MIDI_IN];
    int nmidioutdev, midioutdev[MAXIMUM_MIDI_OUT];
    int i, nindev, noutdev;
    int newmidiindev[9], newmidioutdev[9];
    int alsadevin, alsadevout;

    for (i = 0; i < 9; i++)
    {
        newmidiindev[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
        newmidioutdev[i] = (t_int)atom_getFloatAtIndex(i+9, argc, argv);
    }

    for (i = 0, nindev = 0; i < 9; i++)
    {
        if (newmidiindev[i] > 0)
        {
            newmidiindev[nindev] = newmidiindev[i]-1;
            nindev++;
        }
    }
    for (i = 0, noutdev = 0; i < 9; i++)
    {
        if (newmidioutdev[i] > 0)
        {
            newmidioutdev[noutdev] = newmidioutdev[i]-1;
            noutdev++;
        }
    }
    alsadevin = (t_int)atom_getFloatAtIndex(18, argc, argv);
    alsadevout = (t_int)atom_getFloatAtIndex(19, argc, argv);
#ifdef USEAPI_ALSA
            /* invent a story so that saving/recalling "settings" will
            be able to restore the number of devices.  ALSA MIDI handling
            uses its own set of variables.  LATER figure out how to get
            this to work coherently */
    if (midi_api == API_ALSA)
    {
        nindev = alsadevin;
        noutdev = alsadevout;
        for (i = 0; i < nindev; i++)
            newmidiindev[i] = i;
        for (i = 0; i < noutdev; i++)
            newmidioutdev[i] = i;
    }
#endif
    midi_setDevices(nindev, newmidiindev,
        noutdev, newmidioutdev);
#ifdef USEAPI_ALSA
    if (midi_api == API_ALSA)
    {
        sys_alsa_close_midi();
        midi_open(alsadevin, newmidiindev, alsadevout, newmidioutdev, 1);
    }
    else
#endif
    {
        sys_close_midi();
        midi_open(nindev, newmidiindev, noutdev, newmidioutdev, 1);
    }

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
