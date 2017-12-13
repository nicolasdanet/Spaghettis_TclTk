
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_ATOMIC_WITH_POSIX

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
    return __atomic_load_n (q, __ATOMIC_RELAXED);
}

void pizAtomicInt32Write (int32_t n, PIZInt32Atomic *q)
{
    __atomic_store_n (q, n, __ATOMIC_RELAXED);
}

uint32_t pizAtomicUInt32Read (PIZUInt32Atomic *q)
{
    return __atomic_load_n (q, __ATOMIC_RELAXED);
}

void pizAtomicUInt32Write (uint32_t n, PIZUInt32Atomic *q)
{
    __atomic_store_n (q, n, __ATOMIC_RELAXED);
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
    return __atomic_load_n (q, __ATOMIC_RELAXED);
}

void pizAtomicUInt64Write (uint64_t n, PIZUInt64Atomic *q)
{
    __atomic_store_n (q, n, __ATOMIC_RELAXED);
}

#endif // PD_LOAD_STORE_64_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_ATOMIC_WITH_POSIX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
