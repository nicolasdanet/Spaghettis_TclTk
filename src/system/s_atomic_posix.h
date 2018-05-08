
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __s_atomic_posix_h_
#define __s_atomic_posix_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://gcc.gnu.org/onlinedocs/gcc-4.8.2/gcc/_005f_005fatomic-Builtins.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef volatile int32_t    __attribute__ ((__aligned__ (4)))   t_int32Atomic;
typedef volatile uint32_t   __attribute__ ((__aligned__ (4)))   t_uint32Atomic;
typedef volatile uint64_t   __attribute__ ((__aligned__ (8)))   t_uint64Atomic;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_MEMORY_BARRIER                       __sync_synchronize()

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ATOMIC_INT32_INCREMENT(q)            __atomic_add_fetch ((q), 1, __ATOMIC_RELAXED)
#define PD_ATOMIC_INT32_DECREMENT(q)            __atomic_sub_fetch ((q), 1, __ATOMIC_RELAXED)

#define PD_ATOMIC_INT32_READ(q)                 atomic_int32Read ((q))
#define PD_ATOMIC_INT32_WRITE(value, q)         atomic_int32Write ((value), (q))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ATOMIC_UINT32_SET(mask, q)           __atomic_or_fetch ((q), (mask), __ATOMIC_RELAXED)
#define PD_ATOMIC_UINT32_UNSET(mask, q)         __atomic_and_fetch ((q), (~(mask)), __ATOMIC_RELAXED)

#define PD_ATOMIC_UINT32_READ(q)                atomic_uInt32Read ((q))
#define PD_ATOMIC_UINT32_WRITE(value, q)        atomic_uInt32Write ((value), (q))

#define PD_ATOMIC_UINT32_TRUE(mask, q)          ((mask) & PD_ATOMIC_UINT32_READ (q))
#define PD_ATOMIC_UINT32_FALSE(mask, q)         !((mask) & PD_ATOMIC_UINT32_READ (q))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_ATOMIC_UINT64_READ(q)                atomic_uInt64Read ((q))
#define PD_ATOMIC_UINT64_WRITE(value, q)        atomic_uInt64Write ((value), (q))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int32_t     atomic_int32Read    (t_int32Atomic *q);
void        atomic_int32Write   (int32_t n, t_int32Atomic *q);

uint32_t    atomic_uInt32Read   (t_uint32Atomic *q);
void        atomic_uInt32Write  (uint32_t n, t_uint32Atomic *q);

uint64_t    atomic_uInt64Read   (t_uint64Atomic *q);
void        atomic_uInt64Write  (uint64_t n, t_uint64Atomic *q);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_atomic_posix_h_
