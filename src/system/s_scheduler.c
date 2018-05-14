
/* Copyright (c) 1997-2018 Miller Puckette and others. */

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
    SCHEDULER_RUN       = 0,
    SCHEDULER_QUIT      = 1,
    SCHEDULER_RESTART   = 2,
    SCHEDULER_ERROR     = 3
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile sig_atomic_t scheduler_quit;            /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int          scheduler_audioState;               /* Static. */
static t_systime    scheduler_systime;                  /* Static. */

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

/* A little less than the time spent by one audio vector. */
/* Required to slow down audio computation in case of a large vector size. */
/* Without, regularity of clocks would be very bad. */

double scheduler_getTimeToWaitInMilliseconds (void)
{
    return (1.25 * AUDIO_DEFAULT_SAMPLERATE / audio_getSampleRate());
}

static t_systime scheduler_getSystimePerDSPTick (void)
{
    return (SYSTIME_PER_SECOND * ((double)INTERNAL_BLOCKSIZE / audio_getSampleRate()));
}

static void scheduler_pollStuck (int init)
{
    static double idleTime;     /* Static. */
    
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
    
    if (!scheduler_quit) { instance_dspTick(); }
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
    int timeForward;

    if (scheduler_audioState != SCHEDULER_AUDIO_STOP) {
        if ((timeForward = audio_poll())) { idleCount = 0; }
        else {
            idleCount++; if (!(idleCount % 32)) { scheduler_pollStuck (idleCount == 32); }
        }
        
    } else {
        double realLapse = PD_SECONDS_TO_MILLISECONDS (clock_getRealTimeInSeconds() - realTimeAtStart);
        double logicalLapse = scheduler_getMillisecondsSince (logicalTimeAtStart);

        if (realLapse > logicalLapse) { timeForward = DACS_YES; }
        else {
            timeForward = DACS_NO;
        }
    }
    
    if (!scheduler_quit) {
    //
    double start = 0.0;
    
    if (timeForward != DACS_SLEPT) { start = clock_getRealTimeInSeconds(); }
    
    if (timeForward != DACS_NO) { scheduler_tick(); }
    
    if (!scheduler_quit) {
        midi_poll();
        monitor_nonBlocking();
        gui_flush();
    }
    
    if (!scheduler_quit) {
    //
    if (timeForward != DACS_SLEPT) {    /* It almost never happens with a small vector size. */
    //
    double elapsed = PD_SECONDS_TO_MILLISECONDS (clock_getRealTimeInSeconds() - start);
    double monitor = 1.0;
        
    if (scheduler_audioState != SCHEDULER_AUDIO_STOP) { monitor = scheduler_getTimeToWaitInMilliseconds(); }
    
    monitor -= elapsed; monitor = PD_MAX (0.0, monitor);
    
    monitor_blocking (monitor);
    //
    }
    //
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
