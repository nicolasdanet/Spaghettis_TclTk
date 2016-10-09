
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"
#include "g_graphics.h"

#if defined (__APPLE__) || defined (__FreeBSD__)
#define CLOCKHZ CLK_TCK
#endif
#if defined (__linux__) || defined (__CYGWIN__) || defined (ANDROID)
#define CLOCKHZ sysconf(_SC_CLK_TCK)
#endif
#if defined (__FreeBSD_kernel__) || defined(__GNU__) || defined(__OpenBSD__)
#include <time.h>
#define CLOCKHZ CLOCKS_PER_SEC
#endif



/* -------------------------- random ------------------------------ */
/* this is strictly homebrew and untested. */

static t_class *random_class;

typedef struct _random
{
    t_object x_obj;
    t_float x_f;
    unsigned int x_state;
} t_random;


static int makeseed(void)
{
    static unsigned int random_nextseed = 1489853723;
    random_nextseed = random_nextseed * 435898247 + 938284287;
    return (random_nextseed & PD_INT_MAX);
}

static void *random_new(t_float f)
{
    t_random *x = (t_random *)pd_new(random_class);
    x->x_f = f;
    x->x_state = makeseed();
    inlet_newFloat(&x->x_obj, &x->x_f);
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void random_bang(t_random *x)
{
    int n = x->x_f, nval;
    int range = (n < 1 ? 1 : n);
    unsigned int randval = x->x_state;
    x->x_state = randval = randval * 472940017 + 832416023;
    nval = ((double)range) * ((double)randval)
        * (1./4294967296.);
    if (nval >= range) nval = range-1;
    outlet_float(x->x_obj.te_outlet, nval);
}

static void random_seed(t_random *x, t_float f, t_float glob)
{
    x->x_state = f;
}

static void random_setup(void)
{
    random_class = class_new(sym_random, (t_newmethod)random_new, 0,
        sizeof(t_random), 0, A_DEFFLOAT, 0);
    class_addBang(random_class, random_bang);
    class_addMethod(random_class, (t_method)random_seed,
        sym_seed, A_FLOAT, 0);
}


/* -------------------------- loadbang ------------------------------ */
static t_class *loadbang_class;

typedef struct _loadbang
{
    t_object x_obj;
} t_loadbang;

static void *loadbang_new(void)
{
    t_loadbang *x = (t_loadbang *)pd_new(loadbang_class);
    outlet_new(&x->x_obj, &s_bang);
    return (x);
}

static void loadbang_loadbang(t_loadbang *x)
{
    outlet_bang(x->x_obj.te_outlet);
}

static void loadbang_setup(void)
{
    loadbang_class = class_new (sym_loadbang, (t_newmethod)loadbang_new, 0,
        sizeof(t_loadbang), CLASS_NOINLET, 0);
    class_addMethod(loadbang_class, (t_method)loadbang_loadbang,
        sym_loadbang, 0);
}

/* ------------- namecanvas (delete this later) --------------------- */
static t_class *namecanvas_class;

typedef struct _namecanvas
{
    t_object x_obj;
    t_symbol *x_sym;
    t_pd *x_owner;
} t_namecanvas;

static void *namecanvas_new(t_symbol *s)
{
    t_namecanvas *x = (t_namecanvas *)pd_new(namecanvas_class);
    x->x_owner = (t_pd *)canvas_getCurrent();
    x->x_sym = s;
    if (*s->s_name) pd_bind(x->x_owner, s);
    return (x);
}

static void namecanvas_free(t_namecanvas *x)
{
    if (*x->x_sym->s_name) pd_unbind(x->x_owner, x->x_sym);
}

static void namecanvas_setup(void)
{
    namecanvas_class = class_new(sym_namecanvas,
        (t_newmethod)namecanvas_new, (t_method)namecanvas_free,
            sizeof(t_namecanvas), CLASS_NOINLET, A_DEFSYMBOL, 0);
}

/* ---------------serial ports (_WIN32 only -- hack) ------------------------- */
#define MAXSERIAL 100

static t_class *serial_class;

typedef struct _serial
{
    t_object x_obj;
    int x_portno;
    int x_open;
} t_serial;

static void serial_float(t_serial *x, t_float f)
{
    int n = f;
    char message[MAXSERIAL * 4 + 100];
    if (!x->x_open)
    {
        sys_vGui("com%d_open\n", x->x_portno);
        x->x_open = 1;
    }
    sprintf(message, "com%d_send \"\\%3.3o\"\n", x->x_portno, n);
    sys_gui(message);
}

static void *serial_new(t_float fportno)
{
    int portno = fportno;
    t_serial *x = (t_serial *)pd_new(serial_class);
    if (!portno) portno = 1;
    x->x_portno = portno;
    x->x_open = 0;
    return (x);
}

static void serial_setup(void)
{
    serial_class = class_new(sym_serial, (t_newmethod)serial_new, 0,
        sizeof(t_serial), 0, A_DEFFLOAT, 0);
    class_addFloat(serial_class, serial_float);
}

/* -------------------------- cputime ------------------------------ */

static t_class *cputime_class;

typedef struct _cputime
{
    t_object x_obj;
#ifdef _WIN32
    LARGE_INTEGER x_kerneltime;
    LARGE_INTEGER x_usertime;
    int x_warned;
#else
    struct tms x_setcputime;
#endif /* _WIN32 */
} t_cputime;

static void cputime_bang(t_cputime *x)
{
#ifdef _WIN32
    FILETIME ignorethis, ignorethat;
    BOOL retval;
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&x->x_kerneltime, (FILETIME *)&x->x_usertime);
    if (!retval)
    {
        if (!x->x_warned)
            post("cputime is apparently not supported on your platform");
        x->x_warned = 1;
        x->x_kerneltime.QuadPart = 0;
        x->x_usertime.QuadPart = 0;
    }
#else
    times(&x->x_setcputime);
#endif /* _WIN32 */
}

static void cputime_bang2(t_cputime *x)
{
#ifndef _WIN32
    t_float elapsedcpu;
    struct tms newcputime;
    times(&newcputime);
    elapsedcpu = 1000 * (
        newcputime.tms_utime + newcputime.tms_stime -
            x->x_setcputime.tms_utime - x->x_setcputime.tms_stime) / CLOCKHZ;
    outlet_float(x->x_obj.te_outlet, elapsedcpu);
#else
    t_float elapsedcpu;
    FILETIME ignorethis, ignorethat;
    LARGE_INTEGER usertime, kerneltime;
    BOOL retval;
    
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&kerneltime, (FILETIME *)&usertime);
    if (retval)
        elapsedcpu = 0.0001 *
            ((kerneltime.QuadPart - x->x_kerneltime.QuadPart) +
                (usertime.QuadPart - x->x_usertime.QuadPart));
    else elapsedcpu = 0;
    outlet_float(x->x_obj.te_outlet, elapsedcpu);
#endif /* NOT _WIN32 */
}

static void *cputime_new(void)
{
    t_cputime *x = (t_cputime *)pd_new(cputime_class);
    outlet_new(&x->x_obj, &s_float);

    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_bang, sym_inlet2);
#ifdef _WIN32
    x->x_warned = 0;
#endif
    cputime_bang(x);
    return (x);
}

static void cputime_setup(void)
{
    cputime_class = class_new(sym_cputime, (t_newmethod)cputime_new, 0,
        sizeof(t_cputime), 0, 0);
    class_addBang(cputime_class, cputime_bang);
    class_addMethod(cputime_class, (t_method)cputime_bang2, sym_inlet2, 0);
}

/* -------------------------- realtime ------------------------------ */

static t_class *realtime_class;

typedef struct _realtime
{
    t_object x_obj;
    double x_setrealtime;
} t_realtime;

static void realtime_bang(t_realtime *x)
{
    x->x_setrealtime = sys_getRealTimeInSeconds();
}

static void realtime_bang2(t_realtime *x)
{
    outlet_float(x->x_obj.te_outlet,
        (sys_getRealTimeInSeconds() - x->x_setrealtime) * 1000.);
}

static void *realtime_new(void)
{
    t_realtime *x = (t_realtime *)pd_new(realtime_class);
    outlet_new(&x->x_obj, &s_float);
    inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_bang, sym_inlet2);
    realtime_bang(x);
    return (x);
}

static void realtime_setup(void)
{
    realtime_class = class_new(sym_realtime, (t_newmethod)realtime_new, 0,
        sizeof(t_realtime), 0, 0);
    class_addBang(realtime_class, realtime_bang);
    class_addMethod(realtime_class, (t_method)realtime_bang2, sym_inlet2,
        0);
}

/* ---------- oscparse - parse simple OSC messages ----------------- */

static t_class *oscparse_class;

typedef struct _oscparse
{
    t_object x_obj;
} t_oscparse;

#define ROUNDUPTO4(x) (((x) + 3) & (~3))

#define READINT(x)  ((((int)(((x)  )->a_w.w_float)) & 0xff) << 24) | \
                    ((((int)(((x)+1)->a_w.w_float)) & 0xff) << 16) | \
                    ((((int)(((x)+2)->a_w.w_float)) & 0xff) << 8) | \
                    ((((int)(((x)+3)->a_w.w_float)) & 0xff) << 0)

static t_symbol *grabstring(int argc, t_atom *argv, int *ip, int slash)
{
    char buf[PD_STRING];
    int first, nchar;
    if (slash)
        while (*ip < argc && argv[*ip].a_w.w_float == '/')
            (*ip)++;
    for (nchar = 0; nchar < PD_STRING-1 && *ip < argc; nchar++, (*ip)++)
    {
        char c = argv[*ip].a_w.w_float;
        if (c == 0 || (slash && c == '/'))
            break;
        buf[nchar] = c;
    }
    buf[nchar] = 0;
    if (!slash)
        *ip = ROUNDUPTO4(*ip+1);
    if (*ip > argc)
        *ip = argc;
    return (gensym (buf));
}

static void oscparse_list(t_oscparse *x, t_symbol *s, int argc, t_atom *argv)
{
    int i, j, j2, k, outc = 1, blob = 0, typeonset, dataonset, nfield;
    t_atom *outv;
    if (!argc)
        return;
    for (i = 0; i < argc; i++)
        if (argv[i].a_type != A_FLOAT)
    {
        post_error ("oscparse: takes numbers only");
        return;
    }
    if (argv[0].a_w.w_float == '#') /* it's a bundle */
    {
        if (argv[1].a_w.w_float != 'b' || argc < 16)
        {
            post_error ("oscparse: malformed bundle");
            return;
        }
            /* we ignore the timetag since there's no correct way to
            convert it to Pd logical time that I can think of.  LATER
            consider at least outputting timetag differentially converted
            into Pd time units. */
        for (i = 16; i < argc-4; )
        {
            int msize = READINT(argv+i);
            if (msize <= 0 || msize & 3)
            {
                post_error ("oscparse: bad bundle element size");
                return;
            }
            oscparse_list(x, 0, msize, argv+i+4);
            i += msize+4;
        }
        return;
    }
    else if (argv[0].a_w.w_float != '/')
    {
        post_error ("oscparse: not an OSC message (no leading slash)");
        return;
    }
    for (i = 1; i < argc && argv[i].a_w.w_float != 0; i++)
        if (argv[i].a_w.w_float == '/')
            outc++;
    i = ROUNDUPTO4(i+1);
    if (argv[i].a_w.w_float != ',' || (i+1) >= argc)
    {
        post_error ("oscparse: malformed type string (char %d, index %d)",
            (int)(argv[i].a_w.w_float), i);
        return;
    }
    typeonset = ++i;
    for (; i < argc && argv[i].a_w.w_float != 0; i++)
        if (argv[i].a_w.w_float == 'b')
            blob = 1;
    nfield = i - typeonset;
    if (blob)
        outc += argc - typeonset;
    else outc += nfield;
    outv = (t_atom *)alloca(outc * sizeof(t_atom));
    dataonset = ROUNDUPTO4(i + 1);
    /* post("outc %d, typeonset %d, dataonset %d, nfield %d", outc, typeonset, 
        dataonset, nfield); */
    for (i = j = 0; i < typeonset-1 && argv[i].a_w.w_float != 0 &&
        j < outc; j++)
            SET_SYMBOL(outv+j, grabstring(argc, argv, &i, 1));
    for (i = typeonset, k = dataonset; i < typeonset + nfield; i++)
    {
        union
        {
            float z_f;
            uint32_t z_i;
        } z;
        float f;
        int blobsize;
        switch ((int)(argv[i].a_w.w_float))
        {
        case 'f':
            if (k > argc - 4)
                goto tooshort;
            z.z_i = READINT(argv+k);
            f = z.z_f;
            if (PD_DENORMAL_OR_ZERO(f))
                f = 0;
            if (j >= outc)
            {
                PD_BUG;
                return;
            }
            SET_FLOAT(outv+j, f);
            j++; k += 4;
            break;
        case 'i':
            if (k > argc - 4)
                goto tooshort;
            if (j >= outc)
            {
                PD_BUG;
                return;
            }
            SET_FLOAT(outv+j, READINT(argv+k));
            j++; k += 4;
            break;
        case 's':
            if (j >= outc)
            {
                PD_BUG;
                return;
            }
            SET_SYMBOL(outv+j, grabstring(argc, argv, &k, 0));
            j++;
            break;
        case 'b':
            if (k > argc - 4)
                goto tooshort;
            blobsize = READINT(argv+k);
            k += 4;
            if (blobsize < 0 || blobsize > argc - k)
                goto tooshort;
            if (j + blobsize + 1 > outc)
            {
                PD_BUG;
                return;
            }
            if (k + blobsize > argc)
                goto tooshort;
            SET_FLOAT(outv+j, blobsize);
            j++;
            for (j2 = 0; j2 < blobsize; j++, j2++, k++)
                SET_FLOAT(outv+j, argv[k].a_w.w_float);
            k = ROUNDUPTO4(k);
            break;
        default:
            post_error ("oscparse: unknown tag '%c' (%d)", 
                (int)(argv[i].a_w.w_float), (int)(argv[i].a_w.w_float));
        } 
    }
    outlet_list(x->x_obj.te_outlet, j, outv);
    return;
tooshort:
    post_error ("oscparse: OSC message ended prematurely");
}

static t_oscparse *oscparse_new(t_symbol *s, int argc, t_atom *argv)
{
    t_oscparse *x = (t_oscparse *)pd_new(oscparse_class);
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

void oscparse_setup(void)
{
    oscparse_class = class_new(sym_oscparse, (t_newmethod)oscparse_new,
        0, sizeof(t_oscparse), 0, A_GIMME, 0);
    class_addList(oscparse_class, oscparse_list);
}

void x_misc_setup(void)
{
    random_setup();
    loadbang_setup();
    namecanvas_setup();
    serial_setup();
    cputime_setup();
    realtime_setup();
    oscparse_setup();
}
