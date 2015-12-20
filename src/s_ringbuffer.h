
/*
    Author is Phil Burk.
    
    This program is distributed with the PortAudio Portable Audio Library.
 
    Copyright (c) 1999-2000 Ross Bencina and Phil Burk.

    Any person wishing to distribute modifications to the Software is
    requested to send the modifications to the original developer so that
    they can be incorporated into the canonical version.
    
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www.softsynth.com > */
/* < http://www.audiomulch.com/portaudio/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_ringbuffer_h_
#define __s_ringbuffer_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PA_VOLATILE 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct {
    long                bufferSize;
    PA_VOLATILE long    writeIndex;
    PA_VOLATILE long    readIndex;
    } sys_ringbuf;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

long sys_ringbuf_init                   (PA_VOLATILE sys_ringbuf *rbuf,
                                            long numBytes,
                                            PA_VOLATILE char *dataPtr,
                                            long nfill);
                                        
long sys_ringbuf_getwriteavailable      (PA_VOLATILE sys_ringbuf *rbuf);
long sys_ringbuf_getreadavailable       (PA_VOLATILE sys_ringbuf *rbuf);

long sys_ringbuf_write                  (PA_VOLATILE sys_ringbuf *rbuf,
                                            const void *data,
                                            long numBytes,
                                            PA_VOLATILE char *buffer);

long sys_ringbuf_read                   (PA_VOLATILE sys_ringbuf *rbuf,
                                            void *data, 
                                            long numBytes,
                                            PA_VOLATILE char *buffer);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_ringbuffer_h_
