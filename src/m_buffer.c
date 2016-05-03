
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd pd_objectMaker;
extern t_pd pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BUFFER_MAXIMUM_ARGUMENTS        64
#define BUFFER_PREALLOCATED_ATOMS       64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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
#pragma mark -

t_buffer *buffer_new (void)
{
    t_buffer *x = (t_buffer *)PD_MEMORY_GET (sizeof (t_buffer));
    x->b_vector = (t_atom *)PD_MEMORY_GET (0);
    return x;
}

void buffer_free (t_buffer *x)
{
    PD_MEMORY_FREE (x->b_vector);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int buffer_size (t_buffer *x)
{
    return x->b_size;
}

t_atom *buffer_atoms (t_buffer *x)
{
    return x->b_vector;
}

void buffer_reset (t_buffer *x)
{
    buffer_resize (x, 0);
}

void buffer_append (t_buffer *x, int argc, t_atom *argv)
{
    if (argc > 0) {
    //
    t_atom *a = NULL;
    int i, n = x->b_size + argc;

    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));

    for (a = x->b_vector + x->b_size; argc--; a++) { *a = *(argv++); } x->b_size = n;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_post (t_buffer *x)
{
    post_atoms (x->b_size, x->b_vector);
}

void buffer_resize (t_buffer *x, int n)
{
    PD_ASSERT (n >= 0);
    
    x->b_size   = n;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, x->b_size * sizeof (t_atom), n * sizeof (t_atom));
}

/* < http://stackoverflow.com/a/11270603 > */

void buffer_vAppend (t_buffer *x, char *fmt, ...)
{
    va_list ap;
    t_atom args[BUFFER_MAXIMUM_ARGUMENTS];
    t_atom *a = args;
    int n = 0;
    char *p = fmt;
    int k = 1;
    
    va_start (ap, fmt);
    
    while (k) {

        if (n >= BUFFER_MAXIMUM_ARGUMENTS) { PD_BUG; break; }

        switch (*p++) {
            case 'i'    : SET_FLOAT     (a, va_arg (ap, int));          break;
            case 'f'    : SET_FLOAT     (a, va_arg (ap, double));       break;
            case 's'    : SET_SYMBOL    (a, va_arg (ap, t_symbol *));   break;
            case ';'    : SET_SEMICOLON (a);                            break;
            case ','    : SET_COMMA     (a);                            break;
            default     : k = 0;
        }
        
        if (k) { a++; n++; }
    }
    
    va_end (ap);
    
    buffer_append (x, n, args);
}

void buffer_appendSemicolon (t_buffer *x)
{
    t_atom a;
    SET_SEMICOLON (&a);
    buffer_append (x, 1, &a);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int buffer_isMalformed (char *s, int size)
{
    if (size < 0) { return 1; }
    else {
        int i;
        for (i = 0; i < size; i++) { if (s[i] == 0) { return 1; } }
    }
    
    return 0;
}

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

static int buffer_nextFloatState (int floatState, char c)
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
#pragma mark -

void buffer_parseStringUnzeroed (t_buffer *x, char *s, int size, int preallocated)
{
    int length = 0;
    t_atom *a = NULL;
    
    const char *text = s;
    const char *tBound = s + size;

    if (buffer_isMalformed (s, size)) { PD_BUG; return; }
    
    PD_MEMORY_FREE (x->b_vector);
    x->b_vector = (t_atom *)PD_MEMORY_GET (preallocated * sizeof (t_atom));
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

        if (floatState >= 0) { floatState = buffer_nextFloatState (floatState, c); }
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
    
    if (length == preallocated) {
        size_t oldSize = preallocated * sizeof (t_atom);
        x->b_vector = PD_MEMORY_RESIZE (x->b_vector, oldSize, oldSize * 2);
        preallocated = preallocated * 2;
        a = x->b_vector + length;
    }
    //
    }
    
    /* Crop to truly used memory. */
    
    x->b_size   = length;
    x->b_vector = PD_MEMORY_RESIZE (x->b_vector, preallocated * sizeof (t_atom), length * sizeof (t_atom));
}

void buffer_withStringUnzeroed (t_buffer *x, char *s, int size)
{
    buffer_parseStringUnzeroed (x, s, size, BUFFER_PREALLOCATED_ATOMS);
}

void buffer_toStringUnzeroed (t_buffer *x, char **s, int *size)
{
    char *buf = (char *)PD_MEMORY_GET (0);
    int i, length = 0;

    for (i = 0; i < x->b_size; i++) {
    //
    int n;
    t_error err;
    char t[PD_STRING] = { 0 };
    t_atom *a = x->b_vector + i;
    
    /* Remove whitespace before a semicolon or a comma for cosmetic purpose. */
    
    if (IS_SEMICOLON (a) || IS_COMMA (a)) {
    //
    if (length && buf[length - 1] == ' ') { buf = PD_MEMORY_RESIZE (buf, length, length - 1); length--; }
    //
    }
    
    err = atom_toString (a, t, PD_STRING); PD_ASSERT (!err);
    n = strlen (t) + 1;
    buf = PD_MEMORY_RESIZE (buf, length, length + n);
    strcpy (buf + length, t);
    length += n;
    
    if (IS_SEMICOLON (a)) { buf[length - 1] = '\n'; }
    else { 
        buf[length - 1] = ' ';
    }
    //
    }
    
    /* Remove ending whitespace. */
    
    if (length && buf[length - 1] == ' ') { buf = PD_MEMORY_RESIZE (buf, length, length - 1); length--; }
    
    *s = buf;
    *size = length;
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
        t_error err = atom_withStringUnzeroed (a, s, strlen (s));
        PD_ASSERT (!err);
    }
    //
    }
    
    x->b_size = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_symbol *buffer_getObject (t_atom *v, int argc, t_atom *argv)
{   
    if (IS_DOLLARSYMBOL (v)) { return dollar_expandDollarSymbol (GET_DOLLARSYMBOL (v), argc, argv); }
    else if (IS_DOLLAR  (v)) {
        t_symbol *s = atom_getSymbolAtIndex (GET_DOLLAR (v) - 1, argc, argv); 
        return (s == &s_ ? NULL : s);
    }

    return atom_getSymbol (v);
}

static int buffer_getMessage (t_atom *v, t_pd *object, t_pd **next, t_atom *m, int argc, t_atom *argv)
{
    t_symbol *s = NULL;
    int end = 0;
    
    switch (v->a_type) {
    //
    case A_SEMICOLON    :   if (object == &pd_objectMaker) { SET_SYMBOL (m, gensym (";")); }
                            else { 
                                *next = NULL; end = 1; 
                            }
                            break;
    case A_COMMA        :   if (object == &pd_objectMaker) { SET_SYMBOL (m, gensym (",")); }
                            else { 
                                end = 1; 
                            }
                            break;
    case A_FLOAT        :   *m = *v; break;
    case A_SYMBOL       :   *m = *v; break;
    case A_DOLLAR       :   dollar_expandDollarNumber (v, m, argc, argv); break;
    case A_DOLLARSYMBOL :   s = dollar_expandDollarSymbol (GET_DOLLARSYMBOL (v), argc, argv);
                            if (s) { SET_SYMBOL (m, s); }
                            else {
                                SET_SYMBOL (m, GET_DOLLARSYMBOL (v));
                            }
                            break;
    default             :   end = 1; PD_BUG; 
    //
    }
    
    return end;
}

static t_error buffer_fromFile (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;
    
    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory, name, 0))) {
    //
    int f = file_openRaw (filepath, O_RDONLY);
    
    err = (f < 0);
    
    if (err) { PD_BUG; }
    else {
    //
    off_t length;
    
    err |= ((length = lseek (f, 0, SEEK_END)) < 0);
    err |= (lseek (f, 0, SEEK_SET) < 0); 
    
    if (err) { PD_BUG; }
    else {
        char *t = (char *)PD_MEMORY_GET ((size_t)length);
        err = (read (f, t, length) != length);
        if (err) { PD_BUG; } else { buffer_withStringUnzeroed (x, t, (int)length); }
        PD_MEMORY_FREE (t);
    }
    
    close (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void buffer_eval (t_buffer *x, t_pd *object, int argc, t_atom *argv)
{
    int size = x->b_size;
    t_atom *v = x->b_vector;
    t_atom *message = NULL;
    t_atom *m = NULL;
    t_pd *next = NULL;
    int args = 0;
    
    ATOMS_ALLOCA (message, x->b_size);
    
    while (1) {
    //
    while (!object) {  
         
        t_symbol *s = NULL;
        
        while (size && (IS_SEMICOLON (v) || IS_COMMA (v))) { size--; v++; }
        
        if (size) { s = buffer_getObject (v, argc, argv); }
        else {
            break;
        }
        
        if (s == NULL || !(object = s->s_thing)) {
            if (!s) { post_error (PD_TRANSLATE ("$: invalid expansion")); }  // --
            else {
                post_error (PD_TRANSLATE ("%s: no such object"), s->s_name);    // --
            }
            do { size--; v++; } while (size && !IS_SEMICOLON (v));
            
        } else {
            size--; v++; break;
        }
    }
    
    PD_ASSERT ((object != NULL) || (size == 0));
    
    m    = message; 
    args = 0;
    next = object;
        
    while (1) {
        if (!size || buffer_getMessage (v, object, &next, m, argc, argv)) { break; }
        else {
            m++; args++; size--; v++;
        }
    }
    
    if (args) {
        if (IS_SYMBOL (message)) { pd_message (object, GET_SYMBOL (message), args - 1, message + 1); }
        else if (IS_FLOAT (message)) {
            if (args == 1) { pd_float (object, GET_FLOAT (message)); }
            else { 
                pd_list (object, args, message); 
            }
        }
    }
    
    if (!size) { break; }
    
    object = next;
    size--;
    v++;
    //
    }
    
    ATOMS_FREEA (message, x->b_size);
}

t_error buffer_read (t_buffer *x, char *name, t_glist *glist)
{
    t_error err = PD_ERROR;
    
    char *filepath = NULL;
    char directory[PD_STRING] = { 0 };
    
    int f = canvas_openFile (glist, name, "", directory, &filepath, PD_STRING);
    
    err = (f < 0);
    
    if (err) { post_error (PD_TRANSLATE ("%s: can't open"), name); }    // --
    else {
        close (f);
        err = buffer_fromFile (x, filepath, directory);
    }
    
    return err;
}

t_error buffer_write (t_buffer *x, char *name, char *directory)
{
    t_error err = PD_ERROR;

    char filepath[PD_STRING] = { 0 };

    if (!(err = path_withDirectoryAndName (filepath, PD_STRING, directory, name, 0))) {
    //
    FILE *f = 0;

    err = !(f = file_openWrite (filepath));
    
    if (!err) {
    //
    char *s = NULL;
    int size = 0;
    
    buffer_toStringUnzeroed (x, &s, &size);

    err |= (fwrite (s, size, 1, f) < 1);
    err |= (fflush (f) != 0);

    PD_ASSERT (!err);
    PD_MEMORY_FREE (s);
        
    fclose (f);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error buffer_evalFile (t_symbol *name, t_symbol *directory)
{
    t_error err = PD_ERROR;
    
    int state = dsp_suspend();
    t_buffer *t = buffer_new();
        
    canvas_setActiveFileNameAndDirectory (name, directory);
    
    err = buffer_fromFile (t, name->s_name, directory->s_name);
    
    if (err) { post_error (PD_TRANSLATE ("%s: fails to read"), name->s_name); }     // --
    else {
    //
    t_pd *boundA = s__A.s_thing;
    t_pd *boundN = s__N.s_thing;
    
    s__A.s_thing = NULL; 
    s__N.s_thing = &pd_canvasMaker;
    buffer_eval (t, NULL, 0, NULL);
    
    s__A.s_thing = boundA;
    s__N.s_thing = boundN;
    //
    }
    
    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    
    buffer_free (t);
    dsp_resume (state);
    
    return err;
}

void buffer_openFile (void *dummy, t_symbol *name, t_symbol *directory)
{
    t_pd *x = NULL;
    
    t_pd *boundX = s__X.s_thing;
    int state = dsp_suspend();
    
    s__X.s_thing = NULL;
    buffer_evalFile (name, directory);
    
    while ((x != s__X.s_thing) && s__X.s_thing) {
        x = s__X.s_thing;
        pd_vMessage (x, gensym ("_pop"), "i", 1);
    }
    
    stack_performLoadbang();
    
    dsp_resume (state);
    s__X.s_thing = boundX;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
