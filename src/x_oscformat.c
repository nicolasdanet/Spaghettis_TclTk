
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://opensoundcontrol.org/introduction-osc > */ 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_alloca.h"
#include "s_system.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *oscformat_class;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _oscformat {
    t_object    x_obj;                  /* Must be the first. */
    char        *x_path;
    t_symbol    *x_format;
    t_outlet    *x_outlet;
    } t_oscformat;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int oscformat_isValidTypetag (char c)
{
    return (c == 'i' || c == 'f' || c == 's' || c == 'b');
}

static void oscformat_setString (t_atom *a, int *i, const char *s)
{
    int n = *i;
    
    PD_ASSERT (s);
    
    while (*s) {
        unsigned char byte = (unsigned char)(*s);
        OSC_SETCHAR (a + n, byte);
        n++;
        s++;
    }
    
    OSC_SETCHAR (a + n, 0);
    n++;
    
    while (n & 3) {
        OSC_SETCHAR (a + n, 0);
        n++;
    }
    
    *i = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int oscformat_proceedGetArgumentsSize (t_oscformat *x, int argc, t_atom *argv, int *numberOfTypeTags)
{
    int i = 0; 
    int k = 0;
    int size = 0;
    
    char *t = x->x_format->s_name;
    
    while (i < argc) {
    //
    t_atom *a = argv + i;
    char type = 'f';
    
    if (*t) { type = *t; t++; } else if (IS_SYMBOL (a)) { type = 's'; }
    
    if (type == 's') {
        t_symbol *s = IS_SYMBOL (a) ? GET_SYMBOL (a) : sym___arrobe__;
        size += OSC_ROUND4 ((int)strlen (s->s_name) + 1);
        
    } else if (type == 'b') {
        int blobSize = (IS_FLOAT (a) && GET_FLOAT (a) >= 0) ? GET_FLOAT (a) : PD_INT_MAX;
        blobSize = PD_MIN (argc - i - 1, blobSize);
        size += 4 + OSC_ROUND4 (blobSize);
        i += blobSize;
        
    } else {
        size += 4;
    }
    
    i++;
    k++;
    //
    }
    
    *numberOfTypeTags = k;
    
    return size;
}

static int oscformat_proceedFillFloat (t_oscformat *x, int argc, t_atom *argv, int j, int *m, t_atom *a)
{
    int n = *m;
    
    t_rawcast32 z;
    z.z_f = atom_getFloat (argv + j);
    OSC_WRITE4INT (a + n, z.z_i);
    n += 4;
    
    *m = n; j++; return j;
}

static int oscformat_proceedFillInteger (t_oscformat *x, int argc, t_atom *argv, int j, int *m, t_atom *a)
{
    int n = *m;

    int v = (int)atom_getFloat (argv + j);
    OSC_WRITE4INT (a + n, v);
    n += 4;
    
    *m = n; j++; return j;
}

static int oscformat_proceedFillString (t_oscformat *x, int argc, t_atom *argv, int j, int *m, t_atom *a)
{
    t_symbol *s = IS_SYMBOL (argv + j) ? GET_SYMBOL (argv + j) : sym___arrobe__;
    
    oscformat_setString (a, m, s->s_name);
    
    j++; return j;
}

static int oscformat_proceedFillBlob (t_oscformat *x, int argc, t_atom *argv, int j, int *m, t_atom *a)
{
    int n = *m;

    t_atom *start = argv + j;
    int i, blobSize = (IS_FLOAT (start) && GET_FLOAT (start) >= 0) ? GET_FLOAT (start) : PD_INT_MAX;
    
    blobSize = PD_MIN (argc - j - 1, blobSize);
    OSC_WRITE4INT (a + n, blobSize);
    n += 4;
    
    for (i = 0; i < blobSize; i++) {
    //
    unsigned char byte = 0;
    
    if (IS_FLOAT (start + 1 + i)) { byte = (unsigned char)GET_FLOAT (start + 1 + i); }
    else if (IS_SYMBOL (start + 1 + i)) {
        t_symbol *s = GET_SYMBOL (start + 1 + i); byte = (unsigned char)(*s->s_name);
    }
    
    OSC_SETCHAR (a + n + i, byte);
    //
    }
    
    while (i & 3) { OSC_SETCHAR (a + n + i, 0); i++; }
    
    n += i;
    j += blobSize;
    
    *m = n; j++; return j;
}

static t_error oscformat_proceedFill (t_oscformat *x,
    int argc,
    t_atom *argv,
    int argumentsStart,
    t_atom *a,
    int size)
{
    int i = 0;
    int j = 0;
    int n = argumentsStart;

    char *t = x->x_format->s_name;
    
    oscformat_setString (a, &i, x->x_path);
    
    OSC_SETCHAR (a + i, ',');
    i++;
    
    while (j < argc) {

        char type = 'f';
        
        if (*t) { type = *t; t++; } else if (IS_SYMBOL (argv + j)) { type = 's'; }
        
        OSC_SETCHAR (a + i, type);
        i++;
        
        switch (type) {
            case 'f':   j = oscformat_proceedFillFloat (x, argc, argv, j, &n, a);   break;
            case 'i':   j = oscformat_proceedFillInteger (x, argc, argv, j, &n, a); break;
            case 's':   j = oscformat_proceedFillString (x, argc, argv, j, &n, a);  break;
            case 'b':   j = oscformat_proceedFillBlob (x, argc, argv, j, &n, a);    break;
            default :   return PD_ERROR;
        }
    }
    
    OSC_SETCHAR (a + i, 0);
    i++;
    
    while (i & 3) { OSC_SETCHAR (a + i, 0); i++; }
    
    PD_ASSERT (i == argumentsStart);
    PD_ASSERT (n == size);
    
    return PD_ERROR_NONE;
}

static t_error oscformat_proceed (t_oscformat *x, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    int numberOfTypeTags = 0;
    int argumentsSize    = oscformat_proceedGetArgumentsSize (x, argc, argv, &numberOfTypeTags);
    int argumentsStart   = OSC_ROUND4 ((int)strlen (x->x_path) + 1) + OSC_ROUND4 (numberOfTypeTags + 2);
    int size             = argumentsStart + argumentsSize;
    
    t_atom *a = NULL;
    
    ATOMS_ALLOCA (a, size);

    err = oscformat_proceedFill (x, argc, argv, argumentsStart, a, size);
    
    if (!err) { outlet_list (x->x_outlet, size, a); }
    
    ATOMS_FREEA (a, size);
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void oscformat_list (t_oscformat *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    if ((*x->x_path) == 0) { error_unspecified (sym_oscformat, sym_path); }
    else {
        t_error err = oscformat_proceed (x, argc, argv);
        
        if (err) { 
            error_failed (sym_oscformat);
        }
    }
    //
    }
}

static void oscformat_anything (t_oscformat *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)oscformat_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void oscformat_set (t_oscformat *x, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    int i;

    for (i = 0; i < argc; i++) {
        err |= string_add (t, PD_STRING, "/"); err |= string_addAtom (t, PD_STRING, argv + i);
    }
    
    if (err) { error_invalid (sym_oscformat, sym_path); }
    else {
        err = string_copy (x->x_path, PD_STRING, t); PD_ASSERT (!err);
    }
}

static void oscformat_format (t_oscformat *x, t_symbol *s)
{
    char *t = NULL;
    
    for (t = s->s_name; *t; t++) {
        if (!oscformat_isValidTypetag (*t)) { error_invalid (sym_oscformat, sym_format); return; }
    }
    
    x->x_format = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *oscformat_new (t_symbol *s, int argc, t_atom *argv)
{
    t_oscformat *x = (t_oscformat *)pd_new (oscformat_class);
    
    x->x_path   = (char *)PD_MEMORY_GET (PD_STRING);
    x->x_format = &s_;
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    while (argc > 0) {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);

    if (argc > 1 && IS_SYMBOL (argv + 1) && (t == sym___dash__f || t == sym___dash__format)) { 
        oscformat_format (x, GET_SYMBOL (argv + 1));
        argc -= 2;
        argv += 2;
        
    } else {
        break;
    }
    //
    }

    error__options (s, argc, argv);
    
    oscformat_set (x, NULL, argc, argv);
    
    return x;
}

static void oscformat_free (t_oscformat *x)
{
    PD_MEMORY_FREE (x->x_path);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void oscformat_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_oscformat,
            (t_newmethod)oscformat_new,
            (t_method)oscformat_free,
            sizeof (t_oscformat),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
        
    class_addList (c, (t_method)oscformat_list);
    class_addAnything (c, (t_method)oscformat_anything);
    
    class_addMethod (c, (t_method)oscformat_set,    sym_set,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)oscformat_format, sym_format, A_DEFSYMBOL, A_NULL);

    oscformat_class = c;
}

void oscformat_destroy (void)
{
    CLASS_FREE (oscformat_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
