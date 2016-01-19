
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <pthread.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Notice that values below are related to LCM (32000, 44100, 48000, 88200, 96000). */
    
#define SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND     (double)(32.0 * 441.0)
#define SCHEDULER_SYSTIME_TICKS_PER_SECOND          (SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND * 1000.0)

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
static int      scheduler_didDSP;                                           /* Shared. */
static int      scheduler_sleepGrain;                                       /* Shared. */
static int      scheduler_blockSize     = AUDIO_DEFAULT_BLOCK;              /* Shared. */
static int      scheduler_useAudio      = SCHEDULER_NONE;                   /* Shared. */

static double   scheduler_realTime;                                         /* Shared. */
static double   scheduler_logicalTime;                                      /* Shared. */
static double   scheduler_systimePerDSPTick;                                /* Shared. */

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
    x->c_unit       = SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND;
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
        d = -(x->c_unit * (SCHEDULER_SYSTIME_TICKS_PER_SECOND / sys_dacsr));
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
        if (unit == x->c_unit * SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND) { return; }
    }
    
    if (x->c_systime >= 0.0) {
    //
    double d = (x->c_unit > 0) ? x->c_unit : (x->c_unit * (SCHEDULER_SYSTIME_TICKS_PER_SECOND / sys_dacsr));
    timeLeft = (x->c_systime - pd_this->pd_systime) / d;
    //
    }
    
    if (isSamples) { x->c_unit = -unit; }
    else {
        x->c_unit = unit * SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND; 
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
    return (pd_this->pd_systime + (SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND * ms));
}

double scheduler_getUnitsSince (double systime, double unit, int isSamples)
{
    double d, elapsed = pd_this->pd_systime - systime;
    
    PD_ASSERT (elapsed >= 0.0);
    
    if (isSamples) { d = SCHEDULER_SYSTIME_TICKS_PER_SECOND / sys_dacsr; } 
    else { 
        d = SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND;
    }
    
    return (elapsed / (d * unit));
}

double scheduler_getMillisecondsSince (double systime)
{
    double elapsed = pd_this->pd_systime - systime;
    PD_ASSERT (elapsed >= 0.0);
    return (elapsed / SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND);
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

static void sched_pollformeters(void)
{
    int inclip, outclip, indb, outdb;
    static int sched_nextmeterpolltime, sched_nextpingtime;

        /* if there's no GUI but we're running in "realtime", here is
        where we arrange to ping the watchdog every 2 seconds. */
#if defined(__linux__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__GNU__)
    if (sys_nogui && sys_hipriority && (scheduler_didDSP - sched_nextpingtime > 0))
    {
        global_watchdog(0);
            /* ping every 2 seconds */
        sched_nextpingtime = scheduler_didDSP +
            2 * (int)(sys_dacsr /(double)scheduler_blockSize);
    }
#endif

    if (scheduler_didDSP - sched_nextmeterpolltime < 0)
        return;
    /*
    if (sched_diored && (scheduler_didDSP - sched_dioredtime > 0))
    {
        // sys_vgui("::ui_console::pdtk_pd_dio 0\n");
        sched_diored = 0;
    }*/
    if (0 /* sched_meterson */)
    {
        t_sample inmax, outmax;
        sys_getmeters(&inmax, &outmax);
        indb = 0.5 + rmstodb(inmax);
        outdb = 0.5 + rmstodb(outmax);
        inclip = (inmax > 0.999);
        outclip = (outmax >= 1.0);
    }
    else
    {
        indb = outdb = 0;
        inclip = outclip = 0;
    }/*
    if (inclip != sched_lastinclip || outclip != sched_lastoutclip
        || indb != sched_lastindb || outdb != sched_lastoutdb)
    {
        sys_vgui("pdtk_pd_meters %d %d %d %d\n", indb, outdb, inclip, outclip);
        sched_lastinclip = inclip;
        sched_lastoutclip = outclip;
        sched_lastindb = indb;
        sched_lastoutdb = outdb;
    }*/
    sched_nextmeterpolltime =
        scheduler_didDSP + (int)(sys_dacsr /(double)scheduler_blockSize);
}

void sched_reopenmeplease(void)   /* request from s_audio for deferred reopen */
{
    scheduler_quit = SCHEDULER_RESTART;
}

void sched_set_using_audio(int flag)
{
    scheduler_useAudio = flag;
    if (flag == SCHEDULER_NONE)
    {
        scheduler_realTime = sys_getrealtime();
        scheduler_logicalTime = scheduler_getSystime();
    }
        if (flag == SCHEDULER_CALLBACK &&
            scheduler_useAudio != SCHEDULER_CALLBACK)
                scheduler_quit = SCHEDULER_RESTART;
        if (flag != SCHEDULER_CALLBACK &&
            scheduler_useAudio == SCHEDULER_CALLBACK)
                post("sorry, can't turn off callbacks yet; restart Pd");
                    /* not right yet! */
        
    scheduler_systimePerDSPTick = (SCHEDULER_SYSTIME_TICKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;
    // sys_vgui("::ui_console::pdtk_pd_audio %s\n", flag ? "on" : "off");
}

    /* take the scheduler forward one DSP tick, also handling clock timeouts */
void sched_tick( void)
{
    double next_sys_time = pd_this->pd_systime + scheduler_systimePerDSPTick;
    int countdown = 5000;
    while (pd_this->pd_clocks && 
        pd_this->pd_clocks->c_systime < next_sys_time)
    {
        t_clock *c = pd_this->pd_clocks;
        pd_this->pd_systime = c->c_systime;
        clock_unset(pd_this->pd_clocks);
        (*c->c_fn)(c->c_owner);
        if (!countdown--)
        {
            countdown = 5000;
            sys_pollgui();
        }
        if (scheduler_quit)
            return;
    }
    pd_this->pd_systime = next_sys_time;
    dsp_tick();
    scheduler_didDSP++;
}

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
    scheduler_systimePerDSPTick = (SCHEDULER_SYSTIME_TICKS_PER_SECOND) *
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
        if (scheduler_useAudio != SCHEDULER_NONE)
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
                    if (scheduler_useAudio != SCHEDULER_POLL)
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
                        sched_set_using_audio(SCHEDULER_NONE);
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
            sched_tick();
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
            sched_pollformeters();

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
    sched_tick();
    sys_pollmidiqueue();
    sys_pollgui();
    sched_pollformeters();
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
            sched_tick();
            scheduler_unlock();
        }
    }
}

int m_mainloop(void)
{
    while (scheduler_quit != SCHEDULER_QUIT)
    {
        if (scheduler_useAudio == SCHEDULER_CALLBACK)
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
    scheduler_systimePerDSPTick = (SCHEDULER_SYSTIME_TICKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;
    while (scheduler_quit != SCHEDULER_QUIT)
        sched_tick();
    return (0);
}

void sys_exit(void)
{
    scheduler_quit = SCHEDULER_QUIT;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
