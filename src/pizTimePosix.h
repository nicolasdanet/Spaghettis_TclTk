
/* 
    Copyright (c) 2014, Nicolas Danet, < nicolas.danet@free.fr >. 
*/

/* < http://opensource.org/licenses/MIT > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef PIZ_TIME_POSIX_H
#define PIZ_TIME_POSIX_H

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <time.h>
#include <sys/time.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef uint64_t PIZTime;
typedef uint64_t PIZNano;
typedef uint64_t PIZStamp;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PIZ_ZERO_TIME   0ULL
#define PIZ_ZERO_NANO   0ULL
#define PIZ_ZERO_STAMP  0ULL

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _PIZBase {
    PIZTime         time_;
    struct timeval  tv_;
    } PIZBase; 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

uint64_t    pizSeedMake             (void);
void        pizSeedConstant         (int isConstant);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Do NOT call pizTimeAddNano or pizTimeElapsedNano at initialization time. */

/* < http://www.parashift.com/c++-faq/static-init-order.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        pizTimeSet              (PIZTime *t);
void        pizTimeCopy             (PIZTime *t, const PIZTime *toCopy);
void        pizTimeAddNano          (PIZTime *t, const PIZNano *ns); 
t_error     pizTimeElapsedNano      (const PIZTime *t0, const PIZTime *t1, PIZNano *r);
uint64_t    pizTimeAsUInt64         (PIZTime *t);
void        pizTimeWithUInt64       (PIZTime *t, uint64_t n);
int         pizTimeIsEqual          (PIZTime *t1, PIZTime *t2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        pizNanoSleep            (PIZNano *ns);
void        pizNanoWithDouble       (PIZNano *ns, double f);
uint64_t    pizNanoAsUInt64         (PIZNano *ns);
int         pizNanoIsLessThan       (PIZNano *t1, PIZNano *t2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://en.wikipedia.org/wiki/Network_Time_Protocol > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        pizStampSet             (PIZStamp *stamp);
void        pizStampCopy            (PIZStamp *stamp, const PIZStamp *toCopy);
void        pizStampAddNano         (PIZStamp *stamp, const PIZNano *ns);
t_error     pizStampElapsedNano     (const PIZStamp *t0, const PIZStamp *t1, PIZNano *r);
uint64_t    pizStampAsUInt64        (PIZStamp *stamp);
void        pizStampWithUInt64      (PIZStamp *stamp, uint64_t n);
int         pizStampIsEqual         (PIZStamp *t1, PIZStamp *t2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     pizBaseInit             (PIZBase *base);
t_error     pizBaseTimeToStamp      (const PIZBase *base, const PIZTime *t, PIZStamp *stamp);
t_error     pizBaseStampToTime      (const PIZBase *base, const PIZStamp *stamp, PIZTime *t);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // PIZ_TIME_POSIX_H