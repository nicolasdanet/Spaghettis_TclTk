
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
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *dac_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dac {
    t_object    x_obj;
    t_int       x_size;
    t_int       *x_vector;
    t_float     x_f;
    } t_dac;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dac_set(t_dac *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc && i < x->x_size; i++)
        x->x_vector[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    dsp_update();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dac_dsp(t_dac *x, t_signal **sp)
{
    t_int i, *ip;
    t_signal **sp2;
    for (i = x->x_size, ip = x->x_vector, sp2 = sp; i--; ip++, sp2++)
    {
        int ch = *ip - 1;
        if ((*sp2)->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE)
            post_error ("dac~: bad vector size");
        else if (ch >= 0 && ch < audio_getChannelsOut())
            dsp_add(plus_perform, 4, audio_soundOut + AUDIO_DEFAULT_BLOCKSIZE*ch,
                (*sp2)->s_vector, audio_soundOut + AUDIO_DEFAULT_BLOCKSIZE*ch, AUDIO_DEFAULT_BLOCKSIZE);
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    x->x_size = argc;
    x->x_vector = (t_int *)PD_MEMORY_GET(argc * sizeof(*x->x_vector));
    for (i = 0; i < argc; i++)
        x->x_vector[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    for (i = 1; i < argc; i++)
        inlet_new(&x->x_obj, &x->x_obj.te_g.g_pd, &s_signal, &s_signal);
    x->x_f = 0;
    return (x);
}

static void dac_free(t_dac *x)
{
    PD_MEMORY_FREE(x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dac_setup(void)
{
    dac_class = class_new(sym_dac__tilde__, (t_newmethod)dac_new,
        (t_method)dac_free, sizeof(t_dac), 0, A_GIMME, 0);
    CLASS_SIGNAL(dac_class, t_dac, x_f);
    class_addMethod(dac_class, (t_method)dac_dsp, sym_dsp, A_CANT, 0);
    class_addMethod(dac_class, (t_method)dac_set, sym_set, A_GIMME, 0);
    class_setHelpName(dac_class, sym_adc__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
