
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BUFFER_PREALLOCATED_ATOMS       64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

enum {
    FLOAT_STATE_ERROR                   = -1,
    FLOAT_STATE_START                   =  0,
    FLOAT_STATE_MINUS                   =  1,
    FLOAT_STATE_INTEGER_DIGIT           =  2,
    FLOAT_STATE_LEADING_DOT             =  3,
    FLOAT_STATE_DOT                     =  4,
    FLOAT_STATE_FRACTIONAL_DIGIT        =  5,
    FLOAT_STATE_EXPONENTIAL             =  6,
    FLOAT_STATE_EXPONENTIAL_SIGN        =  7,
    FLOAT_STATE_EXPONENTIAL_DIGIT       =  8
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int buffer_isValidCharacter (char c)
{
    return (!utils_isTokenWhitespace (c) && !utils_isTokenEnd (c));
}

static int buffer_isValidState (int floatState)
{
    return (floatState == FLOAT_STATE_INTEGER_DIGIT // --
            || floatState == FLOAT_STATE_DOT
            || floatState == FLOAT_STATE_FRACTIONAL_DIGIT
            || floatState == FLOAT_STATE_EXPONENTIAL_DIGIT);
}

static int buffer_nextState (int floatState, char c)
{
    int digit       = (c >= '0' && c <= '9');
    int dot         = (c == '.');
    int minus       = (c == '-');
    int plus        = (c == '+');
    int exponential = (c == 'e' || c == 'E');
    
    int k = floatState;
    
    PD_ASSERT (k != FLOAT_STATE_ERROR);
    
    if (floatState == FLOAT_STATE_START)                    {
        if (minus)                                          { k = FLOAT_STATE_MINUS; }
        else if (digit)                                     { k = FLOAT_STATE_INTEGER_DIGIT; }
        else if (dot)                                       { k = FLOAT_STATE_LEADING_DOT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_MINUS)             {
        if (digit)                                          { k = FLOAT_STATE_INTEGER_DIGIT; }
        else if (dot)                                       { k = FLOAT_STATE_LEADING_DOT; }
        else { 
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_INTEGER_DIGIT)     {
        if (dot)                                            { k = FLOAT_STATE_DOT; }
        else if (exponential)                               { k = FLOAT_STATE_EXPONENTIAL; }
        else if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_LEADING_DOT)       {
        if (digit)                                          { k = FLOAT_STATE_FRACTIONAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_DOT)               {
        if (digit)                                          { k = FLOAT_STATE_FRACTIONAL_DIGIT; }
        else if (exponential)                               { k = FLOAT_STATE_EXPONENTIAL; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_FRACTIONAL_DIGIT)  {
        if (exponential)                                    { k = FLOAT_STATE_EXPONENTIAL; }
        else if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL)       {
        if (plus || minus)                                  { k = FLOAT_STATE_EXPONENTIAL_SIGN; }
        else if (digit)                                     { k = FLOAT_STATE_EXPONENTIAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL_SIGN)  {
        if (digit)                                          { k = FLOAT_STATE_EXPONENTIAL_DIGIT; }
        else {
            k = FLOAT_STATE_ERROR;
        }
        
    } else if (floatState == FLOAT_STATE_EXPONENTIAL_DIGIT) {
        if (!digit) {
            k = FLOAT_STATE_ERROR;
        }
    }

    return k;
}

void buffer_parseString (t_buffer *x, char *s, int size, int allocated)
{
    int length = 0;
    t_atom *a = NULL;
    
    const char *text = s;
    const char *tBound = s + size;
    
    PD_ASSERT (size > 0);
    
    PD_MEMORY_FREE (x->b_vector, x->b_size * sizeof (t_atom));
    x->b_vector = PD_MEMORY_GET (allocated * sizeof (t_atom));
    a = x->b_vector;
    x->b_size = length;     /* Inconsistency corrected later. */
    
    while (1) {
    //
    while (utils_isTokenWhitespace (*text) && (text != tBound)) { text++; }         /* Skip whitespaces. */
    
    if (text == tBound)    { break; }
    else if (*text == ';') { SET_SEMICOLON (a); text++; }
    else if (*text == ',') { SET_COMMA (a);     text++; }
    else {
        
        char buf[PD_STRING + 1] = { 0 };
        char *p = buf;
        char *pBound = buf + PD_STRING;
        
        int floatState = 0;
        int slash = 0;
        int lastSlash = 0;
        int dollar = 0;
        
        do {
        //
        char c = *p = *text++;
        
        lastSlash = slash; slash = utils_isTokenEscape (c);

        if (floatState >= 0) { floatState = buffer_nextState (floatState, c); }
        if (!lastSlash && text != tBound && dollar_isPointingToDollarAndNumber (text - 1)) { dollar = 1; }
        
        if (!slash)         { p++; }
        else if (lastSlash) { p++; slash = 0; }
        //
        } while (text != tBound && p != pBound && (slash || (buffer_isValidCharacter (*text))));
                
        *p = 0;

        if (buffer_isValidState (floatState)) {
            SET_FLOAT (a, atof (buf));
                        
        } else if (dollar) {
            if (dollar_isDollarNumber (buf)) { SET_DOLLAR (a, atoi (buf + 1)); }
            else { 
                SET_DOLLARSYMBOL (a, gensym (buf));
            }
            
        } else {
            SET_SYMBOL (a, gensym (buf));
        }
    }
    
    a++;
    length++;
    
    if (text == tBound) { break; }
    
    if (length == allocated) {
        size_t oldSize = allocated * sizeof (t_atom);
        x->b_vector = PD_MEMORY_RESIZE (x->b_vector, oldSize, oldSize * 2);
        allocated = allocated * 2;
        a = x->b_vector + length;
    }
    //
    }
    
    /* Crop to truly used memory. */
    
    x->b_size   = length;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, allocated * sizeof (t_atom), length * sizeof (t_atom));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_withString (t_buffer *x, char *s, int size)
{
    buffer_parseString (x, s, size, BUFFER_PREALLOCATED_ATOMS);
}

void buffer_toString (t_buffer *x, char **s, int *size)
{
    char *buf = NULL;
    int n, length = 0;
    
    buffer_toStringUnzeroed (x, &buf, &length);
    n = length + 1; 
    buf = PD_MEMORY_RESIZE (buf, length, n); 
    buf[n - 1] = 0;
    
    *s = buf; 
    *size = n;
}

void buffer_toStringUnzeroed (t_buffer *x, char **s, int *size)
{
    char *buf = PD_MEMORY_GET (0);
    int i, length = 0;

    for (i = 0; i < x->b_size; i++) {
    //
    char t[PD_STRING] = { 0 };
    int n = length;
    t_error err = atom_toString (x->b_vector + i, t, PD_STRING);
    PD_ASSERT (!err);
    n += strlen (t) + 1;
    buf = PD_MEMORY_RESIZE (buf, length, n);
    strcpy (buf + length, t);
    length = n;
    if (IS_SEMICOLON (x->b_vector + i)) { buf[length - 1] = '\n'; }
    else { 
        buf[length - 1] = ' ';
    }
    //
    }
    
    if (length && buf[length - 1] == ' ') { buf = PD_MEMORY_RESIZE (buf, length, length - 1); length--; }
    
    *s = buf;
    *size = length;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_serialize (t_buffer *x, t_buffer *y)
{
    t_buffer *copy = buffer_new();
    int i;

    buffer_append (copy, y->b_size, y->b_vector);
    
    for (i = 0; i < copy->b_size; i++) {
    //
    t_atom *a = copy->b_vector + i;
    
    PD_ASSERT (!IS_POINTER (a));
    
    if (!IS_FLOAT (a)) {
        char t[PD_STRING] = { 0 };
        t_error err = atom_toString (a, t, PD_STRING);
        PD_ASSERT (!err);
        SET_SYMBOL (a, gensym (t));
    }
    //
    }
    
    buffer_append (x, copy->b_size, copy->b_vector);
}

void buffer_deserialize (t_buffer *x, int argc, t_atom *argv)
{
    int i, n = x->b_size + argc;

    PD_ASSERT (argc >= 0);
    
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));
    
    for (i = 0; i < argc; i++) {
    //
    t_atom *a = x->b_vector + x->b_size + i;
    
    if (!IS_SYMBOL (argv + i)) { *a = *(argv + i); }
    else {
        char *s = GET_SYMBOL (argv + i)->s_name;
        t_error err = atom_withString (a, s, strlen (s));
        PD_ASSERT (!err);
    }
    //
    }
    
    x->b_size = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
