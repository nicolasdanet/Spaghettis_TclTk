
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

extern t_sample *audio_soundIn;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *adc_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _adc {
    t_object    x_obj;              /* Must be the first. */
    int         x_size;
    int         *x_vector;
    } t_adc;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void adc_set (t_adc *x, t_symbol *s, int argc, t_atom *argv)
{
    int i, k = PD_MIN (argc, x->x_size);
    
    for (i = 0; i < k; i++) { x->x_vector[i] = (int)atom_getFloatAtIndex (i, argc, argv); }
    
    dsp_update();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void adc_dsp (t_adc *x, t_signal **sp)
{
    t_signal **s = sp;
    int i;
        
    for (i = 0; i < x->x_size; i++) {
    //
    int channel = x->x_vector[i] - 1;
    int k = audio_getChannelsIn();
    t_signal *t = (*s);
    
    PD_ASSERT (t->s_vectorSize == AUDIO_DEFAULT_BLOCKSIZE);
    PD_ABORT  (t->s_vectorSize != AUDIO_DEFAULT_BLOCKSIZE);
    
    if (channel < 0 || channel >= k) { dsp_addZeroPerform (t->s_vector, AUDIO_DEFAULT_BLOCKSIZE); }
    else {
    //
    t_sample *in = audio_soundIn + (AUDIO_DEFAULT_BLOCKSIZE * channel);
    
    dsp_addCopyPerform (in, t->s_vector, AUDIO_DEFAULT_BLOCKSIZE);
    //
    }
        
    s++;
    //
    }    
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *adc_new (t_symbol *s, int argc, t_atom *argv)
{
    t_adc *x = (t_adc *)pd_new (adc_class);
    int i;
    
    x->x_size   = argc ? argc : 2;
    x->x_vector = (int *)PD_MEMORY_GET (x->x_size * sizeof (int));
    
    if (!argc) { x->x_vector[0] = 1; x->x_vector[1] = 2; }
    else {
        for (i = 0; i < argc; i++) { x->x_vector[i] = (int)atom_getFloatAtIndex (i, argc, argv); }
    }
    
    for (i = 0; i < x->x_size; i++) {
        outlet_new (cast_object (x), &s_signal);
    }
    
    return x;
}

static void adc_free (t_adc *x)
{
    PD_MEMORY_FREE (x->x_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void adc_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_adc__tilde__,
            (t_newmethod)adc_new,
            (t_method)adc_free,
            sizeof (t_adc),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addDSP (c, (t_method)adc_dsp);
    
    class_addMethod (c, (t_method)adc_set, sym_set, A_GIMME, A_NULL);
    
    adc_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------


