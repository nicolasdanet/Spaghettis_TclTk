
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCHEDULER_RUN               0
#define SCHEDULER_QUIT              1
#define SCHEDULER_RESTART           2
#define SCHEDULER_ERROR             3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCHEDULER_BLOCKING_LAPSE    1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile sig_atomic_t scheduler_quit;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      scheduler_audioMode;                    /* Shared. */
static double   scheduler_realTime;                     /* Shared. */
static double   scheduler_logicalTime;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WATCHDOG 
#if PD_WITH_NOGUI

static int      scheduler_didDSP;                       /* Shared. */
static int      scheduler_nextPing;                     /* Shared. */

#endif
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_systime scheduler_getLogicalTime (void)
{
    return pd_this->pd_systime;
}

t_systime scheduler_getLogicalTimeAfter (double ms)
{
    return (pd_this->pd_systime + (SYSTIME_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (t_systime systime, double unit, int isSamples)
{
    double d;
    t_systime elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SYSTIME_PER_SECOND / audio_getSampleRate(); } 
    else { 
        d = SYSTIME_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (t_systime systime)
{
    t_systime elapsed = pd_this->pd_systime - systime;
    
    return (elapsed / SYSTIME_PER_MILLISECOND);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scheduler_setAudioMode (int flag)
{
    scheduler_audioMode = flag;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

static t_systime scheduler_getSystimePerDSPTick (void)
{
    return (SYSTIME_PER_SECOND * ((double)INTERNAL_BLOCKSIZE / audio_getSampleRate()));
}

static void scheduler_pollWatchdog (void)
{
    #if PD_WATCHDOG
    #if PD_WITH_NOGUI
        
    if ((scheduler_didDSP - scheduler_nextPing) > 0) {
    //
    interface_watchdog (NULL);
    scheduler_nextPing = scheduler_didDSP + (2 * (int)(audio_getSampleRate() / (double)INTERNAL_BLOCKSIZE));
    //
    }
    
    #endif
    #endif
}

static void scheduler_pollStuck (int init)
{
    static double idleTime;
    
    if (init) { idleTime = sys_getRealTimeInSeconds(); }
    else {
        if (sys_getRealTimeInSeconds() - idleTime > 1.0) {
            audio_close();
            scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
            if (!scheduler_quit) { scheduler_quit = SCHEDULER_RESTART; }
            error_ioStuck();
        }
    }
}

static void scheduler_tick (void)
{
    t_systime nextSystime = pd_this->pd_systime + scheduler_getSystimePerDSPTick();
    
    while (pd_this->pd_clocks && pd_this->pd_clocks->c_systime < nextSystime) {
    //
    t_clock *c = pd_this->pd_clocks;
    pd_this->pd_systime = c->c_systime;
    clock_unset (c);
    (*c->c_fn)(c->c_owner);
    if (scheduler_quit) { return; }
    //
    }
    
    pd_this->pd_systime = nextSystime;
    
    ugen_tick();
    
    #if PD_WATCHDOG
    #if PD_WITH_NOGUI
    
    scheduler_didDSP++;
        
    #endif
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scheduler_mainLoop (void)
{
    int idleCount = 0;
    
    double realTimeAtStart = sys_getRealTimeInSeconds();
    t_systime logicalTimeAtStart = scheduler_getLogicalTime();
    
    midi_start();
    
    while (!scheduler_quit) {
    //
    int timeForward, didSomething = 0;

    if (scheduler_audioMode != SCHEDULER_AUDIO_NONE) {
        if ((timeForward = audio_pollDSP())) { idleCount = 0; }
        else {
            if (!(++idleCount % 31)) { 
                scheduler_pollStuck (idleCount == 32);
            }
        }
        
    } else {
        double realLapse = SECONDS_TO_MILLISECONDS (sys_getRealTimeInSeconds() - realTimeAtStart);
        double logicalLapse = scheduler_getMillisecondsSince (logicalTimeAtStart);

        if (realLapse > logicalLapse) { timeForward = DACS_YES; }
        else {
            timeForward = DACS_NO;
        }
    }
    
    if (!scheduler_quit) {
    //
    midi_synchronise();
    
    if (timeForward != DACS_NO)  { scheduler_tick(); }
    if (timeForward == DACS_YES) { didSomething = 1; }

    midi_poll();
    
    if (!scheduler_quit && interface_pollOrFlushGui()) { didSomething = 1; }
    if (!scheduler_quit && !didSomething) {
        scheduler_pollWatchdog();
        if (timeForward != DACS_SLEPT) {
            interface_monitorBlocking (SCHEDULER_BLOCKING_LAPSE);
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
    autorelease_run();
    scheduler_mainLoop();
    autorelease_stop();
    audio_close();
    midi_close();
    
    return (scheduler_quit == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
