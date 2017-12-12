
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizDefine.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PIZ_ATOMIC_WITH_POSIX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "pizAtomicPosix.h"

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
    return __atomic_load_n (q, __ATOMIC_RELAXED);
}

void pizAtomicInt32Write (PIZInt32 n, PIZInt32Atomic *q)
{
    __atomic_store_n (q, n, __ATOMIC_RELAXED);
}

PIZUInt32 pizAtomicUInt32Read (PIZUInt32Atomic *q) 
{
    return __atomic_load_n (q, __ATOMIC_RELAXED);
}

void pizAtomicUInt32Write (PIZUInt32 n, PIZUInt32Atomic *q)
{
    __atomic_store_n (q, n, __ATOMIC_RELAXED);
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
    return __atomic_load_n (q, __ATOMIC_RELAXED);
}

void pizAtomicUInt64Write (PIZUInt64 n, PIZUInt64Atomic *q)
{
    __atomic_store_n (q, n, __ATOMIC_RELAXED);
}

#endif // PIZ_LOAD_STORE_64_IS_ATOMIC

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PIZ_ATOMIC_WITH_POSIX

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
