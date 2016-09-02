
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

static t_class *textsequence_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textsequence {
    t_textclient        x_textclient;
    t_float             x_delay;
    int                 x_onset;
    int                 x_leadingNumbersToWait;
    int                 x_isLeadingNumbersEaten;
    int                 x_isLoop;
    int                 x_isAuto;
    int                 x_argc;
    t_atom              *x_argv;
    t_symbol            *x_sendTo;
    t_symbol            *x_symbolToWait;
    t_outlet            *x_outletMain;
    t_outlet            *x_outletWait; 
    t_outlet            *x_outletEnd;
    t_clock             *x_clock;
    } t_textsequence;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsequence_tick (t_textsequence *x);
static void textsequence_tempo (t_textsequence *x, t_symbol *unitname, t_float tempo);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsequence_doit(t_textsequence *x, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_textclient), *b2;
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
        x->x_isLoop = x->x_isAuto = 0;
        outlet_bang(x->x_outletEnd);
        return;
    }
    onset = x->x_onset;

        /* test if leading numbers, or a leading symbol equal to our
        "wait symbol", are directing us to wait */
    if (!x->x_sendTo && (
        vec[onset].a_type == A_FLOAT && x->x_leadingNumbersToWait && !x->x_isLeadingNumbersEaten ||
            vec[onset].a_type == A_SYMBOL &&
                vec[onset].a_w.w_symbol == x->x_symbolToWait))
    {
        if (vec[onset].a_type == A_FLOAT)
        {
            for (i = onset; i < n && i < onset + x->x_leadingNumbersToWait &&
                vec[i].a_type == A_FLOAT; i++)
                    ;
            x->x_isLeadingNumbersEaten = 1;
            eatsemi = 0;
        }
        else
        {
            for (i = onset; i < n && vec[i].a_type != A_SEMICOLON &&
                vec[i].a_type != A_COMMA; i++)
                    ;
            x->x_isLeadingNumbersEaten = 1;
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
        x->x_isLeadingNumbersEaten = 0;
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
        x->x_isLoop = 0;
        x->x_sendTo = 0;
        if (x->x_isAuto && nfield == 1 && outvec[0].a_type == A_FLOAT)
            x->x_delay = outvec[0].a_w.w_float;
        else if (!x->x_outletWait) { PD_BUG; }
        else
        {
            x->x_isAuto = 0;
            outlet_list(x->x_outletWait, 0, nfield, outvec);
        }
    }
    else if (x->x_outletMain)
    {
        int n2 = nfield;
        if (x->x_sendTo)
        {
            memmove(outvec+1, outvec, nfield * sizeof(*outvec));
            SET_SYMBOL(outvec, x->x_sendTo);
            n2++;
        }
        if (!gotcomma)
            x->x_sendTo = 0;
        else if (!x->x_sendTo && nfield && outvec->a_type == A_SYMBOL)
            x->x_sendTo = outvec->a_w.w_symbol;
        outlet_list(x->x_outletMain, 0, n2, outvec);
    }
    else if (nfield)
    {
        t_symbol *tosym = x->x_sendTo;
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
        x->x_sendTo = (gotcomma ? tosym : 0);
        if (to)
        {
            if (nleft > 0 && vecleft[0].a_type == A_SYMBOL)
                pd_message(to, vecleft->a_w.w_symbol, nleft-1, vecleft+1);
            else pd_list(to, nleft, vecleft);
        }
    }
    ATOMS_FREEA(outvec, nfield+1);
}

static void textsequence_list(t_textsequence *x, t_symbol *s, int argc,
    t_atom *argv)
{
    x->x_isLoop = 1;
    while (x->x_isLoop)
    {
        if (argc)
            textsequence_doit(x, argc, argv);
        else textsequence_doit(x, x->x_argc, x->x_argv);
    }
}

static void textsequence_stop(t_textsequence *x)
{
    x->x_isLoop = 0;
    if (x->x_isAuto)
    {
        clock_unset(x->x_clock);
        x->x_isAuto = 0;
    }
}

static void textsequence_tick(t_textsequence *x)  /* clock callback */
{
    x->x_sendTo = 0;
    while (x->x_isAuto)
    {
        x->x_isLoop = 1;
        while (x->x_isLoop)  
            textsequence_doit(x, x->x_argc, x->x_argv);
        if (x->x_delay > 0) 
            break;
    }
    if (x->x_isAuto)
        clock_delay(x->x_clock, x->x_delay);
}

static void textsequence_auto(t_textsequence *x)
{
    x->x_sendTo = 0;
    if (x->x_isAuto)
        clock_unset(x->x_clock);
    x->x_isAuto = 1;
    textsequence_tick(x);
}

static void textsequence_step(t_textsequence *x)
{
    textsequence_stop(x);
    textsequence_doit(x, x->x_argc, x->x_argv);
}

static void textsequence_line(t_textsequence *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer(&x->x_textclient), *b2;
    int n, start, end;
    t_atom *vec;
    if (!b)
       return;
    x->x_sendTo = 0;
    vec = buffer_atoms(b);
    n = buffer_size(b);
    if (!buffer_getMessageAt(b, f, &start, &end))
    {
        post_error ("text sequence: line number %d out of range", (int)f);
        x->x_onset = PD_INT_MAX;
    }
    else x->x_onset = start;
    x->x_isLeadingNumbersEaten = 0;
}

static void textsequence_args(t_textsequence *x, t_symbol *s,
    int argc, t_atom *argv)
{
    int i;
    x->x_argv = PD_MEMORY_RESIZE(x->x_argv,
        x->x_argc * sizeof(t_atom), argc * sizeof(t_atom));
    for (i = 0; i < argc; i++)
        x->x_argv[i] = argv[i];
    x->x_argc = argc;
}

static void textsequence_tempo(t_textsequence *x,
    t_symbol *unitname, t_float tempo)
{
    t_float unit;
    int samps;
    time_parseUnits (tempo, unitname, &unit, &samps);
    if (samps) { clock_setUnitAsSamples (x->x_clock, unit); }
    else {
        clock_setUnitAsMilliseconds (x->x_clock, unit);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textsequence_new(t_symbol *s, int argc, t_atom *argv)
{
    t_textsequence *x = (t_textsequence *)pd_new(textsequence_class);
    int global = 0;
    textclient_init(&x->x_textclient, &argc, &argv);
    x->x_symbolToWait = 0;
    x->x_leadingNumbersToWait = 0;
    x->x_isLeadingNumbersEaten = 0;
    x->x_isLoop = 0;
    x->x_sendTo = 0;
    while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-w") && argc >= 2)
        {
            if (argv[1].a_type == A_SYMBOL)
            {
                x->x_symbolToWait = argv[1].a_w.w_symbol;
                x->x_leadingNumbersToWait = 0;
            }
            else
            {
                x->x_symbolToWait = 0;
                if ((x->x_leadingNumbersToWait = argv[1].a_w.w_float) < 0)
                    x->x_leadingNumbersToWait = 0;
            }
            argc -= 1; argv += 1;
        }
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-g"))
            global = 1;
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-t") && argc >= 3)
        {
            textsequence_tempo(x, atom_getSymbolAtIndex(2, argc, argv),
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
        error__post (argc, argv);
    }
    if (TEXTCLIENT_ASPOINTER (&x->x_textclient))
        inlet_newPointer(cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
    else inlet_newSymbol(cast_object (x), TEXTCLIENT_GETNAME (&x->x_textclient));
    x->x_argc = 0;
    x->x_argv = (t_atom *)PD_MEMORY_GET(0);
    x->x_onset = PD_INT_MAX;
    x->x_outletMain = (!global ? outlet_new(cast_object (x), &s_list) : 0);
    x->x_outletWait = (global || x->x_symbolToWait || x->x_leadingNumbersToWait ?
        outlet_new(cast_object (x), &s_list) : 0);
    x->x_outletEnd = outlet_new(cast_object (x), &s_bang);
    x->x_clock = clock_new(x, (t_method)textsequence_tick);
    if (global)
    {
        if (x->x_leadingNumbersToWait)
            post_error (
       "warning: text sequence: numeric 'w' argument ignored if '-g' given");
        x->x_leadingNumbersToWait = 0x40000000;
    }
    return (x);
}

static void textsequence_free(t_textsequence *x)
{
    PD_MEMORY_FREE(x->x_argv);
    clock_free(x->x_clock);
    textclient_free(&x->x_textclient);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textsequence_setup (void)
{
    textsequence_class = class_new(sym_text__space__sequence,
        (t_newmethod)textsequence_new, (t_method)textsequence_free,
            sizeof(t_textsequence), 0, A_GIMME, 0);
    class_addMethod(textsequence_class, (t_method)textsequence_step, 
        sym_step, 0);
    class_addMethod(textsequence_class, (t_method)textsequence_line, 
        sym_line, A_FLOAT, 0);
    class_addMethod(textsequence_class, (t_method)textsequence_auto, 
        sym_auto, 0); /* LEGACY !!! */
    class_addMethod(textsequence_class, (t_method)textsequence_stop, 
        sym_stop, 0);
    class_addMethod(textsequence_class, (t_method)textsequence_args, 
        sym_args, A_GIMME, 0);   /* LEGACY !!! */
    class_addMethod(textsequence_class, (t_method)textsequence_tempo, 
        sym_tempo, A_FLOAT, A_SYMBOL, 0);  /* LEGACY !!! */
    class_addMethod(textsequence_class, (t_method)textsequence_tempo, 
        sym_unit, A_FLOAT, A_SYMBOL, 0);
    class_addList(textsequence_class, textsequence_list);
    class_setHelpName(textsequence_class, sym_text);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
