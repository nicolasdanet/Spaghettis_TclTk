
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCHEDULER_BLOCKING_LAPSE    1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    SCHEDULER_RUN       = 0,
    SCHEDULER_QUIT      = 1,
    SCHEDULER_RESTART   = 2,
    SCHEDULER_ERROR     = 3
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile sig_atomic_t scheduler_quit;            /* Global. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          scheduler_audioState;               /* Global. */
static t_systime    scheduler_systime;                  /* Global. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_setLogicalTime (t_systime t)
{
    scheduler_systime = t;
}

t_systime scheduler_getLogicalTime (void)
{
    return scheduler_systime;
}

t_systime scheduler_getLogicalTimeAfter (double ms)
{
    return (scheduler_getLogicalTime() + (SYSTIME_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (t_systime systime, double unit, int isSamples)
{
    double d;
    t_systime elapsed = scheduler_getLogicalTime() - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SYSTIME_PER_SECOND / audio_getSampleRate(); } 
    else { 
        d = SYSTIME_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (t_systime systime)
{
    t_systime elapsed = scheduler_getLogicalTime() - systime;
    
    return (elapsed / SYSTIME_PER_MILLISECOND);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_setAudioState (int state)
{
    scheduler_audioState = state;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_needToExit (void)
{
    scheduler_quit = SCHEDULER_QUIT;
}

void scheduler_needToExitWithError (void)
{
    scheduler_quit = SCHEDULER_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_systime scheduler_getSystimePerDSPTick (void)
{
    return (SYSTIME_PER_SECOND * ((double)INTERNAL_BLOCKSIZE / audio_getSampleRate()));
}

static void scheduler_pollStuck (int init)
{
    static double idleTime;
    
    if (init) { idleTime = clock_getRealTimeInSeconds(); }
    else {
        if (clock_getRealTimeInSeconds() - idleTime > 1.0) {
            audio_close();
            scheduler_setAudioState (SCHEDULER_AUDIO_STOP);
            if (!scheduler_quit) { scheduler_quit = SCHEDULER_RESTART; }
            error_ioStuck();
        }
    }
}

static void scheduler_tick (void)
{
    if (!scheduler_quit) { 
    //
    t_systime t = scheduler_getLogicalTime() + scheduler_getSystimePerDSPTick();
    
    instance_clockTick (t);
    
    scheduler_setLogicalTime (t);
    //
    }
    
    if (!scheduler_quit) { ugen_dspTick(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scheduler_mainLoop (void)
{
    int idleCount = 0;
    
    double realTimeAtStart = clock_getRealTimeInSeconds();
    t_systime logicalTimeAtStart = scheduler_getLogicalTime();
    
    midi_start();
    
    while (!scheduler_quit) {
    //
    int timeForward, didSomething = 0;

    if (scheduler_audioState != SCHEDULER_AUDIO_STOP) {
        if ((timeForward = audio_poll())) { idleCount = 0; }
        else {
            if (!(++idleCount % 31)) { 
                scheduler_pollStuck (idleCount == 32);
            }
        }
        
    } else {
        double realLapse = SECONDS_TO_MILLISECONDS (clock_getRealTimeInSeconds() - realTimeAtStart);
        double logicalLapse = scheduler_getMillisecondsSince (logicalTimeAtStart);

        if (realLapse > logicalLapse) { timeForward = DACS_YES; }
        else {
            timeForward = DACS_NO;
        }
    }
    
    if (!scheduler_quit) {
    //
    if (timeForward != DACS_NO)  { scheduler_tick(); }
    if (timeForward == DACS_YES) { didSomething = 1; }

    midi_poll();
    
    if (!scheduler_quit && (monitor_nonBlocking() || gui_flush())) { didSomething = 1; }
    if (!scheduler_quit && !didSomething) {
        if (timeForward != DACS_SLEPT) {
            monitor_blocking (SCHEDULER_BLOCKING_LAPSE);
        }
    }
    //
    }
    
    if (scheduler_quit == SCHEDULER_RESTART) { scheduler_quit = SCHEDULER_RUN; }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error scheduler_main (void)
{
    midi_open();
    
        instance_autoreleaseRun();
        instance_pollingRun();
    
            scheduler_mainLoop();
    
        instance_pollingStop();
        instance_autoreleaseStop();
    
    dsp_suspend();
    audio_close();
    midi_close();
    
    return (scheduler_quit == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
