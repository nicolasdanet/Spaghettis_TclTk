
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
#pragma mark -

#define MIDI_SOMETHING  1       /* First item is "none". */

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
        
        midi_api = (int)f; midi_open();
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

void midi_openWithDevices (int numberOfDevicesIn, int *devicesIn, int numberOfDevicesOut, int *devicesOut)
{
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
    
    midi_setDevices (numberOfDevicesIn, devicesIn, numberOfDevicesOut, devicesOut);

    sys_vGui ("set ::var(apiMidi) %d\n", midi_api);     // --
}

void midi_open (void)
{
    int m, n;
    int i[MAXIMUM_MIDI_IN]  = { 0 };
    int o[MAXIMUM_MIDI_OUT] = { 0 };
    
    midi_getDevices (&m, i, &n, o); midi_openWithDevices (m, i, n, o);
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
        if ((n = midi_numberWithName (0, s)) >= 0) {
            devicesIn[i] = n;
        } else {
            devicesIn[i] = midi_devicesIn[i]; 
        }
    }
        
    for (i = 0; i < midi_numberOfDevicesOut; i++) {
        char *s = &midi_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        if ((n = midi_numberWithName (1, s)) >= 0) {
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
        midi_numberToName (0, devicesIn[i], s, MAXIMUM_DESCRIPTION);
    }
    
    for (i = 0; i < numberOfDevicesOut; i++) {
        char *s = &midi_devicesOutNames[i * MAXIMUM_DESCRIPTION];
        midi_devicesOut[i] = devicesOut[i]; 
        midi_numberToName (1, devicesOut[i], s, MAXIMUM_DESCRIPTION);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error midi_getLists (char *i, int *m, char *o, int *n)
{
    if (API_WITH_ALSA && midi_api == API_ALSA) { midi_alsa_getdevs (i, m, o, n); } 
    else {
        midi_getdevs (i, m, o, n);
    }
    
    return PD_ERROR_NONE;
}

int midi_numberWithName (int isOutput, const char *name)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };

    int k;
    
    if (!midi_getLists (i, &m, o, &n)) {
    //
    if (isOutput) {
        for (k = 0; k < n; k++) { 
            if (!strcmp (name, o + (k * MAXIMUM_DESCRIPTION))) { return k; }
        }
    } else {
        for (k = 0; k < m; k++) {
            if (!strcmp (name, i + (k * MAXIMUM_DESCRIPTION))) { return k; }
        }
    }
    //
    }
    
    return -1;
}

void midi_numberToName (int isOutput, int k, char *dest, size_t size)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    
    t_error err = PD_ERROR;
    
    if (k >= 0 && !midi_getLists (i, &m, o, &n)) { 
        if (isOutput && (k < n))        { err = string_copy (dest, size, o + (k * MAXIMUM_DESCRIPTION)); }
        else if (!isOutput && (k < m))  { err = string_copy (dest, size, i + (k * MAXIMUM_DESCRIPTION)); }
    }
    
    if (err) { *dest = 0; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error midi_requireDialogInitialize (void)
{
    int  m = 0;
    int  n = 0;
    char i[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    char o[MAXIMUM_DEVICES * MAXIMUM_DESCRIPTION] = { 0 };
    
    t_error err = midi_getLists (i, &m, o, &n);
    
    if (!err) {
    //
    char t1[PD_STRING] = { 0 };
    char t2[PD_STRING] = { 0 };
    int k;
    
    err |= string_copy (t1, PD_STRING, "set ::ui_midi::midiIn [list {none}");   // --
    err |= string_copy (t2, PD_STRING, "set ::ui_midi::midiOut [list {none}");  // --
    
    for (k = 0; k < m; k++) {
        err |= string_addSprintf (t1, PD_STRING, " {%s}", i + (k * MAXIMUM_DESCRIPTION));
    }
    for (k = 0; k < n; k++) {
        err |= string_addSprintf (t2, PD_STRING, " {%s}", o + (k * MAXIMUM_DESCRIPTION));
    }
    
    err |= string_add (t1, PD_STRING, "]\n");
    err |= string_add (t2, PD_STRING, "]\n");
    
    post ("%s", t1);    /* ??? */
    post ("%s", t2);    /* ??? */
    
    sys_gui (t1);
    sys_gui (t2);
    //
    }
    
    return err;
}

void midi_requireDialog (void *dummy)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_MIDI_IN]  = { 0 };
    int o[MAXIMUM_MIDI_OUT] = { 0 };
    
    t_error err = midi_requireDialogInitialize();
    
    midi_getDevices (&m, i, &n, o);
    
    if (!err) {
    //
    char t[PD_STRING] = { 0 };
    
    PD_ASSERT (MAXIMUM_MIDI_IN >= 8);
    
    int i1 = (m > 0 && i[0]>= 0 ? i[0] + MIDI_SOMETHING : 0);
    int i2 = (m > 1 && i[1]>= 0 ? i[1] + MIDI_SOMETHING : 0);
    int i3 = (m > 2 && i[2]>= 0 ? i[2] + MIDI_SOMETHING : 0);
    int i4 = (m > 3 && i[3]>= 0 ? i[3] + MIDI_SOMETHING : 0);
    int i5 = (m > 4 && i[4]>= 0 ? i[4] + MIDI_SOMETHING : 0);
    int i6 = (m > 5 && i[5]>= 0 ? i[5] + MIDI_SOMETHING : 0);
    int i7 = (m > 6 && i[6]>= 0 ? i[6] + MIDI_SOMETHING : 0);
    int i8 = (m > 7 && i[7]>= 0 ? i[7] + MIDI_SOMETHING : 0);
    
    PD_ASSERT (MAXIMUM_MIDI_OUT >= 8);
    
    int o1 = (n > 0 && o[0]>= 0 ? o[0] + MIDI_SOMETHING : 0); 
    int o2 = (n > 1 && o[1]>= 0 ? o[1] + MIDI_SOMETHING : 0); 
    int o3 = (n > 2 && o[2]>= 0 ? o[2] + MIDI_SOMETHING : 0); 
    int o4 = (n > 3 && o[3]>= 0 ? o[3] + MIDI_SOMETHING : 0); 
    int o5 = (n > 4 && o[4]>= 0 ? o[4] + MIDI_SOMETHING : 0);
    int o6 = (n > 5 && o[5]>= 0 ? o[5] + MIDI_SOMETHING : 0);
    int o7 = (n > 6 && o[6]>= 0 ? o[6] + MIDI_SOMETHING : 0);
    int o8 = (n > 7 && o[7]>= 0 ? o[7] + MIDI_SOMETHING : 0);

    if (API_WITH_ALSA && midi_api == API_ALSA) {
    
        err = string_sprintf (t, PD_STRING,
            "::ui_midi::show %%s %d %d %d %d 0 0 0 0 %d %d %d %d 0 0 0 0 1\n",  // --
                i1, i2, i3, i4, o1, o2, o3, o4);

    } else {
    
        err = string_sprintf (t, PD_STRING,
            "::ui_midi::show %%s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d 0\n",  // --
                i1, i2, i3, i4, i5, i6, i7, i8, o1, o2, o3, o4, o5, o6, o7, o8);
                                    
    }
    
    post ("%s", t);   /* ??? */
    
    if (!err) {
        gfxstub_deleteforkey (NULL);
        gfxstub_new (&global_object, (void *)midi_requireDialog, t);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void midi_fromDialog (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int m = 0;
    int n = 0;
    int i[MAXIMUM_MIDI_IN]  = { 0 };
    int o[MAXIMUM_MIDI_OUT] = { 0 };
    int alsaIn  = 0;
    int alsaOut = 0;

    int parameters = (argc - 2) / 2;
    int k;
    
    post_atoms (argc, argv);    /* ??? */
    
    PD_ASSERT (parameters == MAXIMUM_MIDI_IN);
    PD_ASSERT (parameters == MAXIMUM_MIDI_OUT);
    
    alsaIn  = (t_int)atom_getFloatAtIndex ((parameters * 2) + 0, argc, argv);
    alsaOut = (t_int)atom_getFloatAtIndex ((parameters * 2) + 1, argc, argv);
    
    if (API_WITH_ALSA && midi_api == API_ALSA) {
    //
    m = PD_CLAMP (alsaIn,  0, MAXIMUM_MIDI_IN);
    n = PD_CLAMP (alsaOut, 0, MAXIMUM_MIDI_OUT);
        
    for (k = 0; k < m; k++) { i[k] = k; }
    for (k = 0; k < n; k++) { o[k] = k; }
    //
    } else {
    //
    for (k = 0; k < parameters; k++) {
        i[k] = (t_int)atom_getFloatAtIndex (k, argc, argv);
        o[k] = (t_int)atom_getFloatAtIndex (k + parameters, argc, argv);
    }

    for (k = 0; k < parameters; k++) { if (i[k] > 0) { i[m] = i[k] - MIDI_SOMETHING; m++; } }
    for (k = 0; k < parameters; k++) { if (o[k] > 0) { o[n] = o[k] - MIDI_SOMETHING; n++; } }
    //
    }
    
    if (API_WITH_ALSA && midi_api == API_ALSA) { sys_alsa_close_midi(); } 
    else {
        sys_close_midi();
    }

    midi_openWithDevices (m, i, n, o);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
