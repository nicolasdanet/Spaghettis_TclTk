
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
#include "m_alloca.h"
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define x_obj x_tc.tc_obj
#define x_sym x_tc.tc_name
#define x_gp x_tc.tc_gpointer
#define x_struct x_tc.tc_templateIdentifier

/* ---------------- text_sequence object - sequencer ----------- */
t_class *text_sequence_class;

typedef struct _text_sequence
{
    t_textclient x_tc;
    t_outlet *x_mainout;    /* outlet for lists, zero if "global" */
    t_outlet *x_waitout;    /* outlet for wait times, zero if we never wait */
    t_outlet *x_endout;    /* bang when hit end */
    int x_onset;
    int x_argc;
    t_atom *x_argv;
    t_symbol *x_waitsym;    /* symbol to initiate wait, zero if none */
    int x_waitargc;         /* how many leading numbers to use for waiting */
    t_clock *x_clock;       /* calback for auto mode */
    t_float x_nextdelay;
    t_symbol *x_lastto;     /* destination symbol if we're after a comma */
    unsigned char x_eaten;  /* true if we've eaten leading numbers already */
    unsigned char x_loop;   /* true if we can send multiple lines */
    unsigned char x_auto;   /* set timer when we hit single-number time list */
} t_text_sequence;

static void text_sequence_tick(t_text_sequence *x);
static void text_sequence_tempo(t_text_sequence *x,
    t_symbol *unitname, t_float tempo);
void parsetimeunits(void *x, t_float amount, t_symbol *unitname,
    t_float *unit, int *samps); /* time unit parsing from x_time.c */

void *textsequence_new(t_symbol *s, int argc, t_atom *argv)
{
    t_text_sequence *x = (t_text_sequence *)pd_new(text_sequence_class);
    int global = 0;
    textclient_init(&x->x_tc, &argc, &argv);
    x->x_waitsym = 0;
    x->x_waitargc = 0;
    x->x_eaten = 0;
    x->x_loop = 0;
    x->x_lastto = 0;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-w") && argc >= 2)
        {
            if (argv[1].a_type == A_SYMBOL)
            {
                x->x_waitsym = argv[1].a_w.w_symbol;
                x->x_waitargc = 0;
            }
            else
            {
                x->x_waitsym = 0;
                if ((x->x_waitargc = argv[1].a_w.w_float) < 0)
                    x->x_waitargc = 0;
            }
            argc -= 1; argv += 1;
        }
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-g"))
            global = 1;
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-t") && argc >= 3)
        {
            text_sequence_tempo(x, atom_getSymbolAtIndex(2, argc, argv),
                atom_getFloatAtIndex(1, argc, argv));
             argc -= 2; argv += 2;
        }
        else
        {
            post_error ("text sequence: unknown flag '%s'...",
                argv->a_w.w_symbol->s_name);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post("warning: text sequence ignoring extra argument: ");
        post_atoms(argc, argv);
    }
    if (x->x_struct)
        inlet_newPointer(&x->x_obj, &x->x_gp);
    else inlet_newSymbol(&x->x_obj, &x->x_sym);
    x->x_argc = 0;
    x->x_argv = (t_atom *)PD_MEMORY_GET(0);
    x->x_onset = PD_INT_MAX;
    x->x_mainout = (!global ? outlet_new(&x->x_obj, &s_list) : 0);
    x->x_waitout = (global || x->x_waitsym || x->x_waitargc ?
        outlet_new(&x->x_obj, &s_list) : 0);
    x->x_endout = outlet_new(&x->x_obj, &s_bang);
    x->x_clock = clock_new(x, (t_method)text_sequence_tick);
    if (global)
    {
        if (x->x_waitargc)
            post_error (
       "warning: text sequence: numeric 'w' argument ignored if '-g' given");
        x->x_waitargc = 0x40000000;
    }
    return (x);
}

static void text_sequence_doit(t_text_sequence *x, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_tc), *b2;
    int n, i, onset, nfield, wait, eatsemi = 1, gotcomma = 0;
    t_atom *vec, *outvec, *ap;
    if (!b)
        goto nosequence;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (x->x_onset >= n)
    {
    nosequence:
        x->x_onset = PD_INT_MAX;
        x->x_loop = x->x_auto = 0;
        outlet_bang(x->x_endout);
        return;
    }
    onset = x->x_onset;

        /* test if leading numbers, or a leading symbol equal to our
        "wait symbol", are directing us to wait */
    if (!x->x_lastto && (
        vec[onset].a_type == A_FLOAT && x->x_waitargc && !x->x_eaten ||
            vec[onset].a_type == A_SYMBOL &&
                vec[onset].a_w.w_symbol == x->x_waitsym))
    {
        if (vec[onset].a_type == A_FLOAT)
        {
            for (i = onset; i < n && i < onset + x->x_waitargc &&
                vec[i].a_type == A_FLOAT; i++)
                    ;
            x->x_eaten = 1;
            eatsemi = 0;
        }
        else
        {
            for (i = onset; i < n && vec[i].a_type != A_SEMICOLON &&
                vec[i].a_type != A_COMMA; i++)
                    ;
            x->x_eaten = 1;
            onset++;    /* symbol isn't part of wait list */
        }
        wait = 1;
    }
    else    /* message to send */
    {
        for (i = onset; i < n && vec[i].a_type != A_SEMICOLON &&
            vec[i].a_type != A_COMMA; i++)
                ;
        wait = 0;
        x->x_eaten = 0;
        if (i < n && vec[i].a_type == A_COMMA)
            gotcomma = 1;
    }
    nfield = i - onset;
    i += eatsemi;
    if (i >= n)
        i = PD_INT_MAX;
    x->x_onset = i;
        /* generate output list, realizing dolar sign atoms.  Allocate one
        extra atom in case we want to prepend a symbol later */
    ATOMS_ALLOCA(outvec, nfield+1);
    for (i = 0, ap = vec+onset; i < nfield; i++, ap++)
    {
        int type = ap->a_type;
        if (type == A_FLOAT || type == A_SYMBOL)
            outvec[i] = *ap;
        else if (type == A_DOLLAR)
        {
            int atno = ap->a_w.w_index-1;
            if (atno < 0 || atno >= argc)
            {
                post_error ("argument $%d out of range", atno+1);
                SET_FLOAT(outvec+i, 0);
            }
            else outvec[i] = argv[atno];
        }
        else if (type == A_DOLLARSYMBOL)
        {
            t_symbol *s =
                dollar_expandDollarSymbol(ap->a_w.w_symbol, argc, argv/*, 0*/);
            if (s)
                SET_SYMBOL(outvec+i, s);
            else
            {
                post_error ("$%s: not enough arguments supplied",
                    ap->a_w.w_symbol->s_name);
                SET_SYMBOL(outvec+i, &s_symbol);
            }
        }
        else { PD_BUG; }
    }
    if (wait)
    {
        x->x_loop = 0;
        x->x_lastto = 0;
        if (x->x_auto && nfield == 1 && outvec[0].a_type == A_FLOAT)
            x->x_nextdelay = outvec[0].a_w.w_float;
        else if (!x->x_waitout) { PD_BUG; }
        else
        {
            x->x_auto = 0;
            outlet_list(x->x_waitout, 0, nfield, outvec);
        }
    }
    else if (x->x_mainout)
    {
        int n2 = nfield;
        if (x->x_lastto)
        {
            memmove(outvec+1, outvec, nfield * sizeof(*outvec));
            SET_SYMBOL(outvec, x->x_lastto);
            n2++;
        }
        if (!gotcomma)
            x->x_lastto = 0;
        else if (!x->x_lastto && nfield && outvec->a_type == A_SYMBOL)
            x->x_lastto = outvec->a_w.w_symbol;
        outlet_list(x->x_mainout, 0, n2, outvec);
    }
    else if (nfield)
    {
        t_symbol *tosym = x->x_lastto;
        t_pd *to = 0;
        t_atom *vecleft = outvec;
        int nleft = nfield;
        if (!tosym)
        {
            if (outvec[0].a_type != A_SYMBOL) { PD_BUG; }
            else tosym = outvec[0].a_w.w_symbol;
            vecleft++;
            nleft--;
        }
        if (tosym)
        {
            if (!(to = tosym->s_thing))
                post_error ("%s: no such object", tosym->s_name);
        }
        x->x_lastto = (gotcomma ? tosym : 0);
        if (to)
        {
            if (nleft > 0 && vecleft[0].a_type == A_SYMBOL)
                pd_message(to, vecleft->a_w.w_symbol, nleft-1, vecleft+1);
            else pd_list(to, nleft, vecleft);
        }
    }
    ATOMS_FREEA(outvec, nfield+1);
}

static void text_sequence_list(t_text_sequence *x, t_symbol *s, int argc,
    t_atom *argv)
{
    x->x_loop = 1;
    while (x->x_loop)
    {
        if (argc)
            text_sequence_doit(x, argc, argv);
        else text_sequence_doit(x, x->x_argc, x->x_argv);
    }
}

static void text_sequence_stop(t_text_sequence *x)
{
    x->x_loop = 0;
    if (x->x_auto)
    {
        clock_unset(x->x_clock);
        x->x_auto = 0;
    }
}

static void text_sequence_tick(t_text_sequence *x)  /* clock callback */
{
    x->x_lastto = 0;
    while (x->x_auto)
    {
        x->x_loop = 1;
        while (x->x_loop)  
            text_sequence_doit(x, x->x_argc, x->x_argv);
        if (x->x_nextdelay > 0) 
            break;
    }
    if (x->x_auto)
        clock_delay(x->x_clock, x->x_nextdelay);
}

static void text_sequence_auto(t_text_sequence *x)
{
    x->x_lastto = 0;
    if (x->x_auto)
        clock_unset(x->x_clock);
    x->x_auto = 1;
    text_sequence_tick(x);
}

static void text_sequence_step(t_text_sequence *x)
{
    text_sequence_stop(x);
    text_sequence_doit(x, x->x_argc, x->x_argv);
}

static void text_sequence_line(t_text_sequence *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_tc), *b2;
    int n, start, end;
    t_atom *vec;
    if (!b)
       return;
    x->x_lastto = 0;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (!buffer_getMessageAt(b, f, &start, &end))
    {
        post_error ("text sequence: line number %d out of range", (int)f);
        x->x_onset = PD_INT_MAX;
    }
    else x->x_onset = start;
    x->x_eaten = 0;
}

static void text_sequence_args(t_text_sequence *x, t_symbol *s,
    int argc, t_atom *argv)
{
    int i;
    x->x_argv = PD_MEMORY_RESIZE(x->x_argv,
        x->x_argc * sizeof(t_atom), argc * sizeof(t_atom));
    for (i = 0; i < argc; i++)
        x->x_argv[i] = argv[i];
    x->x_argc = argc;
}

static void text_sequence_tempo(t_text_sequence *x,
    t_symbol *unitname, t_float tempo)
{
    t_float unit;
    int samps;
    parsetimeunits(x, tempo, unitname, &unit, &samps);
    if (samps) { clock_setUnitAsSamples (x->x_clock, unit); }
    else {
        clock_setUnitAsMilliseconds (x->x_clock, unit);
    }
}

static void text_sequence_free(t_text_sequence *x)
{
    PD_MEMORY_FREE(x->x_argv);
    clock_free(x->x_clock);
    textclient_free(&x->x_tc);
}

void textsequence_setup (void)
{
    text_sequence_class = class_new(sym_text__space__sequence,
        (t_newmethod)textsequence_new, (t_method)text_sequence_free,
            sizeof(t_text_sequence), 0, A_GIMME, 0);
    class_addMethod(text_sequence_class, (t_method)text_sequence_step, 
        sym_step, 0);
    class_addMethod(text_sequence_class, (t_method)text_sequence_line, 
        sym_line, A_FLOAT, 0);
    class_addMethod(text_sequence_class, (t_method)text_sequence_auto, 
        sym_auto, 0); /* LEGACY !!! */
    class_addMethod(text_sequence_class, (t_method)text_sequence_stop, 
        sym_stop, 0);
    class_addMethod(text_sequence_class, (t_method)text_sequence_args, 
        sym_args, A_GIMME, 0);   /* LEGACY !!! */
    class_addMethod(text_sequence_class, (t_method)text_sequence_tempo, 
        sym_tempo, A_FLOAT, A_SYMBOL, 0);  /* LEGACY !!! */
    class_addMethod(text_sequence_class, (t_method)text_sequence_tempo, 
        sym_unit, A_FLOAT, A_SYMBOL, 0);
    class_addList(text_sequence_class, text_sequence_list);
    class_setHelpName(text_sequence_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
