
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that it is largely hacked from PortAudio. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _ringbuffer {
    int32_t         rb_size;
    size_t          rb_bytes;
    t_int32Atomic   rb_read;
    t_int32Atomic   rb_write;
    uint32_t        rb_hiMask;
    uint32_t        rb_loMask;
    char            *rb_vector;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_ringbuffer *ringbuffer_new (int32_t sizeOfElementInBytes, int32_t numberOfElements)
{
    t_ringbuffer *x = (t_ringbuffer *)PD_MEMORY_GET (sizeof (t_ringbuffer));
    
    PD_ASSERT (sizeOfElementInBytes > 0);
    PD_ASSERT (numberOfElements > 0);
    PD_ASSERT (PD_IS_POWER_2 (numberOfElements));
    
    x->rb_size   = numberOfElements;
    x->rb_bytes  = (size_t)sizeOfElementInBytes;
    x->rb_read   = 0;
    x->rb_write  = 0;
    x->rb_hiMask = (numberOfElements * 2) - 1;
    x->rb_loMask = (numberOfElements) - 1;
    
    x->rb_vector = (char *)PD_MEMORY_GET (x->rb_size * x->rb_bytes);
    
    return x;
}

void ringbuffer_free (t_ringbuffer *x)
{
    PD_MEMORY_FREE (x->rb_vector);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int32_t ringbuffer_getAvailableRead (const t_ringbuffer *x)
{
    /* Get absolute value of the distance (assume two's complement representation). */
    
    return ((x->rb_write - x->rb_read) & x->rb_hiMask);
}

int32_t ringbuffer_getAvailableWrite (const t_ringbuffer *x)
{
    return (x->rb_size - ringbuffer_getAvailableRead (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int32_t ringbuffer_getWriteRegions (t_ringbuffer *x, int32_t n,
    void **r1,
    int32_t *size1,
    void **r2,
    int32_t *size2)
{
    int32_t index;
    int32_t available = ringbuffer_getAvailableWrite (x);
    
    n = PD_MIN (available, n);
    
    index = x->rb_write & x->rb_loMask;
    
    if ((index + n) > x->rb_size) {
        
        int32_t firstHalf = x->rb_size - index;
        *r1     = &x->rb_vector[index * x->rb_bytes];
        *size1  = firstHalf;
        *r2     = &x->rb_vector[0];
        *size2  = n - firstHalf;
        
    } else {
    
        *r1     = &x->rb_vector[index * x->rb_bytes];
        *size1  = n;
        *r2     = NULL;
        *size2  = 0;
    }

    if (available) { PD_MEMORY_BARRIER; }
    
    return n;
}

static void ringbuffer_advanceWrite (t_ringbuffer *x, int32_t written)
{
    PD_MEMORY_BARRIER;      /* Release. */
    
    x->rb_write = (x->rb_write + written) & x->rb_hiMask;
}

void ringbuffer_write (t_ringbuffer *x, const void *data, int32_t n)
{
    int32_t size1;
    int32_t size2;
    void    *r1 = NULL;
    void    *r2 = NULL;
    
    int32_t written = ringbuffer_getWriteRegions (x, n, &r1, &size1, &r2, &size2);

    if (size2 > 0) {
        memcpy (r1, data,       size1 * x->rb_bytes);
        data = ((char *)data) + size1 * x->rb_bytes;
        memcpy (r2, data,       size2 * x->rb_bytes);
    } else {
        memcpy (r1, data,       size1 * x->rb_bytes);
    }
    
    ringbuffer_advanceWrite (x, written);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int32_t ringbuffer_getReadRegions (t_ringbuffer *x,
    int32_t n,
    void **r1,
    int32_t *size1,
    void **r2,
    int32_t *size2)
{
    int32_t index;
    int32_t available = ringbuffer_getAvailableRead (x);
    
    n = PD_MIN (available, n);
    
    index = x->rb_read & x->rb_loMask;
    
    if ((index + n) > x->rb_size) {
        int32_t firstHalf = x->rb_size - index;
        *r1     = &x->rb_vector[index * x->rb_bytes];
        *size1  = firstHalf;
        *r2     = &x->rb_vector[0];
        *size2  = n - firstHalf;
        
    } else {
        *r1     = &x->rb_vector[index * x->rb_bytes];
        *size1  = n;
        *r2     = NULL;
        *size2  = 0;
    }
    
    if (available) { PD_MEMORY_BARRIER; }

    return n;
}

static void ringbuffer_advanceRead (t_ringbuffer *x, int32_t n)
{
    PD_MEMORY_BARRIER;

    x->rb_read = (x->rb_read + n) & x->rb_hiMask;
}

void ringbuffer_read (t_ringbuffer *x, void *data, int32_t n)
{
    int32_t size1;
    int32_t size2;
    void *r1 = NULL;
    void *r2 = NULL;
    
    int32_t read = ringbuffer_getReadRegions (x, n, &r1, &size1, &r2, &size2);
    
    if (size2 > 0) {
        memcpy (data, r1,       size1 * x->rb_bytes);
        data = ((char *)data) + size1 * x->rb_bytes;
        memcpy (data, r2,       size2 * x->rb_bytes);
    } else {
        memcpy (data, r1,       size1 * x->rb_bytes);
    }
    
    ringbuffer_advanceRead (x, read);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
