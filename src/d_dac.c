/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  The dac~ and adc~ routines.
*/

#include "m_pd.h"
#include "m_macros.h"
#include "s_system.h"

extern t_sample *sys_soundout;
extern t_sample *sys_soundin;

/* ----------------------------- dac~ --------------------------- */
static t_class *dac_class;

typedef struct _dac
{
    t_object x_obj;
    t_int x_n;
    t_int *x_vec;
    t_float x_f;
} t_dac;

static void *dac_new(t_symbol *s, int argc, t_atom *argv)
{
    t_dac *x = (t_dac *)pd_new(dac_class);
    t_atom defarg[2], *ap;
    int i;
    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SET_FLOAT(&defarg[0], 1);
        SET_FLOAT(&defarg[1], 2);
    }
    x->x_n = argc;
    x->x_vec = (t_int *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
    for (i = 0; i < argc; i++)
        x->x_vec[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    for (i = 1; i < argc; i++)
        inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_f = 0;
    return (x);
}

static void dac_dsp(t_dac *x, t_signal **sp)
{
    t_int i, *ip;
    t_signal **sp2;
    for (i = x->x_n, ip = x->x_vec, sp2 = sp; i--; ip++, sp2++)
    {
        int ch = *ip - 1;
        if ((*sp2)->s_blockSize != AUDIO_DEFAULT_BLOCK)
            post_error ("dac~: bad vector size");
        else if (ch >= 0 && ch < sys_get_outchannels())
            dsp_add(plus_perform, 4, sys_soundout + AUDIO_DEFAULT_BLOCK*ch,
                (*sp2)->s_vector, sys_soundout + AUDIO_DEFAULT_BLOCK*ch, AUDIO_DEFAULT_BLOCK);
    }    
}

static void dac_set(t_dac *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc && i < x->x_n; i++)
        x->x_vec[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    canvas_update_dsp();
}

static void dac_free(t_dac *x)
{
    PD_MEMORY_FREE(x->x_vec, x->x_n * sizeof(*x->x_vec));
}

static void dac_setup(void)
{
    dac_class = class_new(gensym("dac~"), (t_newmethod)dac_new,
        (t_method)dac_free, sizeof(t_dac), 0, A_GIMME, 0);
    CLASS_SIGNAL(dac_class, t_dac, x_f);
    class_addMethod(dac_class, (t_method)dac_dsp, gensym("dsp"), A_CANT, 0);
    class_addMethod(dac_class, (t_method)dac_set, gensym("set"), A_GIMME, 0);
    class_setHelpName(dac_class, gensym("adc~"));
}

/* ----------------------------- adc~ --------------------------- */
static t_class *adc_class;

typedef struct _adc
{
    t_object x_obj;
    t_int x_n;
    t_int *x_vec;
} t_adc;

static void *adc_new(t_symbol *s, int argc, t_atom *argv)
{
    t_adc *x = (t_adc *)pd_new(adc_class);
    t_atom defarg[2], *ap;
    int i;
    if (!argc)
    {
        argv = defarg;
        argc = 2;
        SET_FLOAT(&defarg[0], 1);
        SET_FLOAT(&defarg[1], 2);
    }
    x->x_n = argc;
    x->x_vec = (t_int *)PD_MEMORY_GET(argc * sizeof(*x->x_vec));
    for (i = 0; i < argc; i++)
        x->x_vec[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    for (i = 0; i < argc; i++)
        outlet_new(&x->x_obj, &s_signal);
    return (x);
}

t_int *copy_perform(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    while (n--) *out++ = *in1++; 
    return (w+4);
}

static t_int *copy_perf8(t_int *w)
{
    t_sample *in1 = (t_sample *)(w[1]);
    t_sample *out = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    for (; n; n -= 8, in1 += 8, out += 8)
    {
        t_sample f0 = in1[0];
        t_sample f1 = in1[1];
        t_sample f2 = in1[2];
        t_sample f3 = in1[3];
        t_sample f4 = in1[4];
        t_sample f5 = in1[5];
        t_sample f6 = in1[6];
        t_sample f7 = in1[7];

        out[0] = f0;
        out[1] = f1;
        out[2] = f2;
        out[3] = f3;
        out[4] = f4;
        out[5] = f5;
        out[6] = f6;
        out[7] = f7;
    }
    return (w+4);
}

void dsp_add_copy(t_sample *in, t_sample *out, int n)
{
    if (n&7)
        dsp_add(copy_perform, 3, in, out, n);
    else        
        dsp_add(copy_perf8, 3, in, out, n);
}

static void adc_dsp(t_adc *x, t_signal **sp)
{
    t_int i, *ip;
    t_signal **sp2;
    for (i = x->x_n, ip = x->x_vec, sp2 = sp; i--; ip++, sp2++)
    {
        int ch = *ip - 1;
        if ((*sp2)->s_blockSize != AUDIO_DEFAULT_BLOCK)
            post_error ("adc~: bad vector size");
        else if (ch >= 0 && ch < sys_get_inchannels())
            dsp_add_copy(sys_soundin + AUDIO_DEFAULT_BLOCK*ch,
                (*sp2)->s_vector, AUDIO_DEFAULT_BLOCK);
        else dsp_add_zero((*sp2)->s_vector, AUDIO_DEFAULT_BLOCK);
    }    
}

static void adc_set(t_adc *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc && i < x->x_n; i++)
        x->x_vec[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    canvas_update_dsp();
}

static void adc_free(t_adc *x)
{
    PD_MEMORY_FREE(x->x_vec, x->x_n * sizeof(*x->x_vec));
}

static void adc_setup(void)
{
    adc_class = class_new(gensym("adc~"), (t_newmethod)adc_new,
        (t_method)adc_free, sizeof(t_adc), 0, A_GIMME, 0);
    class_addMethod(adc_class, (t_method)adc_dsp, gensym("dsp"), A_CANT, 0);
    class_addMethod(adc_class, (t_method)adc_set, gensym("set"), A_GIMME, 0);
    class_setHelpName(adc_class, gensym("adc~"));
}

void d_dac_setup(void)
{
    dac_setup();
    adc_setup();
}

