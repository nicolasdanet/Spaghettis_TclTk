
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

/* < http://www.softsynth.com > */
/* < http://www.audiomulch.com/portaudio/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef USEAPI_PORTAUDIO     /* Used only with PortAudio API. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_ringbuffer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Clear buffer. Should only be called when buffer is NOT being read. */

static void ringbuffer_flush (sys_ringbuf *rbuf, void *dataPtr, long nfill)
{
    long n;
    char *s = NULL;
        
    rbuf->readIndex = 0;
    rbuf->writeIndex = nfill;
    
    for (n = nfill, s = dataPtr; n--; s++) { *s = 0; }
}

static long ringbuffer_advanceWriteIndex (sys_ringbuf *rbuf, long numBytes)
{
    long ret = (rbuf->writeIndex + numBytes);
    
    if (ret >= 2 * rbuf->bufferSize) {
        ret -= 2 * rbuf->bufferSize;
    }
    
    return (rbuf->writeIndex = ret);
}

static long ringbuffer_advanceReadIndex (sys_ringbuf *rbuf, long numBytes)
{
    long ret = (rbuf->readIndex + numBytes);
    
    if (ret >= 2 * rbuf->bufferSize) {
        ret -= 2 * rbuf->bufferSize; 
    }
    
    return (rbuf->readIndex = ret);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Get address of regions to which we can write data. */
/* If the region is contiguous, size2 will be zero. */
/* If non-contiguous, size2 will be the size of second region. */
/* Returns room available to be written or numBytes, whichever is smaller. */

static long ringbuffer_getWriteRegions (sys_ringbuf *rbuf,
    long numBytes,
    void **dataPtr1,
    long *sizePtr1,
    void **dataPtr2,
    long *sizePtr2,
    char *buffer)
{
    long index;
    long available = ringbuffer_getWriteAvailable (rbuf);
    
    if (numBytes > available) { numBytes = available; }
    
     /* Check to see if write is not contiguous. */
     
    index = rbuf->writeIndex;
       
    while (index >= rbuf->bufferSize) { index -= rbuf->bufferSize; }
    
    if ((index + numBytes) > rbuf->bufferSize) {
    
        /* Write data in two blocks that wrap the buffer. */
        
        long firstHalf = rbuf->bufferSize - index;
        
        *dataPtr1 = &buffer[index];
        *sizePtr1 = firstHalf;
        *dataPtr2 = &buffer[0];
        *sizePtr2 = numBytes - firstHalf;

    } else {
    
        *dataPtr1 = &buffer[index];
        *sizePtr1 = numBytes;
        *dataPtr2 = NULL;
        *sizePtr2 = 0;
    }
    
    return numBytes;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Get address of regions from which we can read data. */
/* If the region is contiguous, size2 will be zero. */
/* If non-contiguous, size2 will be the size of second region. */
/* Returns room available to be written or numBytes, whichever is smaller. */

static long ringbuffer_getReadRegions (sys_ringbuf *rbuf,
    long numBytes,
    void **dataPtr1,
    long *sizePtr1,
    void **dataPtr2,
    long *sizePtr2,
    char *buffer)
{
    long index;
    long available = ringbuffer_getReadAvailable (rbuf);
    
    if (numBytes > available) { numBytes = available; }
    
    /* Check to see if read is not contiguous. */
    
    index = rbuf->readIndex;
    
    while (index >= rbuf->bufferSize) { index -= rbuf->bufferSize; }
    
    if ((index + numBytes) > rbuf->bufferSize) {
    
        /* Write data in two blocks that wrap the buffer. */
        
        long firstHalf = rbuf->bufferSize - index;
        
        *dataPtr1 = &buffer[index];
        *sizePtr1 = firstHalf;
        *dataPtr2 = &buffer[0];
        *sizePtr2 = numBytes - firstHalf;

    } else {
    
        *dataPtr1 = &buffer[index];
        *sizePtr1 = numBytes;
        *dataPtr2 = NULL;
        *sizePtr2 = 0;
    }
    
    return numBytes;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

long ringbuffer_initialize (sys_ringbuf *rbuf, long numBytes, char *dataPtr, long nfill)
{
    rbuf->bufferSize = numBytes;
    ringbuffer_flush (rbuf, dataPtr, nfill);
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

long ringbuffer_getReadAvailable (sys_ringbuf *rbuf)
{
    long ret = rbuf->writeIndex - rbuf->readIndex;
    
    if (ret < 0) { ret += 2 * rbuf->bufferSize; }
    if (ret < 0 || ret > rbuf->bufferSize) { PD_BUG; }
    
    return ret;
}

long ringbuffer_getWriteAvailable (sys_ringbuf *rbuf)
{
    return (rbuf->bufferSize - ringbuffer_getReadAvailable (rbuf));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

long ringbuffer_write (sys_ringbuf *rbuf, const void *data, long numBytes, char *buffer)
{
    long size1, size2, numWritten;
    void *data1 = NULL;
    void *data2 = NULL;
    
    numWritten = ringbuffer_getWriteRegions (rbuf, numBytes, &data1, &size1, &data2, &size2, buffer);
    
    if (size2 > 0) {
        memcpy ((void *)data1, data, size1);
        data = ((char *)data) + size1;
        memcpy ((void *)data2, data, size2);
    } else {
        memcpy ((void *)data1, data, size1);
    }
    
    ringbuffer_advanceWriteIndex (rbuf, numWritten);
    
    return numWritten;
}

long ringbuffer_read (sys_ringbuf *rbuf, void *data, long numBytes, char *buffer)
{
    long size1, size2, numRead;
    void *data1 = NULL;
    void *data2 = NULL;
    
    numRead = ringbuffer_getReadRegions (rbuf, numBytes, &data1, &size1, &data2, &size2, buffer);
    
    if (size2 > 0) {
        memcpy (data, (void *)data1, size1);
        data = ((char *)data) + size1;
        memcpy (data, (void *)data2, size2);
    } else {
        memcpy (data, (void *)data1, size1);
    }
    
    ringbuffer_advanceReadIndex (rbuf, numRead);
    
    return numRead;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // USEAPI_PORTAUDIO

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
