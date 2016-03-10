
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <jack/weakjack.h>
#include <jack/jack.h>
#include <regex.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_MAXIMUM_CLIENTS    100
#define JACK_MAXIMUM_PORTS      128
#define JACK_MAXIMUM_FRAMES     64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_sample         *audio_soundIn;
extern t_sample         *audio_soundOut;

extern int              audio_channelsIn;
extern int              audio_channelsOut;
extern t_float          audio_sampleRate;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_sample         *jack_bufferIn;                                     /* Shared. */
static t_sample         *jack_bufferOut;                                    /* Shared. */

static char             *jack_client_names[JACK_MAXIMUM_CLIENTS];           /* Shared. */
static jack_client_t    *jack_client;                                       /* Shared. */

static jack_port_t      *jack_portsIn[JACK_MAXIMUM_PORTS];                  /* Shared. */
static jack_port_t      *jack_portsOut[JACK_MAXIMUM_PORTS];                 /* Shared. */

static int              jack_numberOfPortsOut;                              /* Shared. */
static jack_nframes_t   jack_maximumNumberOfFrames;                         /* Shared. */
static jack_nframes_t   jack_filledFrames;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static pthread_cond_t   jack_cond;                                          /* Shared. */
static pthread_mutex_t  jack_mutex;                                         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define JACK_BUFFER_SIZE    4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int pollprocess(jack_nframes_t nframes, void *arg)
{
    int j;
    jack_default_audio_sample_t *out, *in;

    pthread_mutex_lock(&jack_mutex);
    if (nframes > JACK_MAXIMUM_FRAMES) jack_maximumNumberOfFrames = nframes;
    else jack_maximumNumberOfFrames = JACK_MAXIMUM_FRAMES;
    if (jack_filledFrames >= nframes)
    {
        if (jack_filledFrames != nframes)
            fprintf(stderr,"Partial read\n");
        /* hmm, how to find out whether 't_sample' and
            'jack_default_audio_sample_t' are actually the same type??? */
        if (sizeof(t_sample)==sizeof(jack_default_audio_sample_t)) 
        {
            for (j = 0; j < audio_channelsOut;  j++)
            {
                if (out = jack_port_get_buffer(jack_portsOut[j], nframes))
                    memcpy(out, jack_bufferOut + (j * JACK_BUFFER_SIZE),
                        sizeof (jack_default_audio_sample_t) * nframes);
            }
            for (j = 0; j < audio_channelsIn; j++)
            {
                if (in = jack_port_get_buffer(jack_portsIn[j], nframes))
                    memcpy(jack_bufferIn + (j * JACK_BUFFER_SIZE), in,
                        sizeof (jack_default_audio_sample_t) * nframes);
            }
        } 
        else
        {
            unsigned int frame=0;
            t_sample*data;
            for (j = 0; j < audio_channelsOut;  j++)
            {
                if (out = jack_port_get_buffer (jack_portsOut[j], nframes))
                {
                    data = jack_bufferOut + (j * JACK_BUFFER_SIZE);
                    for (frame=0; frame<nframes; frame++)
                        *out++ = *data++;
                }
            }
            for (j = 0; j < audio_channelsIn; j++)
            {
                if (in = jack_port_get_buffer( jack_portsIn[j], nframes))
                {
                    data = jack_bufferIn + (j * JACK_BUFFER_SIZE);
                    for (frame=0; frame<nframes; frame++)
                        *data++ = *in++;
                }
            }
        }
        jack_filledFrames -= nframes;
    }
    else
    {           /* PD could not keep up ! */
        //if (jack_started) jack_dio_error = 1;
        for (j = 0; j < jack_numberOfPortsOut;  j++)
        {
            if (out = jack_port_get_buffer (jack_portsOut[j], nframes))
                memset(out, 0, sizeof (float) * nframes); 
            memset(jack_bufferOut + j * JACK_BUFFER_SIZE, 0, JACK_BUFFER_SIZE * sizeof(t_sample));
        }
        jack_filledFrames = 0;
    }
    pthread_cond_broadcast(&jack_cond);
    pthread_mutex_unlock(&jack_mutex);
    return 0;
}

/*
static int callbackprocess(jack_nframes_t nframes, void *arg)
{
    int chan, j, k;
    unsigned int n;
    jack_default_audio_sample_t *out[JACK_MAXIMUM_PORTS], *in[JACK_MAXIMUM_PORTS], *jp;

    if (nframes % AUDIO_DEFAULT_BLOCKSIZE)
    {
        fprintf(stderr, "jack: nframes %d not a multiple of blocksize %d\n",
            nframes, AUDIO_DEFAULT_BLOCKSIZE);
        nframes -= (nframes % AUDIO_DEFAULT_BLOCKSIZE);
    }
    for (chan = 0; chan < audio_channelsIn; chan++)
        in[chan] = jack_port_get_buffer(jack_portsIn[chan], nframes);
    for (chan = 0; chan < audio_channelsOut; chan++)
        out[chan] = jack_port_get_buffer(jack_portsOut[chan], nframes);
    for (n = 0; n < nframes; n += AUDIO_DEFAULT_BLOCKSIZE)
    {
        t_sample *fp;
        for (chan = 0; chan < audio_channelsIn; chan++)
            if (in[chan])
        {
            for (fp = audio_soundIn + chan*AUDIO_DEFAULT_BLOCKSIZE,
                jp = in[chan] + n, j=0; j < AUDIO_DEFAULT_BLOCKSIZE; j++)
                    *fp++ = *jp++;
        }
        for (chan = 0; chan < audio_channelsOut; chan++)
        {
            for (fp = audio_soundOut + chan*AUDIO_DEFAULT_BLOCKSIZE,
                j = 0; j < AUDIO_DEFAULT_BLOCKSIZE; j++)
                    *fp++ = 0;
        }
        (*jack_callback)();
        for (chan = 0; chan < audio_channelsOut; chan++)
            if (out[chan])
        {
            for (fp = audio_soundOut + chan*AUDIO_DEFAULT_BLOCKSIZE, jp = out[chan] + n,
                j=0; j < AUDIO_DEFAULT_BLOCKSIZE; j++)
                    *jp++ = *fp++;
        }
    }       
    return 0;
}
*/
static int
jack_srate (jack_nframes_t srate, void *arg)
{
        audio_sampleRate = srate;
        return 0;
}

static void
jack_shutdown (void *arg)
{
  post_error ("JACK: server shut down");
  
  jack_deactivate (jack_client);
  //jack_client_close(jack_client); /* likely to hang if the server shut down */
  jack_client = NULL;

    // audio_setAPI (NULL, API_NONE); // set pd_whichapi 0
    
    audio_close();
}

static char** jack_get_clients(void)
{
    const char **jack_ports;
    int i,j;
    int num_clients = 0;
    regex_t port_regex;
    jack_ports = jack_get_ports( jack_client, "", "", 0 );
    regcomp( &port_regex, "^[^:]*", REG_EXTENDED );

    jack_client_names[0] = NULL;

    /* Build a list of clients from the list of ports */
    for( i = 0; jack_ports[i] != NULL; i++ )
    {
        int client_seen;
        regmatch_t match_info;
        char tmp_client_name[100];

        if(num_clients>=JACK_MAXIMUM_CLIENTS)break;


        /* extract the client name from the port name, using a regex
         * that parses the clientname:portname syntax */
        regexec( &port_regex, jack_ports[i], 1, &match_info, 0 );
        memcpy( tmp_client_name, &jack_ports[i][match_info.rm_so],
                match_info.rm_eo - match_info.rm_so );
        tmp_client_name[ match_info.rm_eo - match_info.rm_so ] = '\0';

        /* do we know about this port's client yet? */
        client_seen = 0;

        for( j = 0; j < num_clients; j++ )
            if( strcmp( tmp_client_name, jack_client_names[j] ) == 0 )
                client_seen = 1;

        if( client_seen == 0 )
        {
            jack_client_names[num_clients] = (char*)PD_MEMORY_GET(strlen(tmp_client_name) + 1);

            /* The alsa_pcm client should go in spot 0.  If this
             * is the alsa_pcm client AND we are NOT about to put
             * it in spot 0 put it in spot 0 and move whatever
             * was already in spot 0 to the end. */

            if( strcmp( "alsa_pcm", tmp_client_name ) == 0 && num_clients > 0 )
            {
              char* tmp;
                /* alsa_pcm goes in spot 0 */
              tmp = jack_client_names[ num_clients ];
              jack_client_names[ num_clients ] = jack_client_names[0];
              jack_client_names[0] = tmp;
              strcpy( jack_client_names[0], tmp_client_name);
            }
            else
            {
                /* put the new client at the end of the client list */
                strcpy( jack_client_names[ num_clients ], tmp_client_name );
            }
            num_clients++;
        }
    }

    /*    for (i=0;i<num_clients;i++) post("client: %s",jack_client_names[i]); */

    free( jack_ports );
    return jack_client_names;
}

/*   
 *   Wire up all the ports of one client.
 *
 */

static int jack_connect_ports(char* client)
{
    char  regex_pattern[100]; /* its always the same, ... */
    int i;
    const char **jack_ports;

    if (strlen(client) > 96)  return -1;

    sprintf( regex_pattern, "%s:.*", client );

    jack_ports = jack_get_ports( jack_client, regex_pattern,
                                 NULL, JackPortIsOutput);
    if (jack_ports)
    {
        for (i=0;jack_ports[i] != NULL && i < audio_channelsIn;i++)      
            if (jack_connect (jack_client, jack_ports[i],
               jack_port_name (jack_portsIn[i]))) 
                  post_error ("JACK: cannot connect input ports %s -> %s",
                      jack_ports[i],jack_port_name (jack_portsIn[i]));
        free(jack_ports);
    }
    jack_ports = jack_get_ports( jack_client, regex_pattern,
                                 NULL, JackPortIsInput);
    if (jack_ports)
    {
        for (i=0;jack_ports[i] != NULL && i < audio_channelsOut;i++)      
          if (jack_connect (jack_client, jack_port_name (jack_portsOut[i]),
            jack_ports[i])) 
              post_error ( "JACK: cannot connect output ports %s -> %s",
                jack_port_name (jack_portsOut[i]),jack_ports[i]);

        free(jack_ports);
    }
    return 0;
}


static void pd_jack_error_callback(const char *desc) {
  post_error ("JACKerror: %s", desc);
  return;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int jack_open_audio(int inchans, int outchans, int rate, t_audiocallback callback)
{
    int j;
    char port_name[80] = "";
    char client_name[80] = "";

    int client_iterator = 0;
    int new_jack = 0;
    int srate;
    jack_status_t status;

#ifdef __APPLE__
    if (!jack_client_open)
    {
        post_error ("Can't open Jack (it seems not to be installed on this Mac)");
        return 1;
    }
#endif

    //jack_dio_error = 0;

    if ((inchans == 0) && (outchans == 0)) return 0;

    if (outchans > JACK_MAXIMUM_PORTS) {
        post_error ("JACK: %d output ports not supported, setting to %d",
            outchans, JACK_MAXIMUM_PORTS);
        outchans = JACK_MAXIMUM_PORTS;
    }

    if (inchans > JACK_MAXIMUM_PORTS) {
        post_error ("JACK: %d input ports not supported, setting to %d",
            inchans, JACK_MAXIMUM_PORTS);
        inchans = JACK_MAXIMUM_PORTS;
    }

    /* try to become a client of the JACK server.  (If no JACK server exists,
        jack_client_open() will start uone up by default.  It's not clear
        whether or not this is desirable; see long Pd list thread started by 
        yvan volochine, June 2013) */
    if (!jack_client) {
        do {
          sprintf(client_name,"pure_data_%d",client_iterator);
          client_iterator++;
          jack_client = jack_client_open (client_name, JackNoStartServer,
            &status, NULL);
          if (status & JackServerFailed) {
            post_error ("JACK: unable to connect to JACK server.  Is JACK running?");
            jack_client=NULL;
            break;
          }
        } while (status & JackNameNotUnique);

        if(status) {
          if (status & JackServerStarted) {
             // verbose(1, "JACK: started server");
          } else {
            post_error ("JACK: server returned status %d", status);
          }
        }
        // verbose(1, "JACK: started server as '%s'", client_name);

        if (!jack_client) {
            /* jack spits out enough messages already, do not warn */
            audio_channelsIn = audio_channelsOut = 0;
            return 1;
        }
        
        audio_channelsIn = inchans;
        audio_channelsOut = outchans;
        if (jack_bufferIn)
            free(jack_bufferIn);
        if (audio_channelsIn)
            jack_bufferIn = calloc(sizeof(t_sample), audio_channelsIn * JACK_BUFFER_SIZE); 
        if (jack_bufferOut)
            free(jack_bufferOut);
        if (audio_channelsOut)
            jack_bufferOut = calloc(sizeof(t_sample), audio_channelsOut * JACK_BUFFER_SIZE); 

        jack_get_clients();

        /* set JACK callback functions */

        //jack_callback = callback;
        
        jack_set_process_callback (jack_client, pollprocess, 0);

        jack_set_error_function (pd_jack_error_callback);


        /* tell the JACK server to call `srate()' whenever
           the sample rate of the system changes.
        */

        jack_set_sample_rate_callback (jack_client, jack_srate, 0);


        /* tell the JACK server to call `jack_shutdown()' if
           it ever shuts down, either entirely, or if it
           just decides to stop calling us.
        */

        jack_on_shutdown (jack_client, jack_shutdown, 0);

        for (j=0; j<audio_channelsIn; j++)
             jack_portsIn[j]=NULL;
        for (j=0; j<audio_channelsOut; j++)
             jack_portsOut[j] = NULL;

        new_jack = 1;
    }

    /* display the current sample rate. once the client is activated
       (see below), you should rely on your own sample rate
       callback (see above) for this value.
    */

    srate = jack_get_sample_rate (jack_client);
    audio_sampleRate = srate;

    /* create the ports */

    for (j = 0; j < inchans; j++)
    {
        sprintf(port_name, "input%d", j);
        if (!jack_portsIn[j]) jack_portsIn[j] = jack_port_register (jack_client,
            port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
        if (!jack_portsIn[j])
        {
          post_error ("JACK: can only register %d input ports (of %d requested)",
            j, inchans);
          audio_channelsIn = inchans = j;
          break;
        }
    }

    for (j = 0; j < outchans; j++)
    {
        sprintf(port_name, "output%d", j);
        if (!jack_portsOut[j]) jack_portsOut[j] = jack_port_register (jack_client,
            port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
        if (!jack_portsOut[j])
        {
          post_error ("JACK: can only register %d output ports (of %d requested)",
            j, outchans);
          audio_channelsOut = outchans = j;
          break;
        }
    } 
    jack_numberOfPortsOut = outchans;

    /* tell the JACK server that we are ready to roll */

    if (new_jack)
    {
        if (jack_activate (jack_client)) {
            post_error ("cannot activate client");
            audio_channelsIn = audio_channelsOut = 0;
            return 1;
        }

        for (j = 0; j < outchans; j++) 
            memset(jack_bufferOut + j * JACK_BUFFER_SIZE, 0,
                JACK_BUFFER_SIZE * sizeof(t_sample));

        if (jack_client_names[0])
            jack_connect_ports(jack_client_names[0]);

        pthread_mutex_init(&jack_mutex, NULL);
        pthread_cond_init(&jack_cond, NULL);
    }
    return 0;
}

void jack_close_audio(void) 
{
    if (jack_client){
        jack_deactivate (jack_client);
        jack_client_close(jack_client);
    }

    jack_client=NULL;

    pthread_cond_broadcast(&jack_cond);

    pthread_cond_destroy(&jack_cond);
    pthread_mutex_destroy(&jack_mutex);
    if (jack_bufferIn)
        free(jack_bufferIn), jack_bufferIn = 0;
    if (jack_bufferOut)
        free(jack_bufferOut), jack_bufferOut = 0;
 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int jack_send_dacs(void)
{
    t_sample * fp;
    int j;
    int rtnval =  DACS_YES;
    int timenow;
    int timeref = sys_getRealTimeInSeconds();
    if (!jack_client) return DACS_NO;
    if (!audio_channelsIn && !audio_channelsOut) return (DACS_NO); 
    if (0 /*jack_dio_error*/)
    {
        //sys_log_error(ERROR_RESYNC);
        //jack_dio_error = 0;
    }
    pthread_mutex_lock(&jack_mutex);
    if (jack_filledFrames >= jack_maximumNumberOfFrames)
        pthread_cond_wait(&jack_cond,&jack_mutex);

    if (!jack_client)
    {
        pthread_mutex_unlock(&jack_mutex);
        return DACS_NO;
    }

    fp = audio_soundOut;
    for (j = 0; j < audio_channelsOut; j++)
    {
        memcpy(jack_bufferOut + (j * JACK_BUFFER_SIZE) + jack_filledFrames, fp,
            AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample));
        fp += AUDIO_DEFAULT_BLOCKSIZE;  
    }
    fp = audio_soundIn;
    for (j = 0; j < audio_channelsIn; j++)
    {
        memcpy(fp, jack_bufferIn + (j * JACK_BUFFER_SIZE) + jack_filledFrames,
            AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample));
        fp += AUDIO_DEFAULT_BLOCKSIZE;
    }
    jack_filledFrames += AUDIO_DEFAULT_BLOCKSIZE;
    pthread_mutex_unlock(&jack_mutex);

    if ((timenow = sys_getRealTimeInSeconds()) - timeref > 0.002)
    {
        rtnval = DACS_SLEPT;
    }
    memset(audio_soundOut, 0, AUDIO_DEFAULT_BLOCKSIZE*sizeof(t_sample)*audio_channelsOut);
    return rtnval;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void jack_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs, int *canmulti)
{
    int maxndev = MAXIMUM_DEVICES;
    int devdescsize = MAXIMUM_DESCRIPTION;
    
    int i, ndev;
    *canmulti = 0;  /* supports multiple devices */

    ndev = 1;
    for (i = 0; i < ndev; i++)
    {
        sprintf(indevlist + i * devdescsize, "JACK");
        sprintf(outdevlist + i * devdescsize, "JACK");
    }
    *nindevs = *noutdevs = ndev;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
