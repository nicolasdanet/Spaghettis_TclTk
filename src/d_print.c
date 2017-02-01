
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

static t_class *print_tilde_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _print_tilde {
    t_object    x_obj;                          /* Must be the first. */
    t_float     x_f;
    int         x_count;
    int         x_overflow;
    int         x_index;
    int         x_hasPolling;
    t_sample    *x_buffer;
    t_symbol    *x_name;
    } t_print_tilde;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PRINT_BUFFER_SIZE       4096
#define PRINT_BUFFER_CHUNK      16

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void print_tilde_count (t_print_tilde *, t_float);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void print_tilde_bang (t_print_tilde *x)
{
    print_tilde_count (x, (t_float)1.0);
}

static void print_tilde_count (t_print_tilde *x, t_float f)
{
    x->x_count      = (int)PD_MAX (1.0, f);
    x->x_hasPolling = 1;
    
    poll_add (cast_pd (x));
}

static void print_tilde_polling (t_print_tilde *x)
{
    if (!x->x_count) {
    //
    if (x->x_overflow) { warning_tooManyCharacters (sym_print__tilde__); }
    else {
    //
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    int i;
    
    for (i = 0; i < x->x_index; i++) {
    //
    int dump = ((i % PRINT_BUFFER_CHUNK) == (PRINT_BUFFER_CHUNK - 1));
    dump |= (i == (x->x_index - 1));
    
    if (!dump) { err |= string_addSprintf (t, PD_STRING, "%g ", x->x_buffer[i]); }
    else {
        err |= string_addSprintf (t, PD_STRING, "%g", x->x_buffer[i]); 
        PD_ASSERT (!err);
        post ("%s: %s", x->x_name->s_name, t);  // --
        string_clear (t, PD_STRING);
    }
    //
    }
    //
    }

    x->x_overflow   = 0;
    x->x_index      = 0;
    x->x_hasPolling = 0;
    
    poll_remove (cast_pd (x));
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *print_tilde_perform (t_int *w)
{
    t_print_tilde *x = (t_print_tilde *)(w[1]);
    PD_RESTRICTED in = (t_sample *)(w[2]);
    int n = (int)(w[3]);
    
    if (x->x_count) {
    
        if (x->x_index + n >= PRINT_BUFFER_SIZE) { x->x_overflow = 1; }
        else {
            int i;
            for (i = 0; i < n; i++) {
                x->x_buffer[x->x_index] = in[i]; x->x_index++;
            }
        }
        
        x->x_count--;
    }
    
    return (w + 4);
}

static void print_tilde_dsp (t_print_tilde *x, t_signal **sp)
{
    dsp_add (print_tilde_perform, 3, x, sp[0]->s_vector, sp[0]->s_vectorSize);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *print_tilde_new (t_symbol *s)
{
    t_print_tilde *x = (t_print_tilde *)pd_new (print_tilde_class);
    
    x->x_buffer = (t_sample *)PD_MEMORY_GET (PRINT_BUFFER_SIZE * sizeof (t_sample));
    x->x_name   = (s != &s_ ? s : sym_print__tilde__);
    
    return x;
}

static void print_tilde_free (t_print_tilde *x)
{
    if (x->x_hasPolling) { poll_remove (cast_pd (x)); }
    
    PD_MEMORY_FREE (x->x_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void print_tilde_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_print__tilde__,
            (t_newmethod)print_tilde_new,
            (t_method)print_tilde_free,
            sizeof (t_print_tilde),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    CLASS_SIGNAL (c, t_print_tilde, x_f);
    
    class_addDSP (c, (t_method)print_tilde_dsp);
    class_addBang (c, (t_method)print_tilde_bang);
    class_addPolling (c, (t_method)print_tilde_polling);
    
    class_addMethod (c, (t_method)print_tilde_count, sym_count, A_FLOAT, A_NULL);
    
    print_tilde_class = c;
}

void print_tilde_destroy (void)
{
    CLASS_FREE (print_tilde_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
