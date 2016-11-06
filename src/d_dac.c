/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  The dac~ and adc~ routines.
*/

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "d_dsp.h"

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
        if ((*sp2)->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE)
            post_error ("dac~: bad vector size");
        else if (ch >= 0 && ch < audio_getChannelsOut())
            dsp_add(plus_perform, 4, audio_soundOut + AUDIO_DEFAULT_BLOCKSIZE*ch,
                (*sp2)->s_vector, audio_soundOut + AUDIO_DEFAULT_BLOCKSIZE*ch, AUDIO_DEFAULT_BLOCKSIZE);
    }    
}

static void dac_set(t_dac *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc && i < x->x_n; i++)
        x->x_vec[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    dsp_update();
}

static void dac_free(t_dac *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

void dac_setup(void)
{
    dac_class = class_new(sym_dac__tilde__, (t_newmethod)dac_new,
        (t_method)dac_free, sizeof(t_dac), 0, A_GIMME, 0);
    CLASS_SIGNAL(dac_class, t_dac, x_f);
    class_addMethod(dac_class, (t_method)dac_dsp, sym_dsp, A_CANT, 0);
    class_addMethod(dac_class, (t_method)dac_set, sym_set, A_GIMME, 0);
    class_setHelpName(dac_class, sym_adc__tilde__);
}
