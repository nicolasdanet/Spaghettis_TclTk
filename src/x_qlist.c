
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

static t_class *qlist_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void qlist_donext(t_qlist *x, int drop, int automatic)
{
    t_pd *target = 0;
    if (x->ql_isReentrant)
    {
        post_error ("qlist sent 'next' from within itself");
        return;
    }
    x->ql_isReentrant = 1;
    while (1)
    {
        int argc = buffer_size(textbuffer_getBuffer (&x->ql_textbuffer)),
            count, onset = x->ql_indexOfStart, onset2, wasrewound;
        t_atom *argv = buffer_atoms(textbuffer_getBuffer (&x->ql_textbuffer));
        t_atom *ap = argv + onset, *ap2;
        if (onset >= argc) goto end;
        while (ap->a_type == A_SEMICOLON || ap->a_type == A_COMMA)
        {
            if (ap->a_type == A_SEMICOLON) target = 0;
            onset++, ap++;
            if (onset >= argc) goto end;
        }

        if (!target && ap->a_type == A_FLOAT)
        {
            ap2 = ap + 1;
            onset2 = onset + 1;
            while (onset2 < argc && ap2->a_type == A_FLOAT)
                onset2++, ap2++;
            x->ql_indexOfStart = onset2;
            if (automatic)
            {
                clock_delay(x->ql_clock, ap->a_w.w_float);
            }
            else outlet_list(cast_object (x)->te_outlet, 0, onset2-onset, ap);
            x->ql_isReentrant = 0;
            return;
        }
        ap2 = ap + 1;
        onset2 = onset + 1;
        while (onset2 < argc &&
            (ap2->a_type == A_FLOAT || ap2->a_type == A_SYMBOL))
                onset2++, ap2++;
        x->ql_indexOfStart = onset2;
        count = onset2 - onset;
        if (!target)
        {
            if (ap->a_type != A_SYMBOL) continue;
            else if (!(target = ap->a_w.w_symbol->s_thing))
            {
                post_error ("qlist: %s: no such object",
                    ap->a_w.w_symbol->s_name);
                continue;
            }
            ap++;
            onset++;
            count--;
            if (!count) 
            {
                x->ql_indexOfStart = onset2;
                continue;
            }
        }
        wasrewound = x->ql_hasBeenRewound;
        x->ql_hasBeenRewound = 0;
        if (!drop)
        {   
            if (ap->a_type == A_FLOAT)
                pd_message(target, &s_list, count, ap);
            else if (ap->a_type == A_SYMBOL)
                pd_message(target, ap->a_w.w_symbol, count-1, ap+1);
        }
        if (x->ql_hasBeenRewound)
        {
            x->ql_isReentrant = 0;
            return;
        }
        x->ql_hasBeenRewound = wasrewound;
    }  /* while (1); never falls through */

end:
    x->ql_indexOfStart = PD_INT_MAX;
    x->ql_isReentrant = 0;
    outlet_bang(x->ql_outletRight);
}

static void qlist_tick (t_qlist *x)
{
    qlist_donext (x, 0, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void qlist_bang (t_qlist *x)
{
    qlist_rewind (x);

    if (!x->ql_isReentrant) { qlist_donext (x, 0, 1); }
    else {
        clock_delay (x->ql_clock, 0.0);
    }
}

void qlist_rewind (t_qlist *x)
{
    if (x->ql_clock) { clock_unset (x->ql_clock); }
    
    x->ql_indexOfStart   = 0;
    x->ql_hasBeenRewound = 1;
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

static void qlist_next (t_qlist *x, t_float f)
{
    qlist_donext (x, (f != 0.0), 0);
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
    
    x->ql_indexOfStart      = PD_INT_MAX;
    x->ql_hasBeenRewound    = 0;
    x->ql_isReentrant       = 0;
    x->ql_outletLeft        = outlet_new (cast_object (x), &s_list);
    x->ql_outletRight       = outlet_new (cast_object (x), &s_bang);
    x->ql_clock             = clock_new (x, (t_method)qlist_tick);

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
    class_addMethod (c, (t_method)textbuffer_add,       sym__addline,   A_GIMME, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)qlist_append,         sym_add2,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_unit,           sym_tempo,      A_FLOAT, A_DEFSYMBOL, A_NULL);
   
    #endif
    
    qlist_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
