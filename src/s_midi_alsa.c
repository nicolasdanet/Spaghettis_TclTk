
/* 
    Copyright (c) 1997-2003 Guenter Geiger, Miller Puckette,
    Larry Troxler, Winfried Ritsch, Karl MacMillan, and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "s_midi.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <alsa/asoundlib.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define MIDIALSA_MAXIMUM_EVENTS     512

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int midialsa_numberOfDevicesIn;                                      /* Shared. */
static int midialsa_numberOfDevicesOut;                                     /* Shared. */

static int midialsa_devicesIn[MAXIMUM_MIDI_IN];                             /* Shared. */
static int midialsa_devicesOut[MAXIMUM_MIDI_OUT];                           /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static snd_seq_t            *midialsa_handle;
static snd_midi_event_t     *midialsa_event;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void midi_openNative (int numberOfDevicesIn,
    int *dummyIn,
    int numberOfDevicesOut,
    int *dummyOut)
{
    char portname[50];
    int err = 0;
    int client;
    int i;
    snd_seq_client_info_t *alsainfo;

    midialsa_numberOfDevicesIn = 0;
    midialsa_numberOfDevicesOut = 0;

    if (numberOfDevicesOut == 0 && numberOfDevicesIn == 0) return;

    if(numberOfDevicesIn>MAXIMUM_MIDI_IN )
      {
        post("midi input ports reduced to maximum %d", MAXIMUM_MIDI_IN);
        numberOfDevicesIn=MAXIMUM_MIDI_IN;
      }
    if(numberOfDevicesOut>MAXIMUM_MIDI_OUT)
      {
        post("midi output ports reduced to maximum %d", MAXIMUM_MIDI_OUT);
        numberOfDevicesOut=MAXIMUM_MIDI_OUT;
      }

    if (numberOfDevicesIn>0 && numberOfDevicesOut>0)
        err = snd_seq_open(&midialsa_handle,"default",SND_SEQ_OPEN_DUPLEX,0);
    else if (numberOfDevicesIn > 0)
        err = snd_seq_open(&midialsa_handle,"default",SND_SEQ_OPEN_INPUT,0);
    else if (numberOfDevicesOut > 0)
        err = snd_seq_open(&midialsa_handle,"default",SND_SEQ_OPEN_OUTPUT,0);
    
    if (err!=0)
    {
            //sys_setalarm(1000000);
            post("couldn't open alsa sequencer");
            return;
    }
    for (i=0;i<numberOfDevicesIn;i++)
    {
        int port;
        sprintf(portname,"PureData Midi-In %d",i+1);
        port = snd_seq_create_simple_port(midialsa_handle,portname,
                                          SND_SEQ_PORT_CAP_WRITE |SND_SEQ_PORT_CAP_SUBS_WRITE, 
                                          SND_SEQ_PORT_TYPE_APPLICATION);
        midialsa_devicesIn[i] = port;        
        if (port < 0) goto error;        
    }

    for (i=0;i<numberOfDevicesOut;i++)
    {
        int port;
        sprintf(portname,"PureData Midi-Out %d",i+1);
        port = snd_seq_create_simple_port(midialsa_handle,portname,
                                          SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ, 
                                          SND_SEQ_PORT_TYPE_APPLICATION);
        midialsa_devicesOut[i] = port;       
        if (port < 0) goto error;        
    }
   
    snd_seq_client_info_malloc(&alsainfo);
    snd_seq_get_client_info(midialsa_handle,alsainfo);
    snd_seq_client_info_set_name(alsainfo,"PureData");
    client = snd_seq_client_info_get_client(alsainfo);
    snd_seq_set_client_info(midialsa_handle,alsainfo);
    snd_seq_client_info_free(alsainfo);
    post("Opened Alsa Client %d in:%d out:%d",client,numberOfDevicesIn,numberOfDevicesOut);
    //sys_setalarm(0);
    snd_midi_event_new(MIDIALSA_MAXIMUM_EVENTS,&midialsa_event);
    midialsa_numberOfDevicesOut = numberOfDevicesOut;
    midialsa_numberOfDevicesIn = numberOfDevicesIn;

    return;
 error:
    //sys_setalarm(1000000);
    post("couldn't open alsa MIDI output device");
    return;
}

void midi_pushNextMessageNative(int portno, int a, int b, int c)
{
    int channel;
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    if (portno >= 0 && portno < midialsa_numberOfDevicesOut)
    {
        if (a >= 224)   // pitchbend
        {
            channel = a-224;
            snd_seq_ev_set_pitchbend(&ev, channel, (((c<<7)|b)-8192)); /* b and c are already correct but alsa needs to recalculate them */
        }
        else if (a >= 208)      // touch
        {
            channel = a-208;
            snd_seq_ev_set_chanpress(&ev,channel,b);
        }
        else if (a >= 192)      // program
        {
            channel = a-192;
            snd_seq_ev_set_pgmchange(&ev,channel,b);
        }
        else if (a >= 176)      // controller
        {
            channel = a-176;
            snd_seq_ev_set_controller(&ev,channel,b,c);
        }
        else if (a >= 160)      // polytouch
        {
            channel = a-160;
            snd_seq_ev_set_keypress(&ev,channel,b,c);
        }
        else if (a >= 144)      // note
        {
            channel = a-144;
            if (c)
                snd_seq_ev_set_noteon(&ev,channel,b,c);
            else
                snd_seq_ev_set_noteoff(&ev,channel,b,c);
        }
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_source(&ev,midialsa_devicesOut[portno]);
        snd_seq_event_output_direct(midialsa_handle,&ev);
    }
    //post("%d %d %d\n",a,b,c);
}

void midi_pushNextByteNative(int portno, int byte)
{
    snd_seq_event_t ev;
    snd_seq_ev_clear(&ev);
    if (portno >= 0 && portno < midialsa_numberOfDevicesOut)
    {
        // repack into 1 byte char and put somewhere to point at
        unsigned char data = (unsigned char)byte;

        snd_seq_ev_set_sysex(&ev,1,&data); //...set_variable *should* have worked but didn't
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_source(&ev,midialsa_devicesOut[portno]);
        snd_seq_event_output_direct(midialsa_handle,&ev);
    }
}


    /* this version uses the asynchronous "read()" ... */
void midi_pollNative(void)
{
   unsigned char buf[MIDIALSA_MAXIMUM_EVENTS];
   int count, alsa_source;
   int i;
   snd_seq_event_t *midievent = NULL;

   if (midialsa_numberOfDevicesOut == 0 && midialsa_numberOfDevicesIn == 0) return;
   
   snd_midi_event_init(midialsa_event);

   if (!midialsa_numberOfDevicesOut && !midialsa_numberOfDevicesIn) return;
   count = snd_seq_event_input_pending(midialsa_handle,1);
   if (count != 0)
        count = snd_seq_event_input(midialsa_handle,&midievent);
   if (midievent != NULL)
   {
       count = snd_midi_event_decode(midialsa_event,buf,sizeof(buf),midievent);
       alsa_source = midievent->dest.port;
       for(i=0;i<count;i++)
           midi_receive(alsa_source, (buf[i] & 0xff));
       //post("received %d midi bytes\n",count);
   }
}

void midi_closeNative()
{
    midialsa_numberOfDevicesIn = midialsa_numberOfDevicesOut = 0;
    if(midialsa_handle)
      {
        snd_seq_close(midialsa_handle);
        if(midialsa_event)
          {
            snd_midi_event_free(midialsa_event);
          }
      }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error midi_getListsNative (char *devicesIn, 
    int *numberOfDevicesIn, 
    char *devicesOut, 
    int *numberOfDevicesOut)
{
    t_error err = PD_ERROR_NONE;
    
    err |= string_copy (devicesIn,  MAXIMUM_DESCRIPTION, "ALSA virtual device");
    err |= string_copy (devicesOut, MAXIMUM_DESCRIPTION, "ALSA virtual device");
    
    *numberOfDevicesIn  = 1;
    *numberOfDevicesOut = 1;
  
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
