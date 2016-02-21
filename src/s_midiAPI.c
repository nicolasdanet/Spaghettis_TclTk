
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
    int n = (int)f;
    
    if (n != midi_api) {
        if (API_WITH_ALSA && midi_api == API_ALSA) { sys_alsa_close_midi(); }
        else {
            sys_close_midi();
        }
        
        midi_api = n; midi_reopen();
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

void midi_getDevices (int *numberOfDevicesIn, int *devicesIn, int *numberOfDevicesOut, int *devicesOut)
{
    int i, n;
    
    *numberOfDevicesIn  = midi_numberOfDevicesIn;
    *numberOfDevicesOut = midi_numberOfDevicesOut;
    
    for (i = 0; i < midi_numberOfDevicesIn; i++) {
        char *s = &midi_devicesInNames[i * MAXIMUM_DESCRIPTION];
        if ((n = sys_mididevnametonumber (0, s)) >= 0) {
            devicesIn[i] = n;
        } else {
            devicesIn[i] = midi_devicesIn[i]; 
        }
    }
        
    for (i = 0; i < midi_numberOfDevicesOut; i++) {
        char *s = &midi_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        if ((n = sys_mididevnametonumber (1, s)) >= 0) {
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
        sys_mididevnumbertoname (0, devicesIn[i], s, MAXIMUM_DESCRIPTION);
    }
    
    for (i = 0; i < numberOfDevicesOut; i++) {
        char *s = &midi_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        midi_devicesOut[i] = devicesOut[i]; 
        sys_mididevnumbertoname (1, devicesOut[i], s, MAXIMUM_DESCRIPTION);
    }
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

void midi_reopen (void)
{
    int m, n;
    int i[MAXIMUM_MIDI_IN]  = { 0 };
    int o[MAXIMUM_MIDI_OUT] = { 0 };
    
    midi_getDevices (&m, i, &n, o); midi_open (m, i, n, o, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void midi_getLists (char *devicesIn, int *numberOfDevicesIn, char *devicesOut, int *numberOfDevicesOut)
{
    if (API_WITH_ALSA && midi_api == API_ALSA) {
        midi_alsa_getdevs (devicesIn, numberOfDevicesIn, devicesOut, numberOfDevicesOut);
    } else {
        midi_getdevs (devicesIn, numberOfDevicesIn, devicesOut, numberOfDevicesOut);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_requireDialog (void *dummy)
{
    char buf[1024 + 2 * MAXIMUM_DEVICES*(MAXIMUM_DESCRIPTION+4)];
        /* these are the devices you're using: */
    int nindev, midiindev[MAXIMUM_MIDI_IN];
    int noutdev, midioutdev[MAXIMUM_MIDI_OUT];
    int midiindev1, midiindev2, midiindev3, midiindev4, midiindev5,
        midiindev6, midiindev7, midiindev8, midiindev9,
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, midioutdev5,
        midioutdev6, midioutdev7, midioutdev8, midioutdev9;

        /* these are all the devices on your system: */
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
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

    midi_getDevices(&nindev, midiindev, &noutdev, midioutdev);

    /*if (nindev > 1 || noutdev > 1)
        flongform = 1;*/

    midiindev1 = (nindev > 0 &&  midiindev[0]>= 0 ? midiindev[0]+1 : 0);
    midiindev2 = (nindev > 1 &&  midiindev[1]>= 0 ? midiindev[1]+1 : 0);
    midiindev3 = (nindev > 2 &&  midiindev[2]>= 0 ? midiindev[2]+1 : 0);
    midiindev4 = (nindev > 3 &&  midiindev[3]>= 0 ? midiindev[3]+1 : 0);
    midiindev5 = (nindev > 4 &&  midiindev[4]>= 0 ? midiindev[4]+1 : 0);
    midiindev6 = (nindev > 5 &&  midiindev[5]>= 0 ? midiindev[5]+1 : 0);
    midiindev7 = (nindev > 6 &&  midiindev[6]>= 0 ? midiindev[6]+1 : 0);
    midiindev8 = (nindev > 7 &&  midiindev[7]>= 0 ? midiindev[7]+1 : 0);
    midiindev9 = (nindev > 8 &&  midiindev[8]>= 0 ? midiindev[8]+1 : 0);
    midioutdev1 = (noutdev > 0 && midioutdev[0]>= 0 ? midioutdev[0]+1 : 0); 
    midioutdev2 = (noutdev > 1 && midioutdev[1]>= 0 ? midioutdev[1]+1 : 0); 
    midioutdev3 = (noutdev > 2 && midioutdev[2]>= 0 ? midioutdev[2]+1 : 0); 
    midioutdev4 = (noutdev > 3 && midioutdev[3]>= 0 ? midioutdev[3]+1 : 0); 
    midioutdev5 = (noutdev > 4 && midioutdev[4]>= 0 ? midioutdev[4]+1 : 0);
    midioutdev6 = (noutdev > 5 && midioutdev[5]>= 0 ? midioutdev[5]+1 : 0);
    midioutdev7 = (noutdev > 6 && midioutdev[6]>= 0 ? midioutdev[6]+1 : 0);
    midioutdev8 = (noutdev > 7 && midioutdev[7]>= 0 ? midioutdev[7]+1 : 0);
    midioutdev9 = (noutdev > 8 && midioutdev[8]>= 0 ? midioutdev[8]+1 : 0);

#ifdef USEAPI_ALSA
      if (midi_api == API_ALSA)
    sprintf(buf,
"::ui_midi::show %%s \
%d %d %d %d 0 0 0 0 0 \
%d %d %d %d 0 0 0 0 0 \
1\n",
        midiindev1, midiindev2, midiindev3, midiindev4, 
        midioutdev1, midioutdev2, midioutdev3, midioutdev4);
      else
#endif
    sprintf(buf,
"::ui_midi::show %%s \
%d %d %d %d %d %d %d %d %d \
%d %d %d %d %d %d %d %d %d \
0\n",
        midiindev1, midiindev2, midiindev3, midiindev4, 
        midiindev5, midiindev6, midiindev7, midiindev8, midiindev9, 
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, 
        midioutdev5, midioutdev6, midioutdev7, midioutdev8, midioutdev9);

    gfxstub_deleteforkey(0);
    gfxstub_new(&global_object, (void *)midi_requireDialog, buf);
}

    /* new values from dialog window */
void global_midiDialog(void *dummy, t_symbol *s, int argc, t_atom *argv)
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
#pragma mark -

/* convert a device name to a (1-based) device number.  (Output device if
'output' parameter is true, otherwise input device).  Negative on failure. */

int sys_mididevnametonumber(int output, const char *name)
{
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i;

    midi_getLists(indevlist, &nindevs, outdevlist, &noutdevs);

    if (output)
    {
            /* try first for exact match */
        for (i = 0; i < noutdevs; i++)
            if (!strcmp(name, outdevlist + i * MAXIMUM_DESCRIPTION))
                return (i);
            /* failing that, a match up to end of shorter string */
        for (i = 0; i < noutdevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(outdevlist + i * MAXIMUM_DESCRIPTION))
                comp = strlen(outdevlist + i * MAXIMUM_DESCRIPTION);
            if (!strncmp(name, outdevlist + i * MAXIMUM_DESCRIPTION, comp))
                return (i);
        }
    }
    else
    {
        for (i = 0; i < nindevs; i++)
            if (!strcmp(name, indevlist + i * MAXIMUM_DESCRIPTION))
                return (i);
        for (i = 0; i < nindevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(indevlist + i * MAXIMUM_DESCRIPTION))
                comp = strlen(indevlist + i * MAXIMUM_DESCRIPTION);
            if (!strncmp(name, indevlist + i * MAXIMUM_DESCRIPTION, comp))
                return (i);
        }
    }
    return (-1);
}

/* convert a (1-based) device number to a device name.  (Output device if
'output' parameter is true, otherwise input device).  Empty string on failure.
*/

void sys_mididevnumbertoname(int output, int devno, char *name, int namesize)
{
    char indevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION], outdevlist[MAXIMUM_DEVICES*MAXIMUM_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i;
    if (devno < 0)
    {
        *name = 0;
        return;
    }
    midi_getLists(indevlist, &nindevs, outdevlist, &noutdevs);
    
    if (output && (devno < noutdevs))
        strncpy(name, outdevlist + devno * MAXIMUM_DESCRIPTION, namesize);
    else if (!output && (devno < nindevs))
        strncpy(name, indevlist + devno * MAXIMUM_DESCRIPTION, namesize);
    else *name = 0;
    name[namesize-1] = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
