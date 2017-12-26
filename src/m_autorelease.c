
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A smartest algorithm should be implemented. */
/* Autorelease could be never drained if it is called "often and too close". */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define AUTORELEASE_PERIOD  SECONDS_TO_MILLISECONDS (29.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_autoreleaseDrain (void)
{
    if (pd_hasThingQuiet (sym__autorelease)) {
        pd_message (pd_getThing (sym__autorelease), sym__autorelease, 0, NULL);
    }   
}

static void instance_autoreleaseTask (void *dummy)
{
    instance_autoreleaseDrain();
    clock_delay (instance_get()->pd_autorelease, AUTORELEASE_PERIOD);
}

static void instance_autoreleaseReschedule (void)
{
    clock_delay (instance_get()->pd_autorelease, AUTORELEASE_PERIOD);
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
    if (pd_hasThingQuiet (sym__polling)) {
        pd_message (pd_getThing (sym__polling), sym__polling, 0, NULL);
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
