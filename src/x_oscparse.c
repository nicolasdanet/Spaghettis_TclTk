
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://opensoundcontrol.org/introduction-osc > */ 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that this object should be improved to fully support all the OSC features. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"
#include "x_osc.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *oscparse_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _oscparse {
    t_object    x_obj;                  /* Must be the first. */
    t_outlet    *x_outlet;
    } t_oscparse;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error oscparse_proceed (t_oscparse *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_symbol *oscparse_fetchNextAdressElement (int argc, t_atom *argv, int *i)
{
    int j = *i;
    
    char t[PD_STRING] = { 0 };
    int n = 0;
    
    while (j < argc && OSC_GETCHAR (argv + j) == '/') { j++; }
        
    for (n = 0; n < PD_STRING - 1 && j < argc; n++, j++) {
        char c = OSC_GETCHAR (argv + j);
        if (c == 0 || c == '/') { break; }
        else {
            t[n] = c;
        }
    }
    
    t[n] = 0;
    
    PD_ASSERT (j <= argc);
    
    *i = j;
    
    return gensym (t);
}

static t_symbol *oscparse_fetchString (int argc, t_atom *argv, int *i)
{
    int j = *i;
    
    char t[PD_STRING] = { 0 };
    int n = 0;
    
    for (n = 0; n < PD_STRING - 1 && j < argc; n++, j++) {
        char c = OSC_GETCHAR (argv + j);
        if (c == 0) { break; }
        else {
            t[n] = c;
        }
    }
    
    t[n] = 0;
    
    PD_ASSERT (j <= argc);
    
    *i = OSC_4ROUND (j + 1);
    
    return gensym (t);
}

static int oscparse_isValidTypetag (char c)
{
    return (c == 'i' || c == 'f' || c == 's' || c == 'b');
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error oscparse_proceedArgumentsFloat (t_oscparse *x,
    int argc,
    t_atom *argv,
    int *dataOffset,
    int *atomOffset,
    t_atom *a,
    int size)
{
    t_error err = PD_ERROR_NONE;
    
    int k = *dataOffset;
    int n = *atomOffset;
    
    if (k > argc - 4) { err = PD_ERROR; }
    else {
        t_rawcast32 z;
        z.z_i = OSC_4READ (argv + k);
        t_float f = z.z_f;
        if (PD_IS_DENORMAL_OR_ZERO (f)) { f = (t_float)0.0; }
        SET_FLOAT (a + n, f);
        n++; k += 4;
    }
    
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

static t_error oscparse_proceedArgumentsInteger (t_oscparse *x,
    int argc,
    t_atom *argv,
    int *dataOffset,
    int *atomOffset,
    t_atom *a,
    int size)
{
    t_error err = PD_ERROR_NONE;
    
    int k = *dataOffset;
    int n = *atomOffset;
    
    if (k > argc - 4) { err = PD_ERROR; }
    else {
        SET_FLOAT (a + n, OSC_4READ (argv + k));
        n++; k += 4;
    }
    
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

static t_error oscparse_proceedArgumentsString (t_oscparse *x,
    int argc,
    t_atom *argv,
    int *dataOffset,
    int *atomOffset,
    t_atom *a,
    int size)
{
    t_error err = PD_ERROR_NONE;
    
    int k = *dataOffset;
    int n = *atomOffset;
    
    SET_SYMBOL (a + n, oscparse_fetchString (argc, argv, &k));
    n++;
                
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

static t_error oscparse_proceedArgumentsBlob (t_oscparse *x,
    int argc,
    t_atom *argv,
    int *dataOffset,
    int *atomOffset,
    t_atom *a,
    int size)
{
    t_error err = PD_ERROR_NONE;
    
    int k = *dataOffset;
    int n = *atomOffset;
    
    if (k > argc - 4) { err = PD_ERROR; }
    else {
        int blobSize = OSC_4READ (argv + k);
        k += 4;
        err |= (blobSize < 0 || blobSize > argc - k);
        err |= (n + blobSize >= size);
        if (!err) {
            int j;
            SET_FLOAT (a + n, blobSize);
            n++;
            for (j = 0; j < blobSize; j++) {
                SET_FLOAT (a + n, GET_FLOAT (argv + k));
                n++;
                k++;
            }
            k = OSC_4ROUND (k);
        }
    }
                
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error oscparse_proceedArguments (t_oscparse *x,
    int argc,
    t_atom *argv,
    int i,
    int *k,
    int *n,
    t_atom *a,
    int size)
{
    switch (OSC_GETCHAR (argv + i)) {
        case 'f': return oscparse_proceedArgumentsFloat (x, argc, argv, k, n, a, size);
        case 'i': return oscparse_proceedArgumentsInteger (x, argc, argv, k, n, a, size);
        case 's': return oscparse_proceedArgumentsString (x, argc, argv, k, n, a, size);
        case 'b': return oscparse_proceedArgumentsBlob (x, argc, argv, k, n, a, size);
    }
    
    PD_BUG; return PD_ERROR;
}

static int oscparse_proceedFetch (t_oscparse *x,
    int argc,
    t_atom *argv,
    int typeOnset,
    int numberOfTypeTags,
    t_atom *a,
    int size)
{
    int i, n = 0;

    /* Fill the elements of the adress path. */
    
    for (i = 0; i < typeOnset - 1; n++) {
        if (OSC_GETCHAR (argv + i) == 0) { break; }
        else {
            PD_ASSERT (n < size); SET_SYMBOL (a + n, oscparse_fetchNextAdressElement (argc, argv, &i));
        }
    }
    
    /* Fill the arguments. */
    
    int k = OSC_4ROUND (typeOnset + numberOfTypeTags + 1);
    
    for (i = typeOnset; i < typeOnset + numberOfTypeTags; i++) {
        if (n >= size || oscparse_proceedArguments (x, argc, argv, i, &k, &n, a, size)) { return -1; }
    }
    
    return n;
}

static t_error oscparse_proceedBundle (t_oscparse *x, int argc, t_atom *argv)
{
    t_error err = PD_ERROR;
    
    /* Notice that timetag is ignored. */
    
    const int headerBundle = 16;
    const int headerMessage = 4;
    
    if (argc >= headerBundle && OSC_GETCHAR (argv + 1) == 'b') {
    //
    int i = headerBundle;
        
    while (!err && (i < argc - headerMessage)) {
    //
    int length = OSC_4READ (argv + i);
        
    err = (length <= 0 || length & 3);  /* Must be a multiple of 4. */
    
    if (!err) { 
        err = oscparse_proceed (x, length, argv + i + headerMessage);
        i += headerMessage + length;
    }
    //
    }
    //
    }
    
    if (err) { error_invalid (sym_oscparse, sym_bundle); }
    
    return err;
}

static t_error oscparse_proceed (t_oscparse *x, int argc, t_atom *argv)
{
    if (OSC_GETCHAR (argv) == '#') { return oscparse_proceedBundle (x, argc, argv); }
    else if (OSC_GETCHAR (argv) == '/') {
    //
    t_error err = PD_ERROR_NONE;
    int i, size = 1;

    /* Get the number of elements in the adress path. */
    
    for (i = 1; i < argc; i++) {
        if (OSC_GETCHAR (argv + i) == 0) { break; }
        else {
            if (OSC_GETCHAR (argv + i) == '/') { 
                size++;
            }
        }
    }
    
    i = OSC_4ROUND (i + 1);
    
    /* Test existence of typetags. */
    
    err = ((OSC_GETCHAR (argv + i) != ',') || (i + 1 >= argc));
    
    i++;
    
    if (!err) {
    //
    int typeOnset = i;              /* Index of first typetag. */
    int numberOfTypeTags = 0;
    int hasBlob = 0;
    
    /* Get the number of typetags. */
    
    for (; i < argc; i++) {
        if (OSC_GETCHAR (argv + i) == 0) { break; }
        else {
            if (!oscparse_isValidTypetag (OSC_GETCHAR (argv + i))) { err = PD_ERROR; }
            else {
                hasBlob |= (OSC_GETCHAR (argv + i) == 'b'); numberOfTypeTags++;
            }
        }
    }
    
    if (!err) {
    //
    t_atom *a = NULL;
    int n;
    
    size += hasBlob ? argc - typeOnset : numberOfTypeTags;
    
    PD_ATOMS_ALLOCA (a, size);
    
    /* Fetch adress and arguments. */
    
    n = oscparse_proceedFetch (x, argc, argv, typeOnset, numberOfTypeTags, a, size);
    
    if (n == -1) { err = PD_ERROR; }
    else {
        outlet_list (x->x_outlet, n, a);
    }
    
    PD_ATOMS_FREEA (a, size);
    //
    }
    //
    }
    
    if (err) { error_invalid (sym_oscparse, sym_message); }
    
    return err;
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void oscparse_list (t_oscparse *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    t_error err = PD_ERROR_NONE;
    int i;
    
    for (i = 0; i < argc; i++) {
        err |= (!IS_FLOAT (argv + i) || OSC_GETCHAR (argv + i) < 0 || OSC_GETCHAR (argv + i) > 0xff); 
    }
        
    if (!err) {
        err = oscparse_proceed (x, argc, argv);
    }
    
    if (err) { 
        error_failed (sym_oscparse);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_oscparse *oscparse_new (void)
{
    t_oscparse *x = (t_oscparse *)pd_new (oscparse_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void oscparse_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_oscparse,
            (t_newmethod)oscparse_new,
            NULL,
            sizeof (t_oscparse),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addList (c, (t_method)oscparse_list);
    
    oscparse_class = c;
}

void oscparse_destroy (void)
{
    class_free (oscparse_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
