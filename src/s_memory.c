
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *sys_getMemory (size_t n)
{
    void *r = calloc (n < 1 ? 1 : n, 1);

    PD_ASSERT (r != NULL);
    PD_ABORT  (r == NULL);
    
    return r;
}

void *sys_getMemoryResize (void *ptr, size_t oldSize, size_t newSize)
{
    void *r = NULL;
    
    if (oldSize < 1) { oldSize = 1; }
    if (newSize < 1) { newSize = 1; }
    
    r = realloc (ptr, newSize);
    
    PD_ASSERT (r != NULL);
    PD_ABORT  (r == NULL);
    
    if (newSize > oldSize) { memset (((char *)r) + oldSize, 0, newSize - oldSize); }
    
    return r;
}

void sys_freeMemory (void *ptr)
{
    free (ptr);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
