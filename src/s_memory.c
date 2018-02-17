
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *memory_get (size_t n)
{
    void *r = calloc (n < 1 ? 1 : n, 1);

    PD_ASSERT (r != NULL);
    PD_ABORT  (r == NULL);
    
    return r;
}

void *memory_getResize (void *ptr, size_t oldSize, size_t newSize)
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

void memory_free (void *ptr)
{
    free (ptr);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* In debug build memory leaks for externals are globally tracked. */

#if PD_WITH_DEBUG

void *memory_getForExternal (size_t n)
{
    return leak_getMemoryChecked (n, __FUNCTION__, __LINE__);
}

void *memory_getResizeForExternal (void *ptr, size_t oldSize, size_t newSize)
{
    return leak_getMemoryResizeChecked (ptr, oldSize, newSize, __FUNCTION__, __LINE__);
}

void memory_freeForExternal (void *ptr)
{
    leak_freeMemoryChecked (ptr, __FUNCTION__, __LINE__);
}

#else

void *memory_getForExternal (size_t n)
{
    return memory_get (n);
}

void *memory_getResizeForExternal (void *ptr, size_t oldSize, size_t newSize)
{
    return memory_getResize (ptr, oldSize, newSize);
}

void memory_freeForExternal (void *ptr)
{
    memory_free (ptr);
}

#endif // PD_WITH_DEBUG

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
