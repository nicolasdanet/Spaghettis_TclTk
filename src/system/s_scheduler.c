
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"
#include "../d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    SCHEDULER_RUN   = 0,
    SCHEDULER_QUIT  = 1,
    SCHEDULER_ERROR = 2
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_int32Atomic    scheduler_quit;             /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_float64Atomic  scheduler_systime;          /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCHEDULER_JOB   20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_setLogicalTime (t_systime t)
{
    PD_ATOMIC_FLOAT64_WRITE (t, &scheduler_systime);
}

t_systime scheduler_getLogicalTime (void)
{
    return PD_ATOMIC_FLOAT64_READ (&scheduler_systime);
}

t_systime scheduler_getLogicalTimeAfter (double ms)
{
    return (scheduler_getLogicalTime() + ms);
}

double scheduler_getMillisecondsSince (t_systime systime)
{
    return (scheduler_getLogicalTime() - systime);
}

double scheduler_getUnitsSince (t_systime systime, double unit, int isSamples)
{
    double d = 1.0; double elapsed = scheduler_getMillisecondsSince (systime);
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = 1000.0 / audio_getSampleRate(); }
    
    return (elapsed / (d * unit));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scheduler_needToExit (void)
{
    PD_ATOMIC_INT32_WRITE (SCHEDULER_QUIT, &scheduler_quit);
}

void scheduler_needToExitWithError (void)
{
    PD_ATOMIC_INT32_WRITE (SCHEDULER_ERROR, &scheduler_quit);
}

int scheduler_isExiting (void)
{
    int n = PD_ATOMIC_INT32_READ (&scheduler_quit); return (n == SCHEDULER_QUIT || n == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scheduler_tick (void)
{
    if (!PD_ATOMIC_INT32_READ (&scheduler_quit)) {
    //
    t_systime t = scheduler_getLogicalTime() + 1.0;
    
    instance_clocksTick (t);
    
    scheduler_setLogicalTime (t);
    //
    }
}

static void scheduler_clean (void)
{
    if (!PD_ATOMIC_INT32_READ (&scheduler_quit) && !audio_isOpened()) {
    //
    instance_clocksClean();
    instance_dspClean();
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int scheduler_mainLoopNeedToTick (double realStart, t_systime logicalStart, double *t)
{
    if (!PD_ATOMIC_INT32_READ (&scheduler_quit)) {
    //
    double now          = clock_getRealTimeInSeconds();
    double realLapse    = PD_SECONDS_TO_MILLISECONDS (now - realStart);
    double logicalLapse = scheduler_getMillisecondsSince (logicalStart);
    
    if (*t == 0.0) { *t = now; }
    
    return (realLapse > logicalLapse);
    //
    }
    
    return 0;
}

static void scheduler_mainLoop (void)
{
    const double realStart       = clock_getRealTimeInSeconds();
    const t_systime logicalStart = scheduler_getLogicalTime();
    uint64_t count = 0;
    
    midi_start();
    
    while (!PD_ATOMIC_INT32_READ (&scheduler_quit)) {
    //
    double t = 0.0;
    
    while (scheduler_mainLoopNeedToTick (realStart, logicalStart, &t)) {
        scheduler_tick();
        midi_poll();
        monitor_nonBlocking();
    }
        
    if (!PD_ATOMIC_INT32_READ (&scheduler_quit)) {
        if (count++ % SCHEDULER_JOB == 0) { gui_jobFlush(); }   // --
        gui_flush();
        scheduler_clean();
    }
    
    if (!PD_ATOMIC_INT32_READ (&scheduler_quit)) {
        double elapsed = PD_SECONDS_TO_MILLISECONDS (clock_getRealTimeInSeconds() - t);
        double monitor = (0.75 - elapsed);
        if (monitor > 0.0) { monitor_blocking (monitor); }
    //
    }
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
    
            scheduler_mainLoop(); dsp_close();
    
        instance_pollingStop();
        instance_autoreleaseStop();
    
    audio_close();
    midi_close();
    
    return (PD_ATOMIC_INT32_READ (&scheduler_quit) == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
