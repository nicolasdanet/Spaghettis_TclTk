
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
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

#define SCHEDULER_RUN       0
#define SCHEDULER_QUIT      1
#define SCHEDULER_RESTART   2
#define SCHEDULER_ERROR     3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int      audio_advanceInMicroseconds;
extern t_float  audio_sampleRate;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile sig_atomic_t scheduler_quit;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      scheduler_sleepGrain;                   /* Shared. */
static int      scheduler_audioMode;                    /* Shared. */

static double   scheduler_realTime;                     /* Shared. */
static double   scheduler_logicalTime;                  /* Shared. */
static double   scheduler_systimePerDSPTick;            /* Shared. */

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

double scheduler_getLogicalTime (void)
{
    return pd_this->pd_systime;
}

double scheduler_getLogicalTimeAfter (double ms)
{
    return (pd_this->pd_systime + (SYSTIME_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (double systime, double unit, int isSamples)
{
    double d, elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SYSTIME_PER_SECOND / audio_sampleRate; } 
    else { 
        d = SYSTIME_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (double systime)
{
    double elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    return (elapsed / SYSTIME_PER_MILLISECOND);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scheduler_setAudioMode (int flag)
{
    PD_ASSERT (flag != SCHEDULER_AUDIO_CALLBACK);           /* Not fully tested yet. */
    PD_ABORT  (flag == SCHEDULER_AUDIO_CALLBACK);

    scheduler_audioMode = flag;
}

void scheduler_needToRestart (void)
{
    scheduler_quit = SCHEDULER_RESTART;
}

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

static double scheduler_getSystimePerDSPTick (void)
{
    return (SYSTIME_PER_SECOND * ((double)INTERNAL_BLOCKSIZE / audio_sampleRate));
}

static void scheduler_pollWatchdog (void)
{
    #if PD_WATCHDOG
    #if PD_WITH_NOGUI
        
    if ((scheduler_didDSP - scheduler_nextPing) > 0) {
    //
    interface_watchdog (NULL);
    scheduler_nextPing = scheduler_didDSP + (2 * (int)(audio_sampleRate / (double)INTERNAL_BLOCKSIZE));
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
    //
    if (sys_getRealTimeInSeconds() - idleTime > 1.0) {
        audio_close();
        scheduler_setAudioMode (SCHEDULER_AUDIO_NONE);
        scheduler_quit = SCHEDULER_RESTART;
        post_error (PD_TRANSLATE ("audio: I/O stuck"));     // --
    }
    //
    }
}

static void scheduler_tick (void)
{
    double nextSystime = pd_this->pd_systime + scheduler_systimePerDSPTick;
    
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

static void scheduler_withLoop (void)
{
    int idleCount = 0;
    
    double realTimeAtStart    = sys_getRealTimeInSeconds();
    double logicalTimeAtStart = scheduler_getLogicalTime();
    
    scheduler_sleepGrain = PD_CLAMP (audio_advanceInMicroseconds / 4, 100, 5000);
    scheduler_systimePerDSPTick = scheduler_getSystimePerDSPTick();

    midi_initialize();
    
    while (!scheduler_quit) {
    //
    int timeForward, didSomething = 0;

    if (scheduler_audioMode != SCHEDULER_AUDIO_NONE) {

        timeForward = audio_pollDSP();

        if (timeForward) { idleCount = 0; }
        else if (!(++idleCount % 31)) { scheduler_pollStuck (idleCount == 32); }
        
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
    //
    scheduler_pollWatchdog();

    if (timeForward != DACS_SLEPT) { interface_monitorBlocking (scheduler_sleepGrain); }
    //
    }
    //
    }
    //
    }
}

static void scheduler_withCallback (void)
{
    midi_initialize();
    
    while (!scheduler_quit) {
    //
    #if PD_WINDOWS
        Sleep (1000);
    #else
        sleep (1);
    #endif
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scheduler_audioCallback (void)
{
    midi_synchronise();
    scheduler_tick();
    midi_poll();
    interface_pollOrFlushGui();
    scheduler_pollWatchdog();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error scheduler_main (void)
{
    while (!scheduler_quit) {
    //
    if (scheduler_audioMode == SCHEDULER_AUDIO_CALLBACK) { scheduler_withCallback(); }
    else {
        scheduler_withLoop();
    }
    
    if (scheduler_quit == SCHEDULER_RESTART) {
        audio_close(); audio_open(); scheduler_quit = SCHEDULER_RUN;
    } 
    
    if (scheduler_quit == SCHEDULER_ERROR) { audio_close(); midi_close(); }
    //
    }
    
    return (scheduler_quit == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
