
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
    int                 x_leadingToWait;
    int                 x_indexOfStart;
    int                 x_isAutomatic;
    int                 x_isLooping;
    int                 x_isLeadingEaten;
    int                 x_indexOfEnd;
    int                 x_isEndingWithComma;
    int                 x_needToWait;
    int                 x_needToEatEnding;
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

static int textsequence_performNeedToWait (t_textsequence *x, t_buffer *b) 
{
    t_atom *first = buffer_atomAtIndex (b, x->x_indexOfStart);
    
    int waitOnNumber = IS_FLOAT (first) && x->x_leadingToWait && !x->x_isLeadingEaten;
    int waitOnSymbol = IS_SYMBOL (first) && (GET_SYMBOL (first) == x->x_symbolToWait);
    
    return (!x->x_sendTo && (waitOnNumber || waitOnSymbol));
}

static int textsequence_performGetEndWait (t_textsequence *x, t_buffer *b)
{
    int i = x->x_indexOfStart;
    
    if (IS_FLOAT (buffer_atomAtIndex (b, x->x_indexOfStart))) {
        int n = x->x_indexOfStart + x->x_leadingToWait;
        n = (n < 0 ? PD_INT_MAX : n);
        while (i < buffer_size (b) && i < n && IS_FLOAT (buffer_atomAtIndex (b, i))) {
            i++;
        }
        x->x_needToEatEnding = 0;
            
    } else {
        while (i < buffer_size (b) && !IS_SEMICOLON_OR_COMMA (buffer_atomAtIndex (b, i))) {
            i++;
        }
        x->x_indexOfStart += 1;   /* Skip the symbol used to wait. */
    }
    
    x->x_isLeadingEaten = 1;
    x->x_needToWait     = 1;
     
    return i;
}

static int textsequence_performGetEnd (t_textsequence *x, t_buffer *b)
{
    int i = x->x_indexOfStart;
    
    while (i < buffer_size (b) && !IS_SEMICOLON_OR_COMMA (buffer_atomAtIndex (b, i))) {
        i++;
    }
    
    x->x_isLeadingEaten    = 0;
    x->x_isEndingWithComma = (i < buffer_size (b) && IS_COMMA (buffer_atomAtIndex (b, i)));
    
    return i;
}

static void textsequence_performOut (t_textsequence *x, t_buffer *b, int argc, t_atom *argv)
{
    int numberOfFields = x->x_indexOfEnd - x->x_indexOfStart;
    int size = numberOfFields + 1;
    t_atom *t = NULL;

    ATOMS_ALLOCA (t, size);     /* Extra size reserved for possible labelling further below. */
    
    dollar_copyExpandAtoms (buffer_atomAtIndex (b, x->x_indexOfStart), 
        numberOfFields,
        t, 
        numberOfFields,
        argc, 
        argv, 
        textclient_fetchView (&x->x_textclient));
    
    if (x->x_needToWait) {
    
        x->x_sendTo    = NULL;
        x->x_isLooping = 0;

        if (x->x_isAutomatic && numberOfFields == 1 && IS_FLOAT (t)) { x->x_delay = GET_FLOAT (t); }
        else {
            PD_ASSERT (x->x_outletWait);
            x->x_isAutomatic = 0;
            outlet_list (x->x_outletWait, NULL, numberOfFields, t);
        }
        
    } else if (x->x_outletMain) {
        
        /* Ensure that comma separated messages are well prepended. */
        /* For instance "toto a b c, 1 2;" results in "toto a b c" and "toto 1 2". */
        
        if (x->x_sendTo) {
            memmove (t + 1, t, numberOfFields * sizeof (t_atom));
            SET_SYMBOL (t, x->x_sendTo);
            numberOfFields++;
        }
        
        if (!x->x_isEndingWithComma) { x->x_sendTo = NULL; }
        else {
            if (!x->x_sendTo && numberOfFields && IS_SYMBOL (t)) { x->x_sendTo = GET_SYMBOL (t); }
        }
        
        outlet_list (x->x_outletMain, NULL, numberOfFields, t);
        
    } else if (numberOfFields) {        /* Global dispatching. */
    
        int shitfRight = 0;
        t_symbol *send = x->x_sendTo;
        
        if (!send) { 
            if (numberOfFields > 0 && IS_SYMBOL (t)) { send = GET_SYMBOL (t); } else { PD_BUG; } 
            shitfRight = 1;
        }
        
        if (pd_isThing (send)) {
            
            int n = numberOfFields - shitfRight;
            t_atom *v = t + shitfRight;
            
            if (n > 0 && IS_SYMBOL (v)) { pd_message (send->s_thing, GET_SYMBOL (v), n - 1, v + 1); }
            else {
                pd_list (send->s_thing, n, v);
            }
        }
        
        x->x_sendTo = (x->x_isEndingWithComma ? send : NULL);
    }
    
    ATOMS_FREEA (t, size);
}

static void textsequence_perform (t_textsequence *x, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    x->x_indexOfEnd         = 0;
    x->x_isEndingWithComma  = 0;
    x->x_needToWait         = 0;
    x->x_needToEatEnding    = 1;
        
    if (b && (x->x_indexOfStart < buffer_size (b))) { 

        if (!textsequence_performNeedToWait (x, b)) { x->x_indexOfEnd = textsequence_performGetEnd (x, b); }
        else {
            x->x_indexOfEnd = textsequence_performGetEndWait (x, b); 
        }
        
        textsequence_performOut (x, b, argc, argv);
        
        x->x_indexOfStart = x->x_indexOfEnd + x->x_needToEatEnding;

    } else { 
    
        if (!b) { error_undefined (sym_text__space__search, sym_text); }
        x->x_indexOfStart = PD_INT_MAX;
        x->x_isLooping   = 0;
        x->x_isAutomatic = 0;
        outlet_bang (x->x_outletEnd);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void textsequence_task (t_textsequence *x)
{
    x->x_sendTo = NULL;
    
    while (x->x_isAutomatic) {
        x->x_isLooping = 1; while (x->x_isLooping) { textsequence_perform (x, x->x_argc, x->x_argv); }
        if (x->x_delay > 0.0) { break; }
    }
    
    if (x->x_isAutomatic) { clock_delay (x->x_clock, x->x_delay); }
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
    
    textsequence_task (x);
}

static void textsequence_line (t_textsequence *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);

    if (b) {
    //
    int start, end;
    
    x->x_sendTo = NULL;
    x->x_isLeadingEaten = 0;
    
    if (!buffer_getMessageAt (b, f, &start, &end)) { x->x_indexOfStart = PD_INT_MAX; }
    else {
        x->x_indexOfStart = start;
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
                
                if (!x->x_symbolToWait && !x->x_leadingToWait) {
                //
                if (IS_SYMBOL (argv + 1)) { x->x_symbolToWait = atom_getSymbol (argv + 1); }
                else {
                    x->x_leadingToWait = PD_MAX (0, (int)atom_getFloat (argv + 1));
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
        
        hasWait = (useGlobal || x->x_symbolToWait || x->x_leadingToWait);
        
        x->x_indexOfStart = PD_INT_MAX;
        x->x_argc   = 0;
        x->x_argv   = (t_atom *)PD_MEMORY_GET (0);
        x->x_clock  = clock_new (x, (t_method)textsequence_task);
        
        if (useGlobal)  { x->x_leadingToWait = PD_INT_MAX; }
        if (!useGlobal) { x->x_outletMain = outlet_new (cast_object (x), &s_list); }
        if (hasWait)    { x->x_outletWait = outlet_new (cast_object (x), &s_list); }
        
        x->x_outletEnd = outlet_new (cast_object (x), &s_bang);
        
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
