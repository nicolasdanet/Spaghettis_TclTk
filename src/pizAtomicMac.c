
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

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

#include "pizAtomicMac.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PIZ_LOAD_STORE_32_IS_ATOMIC

PIZInt32 pizAtomicInt32Read (PIZInt32Atomic *q) 
{
    return (*q);
}

void pizAtomicInt32Write (PIZInt32 n, PIZInt32Atomic *q)
{
    (*q) = n;
}

PIZUInt32 pizAtomicUInt32Read (PIZUInt32Atomic *q) 
{
    return (*q);
}

void pizAtomicUInt32Write (PIZUInt32 n, PIZUInt32Atomic *q)
{
    (*q) = n;
}

#else

PIZInt32 pizAtomicInt32Read (PIZInt32Atomic *q) 
{
    return (PIZInt32)OSAtomicAdd32 (0, (int32_t *)q);
}

void pizAtomicInt32Write (PIZInt32 n, PIZInt32Atomic *q)
{
    while (1) {
        PIZInt32 old = *q;
        if (OSAtomicCompareAndSwap32 ((int32_t)old, (int32_t)n, (int32_t *)q)) { break; }
    }
}

PIZUInt32 pizAtomicUInt32Read (PIZUInt32Atomic *q) 
{
    return (PIZUInt32)OSAtomicAdd32 (0, (int32_t *)q);
}

void pizAtomicUInt32Write (PIZUInt32 n, PIZUInt32Atomic *q)
{
    while (1) {
        PIZUInt32 old = *q;
        if (OSAtomicCompareAndSwap32 ((int32_t)old, (int32_t)n, (int32_t *)q)) { break; }
    }
}

#endif // PIZ_LOAD_STORE_32_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PIZ_LOAD_STORE_64_IS_ATOMIC

PIZUInt64 pizAtomicUInt64Read (PIZUInt64Atomic *q)
{
    return (*q);
}

void pizAtomicUInt64Write (PIZUInt64 n, PIZUInt64Atomic *q)
{
    (*q) = n;
}

#else

PIZUInt64 pizAtomicUInt64Read (PIZUInt64Atomic *q)
{
    return (PIZUInt64)OSAtomicAdd64 (0, (int64_t *)q);
}

void pizAtomicUInt64Write (PIZUInt64 n, PIZUInt64Atomic *q)
{
    while (1) {
        PIZUInt64 old = *q;
        if (OSAtomicCompareAndSwap64 ((int64_t)old, (int64_t)n, (int64_t *)q)) { break; }
    }
}

#endif // PIZ_LOAD_STORE_64_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_ATOMIC_WITH_MAC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
