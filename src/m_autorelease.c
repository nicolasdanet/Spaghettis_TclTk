
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define AUTORELEASE_PERIOD      PD_SECONDS_TO_MILLISECONDS (7.0)
#define AUTORELEASE_THRESHOLD   64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_autoreleaseDrain (void)
{
    if (symbol_hasThingQuiet (sym__autorelease)) {
        pd_message (symbol_getThing (sym__autorelease), sym__autorelease, 0, NULL);
    }   
}

static void instance_autoreleaseTask (void *dummy)
{
    instance_autoreleaseDrain();
    clock_delay (instance_get()->pd_autorelease, AUTORELEASE_PERIOD);
}

/* To avoid congestion rescheduling time is progressively diminished. */

static void instance_autoreleaseReschedule (void)
{
    int t = symbol_getNumberOfThings (sym__autorelease);
    double d = (AUTORELEASE_THRESHOLD - (double)t) / AUTORELEASE_THRESHOLD;
    clock_delay (instance_get()->pd_autorelease, d * AUTORELEASE_PERIOD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_autoreleaseRun (void)
{
    instance_get()->pd_autorelease = clock_new ((void *)NULL, (t_method)instance_autoreleaseTask);
    clock_delay (instance_get()->pd_autorelease, AUTORELEASE_PERIOD);
}

void instance_autoreleaseStop (void)
{
    instance_autoreleaseDrain();
    clock_free (instance_get()->pd_autorelease);
    instance_get()->pd_autorelease = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_autoreleaseRegister (t_pd *x)
{
    pd_bind (x, sym__autorelease);
    
    if (instance_get()->pd_autorelease) { instance_autoreleaseReschedule(); }
    else {
        instance_autoreleaseDrain();    /* While quitting the application. */
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Function that might be called at last by registered object while autoreleased. */

void instance_autoreleaseProceed (t_pd *x)
{
    pd_unbind (x, sym__autorelease); pd_free (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define POLLING_PERIOD  47.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_pollingTask (void *dummy)
{
    if (symbol_hasThingQuiet (sym__polling)) {
        pd_message (symbol_getThing (sym__polling), sym__polling, 0, NULL);
    }  
    
    clock_delay (instance_get()->pd_polling, POLLING_PERIOD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_pollingRun (void)
{
    instance_get()->pd_polling = clock_new ((void *)NULL, (t_method)instance_pollingTask);
    clock_delay (instance_get()->pd_polling, POLLING_PERIOD);
}

void instance_pollingStop (void)
{
    clock_free (instance_get()->pd_polling);
    instance_get()->pd_polling = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_pollingRegister (t_pd *x)
{
    pd_bind (x, sym__polling);
}

void instance_pollingUnregister (t_pd *x)
{
    pd_unbind (x, sym__polling);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
