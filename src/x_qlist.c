
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *qlist_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void qlist_proceedWait (t_qlist *x, int doNotSend, int isAutomatic, int start, int end)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    int i = start + 1;
        
    while (i < buffer_getSize (b) && IS_FLOAT (buffer_getAtomAtIndex (b, i))) { i++; }
    
    x->ql_waitCount = i - start;
    
    if (isAutomatic) { clock_delay (x->ql_clock, (double)GET_FLOAT (buffer_getAtomAtIndex (b, start))); }
    else {
        outlet_list (x->ql_outletLeft, x->ql_waitCount, buffer_getAtomAtIndex (b, start));
    }
}

static int qlist_proceedNext (t_qlist *x,
    int doNotSend,
    int isAutomatic,
    int start,
    int end,
    t_atomtype type)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    
    int count = end - start;
    
    if (count && !x->ql_target) {
        if (!IS_SYMBOL (buffer_getAtomAtIndex (b, start))) { return 0; }
        else {
            t_symbol *t = GET_SYMBOL (buffer_getAtomAtIndex (b, start));
            if (pd_hasThing (t)) { x->ql_target = pd_getThing (t); }
            else {
                return 0;
            }
        }
        start++;
        count--;
    }
    
    if (x->ql_target) {
    //
    x->ql_flagRewound = 0;
    
    if (!doNotSend) {
        if (!count) { pd_message (x->ql_target, &s_list, 0, NULL); }
        else {
            t_atom *first = buffer_getAtomAtIndex (b, start);
            if (IS_FLOAT (first)) { pd_message (x->ql_target, &s_list, count, first); }
            else if (IS_SYMBOL (first)) { 
                pd_message (x->ql_target, GET_SYMBOL (first), count - 1, first + 1); 
            }
        }
    }
    
    if (type != A_COMMA)   { x->ql_target = NULL; }
    if (x->ql_flagRewound) { return 1; }
    //
    }

    return 0;
}

static void qlist_proceed (t_qlist *x, int doNotSend, int isAutomatic)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    t_error err = PD_ERROR_NONE;
    
    PD_ASSERT (!x->ql_flagReentrant);
        
    x->ql_target = NULL;
    x->ql_flagReentrant = 1;
    
    while (!err) {
    //
    int i = x->ql_indexOfMessage;
    t_atomtype type;
    int start, end, size;
    
    err = (buffer_getMessageAtWithTypeOfEnd (b, i, &start, &end, &type) == 0);
    
    PD_ASSERT (x->ql_waitCount <= end - start);
    
    if (!err) {
        
        start += x->ql_waitCount;
        size = end - start;
        
        if (size && !x->ql_target && IS_FLOAT (buffer_getAtomAtIndex (b, start))) {
            qlist_proceedWait (x, doNotSend, isAutomatic, start, end);
            break;
            
        } else {
            x->ql_waitCount = 0;
            x->ql_indexOfMessage += 1;
            
            if (qlist_proceedNext (x, doNotSend, isAutomatic, start, end, type)) {
                break;
            }
        }
    }
    //
    }

    x->ql_flagReentrant = 0;
    
    if (err) { x->ql_indexOfMessage = PD_INT_MAX; outlet_bang (x->ql_outletRight); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void qlist_task (t_qlist *x)
{
    qlist_proceed (x, 0, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void qlist_bang (t_qlist *x)
{
    qlist_rewind (x);

    if (!x->ql_flagReentrant) { qlist_proceed (x, 0, 1); }
    else {
        clock_delay (x->ql_clock, 0.0);
    }
}

static void qlist_next (t_qlist *x, t_float f)
{
    if (!x->ql_flagReentrant) { qlist_proceed (x, (f != 0.0), 0); }
    else {
        error_unexpected (sym_qlist, sym_next);
    }
}

void qlist_rewind (t_qlist *x)
{
    if (x->ql_clock) { clock_unset (x->ql_clock); }
    
    x->ql_indexOfMessage = 0;
    x->ql_waitCount      = 0;
    x->ql_flagRewound    = 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void qlist_clear (t_qlist *x)
{
    qlist_rewind (x);
    buffer_clear (textbuffer_getBuffer (&x->ql_textbuffer));
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

/* Note that float arguments are always passed at last. */

static void qlist_unit (t_qlist *x, t_symbol *unitName, t_float f)
{
    t_error err = clock_setUnitParsed (x->ql_clock, f, (unitName == &s_ ? sym_millisecond : unitName));
    
    if (err) {
        error_invalid (sym_qlist, sym_unit); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *qlist_new (void)
{
    t_qlist *x = (t_qlist *)pd_new (qlist_class);
    
    textbuffer_init (&x->ql_textbuffer);
    
    x->ql_indexOfMessage = 0;
    x->ql_waitCount      = 0;
    x->ql_outletLeft     = outlet_new (cast_object (x), &s_list);
    x->ql_outletRight    = outlet_new (cast_object (x), &s_bang);
    x->ql_clock          = clock_new ((void *)x, (t_method)qlist_task);

    return x;
}

static void qlist_free (t_qlist *x)
{
    clock_free (x->ql_clock);
    textbuffer_free (&x->ql_textbuffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void qlist_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_qlist,
            (t_newmethod)qlist_new,
            (t_method)qlist_free,
            sizeof (t_qlist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, (t_method)qlist_bang);
    class_addClick (c, (t_method)textbuffer_click);
        
    class_addMethod (c, (t_method)qlist_rewind,         sym_rewind,     A_NULL);
    class_addMethod (c, (t_method)qlist_clear,          sym_clear,      A_NULL);
    class_addMethod (c, (t_method)qlist_set,            sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_add,            sym_add,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_append,         sym_append,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_next,           sym_next,       A_DEFFLOAT, A_NULL); 
    class_addMethod (c, (t_method)qlist_read,           sym_read,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_write,          sym_write,      A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_unit,           sym_unit,       A_FLOAT, A_DEFSYMBOL, A_NULL); 

    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_addLine,   sym__addline,   A_GIMME, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)qlist_append,         sym_add2,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_unit,           sym_tempo,      A_FLOAT, A_DEFSYMBOL, A_NULL);
   
    #endif
    
    qlist_class = c;
}

void qlist_destroy (void)
{
    CLASS_FREE (qlist_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
