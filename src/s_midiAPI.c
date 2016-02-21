
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

void midi_requireDialog (void *dummy)
{
    char t[PD_STRING] = { 0 };
    
    int numberOfDevicesIn  = 0;
    int numberOfDevicesOut = 0;
    int devicesIn[MAXIMUM_MIDI_IN]   = { 0 };
    int devicesOut[MAXIMUM_MIDI_OUT] = { 0 };
    
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION] = { 0 };
    char outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION] = { 0 };
    
    int nindevs = 0, noutdevs = 0, i;

    midi_getLists (indevlist, &nindevs, outdevlist, &noutdevs);

    sys_gui("set ::ui_midi::midiIn {none}\n");
    for (i = 0; i < nindevs; i++)
        sys_vGui("lappend ::ui_midi::midiIn {%s}\n",
            indevlist + i * MAXIMUM_DESCRIPTION);

    sys_gui("set ::ui_midi::midiOut {none}\n");
    for (i = 0; i < noutdevs; i++)
        sys_vGui("lappend ::ui_midi::midiOut {%s}\n",
            outdevlist + i * MAXIMUM_DESCRIPTION);

    midi_getDevices(&numberOfDevicesIn, devicesIn, &numberOfDevicesOut, devicesOut);

    /*if (nindev > 1 || noutdev > 1)
        flongform = 1;*/

    int midiindev1, midiindev2, midiindev3, midiindev4, midiindev5,
        midiindev6, midiindev7, midiindev8, midiindev9,
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, midioutdev5,
        midioutdev6, midioutdev7, midioutdev8, midioutdev9;
        
    midiindev1 = (numberOfDevicesIn > 0 &&  devicesIn[0]>= 0 ? devicesIn[0]+1 : 0);
    midiindev2 = (numberOfDevicesIn > 1 &&  devicesIn[1]>= 0 ? devicesIn[1]+1 : 0);
    midiindev3 = (numberOfDevicesIn > 2 &&  devicesIn[2]>= 0 ? devicesIn[2]+1 : 0);
    midiindev4 = (numberOfDevicesIn > 3 &&  devicesIn[3]>= 0 ? devicesIn[3]+1 : 0);
    midiindev5 = (numberOfDevicesIn > 4 &&  devicesIn[4]>= 0 ? devicesIn[4]+1 : 0);
    midiindev6 = (numberOfDevicesIn > 5 &&  devicesIn[5]>= 0 ? devicesIn[5]+1 : 0);
    midiindev7 = (numberOfDevicesIn > 6 &&  devicesIn[6]>= 0 ? devicesIn[6]+1 : 0);
    midiindev8 = (numberOfDevicesIn > 7 &&  devicesIn[7]>= 0 ? devicesIn[7]+1 : 0);
    midiindev9 = (numberOfDevicesIn > 8 &&  devicesIn[8]>= 0 ? devicesIn[8]+1 : 0);
    midioutdev1 = (numberOfDevicesOut > 0 && devicesOut[0]>= 0 ? devicesOut[0]+1 : 0); 
    midioutdev2 = (numberOfDevicesOut > 1 && devicesOut[1]>= 0 ? devicesOut[1]+1 : 0); 
    midioutdev3 = (numberOfDevicesOut > 2 && devicesOut[2]>= 0 ? devicesOut[2]+1 : 0); 
    midioutdev4 = (numberOfDevicesOut > 3 && devicesOut[3]>= 0 ? devicesOut[3]+1 : 0); 
    midioutdev5 = (numberOfDevicesOut > 4 && devicesOut[4]>= 0 ? devicesOut[4]+1 : 0);
    midioutdev6 = (numberOfDevicesOut > 5 && devicesOut[5]>= 0 ? devicesOut[5]+1 : 0);
    midioutdev7 = (numberOfDevicesOut > 6 && devicesOut[6]>= 0 ? devicesOut[6]+1 : 0);
    midioutdev8 = (numberOfDevicesOut > 7 && devicesOut[7]>= 0 ? devicesOut[7]+1 : 0);
    midioutdev9 = (numberOfDevicesOut > 8 && devicesOut[8]>= 0 ? devicesOut[8]+1 : 0);

#ifdef USEAPI_ALSA
      if (midi_api == API_ALSA)
    sprintf(t,
"::ui_midi::show %%s \
%d %d %d %d 0 0 0 0 0 \
%d %d %d %d 0 0 0 0 0 \
1\n",
        midiindev1, midiindev2, midiindev3, midiindev4, 
        midioutdev1, midioutdev2, midioutdev3, midioutdev4);
      else
#endif
    sprintf(t,
"::ui_midi::show %%s \
%d %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d %d \
0\n",
        midiindev1, midiindev2, midiindev3, midiindev4, 
        midiindev5, midiindev6, midiindev7, midiindev8, midiindev9, 
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, 
        midioutdev5, midioutdev6, midioutdev7, midioutdev8, midioutdev9);

    gfxstub_deleteforkey(0);
    gfxstub_new(&global_object, (void *)midi_requireDialog, t);
}

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
