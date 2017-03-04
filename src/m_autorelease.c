
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define AUTORELEASE_PERIOD  SECONDS_TO_MILLISECONDS (5.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void autorelease_task (void *dummy)
{
    autorelease_drain();
    clock_delay (pd_this->pd_autorelease, AUTORELEASE_PERIOD);
}

static void autorelease_reschedule (void)
{
    clock_unset (pd_this->pd_autorelease); 
    clock_delay (pd_this->pd_autorelease, AUTORELEASE_PERIOD);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void autorelease_run (void)
{
    pd_this->pd_autorelease = clock_new ((void *)NULL, (t_method)autorelease_task);
    clock_delay (pd_this->pd_autorelease, AUTORELEASE_PERIOD);
}

void autorelease_stop (void)
{
    autorelease_drain();
    clock_free (pd_this->pd_autorelease); pd_this->pd_autorelease = NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void autorelease_drain (void)
{
    if (pd_isThingQuiet (sym__autorelease)) {
    //
    pd_message (pd_getThing (sym__autorelease), sym__autorelease, 0, NULL);
    //
    }   
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void autorelease_add (t_pd *x)
{
    pd_bind (x, sym__autorelease);
    
    if (!pd_this->pd_autorelease) { autorelease_drain(); }      /* While quitting the application. */
    else {
        autorelease_reschedule(); 
    }
}

void autorelease_proceed (t_pd *x)
{
    pd_unbind (x, sym__autorelease); pd_free (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
