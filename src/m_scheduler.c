
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

#if PD_WITH_LOCK
    #include <pthread.h>
#endif

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

extern int      sys_nogui;
extern int      sys_hipriority;
extern int      sys_schedadvance;
extern t_float  sys_dacsr;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int scheduler_sleepGrain;                            /* Shared. */
static int scheduler_blockSize = AUDIO_DEFAULT_BLOCK;       /* Shared. */
static int scheduler_quit;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_clockfn)(void *client);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _clock {
    double          c_systime;  /* Set as negative for unset clocks. */
    double          c_unit;     /* A positive value is in ticks, a negative value is in number of samples. */
    t_clockfn       c_fn;
    void            *c_owner;
    struct _clock   *c_next;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_clock *clock_new (void *owner, t_method fn)
{
    t_clock *x = (t_clock *)PD_MEMORY_GET (sizeof (t_clock));
    
    x->c_systime = -1.0;
    x->c_unit    = SCHEDULER_SYSTIME_TICKS_PER_MILLISECOND;
    x->c_fn      = (t_clockfn)fn;
    x->c_owner   = owner;
    x->c_next    = NULL;

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

/* the following routines maintain a real-execution-time histogram of the
various phases of real-time execution. */

static int sys_bin[] = {0, 2, 5, 10, 20, 30, 50, 100, 1000};
#define NBIN (sizeof(sys_bin)/sizeof(*sys_bin))
#define NHIST 10
static int sys_histogram[NHIST][NBIN];
static double sys_histtime;
static int sched_diddsp, sched_didpoll, sched_didnothing;

void sys_clearhist( void)
{
    unsigned int i, j;
    for (i = 0; i < NHIST; i++)
        for (j = 0; j < NBIN; j++) sys_histogram[i][j] = 0;
    sys_histtime = sys_getrealtime();
    sched_diddsp = sched_didpoll = sched_didnothing = 0;
}

void sys_printhist( void)
{
    unsigned int i, j;
    for (i = 0; i < NHIST; i++)
    {
        int doit = 0;
        for (j = 0; j < NBIN; j++) if (sys_histogram[i][j]) doit = 1;
        if (doit)
        {
            post("%2d %8d %8d %8d %8d %8d %8d %8d %8d", i,
                sys_histogram[i][0],
                sys_histogram[i][1],
                sys_histogram[i][2],
                sys_histogram[i][3],
                sys_histogram[i][4],
                sys_histogram[i][5],
                sys_histogram[i][6],
                sys_histogram[i][7]);
        }
    }
    post("dsp %d, pollgui %d, nothing %d",
        sched_diddsp, sched_didpoll, sched_didnothing);
}

static int sys_histphase;

int sys_addhist(int phase)
{
    int i, j, phasewas = sys_histphase;
    double newtime = sys_getrealtime();
    int msec = (newtime - sys_histtime) * 1000.;
    for (j = NBIN-1; j >= 0; j--)
    {
        if (msec >= sys_bin[j]) 
        {
            sys_histogram[phasewas][j]++;
            break;
        }
    }
    sys_histtime = newtime;
    sys_histphase = phase;
    return (phasewas);
}

#define NRESYNC 20

typedef struct _resync
{
    int r_ntick;
    int r_error;
} t_resync;

static int oss_resyncphase = 0;
static int oss_nresync = 0;
static t_resync oss_resync[NRESYNC];


static char *(oss_errornames[]) = {
"unknown",
"ADC blocked",
"DAC blocked",
"A/D/A sync",
"data late"
};

/*
void glob_audiostatus (void *dummy)
{
    int dev, nresync, nresyncphase, i;
    nresync = (oss_nresync >= NRESYNC ? NRESYNC : oss_nresync);
    nresyncphase = oss_resyncphase - 1;
    post("audio I/O error history:");
    post("seconds ago\terror type");
    for (i = 0; i < nresync; i++)
    {
        int errtype;
        if (nresyncphase < 0)
            nresyncphase += NRESYNC;
        errtype = oss_resync[nresyncphase].r_error;
        if (errtype < 0 || errtype > 4)
            errtype = 0;
        
        post("%9.2f\t%s",
            (sched_diddsp - oss_resync[nresyncphase].r_ntick)
                * ((double)scheduler_blockSize) / sys_dacsr,
            oss_errornames[errtype]);
        nresyncphase--;
    }
} */

static int sched_diored;
static int sched_dioredtime;
static int sched_meterson;

void sys_log_error(int type)
{
    oss_resync[oss_resyncphase].r_ntick = sched_diddsp;
    oss_resync[oss_resyncphase].r_error = type;
    oss_nresync++;
    if (++oss_resyncphase == NRESYNC) oss_resyncphase = 0;
    if (type != ERROR_NONE && !sched_diored &&
        (sched_diddsp >= sched_dioredtime))
    {
        // sys_vgui("::ui_console::pdtk_pd_dio 1\n");
        sched_diored = 1;
    }
    sched_dioredtime =
        sched_diddsp + (int)(sys_dacsr /(double)scheduler_blockSize);
}

static int sched_lastinclip, sched_lastoutclip,
    sched_lastindb, sched_lastoutdb;

void global_watchdog(void *dummy);

static void sched_pollformeters( void)
{
    int inclip, outclip, indb, outdb;
    static int sched_nextmeterpolltime, sched_nextpingtime;

        /* if there's no GUI but we're running in "realtime", here is
        where we arrange to ping the watchdog every 2 seconds. */
#if defined(__linux__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__GNU__)
    if (sys_nogui && sys_hipriority && (sched_diddsp - sched_nextpingtime > 0))
    {
        global_watchdog(0);
            /* ping every 2 seconds */
        sched_nextpingtime = sched_diddsp +
            2 * (int)(sys_dacsr /(double)scheduler_blockSize);
    }
#endif

    if (sched_diddsp - sched_nextmeterpolltime < 0)
        return;
    if (sched_diored && (sched_diddsp - sched_dioredtime > 0))
    {
        // sys_vgui("::ui_console::pdtk_pd_dio 0\n");
        sched_diored = 0;
    }
    if (sched_meterson)
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
    }
    if (inclip != sched_lastinclip || outclip != sched_lastoutclip
        || indb != sched_lastindb || outdb != sched_lastoutdb)
    {
        sys_vgui("pdtk_pd_meters %d %d %d %d\n", indb, outdb, inclip, outclip);
        sched_lastinclip = inclip;
        sched_lastoutclip = outclip;
        sched_lastindb = indb;
        sched_lastoutdb = outdb;
    }
    sched_nextmeterpolltime =
        sched_diddsp + (int)(sys_dacsr /(double)scheduler_blockSize);
}

void dsp_tick(void);

static int sched_useaudio = SCHEDULER_NONE;
static double sched_referencerealtime, sched_referencelogicaltime;
static double sys_time_per_dsp_tick;

void sched_reopenmeplease(void)   /* request from s_audio for deferred reopen */
{
    scheduler_quit = SCHEDULER_RESTART;
}

void sched_set_using_audio(int flag)
{
    sched_useaudio = flag;
    if (flag == SCHEDULER_NONE)
    {
        sched_referencerealtime = sys_getrealtime();
        sched_referencelogicaltime = scheduler_getSystime();
    }
        if (flag == SCHEDULER_CALLBACK &&
            sched_useaudio != SCHEDULER_CALLBACK)
                scheduler_quit = SCHEDULER_RESTART;
        if (flag != SCHEDULER_CALLBACK &&
            sched_useaudio == SCHEDULER_CALLBACK)
                post("sorry, can't turn off callbacks yet; restart Pd");
                    /* not right yet! */
        
    sys_time_per_dsp_tick = (SCHEDULER_SYSTIME_TICKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;
    // sys_vgui("::ui_console::pdtk_pd_audio %s\n", flag ? "on" : "off");
}

    /* take the scheduler forward one DSP tick, also handling clock timeouts */
void sched_tick( void)
{
    double next_sys_time = pd_this->pd_systime + sys_time_per_dsp_tick;
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
    sched_diddsp++;
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

void sys_pollmidiqueue( void);
void sys_initmidiqueue( void);

static void m_pollingscheduler( void)
{
    int idlecount = 0;
    sys_time_per_dsp_tick = (SCHEDULER_SYSTIME_TICKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;

#if PD_WITH_LOCK
        sys_lock();
#endif

    sys_clearhist();
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

        sys_addhist(0);
    waitfortick:
        if (sched_useaudio != SCHEDULER_NONE)
        {
#if PD_WITH_LOCK
            /* T.Grill - send_dacs may sleep -> 
                unlock thread lock make that time available 
                - could messaging do any harm while sys_send_dacs is running?
            */
            sys_unlock();
#endif
            timeforward = sys_send_dacs();
#if PD_WITH_LOCK
            /* T.Grill - done */
            sys_lock();
#endif
                /* if dacs remain "idle" for 1 sec, they're hung up. */
            if (timeforward != 0)
                idlecount = 0;
            else
            {
                idlecount++;
                if (!(idlecount & 31))
                {
                    static double idletime;
                    if (sched_useaudio != SCHEDULER_POLL)
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
            if (1000. * (sys_getrealtime() - sched_referencerealtime)
                > scheduler_getMillisecondsSince(sched_referencelogicaltime))
                    timeforward = DACS_YES;
            else timeforward = DACS_NO;
        }
        sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
        sys_addhist(1);
        if (timeforward != DACS_NO)
            sched_tick();
        if (timeforward == DACS_YES)
            didsomething = 1;

        sys_addhist(2);
        sys_pollmidiqueue();
        if (sys_pollgui())
        {
            if (!didsomething)
                sched_didpoll++;
            didsomething = 1;
        }
        sys_addhist(3);
            /* test for idle; if so, do graphics updates. */
        if (!didsomething)
        {
            sched_pollformeters();
#if PD_WITH_LOCK
            sys_unlock();   /* unlock while we idle */
#endif
            if (timeforward != DACS_SLEPT) { sys_microsleep(scheduler_sleepGrain); }
#if PD_WITH_LOCK
            sys_lock();
#endif
            sys_addhist(5);
            sched_didnothing++;
        }
    }

#if PD_WITH_LOCK
    sys_unlock();
#endif
}

void sched_audio_callbackfn(void)
{
    sys_lock();
    sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
    sys_addhist(1);
    sched_tick();
    sys_addhist(2);
    sys_pollmidiqueue();
    sys_addhist(3);
    sys_pollgui();
    sys_addhist(5);
    sched_pollformeters();
    sys_addhist(0);
    sys_unlock();
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
            sys_lock();
            sys_pollgui();
            sched_tick();
            sys_unlock();
        }
    }
}

int m_mainloop(void)
{
    while (scheduler_quit != SCHEDULER_QUIT)
    {
        if (sched_useaudio == SCHEDULER_CALLBACK)
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
    sys_time_per_dsp_tick = (SCHEDULER_SYSTIME_TICKS_PER_SECOND) *
        ((double)scheduler_blockSize) / sys_dacsr;
    while (scheduler_quit != SCHEDULER_QUIT)
        sched_tick();
    return (0);
}

/* ------------ thread locking ------------------- */

#if PD_WITH_LOCK
static pthread_mutex_t sys_mutex = PTHREAD_MUTEX_INITIALIZER;

void sys_lock(void)
{
    pthread_mutex_lock(&sys_mutex);
}

void sys_unlock(void)
{
    pthread_mutex_unlock(&sys_mutex);
}

int sys_trylock(void)
{
    return pthread_mutex_trylock(&sys_mutex);
}

#else

void sys_lock(void) {}
void sys_unlock(void) {}
int sys_trylock(void) {return (1);}

#endif

void sys_exit(void)
{
    scheduler_quit = SCHEDULER_QUIT;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
