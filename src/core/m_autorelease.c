
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define AUTORELEASE_PERIOD  PD_SECONDS_TO_MILLISECONDS (3.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pd_unbindQuiet (t_pd *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_autoreleaseDrainProceed (t_symbol *s)
{
    if (symbol_hasThingQuiet (s)) { pd_message (symbol_getThing (s), sym__autorelease, 0, NULL); }
}

static void instance_autoreleaseDrain (void)
{
    instance_autoreleaseDrainProceed (sym__autoreleaseA);
    instance_autoreleaseDrainProceed (sym__autoreleaseB);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_autoreleaseTask (void *dummy)
{
    instance_get()->pd_autoreleaseCount++;
    
    {
        int n = (instance_get()->pd_autoreleaseCount & 1ULL);
    
        instance_autoreleaseDrainProceed (n ? sym__autoreleaseB : sym__autoreleaseA);
    }
    
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
    instance_autoreleaseDrain(); clock_free (instance_get()->pd_autorelease);
    instance_get()->pd_autorelease = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void instance_autoreleaseRegister (t_pd *x)
{
    int n = (instance_get()->pd_autoreleaseCount & 1ULL);
    
    pd_bind (x, n ? sym__autoreleaseB : sym__autoreleaseA);

    /* While quitting the application. */
    
    if (!instance_get()->pd_autorelease) { instance_autoreleaseDrain(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Function that might be called at last by registered object while autoreleased. */

void instance_autoreleaseProceed (t_pd *x)
{
    pd_unbindQuiet (x, sym__autoreleaseA);
    pd_unbindQuiet (x, sym__autoreleaseB);
    pd_free (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define POLLING_PERIOD  47.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void startup_openFilesTemplates (void);
void startup_openFilesPended (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void instance_pollingTask (void *dummy)
{
    if (symbol_hasThingQuiet (sym__polling)) {
        pd_message (symbol_getThing (sym__polling), sym__polling, 0, NULL);
    }  
    
    instance_get()->pd_pollingCount++;
    
    if (instance_get()->pd_pollingCount == 10) {
        startup_openFilesTemplates();
        startup_openFilesPended();
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
