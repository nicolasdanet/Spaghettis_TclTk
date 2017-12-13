
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_ATOMIC_WITH_MAC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_LOAD_STORE_32_IS_ATOMIC

int32_t pizAtomicInt32Read (PIZInt32Atomic *q)
{
    return (*q);
}

void pizAtomicInt32Write (int32_t n, PIZInt32Atomic *q)
{
    (*q) = n;
}

uint32_t pizAtomicUInt32Read (PIZUInt32Atomic *q)
{
    return (*q);
}

void pizAtomicUInt32Write (uint32_t n, PIZUInt32Atomic *q)
{
    (*q) = n;
}

#else

int32_t pizAtomicInt32Read (PIZInt32Atomic *q)
{
    return (int32_t)OSAtomicAdd32 (0, (int32_t *)q);
}

void pizAtomicInt32Write (int32_t n, PIZInt32Atomic *q)
{
    while (1) {
        int32_t old = *q;
        if (OSAtomicCompareAndSwap32 ((int32_t)old, (int32_t)n, (int32_t *)q)) { break; }
    }
}

uint32_t pizAtomicUInt32Read (PIZUInt32Atomic *q)
{
    return (uint32_t)OSAtomicAdd32 (0, (int32_t *)q);
}

void pizAtomicUInt32Write (uint32_t n, PIZUInt32Atomic *q)
{
    while (1) {
        uint32_t old = *q;
        if (OSAtomicCompareAndSwap32 ((int32_t)old, (int32_t)n, (int32_t *)q)) { break; }
    }
}

#endif // PD_LOAD_STORE_32_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_LOAD_STORE_64_IS_ATOMIC

uint64_t pizAtomicUInt64Read (PIZUInt64Atomic *q)
{
    return (*q);
}

void pizAtomicUInt64Write (uint64_t n, PIZUInt64Atomic *q)
{
    (*q) = n;
}

#else

uint64_t pizAtomicUInt64Read (PIZUInt64Atomic *q)
{
    return (uint64_t)OSAtomicAdd64 (0, (int64_t *)q);
}

void pizAtomicUInt64Write (uint64_t n, PIZUInt64Atomic *q)
{
    while (1) {
        uint64_t old = *q;
        if (OSAtomicCompareAndSwap64 ((int64_t)old, (int64_t)n, (int64_t *)q)) { break; }
    }
}

#endif // PD_LOAD_STORE_64_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_ATOMIC_WITH_MAC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
