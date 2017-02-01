
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_sample *audio_soundOut;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *dac_tilde_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _dac_tilde {
    t_object    x_obj;              /* Must be the first. */
    t_float     x_f;
    int         x_size;
    int         *x_vector;
    } t_dac_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dac_tilde_set (t_dac_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
    int i, k = PD_MIN (argc, x->x_size);
    
    for (i = 0; i < k; i++) { x->x_vector[i] = (int)atom_getFloatAtIndex (i, argc, argv); }
    
    dsp_update();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void dac_tilde_dsp (t_dac_tilde *x, t_signal **sp)
{
    t_signal **s = sp;
    int i;
        
    for (i = 0; i < x->x_size; i++) {
    //
    int channel = x->x_vector[i] - 1;
    int k = audio_getChannelsOut();
    t_signal *t = (*s);
    
    PD_ASSERT (t->s_vectorSize == AUDIO_DEFAULT_BLOCKSIZE);
    PD_ABORT  (t->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE);
    
    if (channel >= 0 && channel < k) {
    //
    t_sample *out = audio_soundOut + (AUDIO_DEFAULT_BLOCKSIZE * channel);
    
    dsp_addPlusPerform (out, t->s_vector, out, AUDIO_DEFAULT_BLOCKSIZE);
    //
    }
        
    s++;
    //
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *dac_tilde_new (t_symbol *s, int argc, t_atom *argv)
{
    t_dac_tilde *x = (t_dac_tilde *)pd_new (dac_tilde_class);
    int i;
    
    x->x_size   = argc ? argc : 2;
    x->x_vector = (int *)PD_MEMORY_GET (x->x_size * sizeof (int));
    
    if (!argc) { x->x_vector[0] = 1; x->x_vector[1] = 2; }
    else {
        for (i = 0; i < argc; i++) { x->x_vector[i] = (int)atom_getFloatAtIndex (i, argc, argv); }
    }
    
    for (i = 1; i < x->x_size; i++) { inlet_newSignal (cast_object (x)); }
    
    return x;
}

static void dac_tilde_free (t_dac_tilde *x)
{
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void dac_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_dac__tilde__,
            (t_newmethod)dac_tilde_new,
            (t_method)dac_tilde_free,
            sizeof (t_dac_tilde),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    CLASS_SIGNAL (c, t_dac_tilde, x_f);
    
    class_addDSP (c, (t_method)dac_tilde_dsp);
    
    class_addMethod (c, (t_method)dac_tilde_set, sym_set, A_GIMME, A_NULL);
    
    class_setHelpName (c, sym_adc__tilde__);
    
    dac_tilde_class = c;
}

void dac_tilde_destroy (void)
{
    CLASS_FREE (dac_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
