
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
    if (x->ql_checkIfNextIsRecursive)
    {
        post_error ("qlist sent 'next' from within itself");
        return;
    }
    x->ql_checkIfNextIsRecursive = 1;
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
                clock_delay(x->ql_clock,
                    x->ql_delay = ap->a_w.w_float * x->ql_unit);
                x->ql_lastLogicalTime = scheduler_getLogicalTime();
            }
            else outlet_list(cast_object (x)->te_outlet, 0, onset2-onset, ap);
            x->ql_checkIfNextIsRecursive = 0;
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
            x->ql_checkIfNextIsRecursive = 0;
            return;
        }
        x->ql_hasBeenRewound = wasrewound;
    }  /* while (1); never falls through */

end:
    x->ql_indexOfStart = PD_INT_MAX;
    x->ql_lastLogicalTime = 0;
    x->ql_checkIfNextIsRecursive = 0;
    outlet_bang(x->ql_outletRight);
}

static void qlist_tick(t_qlist *x)
{
    x->ql_lastLogicalTime = 0;
    qlist_donext(x, 0, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void qlist_bang(t_qlist *x)
{
    qlist_rewind(x);
        /* if we're restarted reentrantly from a "next" message set ourselves
        up to do this non-reentrantly after a delay of 0 */
    if (x->ql_checkIfNextIsRecursive)
    {
        x->ql_lastLogicalTime = scheduler_getLogicalTime();
        x->ql_delay = 0;
        clock_delay(x->ql_clock, 0);
    }
    else qlist_donext(x, 0, 1);
}

void qlist_rewind (t_qlist *x)
{
    x->ql_indexOfStart = 0;
    if (x->ql_clock) clock_unset(x->ql_clock);
    x->ql_lastLogicalTime = 0;
    x->ql_hasBeenRewound = 1;
}

void qlist_clear(t_qlist *x)
{
    qlist_rewind(x);
    buffer_reset(textbuffer_getBuffer (&x->ql_textbuffer));
}

void qlist_set(t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    qlist_clear(x);
    qlist_add(x, s, argc, argv);
}

void qlist_add(t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom a;
    SET_SEMICOLON(&a);
    buffer_append(textbuffer_getBuffer (&x->ql_textbuffer), argc, argv);
    buffer_appendAtom(textbuffer_getBuffer (&x->ql_textbuffer), &a);
}

void qlist_add2(t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append(textbuffer_getBuffer (&x->ql_textbuffer), argc, argv);
}

static void qlist_tempo(t_qlist *x, t_float f)
{
    t_float newtempo;
    if (f < 1e-20) f = 1e-20;
    else if (f > 1e20) f = 1e20;
    newtempo = 1./f;
    if (x->ql_lastLogicalTime != 0)
    {
        t_float elapsed = scheduler_getMillisecondsSince(x->ql_lastLogicalTime);
        t_float left = x->ql_delay - elapsed;
        if (left < 0) left = 0;
        left *= newtempo / x->ql_unit;
        clock_delay(x->ql_clock, left);
    }
    x->ql_unit = newtempo;
}

static void qlist_next(t_qlist *x, t_float drop)
{
    qlist_donext(x, drop != 0, 0);
}

void qlist_read(t_qlist *x, t_symbol *filename)
{
    /*
    int cr = 0;
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);
    */
    
    if (buffer_read(textbuffer_getBuffer (&x->ql_textbuffer), filename, textbuffer_getView (&x->ql_textbuffer)))
            post_error ("%s: read failed", filename->s_name);
    x->ql_indexOfStart = PD_INT_MAX;
    x->ql_hasBeenRewound = 1;
}

void qlist_write(t_qlist *x, t_symbol *filename)
{
    int cr = 0;
    char buf[PD_STRING];
    canvas_makeFilePath(textbuffer_getView (&x->ql_textbuffer), filename->s_name,
        buf, PD_STRING);
    /*
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);*/
        
    if (buffer_write(textbuffer_getBuffer (&x->ql_textbuffer), buf, ""))
            post_error ("%s: write failed", filename->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *qlist_new (void)
{
    t_qlist *x = (t_qlist *)pd_new (qlist_class);
    
    textbuffer_init (&x->ql_textbuffer);
    
    x->ql_unit                   = 1.0;
    x->ql_delay                  = 0.0;
    x->ql_lastLogicalTime        = 0.0;
    x->ql_indexOfStart           = PD_INT_MAX;
    x->ql_hasBeenRewound         = 0;
    x->ql_checkIfNextIsRecursive = 0;
    x->ql_outletLeft             = outlet_new (cast_object (x), &s_list);
    x->ql_outletRight            = outlet_new (cast_object (x), &s_bang);
    x->ql_clock                  = clock_new (x, (t_method)qlist_tick);

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
    class_addMethod (c, (t_method)qlist_add2,           sym_append,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_tempo,          sym_unit,       A_FLOAT, A_NULL); 
    class_addMethod (c, (t_method)qlist_next,           sym_next,       A_DEFFLOAT, A_NULL); 
    class_addMethod (c, (t_method)qlist_read,           sym_read,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_write,          sym_write,      A_SYMBOL, A_NULL);

    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_add,       sym__addline,   A_GIMME, A_NULL);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)qlist_add2,           sym_add2,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_tempo,          sym_tempo,      A_FLOAT, A_NULL);
   
    #endif
    
    qlist_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
