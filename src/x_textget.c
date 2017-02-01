
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
#include "m_alloca.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *textget_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textget {
    t_textclient    x_textclient;               /* Must be the first. */
    t_float         x_fieldStart;
    t_float         x_fieldCount;
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_textget;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textget_float (t_textget *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
        
    if (b) {
    //
    int field = (int)x->x_fieldStart;
    int count = (int)x->x_fieldCount;
    
    int start, end, match = 0;
    t_atomtype type;
    
    if (buffer_getMessageAtWithTypeOfEnd (b, f, &start, &end, &type)) {
    
        t_atom *t = NULL;
        int size = end - start;
        int i;
                
        if (field < 0) {
        
            outlet_float (x->x_outletRight, (t_float)(type == A_COMMA));
            
            ATOMS_ALLOCA (t, size);
            for (i = 0; i < size; i++) { buffer_getAtomAtIndex (b, start + i, t + i); }
            outlet_list (x->x_outletLeft, size, t); 
            match = 1;
            ATOMS_FREEA (t, size);
            
        } else if (field < size) {
        
            outlet_float (x->x_outletRight, (t_float)(type == A_COMMA));
            
            count = PD_MIN (count, size - field);
            ATOMS_ALLOCA (t, count);
            for (i = 0; i < count; i++) { buffer_getAtomAtIndex (b, start + field + i, t + i); }
            outlet_list (x->x_outletLeft, count, t);
            match = 1;
            ATOMS_FREEA (t, count);
        }
    } 

    if (!match) { outlet_float (x->x_outletRight, 2); outlet_list (x->x_outletLeft, 0, NULL); }
    //
    } else { error_undefined (sym_text__space__get, sym_text); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textget_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textget *x = (t_textget *)pd_new (textget_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);         /* It may consume arguments. */
    
    if (!err) {
    
        x->x_fieldStart  = -1;
        x->x_fieldCount  = 1;
        x->x_outletLeft  = outlet_new (cast_object (x), &s_list);
        x->x_outletRight = outlet_new (cast_object (x), &s_float);
        
        inlet_newFloat (cast_object (x), &x->x_fieldStart);
        inlet_newFloat (cast_object (x), &x->x_fieldCount);
        
        if (argc && IS_FLOAT (argv)) { x->x_fieldStart = GET_FLOAT (argv); argc--; argv++; }
        if (argc && IS_FLOAT (argv)) { x->x_fieldCount = GET_FLOAT (argv); argc--; argv++; }

        if (argc) { warning_unusedArguments (sym_text__space__get, argc, argv); }
         
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
        
    } else {
    
        error_invalidArguments (sym_text__space__get, argc, argv);
        pd_free (cast_pd (x)); x = NULL; 
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textget_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__get,
            (t_newmethod)textget_new,
            (t_method)textclient_free,
            sizeof (t_textget),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, (t_method)textget_float);
    
    class_setHelpName (c, sym_text);
    
    textget_class = c;
}

void textget_destroy (void)
{
    CLASS_FREE (textget_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
