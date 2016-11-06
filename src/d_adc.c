
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

static t_class *adc_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _adc
{
    t_object x_obj;
    t_int x_n;
    t_int *x_vec;
} t_adc;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void adc_set(t_adc *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc && i < x->x_n; i++)
        x->x_vec[i] = (t_int)atom_getFloatAtIndex(i, argc, argv);
    dsp_update();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void adc_dsp(t_adc *x, t_signal **sp)
{
    t_int i, *ip;
    t_signal **sp2;
    for (i = x->x_n, ip = x->x_vec, sp2 = sp; i--; ip++, sp2++)
    {
        int ch = *ip - 1;
        if ((*sp2)->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE)
            post_error ("adc~: bad vector size");
        else if (ch >= 0 && ch < audio_getChannelsIn())
            dsp_addCopyPerform(audio_soundIn + AUDIO_DEFAULT_BLOCKSIZE*ch,
                (*sp2)->s_vector, AUDIO_DEFAULT_BLOCKSIZE);
        else dsp_addZeroPerform((*sp2)->s_vector, AUDIO_DEFAULT_BLOCKSIZE);
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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


static void adc_free(t_adc *x)
{
    PD_MEMORY_FREE(x->x_vec);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void adc_setup(void)
{
    adc_class = class_new(sym_adc__tilde__, (t_newmethod)adc_new,
        (t_method)adc_free, sizeof(t_adc), 0, A_GIMME, 0);
    class_addMethod(adc_class, (t_method)adc_dsp, sym_dsp, A_CANT, 0);
    class_addMethod(adc_class, (t_method)adc_set, sym_set, A_GIMME, 0);
    class_setHelpName(adc_class, sym_adc__tilde__);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


