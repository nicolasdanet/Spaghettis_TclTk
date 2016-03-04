
/*
    Author is Phil Burk.
    
    This program is distributed with the PortAudio Portable Audio Library.
 
    Copyright (c) 1999-2000 Ross Bencina and Phil Burk.

    Any person wishing to distribute modifications to the Software is
    requested to send the modifications to the original developer so that
    they can be incorporated into the canonical version.
    
    Modified 2002/07/13 by olaf.matthes@gmx.de to allow any number of channels.
    
    Extensively hacked by msp@ucsd.edu for various reasons.
    
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

typedef struct {
    long bufferSize;
    long writeIndex;
    long readIndex;
    } sys_ringbuf;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

long ringbuffer_initialize          (sys_ringbuf *rbuf, long numBytes, char *dataPtr, long nfill);
long ringbuffer_getWriteAvailable   (sys_ringbuf *rbuf);
long ringbuffer_getReadAvailable    (sys_ringbuf *rbuf);
long ringbuffer_write               (sys_ringbuf *rbuf, const void *data, long numBytes, char *buffer);
long ringbuffer_read                (sys_ringbuf *rbuf, void *data, long numBytes, char *buffer);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_ringbuffer_h_
