
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
#define SCHEDULER_ERROR     4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_LOCK

#define SCHEDULER_LOCK      scheduler_lock()
#define SCHEDULER_UNLOCK    scheduler_unlock()
    
#else
    
#define SCHEDULER_LOCK
#define SCHEDULER_UNLOCK
    
#endif // PD_WITH_LOCK

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int      sys_schedadvance;
extern t_float  sys_dacsr;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static volatile sig_atomic_t scheduler_quit;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      scheduler_sleepGrain;                                       /* Shared. */
static int      scheduler_blockSize     = AUDIO_DEFAULT_BLOCK;              /* Shared. */
static int      scheduler_audioMode     = SCHEDULER_AUDIO_NONE;             /* Shared. */

static double   scheduler_realTime;                                         /* Shared. */
static double   scheduler_logicalTime;                                      /* Shared. */
static double   scheduler_systimePerDSPTick;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_WATCHDOG 

static int      scheduler_didDSP;                                           /* Shared. */
static int      scheduler_nextPing;                                         /* Shared. */

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static pthread_mutex_t sys_mutex = PTHREAD_MUTEX_INITIALIZER;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

double scheduler_getSystime (void)
{
    return pd_this->pd_systime;
}

double scheduler_getSystimeAfter (double ms)
{
    return (pd_this->pd_systime + (SYSTIME_CLOCKS_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (double systime, double unit, int isSamples)
{
    double d, elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SYSTIME_CLOCKS_PER_SECOND / sys_dacsr; } 
    else { 
        d = SYSTIME_CLOCKS_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (double systime)
{
    double elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    return (elapsed / SYSTIME_CLOCKS_PER_MILLISECOND);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scheduler_lock (void)
{
    pthread_mutex_lock (&sys_mutex);
}

void scheduler_unlock (void)
{
    pthread_mutex_unlock (&sys_mutex);
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
    return (SYSTIME_CLOCKS_PER_SECOND * ((double)scheduler_blockSize / sys_dacsr));
}

static void scheduler_pollWatchdog (void)
{
    #if PD_WITH_WATCHDOG
    #if PD_WITH_NOGUI
    #if PD_WITH_REALTIME
        
    if ((scheduler_didDSP - scheduler_nextPing) > 0) {
        global_watchdog (NULL);
        scheduler_nextPing = scheduler_didDSP + (2 * (int)(sys_dacsr / (double)scheduler_blockSize));
    }
    
    #endif
    #endif
    #endif
}

static void scheduler_pollStuck (int init)
{
    static double idleTime;
    
    if (init) { idleTime = sys_getRealTime(); }
    else {
    //
    if (sys_getRealTime() - idleTime > 1.0) {
        sys_close_audio();
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
    
    dsp_tick();
    
    #if PD_WITH_WATCHDOG
    
    scheduler_didDSP++;
        
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scheduler_withLoop (void)
{
    double realTime, logicalTime;
    int idleCount = 0;
        
    if (scheduler_audioMode == SCHEDULER_AUDIO_NONE) {
    //
    realTime = sys_getRealTime();
    logicalTime = scheduler_getSystime();
    //
    }
    
    scheduler_sleepGrain = PD_CLAMP (sys_schedadvance / 4, 100, 5000);
    scheduler_systimePerDSPTick = scheduler_getSystimePerDSPTick();

    sys_initmidiqueue();
    
    SCHEDULER_LOCK;
    
    while (!scheduler_quit) {
    //
    int timeForward, didSomething = 0;

    if (scheduler_audioMode != SCHEDULER_AUDIO_NONE) {

        SCHEDULER_UNLOCK;
        timeForward = sys_send_dacs();
        SCHEDULER_LOCK;

        if (timeForward) { idleCount = 0; }
        else if (!(++idleCount % 31)) { scheduler_pollStuck (idleCount == 32); }
        
    } else {
    
        double realElapsed = (sys_getRealTime() - realTime) * 1000.0;
        double logicalElapsed = scheduler_getMillisecondsSince (logicalTime);

        if (realElapsed > logicalElapsed) { timeForward = DACS_YES; }
        else {
            timeForward = DACS_NO;
        }
    }
    
    if (!scheduler_quit) {
    //
    sys_setmiditimediff (0.0, 1e-6 * sys_schedadvance);
    
    if (timeForward != DACS_NO)  { scheduler_tick(); }
    if (timeForward == DACS_YES) { didSomething = 1; }

    sys_pollmidiqueue();
    
    if (!scheduler_quit && sys_pollgui()) { didSomething = 1; }

    if (!scheduler_quit && !didSomething) {
    //
    scheduler_pollWatchdog();

    SCHEDULER_UNLOCK;
    if (timeForward != DACS_SLEPT) { interface_socketPollBlocking (scheduler_sleepGrain); }
    SCHEDULER_LOCK;
    //
    }
    //
    }
    //
    }

    SCHEDULER_UNLOCK;
}

static void scheduler_withCallback (void)
{
    sys_initmidiqueue();
    
    while (!scheduler_quit) {
    //
    double logicalTime = pd_this->pd_systime;
    
    #if PD_WINDOWS
        Sleep (1000);
    #else
        sleep (1);
    #endif
    
    if (!scheduler_quit && (pd_this->pd_systime == logicalTime)) {
    //
    SCHEDULER_LOCK;
    
    sys_pollgui();
    scheduler_tick();
    
    SCHEDULER_UNLOCK;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scheduler_audioCallback (void)
{
    SCHEDULER_LOCK;
    
    sys_setmiditimediff (0.0, 1e-6 * sys_schedadvance);
    scheduler_tick();
    sys_pollmidiqueue();
    sys_pollgui();
    scheduler_pollWatchdog();
    
    SCHEDULER_UNLOCK;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error scheduler_main (void)
{
    while (scheduler_quit != SCHEDULER_QUIT) {
    //
    if (scheduler_audioMode == SCHEDULER_AUDIO_CALLBACK) { scheduler_withCallback(); }
    else {
        scheduler_withLoop();
    }
    
    if (scheduler_quit == SCHEDULER_RESTART) {
        if (audio_isopen()) { sys_close_audio(); sys_reopen_audio(); } scheduler_quit = SCHEDULER_RUN;
        
    } else {
        sys_close_audio();
        sys_close_midi();
    }
    //
    }
    
    return (scheduler_quit == SCHEDULER_ERROR);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
