
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

static t_class *textsequence_class;                 /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textsequence {
    t_textclient        x_textclient;
    t_float             x_delay;
    int                 x_onset;
    int                 x_leadingNumbersToWait;
    int                 x_isLeadingNumbersEaten;
    int                 x_isAutomatic;
    int                 x_isLooping;
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

static void textsequence_stop   (t_textsequence *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsequence_perform(t_textsequence *x, int argc, t_atom *argv)
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
        x->x_isLooping = x->x_isAutomatic = 0;
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
        x->x_isLooping = 0;
        x->x_sendTo = 0;
        if (x->x_isAutomatic && nfield == 1 && outvec[0].a_type == A_FLOAT)
            x->x_delay = outvec[0].a_w.w_float;
        else if (!x->x_outletWait) { PD_BUG; }
        else
        {
            x->x_isAutomatic = 0;
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

static void textsequence_tick (t_textsequence *x)
{
    x->x_sendTo = 0;
    while (x->x_isAutomatic)
    {
        x->x_isLooping = 1;
        while (x->x_isLooping)  
            textsequence_perform(x, x->x_argc, x->x_argv);
        if (x->x_delay > 0) 
            break;
    }
    if (x->x_isAutomatic)
        clock_delay(x->x_clock, x->x_delay);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsequence_bang (t_textsequence *x)
{
    x->x_isLooping = 1; while (x->x_isLooping) { textsequence_perform (x, x->x_argc, x->x_argv); }
}

static void textsequence_list (t_textsequence *x, t_symbol *s, int argc, t_atom *argv)
{
    x->x_isLooping = 1;
    
    while (x->x_isLooping) {
    //
    if (argc) { textsequence_perform (x, argc, argv); }
    else {
        textsequence_perform (x, x->x_argc, x->x_argv);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsequence_stop (t_textsequence *x)
{
    x->x_isLooping = 0; if (x->x_isAutomatic) { clock_unset (x->x_clock); x->x_isAutomatic = 0; }
}

static void textsequence_step (t_textsequence *x)
{
    textsequence_stop (x); textsequence_perform (x, x->x_argc, x->x_argv);
}

static void textsequence_automatic (t_textsequence *x)
{
    x->x_sendTo = NULL;
    
    if (x->x_isAutomatic) { clock_unset (x->x_clock); } else { x->x_isAutomatic = 1; }
    
    textsequence_tick (x);
}

static void textsequence_line (t_textsequence *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);

    if (b) {
    //
    int start, end;
    
    x->x_sendTo = NULL;
    x->x_isLeadingNumbersEaten = 0;
    
    if (!buffer_getMessageAt (b, f, &start, &end)) { x->x_onset = PD_INT_MAX; }
    else {
        x->x_onset = start;
    }
    //
    } else { error_undefined (sym_text__space__sequence, sym_text); }
}

static void textsequence_arguments (t_textsequence *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    size_t oldSize = x->x_argc * sizeof (t_atom);
    size_t newSize = argc * sizeof(t_atom);
    
    x->x_argc = argc;
    x->x_argv = PD_MEMORY_RESIZE (x->x_argv, oldSize, newSize);
    
    for (i = 0; i < argc; i++) { x->x_argv[i] = argv[i]; }
}

static void textsequence_unit (t_textsequence *x, t_float f, t_symbol *unitName)
{
    t_float n; int isSamples;
    t_error err = clock_parseUnit (f, unitName, &n, &isSamples);
    
    if (err) { error_invalid (sym_text__space__sequence, sym_unit); }
    else {
        if (isSamples) { clock_setUnitAsSamples (x->x_clock, n); }
        else {
            clock_setUnitAsMilliseconds (x->x_clock, n);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textsequence_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textsequence *x = (t_textsequence *)pd_new (textsequence_class);
        
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);         /* It may consume arguments. */
    
    if (!err) {
    
        int hasWait = 0;
        int useGlobal = 0;
        
        while (argc > 0) {

            t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
        
            if (t == sym___dash__g || t == sym___dash__global) { 
            
                useGlobal = 1; argc--; argv++; 
                
            } else if (argc >= 2 && (t == sym___dash__w || t == sym___dash__wait)) {
                
                if (!x->x_symbolToWait && !x->x_leadingNumbersToWait) {
                //
                if (IS_SYMBOL (argv + 1)) { x->x_symbolToWait = atom_getSymbol (argv + 1); }
                else {
                    x->x_leadingNumbersToWait = PD_MAX (0, (int)atom_getFloat (argv + 1));
                }
                argc -= 2; argv += 2;
                //
                }
                
            } else if (argc >= 3 && (t == sym___dash__u || t == sym___dash__unit)) {
            
                textsequence_unit (x, atom_getFloat (argv + 1), atom_getSymbol (argv + 2));
                argc -= 3; argv += 3;
                
            } else {
                break;
            }
        }
        
        error__options (s, argc, argv);

        if (argc) { warning_unusedArguments (s, argc, argv); }
        
        hasWait = (useGlobal || x->x_symbolToWait || x->x_leadingNumbersToWait);
        
        x->x_onset = PD_INT_MAX;
        x->x_argc  = 0;
        x->x_argv  = (t_atom *)PD_MEMORY_GET (0);
        x->x_clock = clock_new (x, (t_method)textsequence_tick);
        
        if (!useGlobal) { x->x_outletMain = outlet_new (cast_object (x), &s_list); }
        if (hasWait)    { x->x_outletWait = outlet_new (cast_object (x), &s_list); }
        
        x->x_outletEnd = outlet_new (cast_object (x), &s_bang);
        
        if (useGlobal) { x->x_leadingNumbersToWait = 0x40000000; }      /* ASAP. */
    
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x), TEXTCLIENT_GETNAME (&x->x_textclient));
        }
        
    } else {
        error_invalidArguments (sym_text__space__search, argc, argv);
        pd_free (x); x = NULL;
    }
    
    return x;
}

static void textsequence_free (t_textsequence *x)
{
    if (x->x_argv)  { PD_MEMORY_FREE (x->x_argv); }
    if (x->x_clock) { clock_free (x->x_clock); }
    
    textclient_free (&x->x_textclient);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textsequence_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__sequence,
            (t_newmethod)textsequence_new,
            (t_method)textsequence_free,
            sizeof (t_textsequence),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, textsequence_bang);
    class_addList (c, textsequence_list);
    
    class_addMethod (c, (t_method)textsequence_stop,        sym_stop,       A_NULL);
    class_addMethod (c, (t_method)textsequence_step,        sym_step,       A_NULL);
    class_addMethod (c, (t_method)textsequence_automatic,   sym_automatic,  A_NULL);
    class_addMethod (c, (t_method)textsequence_line,        sym_line,       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)textsequence_arguments,   sym_arguments,  A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textsequence_unit,        sym_unit,       A_FLOAT, A_SYMBOL, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)textsequence_automatic,   sym_auto,       A_NULL);
    class_addMethod (c, (t_method)textsequence_arguments,   sym_args,       A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textsequence_unit,        sym_tempo,      A_FLOAT, A_SYMBOL, A_NULL);
        
    #endif 

    class_setHelpName (c, sym_text);
    
    textsequence_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
