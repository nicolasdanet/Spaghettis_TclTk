
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PIZ_TYPES_H
#define PIZ_TYPES_H

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef unsigned char               PIZBool;
typedef unsigned long               PIZError;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_GOOD                    0UL
#define PIZ_ERROR                   1UL

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_ILP32

typedef signed char                 PIZInt8;
typedef signed short                PIZInt16;
typedef signed long                 PIZInt32;
typedef signed long long            PIZInt64;

typedef unsigned char               PIZUInt8;
typedef unsigned short              PIZUInt16;
typedef unsigned long               PIZUInt32;
typedef unsigned long long          PIZUInt64;

#else
#if PD_LP64

typedef signed char                 PIZInt8;
typedef signed short                PIZInt16;
typedef signed int                  PIZInt32;
typedef signed long                 PIZInt64;

typedef unsigned char               PIZUInt8;
typedef unsigned short              PIZUInt16;
typedef unsigned int                PIZUInt32;
typedef unsigned long               PIZUInt64;

#endif // PD_LP64
#endif // PD_ILP32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PIZ_TYPES_H
