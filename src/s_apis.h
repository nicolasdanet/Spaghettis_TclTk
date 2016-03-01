
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_apis_h_
#define __s_apis_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_audiocallback)(void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    midi_openNative                 (int numberOfDevicesIn, 
                                            int *devicesIn, 
                                            int numberOfDevicesOut, 
                                            int *devicesOut);
                                        
void    midi_closeNative                (void);
void    midi_pushNextMessageNative      (int port, int a, int b, int c);
void    midi_pushNextByteNative         (int port, int a);
void    midi_pollNative                 (void);

t_error midi_getListOfDevicesNative     (char *devicesIn, 
                                            int  *numberOfDevicesIn, 
                                            char *devicesOut, 
                                            int  *numberOfDevicesOut);
                                        
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void sys_alsa_close_midi        (void);
void sys_alsa_putmidimess       (int port, int a, int b, int c);
void sys_alsa_putmidibyte       (int port, int a);
void sys_alsa_poll_midi         (void);
void sys_alsa_setmiditimediff   (double inbuftime, double outbuftime);
void sys_alsa_midibytein        (int port, int byte);
void midi_alsa_setndevs         (int i, int o);
void midi_alsa_getdevs          (char *indevlist, int *nindevs, char *outdevlist, int *noutdevs);
void sys_alsa_do_open_midi      (int nmidiindev, int *midiindev, int nmidioutdev, int *midioutdev);
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int  pa_send_dacs               (void);
void pa_listdevs                (void);
void pa_close_audio             (void);
int  pa_open_audio              (int inchans,
                                    int outchans,
                                    int rate,
                                    t_sample *soundin,
                                    t_sample *soundout,
                                    int framesperbuf,
                                    int nbuffers,
                                    int indeviceno,
                                    int outdeviceno,
                                    t_audiocallback callback);
                                
void pa_getdevs                 (char *indevlist,
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int *canmulti, 
                                    int *canCallback);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void oss_initialize             (void);
int  oss_send_dacs              (void);
void oss_close_audio            (void);
int  oss_open_audio             (int naudioindev,
                                    int *audioindev,
                                    int nchindev,
                                    int *chindev,
                                    int naudiooutdev,
                                    int *audiooutdev,
                                    int nchoutdev,
                                    int *choutdev,
                                    int rate,
                                    int blocksize);
                                
void oss_getdevs                (char *indevlist,
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int *canmulti, 
                                    int *canCallback);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void alsa_putzeros              (int iodev, int n);
void alsa_getzeros              (int iodev, int n);
void alsa_printstate            (void);
void alsa_adddev                (char *name);

int  alsa_send_dacs             (void);
void alsa_close_audio           (void);
int  alsa_open_audio            (int naudioindev,
                                    int *audioindev,
                                    int nchindev,
                                    int *chindev,
                                    int naudiooutdev,
                                    int *audiooutdev,
                                    int nchoutdev,
                                    int *choutdev,
                                    int rate,
                                    int blocksize);
                                
void alsa_getdevs               (char *indevlist,
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int *canmulti, 
                                    int *canCallback);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void jack_listdevs              (void);
int  jack_send_dacs             (void);
void jack_close_audio           (void);
int  jack_open_audio            (int wantinchans,
                                    int wantoutchans,
                                    int srate,
                                    t_audiocallback callback);
                                    
void jack_getdevs               (char *indevlist,
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int *canmulti, 
                                    int *canCallback);
                                
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int  mmio_send_dacs             (void);
void mmio_close_audio           (void);
int  mmio_open_audio            (int naudioindev,
                                    int *audioindev,
                                    int nchindev,
                                    int *chindev,
                                    int naudiooutdev,
                                    int *audiooutdev,
                                    int nchoutdev,
                                    int *choutdev,
                                    int rate,
                                    int blocksize);
                                
void mmio_getdevs               (char *indevlist,
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int *canmulti, 
                                    int *canCallback);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dummy_listdevs             (void);
int  dummy_send_dacs            (void);
void dummy_close_audio          (void);
int  dummy_open_audio           (void);
void dummy_getdevs              (char *indevlist, 
                                    int *nindevs,
                                    char *outdevlist,
                                    int *noutdevs,
                                    int *canmulti, 
                                    int *canCallback);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_apis_h_
