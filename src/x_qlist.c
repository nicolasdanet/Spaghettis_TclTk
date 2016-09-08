
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *qlist_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void qlist_performWait (t_qlist *x, int doNotSend, int isAutomatic, int onset)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    int i = onset + 1;
        
    while (i < buffer_size (b) && IS_FLOAT (buffer_atomAtIndex (b, i))) { i++; }
    
    x->ql_indexOfStart = i;
    
    if (isAutomatic) { clock_delay (x->ql_clock, (double)GET_FLOAT (buffer_atomAtIndex (b, onset))); }
    else {
        outlet_list (x->ql_outletLeft, NULL, i - onset, buffer_atomAtIndex (b, onset));
    }
}

static int qlist_performNext (t_qlist *x, int doNotSend, int isAutomatic, int onset)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    
    int count, i = onset + 1;
    
    while (i < buffer_size (b) && IS_SYMBOL_OR_FLOAT (buffer_atomAtIndex (b, i))) { i++; }
    
    if (!x->ql_target) {
        if (!IS_SYMBOL (buffer_atomAtIndex (b, onset))) { return 0; }
        else {
            t_symbol *t = GET_SYMBOL (buffer_atomAtIndex (b, onset));
            if (pd_isThing (t)) { x->ql_target = t->s_thing; }
            else {
                return 0;
            }
        }
        onset++;
    }
    
    x->ql_indexOfStart = i;
    
    if ((count = i - onset)) {

        x->ql_flagRewound = 0;
        
        if (!doNotSend) {
            t_atom *first = buffer_atomAtIndex (b, onset);
            if (IS_FLOAT (first)) { pd_message (x->ql_target, &s_list, count, first); }
            else if (IS_SYMBOL (first)) { 
                pd_message (x->ql_target, GET_SYMBOL (first), count - 1, first + 1); 
            }
        }
        
        if (x->ql_flagRewound) { return 1; }
    }
    
    return 0;
}

static void qlist_perform (t_qlist *x, int doNotSend, int isAutomatic)
{
    if (x->ql_flagReentrant) { error_unexpected (sym_qlist, sym_next); }
    else {
    //
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    t_error err = PD_ERROR_NONE;
    
    x->ql_target = NULL;
    
    x->ql_flagReentrant = 1;
    
    while (!err) {
    //
    int onset = x->ql_indexOfStart;
    
    err = (onset >= buffer_size (b));
    
    if (!err) {

        while (!err && IS_SEMICOLON_OR_COMMA (buffer_atomAtIndex (b, onset))) {
            if (IS_SEMICOLON (buffer_atomAtIndex (b, onset))) { x->ql_target = NULL; }
            onset++;
            err = (onset >= buffer_size (b));
        }
        
        if (!err) {
            if (!x->ql_target && IS_FLOAT (buffer_atomAtIndex (b, onset))) {
                qlist_performWait (x, doNotSend, isAutomatic, onset);
                break;
                
            } else {
                if (qlist_performNext (x, doNotSend, isAutomatic, onset)) {
                    break;
                }
            }
        }
    }
    //
    }

    x->ql_flagReentrant = 0;
    
    if (err) {
        x->ql_indexOfStart = PD_INT_MAX;
        outlet_bang (x->ql_outletRight);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void qlist_task (t_qlist *x)
{
    qlist_perform (x, 0, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void qlist_bang (t_qlist *x)
{
    qlist_rewind (x);

    if (!x->ql_flagReentrant) { qlist_perform (x, 0, 1); }
    else {
        clock_delay (x->ql_clock, 0.0);
    }
}

static void qlist_next (t_qlist *x, t_float f)
{
    qlist_perform (x, (f != 0.0), 0);
}

void qlist_rewind (t_qlist *x)
{
    if (x->ql_clock) { clock_unset (x->ql_clock); }
    
    x->ql_indexOfStart  = 0;
    x->ql_flagRewound   = 1;
}

void qlist_clear (t_qlist *x)
{
    qlist_rewind (x);
    buffer_reset (textbuffer_getBuffer (&x->ql_textbuffer));
    textbuffer_update (&x->ql_textbuffer);
}

void qlist_set (t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    qlist_clear (x);
    qlist_add (x, s, argc, argv);
}

void qlist_add (t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom a;
    SET_SEMICOLON (&a);
    buffer_append (textbuffer_getBuffer (&x->ql_textbuffer), argc, argv);
    buffer_appendAtom (textbuffer_getBuffer (&x->ql_textbuffer), &a);
    textbuffer_update (&x->ql_textbuffer);
}

void qlist_append (t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append (textbuffer_getBuffer (&x->ql_textbuffer), argc, argv);
    textbuffer_update (&x->ql_textbuffer);
}

static void qlist_unit (t_qlist *x, t_float f, t_symbol *unitName)
{

}

void qlist_read (t_qlist *x, t_symbol *s)
{
    t_atom a;
    SET_SYMBOL (&a, s);
    qlist_clear (x);
    textbuffer_read (&x->ql_textbuffer, sym_read, 1, &a);
    textbuffer_update (&x->ql_textbuffer);
}

void qlist_write (t_qlist *x, t_symbol *s)
{
    t_atom a;
    SET_SYMBOL (&a, s);
    textbuffer_write (&x->ql_textbuffer, sym_read, 1, &a);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *qlist_new (void)
{
    t_qlist *x = (t_qlist *)pd_new (qlist_class);
    
    textbuffer_init (&x->ql_textbuffer);
    
    x->ql_indexOfStart  = PD_INT_MAX;
    x->ql_outletLeft    = outlet_new (cast_object (x), &s_list);
    x->ql_outletRight   = outlet_new (cast_object (x), &s_bang);
    x->ql_clock         = clock_new (x, (t_method)qlist_task);

    return x;
}

static void qlist_free(t_qlist *x)
{
    clock_free (x->ql_clock);
    textbuffer_free (&x->ql_textbuffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void qlist_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_qlist,
            (t_newmethod)qlist_new,
            (t_method)qlist_free,
            sizeof (t_qlist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, qlist_bang);
    
    class_addClick (c, textbuffer_click);
        
    class_addMethod (c, (t_method)qlist_rewind,         sym_rewind,     A_NULL);
    class_addMethod (c, (t_method)qlist_clear,          sym_clear,      A_NULL);
    class_addMethod (c, (t_method)qlist_set,            sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_add,            sym_add,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_append,         sym_append,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_unit,           sym_unit,       A_FLOAT, A_DEFSYMBOL, A_NULL); 
    class_addMethod (c, (t_method)qlist_next,           sym_next,       A_DEFFLOAT, A_NULL); 
    class_addMethod (c, (t_method)qlist_read,           sym_read,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_write,          sym_write,      A_SYMBOL, A_NULL);

    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_addLine,   sym__addline,   A_GIMME, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)qlist_append,         sym_add2,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_unit,           sym_tempo,      A_FLOAT, A_DEFSYMBOL, A_NULL);
   
    #endif
    
    qlist_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
