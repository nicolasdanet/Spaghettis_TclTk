
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
static t_class *textfile_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _qlist {
    t_textbuffer x_textbuf;
    t_outlet *x_bangout;
    int x_onset;                /* playback position */
    t_clock *x_clock;
    t_float x_tempo;
    double x_whenclockset;
    t_float x_clockdelay;
    int x_rewound;              /* we've been rewound since last start */
    int x_innext;               /* we're currently inside the "next" routine */
    } t_qlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void qlist_rewind(t_qlist *x)
{
    x->x_onset = 0;
    if (x->x_clock) clock_unset(x->x_clock);
    x->x_whenclockset = 0;
    x->x_rewound = 1;
}

static void qlist_donext(t_qlist *x, int drop, int automatic)
{
    t_pd *target = 0;
    if (x->x_innext)
    {
        post_error ("qlist sent 'next' from within itself");
        return;
    }
    x->x_innext = 1;
    while (1)
    {
        int argc = buffer_size(textbuffer_getBuffer (&x->x_textbuf)),
            count, onset = x->x_onset, onset2, wasrewound;
        t_atom *argv = buffer_atoms(textbuffer_getBuffer (&x->x_textbuf));
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
            x->x_onset = onset2;
            if (automatic)
            {
                clock_delay(x->x_clock,
                    x->x_clockdelay = ap->a_w.w_float * x->x_tempo);
                x->x_whenclockset = scheduler_getLogicalTime();
            }
            else outlet_list(cast_object (x)->te_outlet, 0, onset2-onset, ap);
            x->x_innext = 0;
            return;
        }
        ap2 = ap + 1;
        onset2 = onset + 1;
        while (onset2 < argc &&
            (ap2->a_type == A_FLOAT || ap2->a_type == A_SYMBOL))
                onset2++, ap2++;
        x->x_onset = onset2;
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
                x->x_onset = onset2;
                continue;
            }
        }
        wasrewound = x->x_rewound;
        x->x_rewound = 0;
        if (!drop)
        {   
            if (ap->a_type == A_FLOAT)
                pd_message(target, &s_list, count, ap);
            else if (ap->a_type == A_SYMBOL)
                pd_message(target, ap->a_w.w_symbol, count-1, ap+1);
        }
        if (x->x_rewound)
        {
            x->x_innext = 0;
            return;
        }
        x->x_rewound = wasrewound;
    }  /* while (1); never falls through */

end:
    x->x_onset = PD_INT_MAX;
    x->x_whenclockset = 0;
    x->x_innext = 0;
    outlet_bang(x->x_bangout);
}

static void qlist_next(t_qlist *x, t_float drop)
{
    qlist_donext(x, drop != 0, 0);
}

static void qlist_bang(t_qlist *x)
{
    qlist_rewind(x);
        /* if we're restarted reentrantly from a "next" message set ourselves
        up to do this non-reentrantly after a delay of 0 */
    if (x->x_innext)
    {
        x->x_whenclockset = scheduler_getLogicalTime();
        x->x_clockdelay = 0;
        clock_delay(x->x_clock, 0);
    }
    else qlist_donext(x, 0, 1);
}

static void qlist_tick(t_qlist *x)
{
    x->x_whenclockset = 0;
    qlist_donext(x, 0, 1);
}

static void qlist_add(t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom a;
    SET_SEMICOLON(&a);
    buffer_append(textbuffer_getBuffer (&x->x_textbuf), argc, argv);
    buffer_appendAtom(textbuffer_getBuffer (&x->x_textbuf), &a);
}

static void qlist_add2(t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_append(textbuffer_getBuffer (&x->x_textbuf), argc, argv);
}

static void qlist_clear(t_qlist *x)
{
    qlist_rewind(x);
    buffer_reset(textbuffer_getBuffer (&x->x_textbuf));
}

static void qlist_set(t_qlist *x, t_symbol *s, int argc, t_atom *argv)
{
    qlist_clear(x);
    qlist_add(x, s, argc, argv);
}

static void qlist_read(t_qlist *x, t_symbol *filename, t_symbol *format)
{
    int cr = 0;
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);

    if (buffer_read(textbuffer_getBuffer (&x->x_textbuf), filename, textbuffer_getView (&x->x_textbuf)))
            post_error ("%s: read failed", filename->s_name);
    x->x_onset = PD_INT_MAX;
    x->x_rewound = 1;
}

static void qlist_write(t_qlist *x, t_symbol *filename, t_symbol *format)
{
    int cr = 0;
    char buf[PD_STRING];
    canvas_makeFilePath(textbuffer_getView (&x->x_textbuf), filename->s_name,
        buf, PD_STRING);
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);
    if (buffer_write(textbuffer_getBuffer (&x->x_textbuf), buf, ""))
            post_error ("%s: write failed", filename->s_name);
}

static void qlist_tempo(t_qlist *x, t_float f)
{
    t_float newtempo;
    if (f < 1e-20) f = 1e-20;
    else if (f > 1e20) f = 1e20;
    newtempo = 1./f;
    if (x->x_whenclockset != 0)
    {
        t_float elapsed = scheduler_getMillisecondsSince(x->x_whenclockset);
        t_float left = x->x_clockdelay - elapsed;
        if (left < 0) left = 0;
        left *= newtempo / x->x_tempo;
        clock_delay(x->x_clock, left);
    }
    x->x_tempo = newtempo;
}

static void qlist_free(t_qlist *x)
{
    textbuffer_free (&x->x_textbuf);
    clock_free(x->x_clock);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *textfile_new( void)
{
    t_qlist *x = (t_qlist *)pd_new(textfile_class);
    textbuffer_init (&x->x_textbuf);
    outlet_new(cast_object (x), &s_list);
    x->x_bangout = outlet_new(cast_object (x), &s_bang);
    x->x_onset = PD_INT_MAX;
    x->x_rewound = 0;
    x->x_tempo = 1;
    x->x_whenclockset = 0;
    x->x_clockdelay = 0;
    x->x_clock = NULL;
    return (x);
}

static void textfile_bang(t_qlist *x)
{
    int argc = buffer_size(textbuffer_getBuffer (&x->x_textbuf)),
        count, onset = x->x_onset, onset2;
    t_atom *argv = buffer_atoms(textbuffer_getBuffer (&x->x_textbuf));
    t_atom *ap = argv + onset, *ap2;
    while (onset < argc &&
        (ap->a_type == A_SEMICOLON || ap->a_type == A_COMMA))
            onset++, ap++;
    onset2 = onset;
    ap2 = ap;
    while (onset2 < argc &&
        (ap2->a_type != A_SEMICOLON && ap2->a_type != A_COMMA))
            onset2++, ap2++;
    if (onset2 > onset)
    {
        x->x_onset = onset2;
        if (ap->a_type == A_SYMBOL)
            outlet_anything(cast_object (x)->te_outlet, ap->a_w.w_symbol,
                onset2-onset-1, ap+1);
        else outlet_list(cast_object (x)->te_outlet, 0, onset2-onset, ap);
    }
    else
    {
        x->x_onset = PD_INT_MAX;
        outlet_bang(x->x_bangout);
    }
}

static void textfile_rewind(t_qlist *x)
{
    x->x_onset = 0;
}


static void *qlist_new( void)
{
    t_qlist *x = (t_qlist *)pd_new(qlist_class);
    textbuffer_init (&x->x_textbuf);
    x->x_clock = clock_new(x, (t_method)qlist_tick);
    outlet_new(cast_object (x), &s_list);
    x->x_bangout = outlet_new(cast_object (x), &s_bang);
    x->x_onset = PD_INT_MAX;
    x->x_tempo = 1;
    x->x_whenclockset = 0;
    x->x_clockdelay = 0;
    x->x_rewound = x->x_innext = 0;
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void x_qlist_setup(void )
{
    qlist_class = class_new(sym_qlist, (t_newmethod)qlist_new,
        (t_method)qlist_free, sizeof(t_qlist), 0, 0);
    class_addMethod(qlist_class, (t_method)qlist_rewind, sym_rewind, 0);
    class_addMethod(qlist_class, (t_method)qlist_next,
        sym_next, A_DEFFLOAT, 0);  
    class_addMethod(qlist_class, (t_method)qlist_set, sym_set,
        A_GIMME, 0);
    class_addMethod(qlist_class, (t_method)qlist_clear, sym_clear, 0);
    class_addMethod(qlist_class, (t_method)qlist_add, sym_add,
        A_GIMME, 0);
    class_addMethod(qlist_class, (t_method)qlist_add2, sym_add2, /* LEGACY !!! */
        A_GIMME, 0);
    class_addMethod(qlist_class, (t_method)qlist_add, sym_append, /* LEGACY !!! */
        A_GIMME, 0);
    class_addMethod(qlist_class, (t_method)qlist_read, sym_read,
        A_SYMBOL, A_DEFSYMBOL, 0);
    class_addMethod(qlist_class, (t_method)qlist_write, sym_write,
        A_SYMBOL, A_DEFSYMBOL, 0);
    class_addClick (qlist_class, textbuffer_click);
    //class_addMethod(qlist_class, (t_method)textbuffer_open, sym_click, 0);
    class_addMethod(qlist_class, (t_method)textbuffer_close, sym_close, 0);
    class_addMethod(qlist_class, (t_method)textbuffer_add, 
        sym__addline, A_GIMME, 0);
    /*class_addMethod(qlist_class, (t_method)qlist_print, gen_sym ("print"),
        A_DEFSYMBOL, 0);*/
    class_addMethod(qlist_class, (t_method)qlist_tempo,
        sym_tempo, A_FLOAT, 0); /* LEGACY !!! */
    class_addMethod(qlist_class, (t_method)qlist_tempo,
        sym_unit, A_FLOAT, 0);
    class_addBang(qlist_class, qlist_bang);

    textfile_class = class_new(sym_textfile, (t_newmethod)textfile_new,
        (t_method)textbuffer_free, sizeof(t_qlist), 0, 0);
    class_addMethod(textfile_class, (t_method)textfile_rewind, sym_rewind,
        0);
    class_addMethod(textfile_class, (t_method)qlist_set, sym_set,
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_clear, sym_clear, 0);
    class_addMethod(textfile_class, (t_method)qlist_add, sym_add,
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_add2, sym_add2, /* LEGACY !!! */
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_add, sym_append, /* LEGACY !!! */
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_read, sym_read, 
        A_SYMBOL, A_DEFSYMBOL, 0);
    class_addMethod(textfile_class, (t_method)qlist_write, sym_write, 
        A_SYMBOL, A_DEFSYMBOL, 0);
    class_addClick (textfile_class, textbuffer_click);
    //class_addMethod(textfile_class, (t_method)textbuffer_open, sym_click, 0);
    class_addMethod(textfile_class, (t_method)textbuffer_close, sym_close, 
        0);
    class_addMethod(textfile_class, (t_method)textbuffer_add, 
        sym__addline, A_GIMME, 0);
    /*class_addMethod(textfile_class, (t_method)qlist_print, gen_sym ("print"),
        A_DEFSYMBOL, 0);*/
    class_addBang(textfile_class, textfile_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
