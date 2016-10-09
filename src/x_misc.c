
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

void x_misc_setup(void)
{
    random_setup();
    loadbang_setup();
    namecanvas_setup();
    serial_setup();
}
