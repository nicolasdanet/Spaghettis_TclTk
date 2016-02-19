
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

int midi_api = API_DEFAULT_MIDI;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int midi_nmidiindev;
static int midi_midiindev[MAXIMUM_MIDI_IN];
static char midi_indevnames[MAXIMUM_MIDI_IN * MAXIMUM_DEVICE_DESCRIPTION];
static int midi_nmidioutdev;
static int midi_midioutdev[MAXIMUM_MIDI_OUT];
static char midi_outdevnames[MAXIMUM_MIDI_IN * MAXIMUM_DEVICE_DESCRIPTION];

void sys_get_midi_apis(char *buf)
{
    int n = 0;
    strcpy(buf, "{ ");
#ifdef USEAPI_OSS
    sprintf(buf + strlen(buf), "{OSS-MIDI %d} ", API_DEFAULT_MIDI); n++;
#endif
#ifdef USEAPI_ALSA
    sprintf(buf + strlen(buf), "{ALSA-MIDI %d} ", API_ALSA); n++;
#endif
    strcat(buf, "}");
        /* then again, if only one API (or none) we don't offer any choice. */
    if (n < 2)
        strcpy(buf, "{}");
}

void sys_get_midi_params(int *pnmidiindev, int *pmidiindev,
    int *pnmidioutdev, int *pmidioutdev)
{
    int i, devn;
    *pnmidiindev = midi_nmidiindev;
    for (i = 0; i < midi_nmidiindev; i++)
    {
        if ((devn = sys_mididevnametonumber(0,
            &midi_indevnames[i * MAXIMUM_DEVICE_DESCRIPTION])) >= 0)
                pmidiindev[i] = devn;
        else pmidiindev[i] = midi_midiindev[i]; 
    }
    *pnmidioutdev = midi_nmidioutdev;
    for (i = 0; i < midi_nmidioutdev; i++)
    {
        if ((devn = sys_mididevnametonumber(1,
            &midi_outdevnames[i * MAXIMUM_DEVICE_DESCRIPTION])) >= 0)
                pmidioutdev[i] = devn;
        else pmidioutdev[i] = midi_midioutdev[i]; 
    }
}

static void sys_save_midi_params(
    int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev)
{
    int i;
    midi_nmidiindev = nmidiindev;
    for (i = 0; i < nmidiindev; i++)
    {
        midi_midiindev[i] = midiindev[i];
        sys_mididevnumbertoname(0, midiindev[i],
            &midi_indevnames[i * MAXIMUM_DEVICE_DESCRIPTION], MAXIMUM_DEVICE_DESCRIPTION);
    }
    midi_nmidioutdev = nmidioutdev;
    for (i = 0; i < nmidioutdev; i++)
    {
        midi_midioutdev[i] = midioutdev[i]; 
        sys_mididevnumbertoname(1, midioutdev[i],
            &midi_outdevnames[i * MAXIMUM_DEVICE_DESCRIPTION], MAXIMUM_DEVICE_DESCRIPTION);
    }
}

void sys_open_midi(int nmidiindev, int *midiindev,
    int nmidioutdev, int *midioutdev, int enable)
{
    if (enable)
    {
#ifdef USEAPI_ALSA
        midi_initializeALSA();
#endif
#ifdef USEAPI_OSS
        midi_initializeOSS();
#endif
#ifdef USEAPI_ALSA
        if (midi_api == API_ALSA)
            sys_alsa_do_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev);
        else
#endif /* ALSA */
            sys_do_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev);
    }
    sys_save_midi_params(nmidiindev, midiindev,
        nmidioutdev, midioutdev);

    sys_vGui("set ::var(apiMidi) %d\n", midi_api);

}

    /* open midi using whatever parameters were last used */
void sys_reopen_midi( void)
{
    int nmidiindev, midiindev[MAXIMUM_MIDI_IN];
    int nmidioutdev, midioutdev[MAXIMUM_MIDI_OUT];
    sys_get_midi_params(&nmidiindev, midiindev, &nmidioutdev, midioutdev);
    sys_open_midi(nmidiindev, midiindev, nmidioutdev, midioutdev, 1);
}

/*
void sys_listmididevs(void )
{
    char indevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION], outdevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i;

    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXIMUM_DEVICE_NUMBER, MAXIMUM_DEVICE_DESCRIPTION);

    if (!nindevs)
        post("no midi input devices found");
    else
    {
        post("MIDI input devices:");
        for (i = 0; i < nindevs; i++)
            post("%d. %s", i+1, indevlist + i * MAXIMUM_DEVICE_DESCRIPTION);
    }
    if (!noutdevs)
        post("no midi output devices found");
    else
    {
        post("MIDI output devices:");
        for (i = 0; i < noutdevs; i++)
            post("%d. %s", i+DEVONSET, outdevlist + i * MAXIMUM_DEVICE_DESCRIPTION);
    }
}
*/
void sys_set_midi_api(int which)
{
     midi_api = which;
     if (0)
        post("midi_api %d", midi_api);
}

void global_midiProperties(void *dummy, t_float flongform);
void midi_alsa_setndevs(int in, int out);

void global_midiAPI(void *dummy, t_float f)
{
    int newapi = f;
    if (newapi != midi_api)
    {
#ifdef USEAPI_ALSA
        if (midi_api == API_ALSA)
            sys_alsa_close_midi();
        else
#endif
              sys_close_midi();
        midi_api = newapi;
        sys_reopen_midi();
    }
#ifdef USEAPI_ALSA
    midi_alsa_setndevs(midi_nmidiindev, midi_nmidioutdev);
#endif
    global_midiProperties(0, (midi_nmidiindev > 1 || midi_nmidioutdev > 1));
}

extern t_class *global_object;

    /* start an midi settings dialog window */
void global_midiProperties(void *dummy, t_float flongform)
{
    char buf[1024 + 2 * MAXIMUM_DEVICE_NUMBER*(MAXIMUM_DEVICE_DESCRIPTION+4)];
        /* these are the devices you're using: */
    int nindev, midiindev[MAXIMUM_MIDI_IN];
    int noutdev, midioutdev[MAXIMUM_MIDI_OUT];
    int midiindev1, midiindev2, midiindev3, midiindev4, midiindev5,
        midiindev6, midiindev7, midiindev8, midiindev9,
        midioutdev1, midioutdev2, midioutdev3, midioutdev4, midioutdev5,
        midioutdev6, midioutdev7, midioutdev8, midioutdev9;

        /* these are all the devices on your system: */
    char indevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION], outdevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i;

    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXIMUM_DEVICE_NUMBER, MAXIMUM_DEVICE_DESCRIPTION);

    sys_gui("set ::ui_midi::midiIn {none}\n");
    for (i = 0; i < nindevs; i++)
        sys_vGui("lappend ::ui_midi::midiIn {%s}\n",
            indevlist + i * MAXIMUM_DEVICE_DESCRIPTION);

    sys_gui("set ::ui_midi::midiOut {none}\n");
    for (i = 0; i < noutdevs; i++)
        sys_vGui("lappend ::ui_midi::midiOut {%s}\n",
            outdevlist + i * MAXIMUM_DEVICE_DESCRIPTION);

    sys_get_midi_params(&nindev, midiindev, &noutdev, midioutdev);

    if (nindev > 1 || noutdev > 1)
        flongform = 1;

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
    gfxstub_new(&global_object, (void *)global_midiProperties, buf);
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
    sys_save_midi_params(nindev, newmidiindev,
        noutdev, newmidioutdev);
#ifdef USEAPI_ALSA
    if (midi_api == API_ALSA)
    {
        sys_alsa_close_midi();
        sys_open_midi(alsadevin, newmidiindev, alsadevout, newmidioutdev, 1);
    }
    else
#endif
    {
        sys_close_midi();
        sys_open_midi(nindev, newmidiindev, noutdev, newmidioutdev, 1);
    }

}

void sys_get_midi_devs(char *indevlist, int *nindevs,
                       char *outdevlist, int *noutdevs, 
                       int maxndevs, int devdescsize)
{

#ifdef USEAPI_ALSA
  if (midi_api == API_ALSA)
    midi_alsa_getdevs(indevlist, nindevs, outdevlist, noutdevs, 
                      maxndevs, devdescsize);
  else
#endif /* ALSA */
  midi_getdevs(indevlist, nindevs, outdevlist, noutdevs, maxndevs, devdescsize);
}

/* convert a device name to a (1-based) device number.  (Output device if
'output' parameter is true, otherwise input device).  Negative on failure. */

int sys_mididevnametonumber(int output, const char *name)
{
    char indevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION], outdevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i;

    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXIMUM_DEVICE_NUMBER, MAXIMUM_DEVICE_DESCRIPTION);

    if (output)
    {
            /* try first for exact match */
        for (i = 0; i < noutdevs; i++)
            if (!strcmp(name, outdevlist + i * MAXIMUM_DEVICE_DESCRIPTION))
                return (i);
            /* failing that, a match up to end of shorter string */
        for (i = 0; i < noutdevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(outdevlist + i * MAXIMUM_DEVICE_DESCRIPTION))
                comp = strlen(outdevlist + i * MAXIMUM_DEVICE_DESCRIPTION);
            if (!strncmp(name, outdevlist + i * MAXIMUM_DEVICE_DESCRIPTION, comp))
                return (i);
        }
    }
    else
    {
        for (i = 0; i < nindevs; i++)
            if (!strcmp(name, indevlist + i * MAXIMUM_DEVICE_DESCRIPTION))
                return (i);
        for (i = 0; i < nindevs; i++)
        {
            unsigned int comp = strlen(name);
            if (comp > strlen(indevlist + i * MAXIMUM_DEVICE_DESCRIPTION))
                comp = strlen(indevlist + i * MAXIMUM_DEVICE_DESCRIPTION);
            if (!strncmp(name, indevlist + i * MAXIMUM_DEVICE_DESCRIPTION, comp))
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
    char indevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION], outdevlist[MAXIMUM_DEVICE_NUMBER*MAXIMUM_DEVICE_DESCRIPTION];
    int nindevs = 0, noutdevs = 0, i;
    if (devno < 0)
    {
        *name = 0;
        return;
    }
    sys_get_midi_devs(indevlist, &nindevs, outdevlist, &noutdevs,
        MAXIMUM_DEVICE_NUMBER, MAXIMUM_DEVICE_DESCRIPTION);
    if (output && (devno < noutdevs))
        strncpy(name, outdevlist + devno * MAXIMUM_DEVICE_DESCRIPTION, namesize);
    else if (!output && (devno < nindevs))
        strncpy(name, indevlist + devno * MAXIMUM_DEVICE_DESCRIPTION, namesize);
    else *name = 0;
    name[namesize-1] = 0;
}
