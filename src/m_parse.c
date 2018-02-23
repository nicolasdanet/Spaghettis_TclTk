
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    FLOAT_STATE_ERROR                   = -1,
    FLOAT_STATE_START                   = 0,
    FLOAT_STATE_MINUS                   = 1,
    FLOAT_STATE_INTEGER_DIGIT,
    FLOAT_STATE_LEADING_DOT,
    FLOAT_STATE_DOT,
    FLOAT_STATE_FRACTIONAL_DIGIT,
    FLOAT_STATE_EXPONENTIAL,
    FLOAT_STATE_EXPONENTIAL_SIGN,
    FLOAT_STATE_EXPONENTIAL_DIGIT
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int buffer_parseIsMalformed (const char *s, int size)
{
    if (size < 0) { return 1; }
    else {
        int i;
        for (i = 0; i < size; i++) { if (s[i] == 0) { return 1; } }
    }
    
    return 0;
}

static int buffer_parseIsValidCharacter (char c)
{
    return (!char_isWhitespace (c) && !char_isEnd (c));
}

static int buffer_parseIsValidState (int floatState)
{
    return (floatState == FLOAT_STATE_INTEGER_DIGIT
            || floatState == FLOAT_STATE_DOT
            || floatState == FLOAT_STATE_FRACTIONAL_DIGIT
            || floatState == FLOAT_STATE_EXPONENTIAL_DIGIT);
}

static int buffer_parseNextFloatState (int floatState, char c)
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_withStringUnzeroed (t_buffer *x, const char *s, int size)
{
    const char *text = s;
    const char *tBound = s + size;

    buffer_clear (x);
    
    if (buffer_parseIsMalformed (s, size)) { PD_BUG; return; }
    
    while (1) {
    //
    while (char_isWhitespace (*text) && (text != tBound)) { text++; }         /* Skip whitespaces. */
    
    if (text == tBound)    { break; }
    else if (*text == ';') { buffer_appendSemicolon (x); text++; }
    else if (*text == ',') { buffer_appendComma (x);     text++; }
    else {
        
        char buffer[PD_STRING + 1] = { 0 };
        char *p = buffer;
        char *pBound = buffer + PD_STRING;
        
        int floatState = 0;
        int slash = 0;
        int lastSlash = 0;
        int dollar = 0;
        
        do {
        //
        char c = *p = *text++;
        
        lastSlash = slash; slash = char_isEscape (c);

        if (floatState >= 0) { floatState = buffer_parseNextFloatState (floatState, c); }
        if (!lastSlash && text != tBound && string_startWithOneDollarAndOneNumber (text - 1)) { dollar = 1; }
        
        if (!slash)         { p++; }
        else if (lastSlash) { p++; slash = 0; }
        //
        } while (text != tBound && p != pBound && (slash || (buffer_parseIsValidCharacter (*text))));
                
        *p = 0;

        if (buffer_parseIsValidState (floatState)) {
            buffer_appendFloat (x, (t_float)atof (buffer));
                        
        } else if (dollar) {
            if (string_containsOneDollarFollowingByNumbers (buffer)) {
                buffer_appendDollar (x, atoi (buffer + 1));
            } else {
                buffer_appendDollarSymbol (x, gensym (buffer));
            }
            
        } else {
            buffer_appendSymbol (x, gensym (buffer));
        }
    }
    //
    }
}

void buffer_toStringUnzeroed (t_buffer *x, char **s, int *size)     /* Caller acquires string ownership. */
{
    char *buffer = (char *)PD_MEMORY_GET (0);
    int i, length = 0;

    for (i = 0; i < buffer_getSize (x); i++) {
    //
    int n;
    t_error err;
    char t[PD_STRING] = { 0 };
    t_atom *a = buffer_getAtomAtIndex (x, i);
    
    /* Remove whitespace before a semicolon or a comma for cosmetic purpose. */
    
    if (IS_SEMICOLON_OR_COMMA (a)) {
    //
    if (length && buffer[length - 1] == ' ') {
        buffer = (char *)PD_MEMORY_RESIZE (buffer, length, length - 1); length--;
    }
    //
    }
    
    err = atom_toString (a, t, PD_STRING); PD_UNUSED (err); PD_ASSERT (!err);
    
    n = (int)(strlen (t) + 1);
    
    if (length > (PD_INT_MAX - n)) { PD_BUG; }
    else {
        buffer = (char *)PD_MEMORY_RESIZE (buffer, length, length + n);
        strcpy (buffer + length, t);
        length += n;
    }
    
    if (IS_SEMICOLON (a)) { buffer[length - 1] = '\n'; }
    else {
        buffer[length - 1] = ' ';
    }
    //
    }
    
    /* Remove ending whitespace. */
    
    if (length && buffer[length - 1] == ' ') {
        buffer = (char *)PD_MEMORY_RESIZE (buffer, length, length - 1); length--;
    }
    
    *s = buffer;
    *size = length;
}

char *buffer_toString (t_buffer *x)
{
    char *s = NULL;
    int n, length = 0;
    
    buffer_toStringUnzeroed (x, &s, &length);
    n = length + 1; 
    s = (char *)PD_MEMORY_RESIZE (s, length, n);
    s[n - 1] = 0;
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int buffer_isLastMessageProperlyEnded (t_buffer *x)
{
    int size = buffer_getSize (x);
    
    if (size) { t_atom *a = buffer_getAtomAtIndex (x, size - 1); return IS_SEMICOLON_OR_COMMA (a); }
    else {
        return 1;
    }
}

int buffer_getNumberOfMessages (t_buffer *x)
{
    int i, count = 0;

    for (i = 0; i < buffer_getSize (x); i++) {
        t_atom *a = buffer_getAtomAtIndex (x, i); if (IS_SEMICOLON_OR_COMMA (a)) { count++; }
    }
    
    if (!buffer_isLastMessageProperlyEnded (x)) { count++; }
    
    return count;
}

t_error buffer_getMessageAt (t_buffer *x, int n, int *start, int *end)
{
    *start = 0; *end = 0;
    
    if (n >= 0) {
    //
    int i, k = 0;
    
    for (i = 0; i < buffer_getSize (x); i++) {
    //
    if (k != n) {
        if (IS_SEMICOLON_OR_COMMA (buffer_getAtomAtIndex (x, i))) { k++; }
        
    } else {
        int j = i;
        while (j < x->b_size && !IS_SEMICOLON_OR_COMMA (buffer_getAtomAtIndex (x, j))) { j++; }
        *start = i;
        *end   = j;
        return PD_ERROR_NONE;
    }
    //
    }
    //
    }
    
    return PD_ERROR;
}

t_error buffer_getMessageAtWithTypeOfEnd (t_buffer *x, int n, int *start, int *end, t_atomtype *type)
{
    t_error err = buffer_getMessageAt (x, n, start, end);
    
    if (!err) {
    //
    if (buffer_getAtomAtIndexChecked (x, *end)) { *type = atom_getType (buffer_getAtomAtIndex (x, *end)); }
    else {
        *type = A_NULL;
    }
    //
    }
    
    return err;
}
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void buffer_serialize (t_buffer *x, t_buffer *y)
{
    t_buffer *copy = buffer_new();
    int i;

    buffer_appendBuffer (copy, y);
    
    for (i = 0; i < buffer_getSize (copy); i++) {
    //
    t_atom *a = buffer_getAtomAtIndex (copy, i);
    
    PD_ASSERT (!IS_POINTER (a));
    
    if (!IS_FLOAT (a)) {
        char t[PD_STRING] = { 0 };
        t_error err = atom_toString (a, t, PD_STRING);
        PD_UNUSED (err); PD_ASSERT (!err);
        SET_SYMBOL (a, gensym (t));
    }
    //
    }
    
    buffer_appendBuffer (x, copy);
    buffer_free (copy);
}

void buffer_deserialize (t_buffer *x, int argc, t_atom *argv)
{
    int i;
    
    for (i = 0; i < argc; i++) {
    //
    if (!IS_SYMBOL (argv + i)) { buffer_appendAtom (x, argv + i); }
    else {
        t_atom a;
        const char *s = GET_SYMBOL (argv + i)->s_name;
        t_error err = atom_withStringUnzeroed (&a, s, (int)strlen (s));
        PD_UNUSED (err); PD_ASSERT (!err);
        buffer_appendAtom (x, &a);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
