
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

#include <pthread.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Notice that values below are related to LCM (32000, 44100, 48000, 88200, 96000). */
    
#define SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND    (double)(32.0 * 441.0)
#define SCHEDULER_SYSTIME_CLOCKS_PER_SECOND         (SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND * 1000.0)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SCHEDULER_QUIT      1
#define SCHEDULER_RESTART   2

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

extern int      sys_nogui;
extern int      sys_hipriority;
extern int      sys_schedadvance;
extern t_float  sys_dacsr;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int      scheduler_quit;                                             /* Shared. */
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

typedef void (*t_clockfn)(void *client);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _clock {
    double          c_systime;      /* Negative for unset clocks. */
    double          c_unit;         /* A positive value is in ticks, negative for number of samples. */
    t_clockfn       c_fn;
    void            *c_owner;
    struct _clock   *c_next;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_clock *clock_new (void *owner, t_method fn)
{
    t_clock *x = (t_clock *)PD_MEMORY_GET (sizeof (t_clock));
    
    x->c_systime    = -1.0;
    x->c_unit       = SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND;
    x->c_fn         = (t_clockfn)fn;
    x->c_owner      = owner;
    x->c_next       = NULL;

    return x;
}

void clock_free (t_clock *x)
{
    clock_unset (x);
    
    PD_MEMORY_FREE (x, sizeof (t_clock));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void clock_unset (t_clock *x)
{
    if (x->c_systime >= 0.0) {
        if (x == pd_this->pd_clocks) { pd_this->pd_clocks = x->c_next; }
        else {
            t_clock *c = pd_this->pd_clocks;
            while (c->c_next != x) { c = c->c_next; } c->c_next = x->c_next;
        }
        x->c_systime = -1.0;
    }
}

static void clock_set (t_clock *x, double time)
{
    if (time < pd_this->pd_systime) { time = pd_this->pd_systime; }
    
    clock_unset (x);
    
    x->c_systime = time;
    
    if (pd_this->pd_clocks && pd_this->pd_clocks->c_systime <= time) {
    
        t_clock *m = NULL;
        t_clock *n = NULL;
        
        for (m = pd_this->pd_clocks, n = pd_this->pd_clocks->c_next; m; m = n, n = m->c_next) {
            if (!n || n->c_systime > time) {
                m->c_next = x; x->c_next = n; return;
            }
        }
        
    } else {
        x->c_next = pd_this->pd_clocks; pd_this->pd_clocks = x;
    }
}

void clock_delay (t_clock *x, double delay)     /* Could be in milliseconds or in samples. */
{
    double d, time;
    
    if (x->c_unit > 0) { d = x->c_unit; }
    else {
        d = -(x->c_unit * (SCHEDULER_SYSTIME_CLOCKS_PER_SECOND / sys_dacsr));
    }

    time = pd_this->pd_systime + (d * delay);
    
    clock_set (x, time);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void clock_setUnit (t_clock *x, double unit, int isSamples)
{
    double timeLeft = -1.0;
    
    if (unit <= 0.0) { unit = 1.0; }
    
    if (isSamples) { if (unit == -x->c_unit) { return; } }
    else { 
        if (unit == x->c_unit * SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND) { return; }
    }
    
    if (x->c_systime >= 0.0) {
    //
    double d = (x->c_unit > 0) ? x->c_unit : (x->c_unit * (SCHEDULER_SYSTIME_CLOCKS_PER_SECOND / sys_dacsr));
    timeLeft = (x->c_systime - pd_this->pd_systime) / d;
    //
    }
    
    if (isSamples) { x->c_unit = -unit; }
    else {
        x->c_unit = unit * SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND; 
    }
    
    if (timeLeft >= 0.0) { clock_delay (x, timeLeft); }
}

void clock_setUnitAsSamples (t_clock *x, double samples) 
{
    clock_setUnit (x, samples, 1);
}

void clock_setUnitAsMilliseconds (t_clock *x, double ms) 
{
    clock_setUnit (x, ms, 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

double scheduler_getSystime (void)
{
    return pd_this->pd_systime;
}

double scheduler_getSystimeAfter (double ms)
{
    return (pd_this->pd_systime + (SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (double systime, double unit, int isSamples)
{
    double d, elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SCHEDULER_SYSTIME_CLOCKS_PER_SECOND / sys_dacsr; } 
    else { 
        d = SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (double systime)
{
    double elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    return (elapsed / SCHEDULER_SYSTIME_CLOCKS_PER_MILLISECOND);
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

static double scheduler_getSystimePerDSPTick (void)
{
    return (SCHEDULER_SYSTIME_CLOCKS_PER_SECOND * ((double)scheduler_blockSize / sys_dacsr));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

static void scheduler_pollWatchdog (void)
{
    # if PD_WITH_WATCHDOG
    
    if (sys_nogui && sys_hipriority && (scheduler_didDSP - scheduler_nextPing > 0)) {
        global_watchdog (NULL);
        scheduler_nextPing = scheduler_didDSP + (2 * (int)(sys_dacsr /(double)scheduler_blockSize));
    }
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scheduler_needToRestart (void)
{
    scheduler_quit = SCHEDULER_RESTART;
}

void scheduler_setAudio (int flag)
{
    PD_ASSERT (flag != SCHEDULER_AUDIO_CALLBACK);           /* Not fully implemented yet. */
    PD_ABORT  (flag == SCHEDULER_AUDIO_CALLBACK);

    if (flag == SCHEDULER_AUDIO_NONE) {
    //
    scheduler_realTime = sys_getrealtime();
    scheduler_logicalTime = scheduler_getSystime();
    //
    }
    
    scheduler_audioMode = flag;
    scheduler_systimePerDSPTick = scheduler_getSystimePerDSPTick();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/*
Here is Pd's "main loop."  This routine dispatches clock timeouts and DSP
"ticks" deterministically, and polls for input from MIDI and the GUI.  If
we're left idle we also poll for graphics updates; but these are considered
lower priority than the rest.

The time source is normally the audio I/O subsystem via the "sys_send_dacs()"
call.  This call returns true if samples were transferred; false means that
the audio I/O system is still busy with previous transfers.
*/

static void m_pollingscheduler( void)
{
    int idlecount = 0;
    scheduler_systimePerDSPTick = (SCHEDULER_SYSTIME_CLOCKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;

        SCHEDULER_LOCK;

    if (scheduler_sleepGrain < 100)
        scheduler_sleepGrain = sys_schedadvance/4;
    if (scheduler_sleepGrain < 100)
        scheduler_sleepGrain = 100;
    else if (scheduler_sleepGrain > 5000)
        scheduler_sleepGrain = 5000;
    sys_initmidiqueue();
    while (!scheduler_quit)
    {
        int didsomething = 0;
        int timeforward;

    waitfortick:
        if (scheduler_audioMode != SCHEDULER_AUDIO_NONE)
        {
            /* T.Grill - send_dacs may sleep -> 
                unlock thread lock make that time available 
                - could messaging do any harm while sys_send_dacs is running?
            */
            SCHEDULER_UNLOCK;
            timeforward = sys_send_dacs();

            /* T.Grill - done */
            SCHEDULER_LOCK;

                /* if dacs remain "idle" for 1 sec, they're hung up. */
            if (timeforward != 0)
                idlecount = 0;
            else
            {
                idlecount++;
                if (!(idlecount & 31))
                {
                    static double idletime;
                    if (scheduler_audioMode != SCHEDULER_AUDIO_POLL)
                    {
                            PD_BUG;
                            return;
                    }
                        /* on 32nd idle, start a clock watch;  every
                        32 ensuing idles, check it */
                    if (idlecount == 32)
                        idletime = sys_getrealtime();
                    else if (sys_getrealtime() - idletime > 1.)
                    {
                        post_error ("audio I/O stuck... closing audio\n");
                        sys_close_audio();
                        scheduler_setAudio(SCHEDULER_AUDIO_NONE);
                        goto waitfortick;
                    }
                }
            }
        }
        else
        {
            if (1000. * (sys_getrealtime() - scheduler_realTime)
                > scheduler_getMillisecondsSince(scheduler_logicalTime))
                    timeforward = DACS_YES;
            else timeforward = DACS_NO;
        }
        sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
        if (timeforward != DACS_NO)
            scheduler_tick();
        if (timeforward == DACS_YES)
            didsomething = 1;

        sys_pollmidiqueue();
        if (sys_pollgui())
        {
            if (!didsomething) {}
                //sched_didpoll++;
            didsomething = 1;
        }
            /* test for idle; if so, do graphics updates. */
        if (!didsomething)
        {
            scheduler_pollWatchdog();

            SCHEDULER_UNLOCK;   /* unlock while we idle */

            if (timeforward != DACS_SLEPT) { sys_microsleep(scheduler_sleepGrain); }

            SCHEDULER_LOCK;

            //sched_didnothing++;
        }
    }

    SCHEDULER_UNLOCK;
}

void sched_audio_callbackfn(void)
{
    scheduler_lock();
    sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
    scheduler_tick();
    sys_pollmidiqueue();
    sys_pollgui();
    scheduler_pollWatchdog();
    scheduler_unlock();
}

static void m_callbackscheduler(void)
{
    sys_initmidiqueue();
    while (!scheduler_quit)
    {
        double timewas = pd_this->pd_systime;
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
        if (pd_this->pd_systime == timewas)
        {
            scheduler_lock();
            sys_pollgui();
            scheduler_tick();
            scheduler_unlock();
        }
    }
}

int m_mainloop(void)
{
    while (scheduler_quit != SCHEDULER_QUIT)
    {
        if (scheduler_audioMode == SCHEDULER_AUDIO_CALLBACK)
            m_callbackscheduler();
        else m_pollingscheduler();
        if (scheduler_quit == SCHEDULER_RESTART)
        {
            scheduler_quit = 0;
            if (audio_isopen())
            {
                sys_close_audio();
                sys_reopen_audio();
            }
        }
    }
    return (0);
}

int m_batchmain(void)
{
    scheduler_systimePerDSPTick = (SCHEDULER_SYSTIME_CLOCKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;
    while (scheduler_quit != SCHEDULER_QUIT)
        scheduler_tick();
    return (0);
}

void sys_exit(void)
{
    scheduler_quit = SCHEDULER_QUIT;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
