
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
#include "m_macros.h"
#include "m_alloca.h"
#include "s_system.h"
#include "x_control.h"

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
#pragma mark -

static t_error oscparse_perform (t_oscparse *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    *i = OSC_ROUND4 (j + 1);
    
    return gensym (t);
}

static int oscparse_isValidTypetag (char c)
{
    return (c == 'i' || c == 'f' || c == 's' || c == 'b');
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error oscparse_performArgumentsFloat (t_oscparse *x,
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
        t_rawcast z;
        z.z_i = OSC_READ4INT (argv + k);
        t_float f = z.z_f;
        if (PD_DENORMAL_OR_ZERO (f)) { f = 0.0; }
        SET_FLOAT (a + n, f);
        n++; k += 4;
    }
    
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

static t_error oscparse_performArgumentsInteger (t_oscparse *x,
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
        SET_FLOAT (a + n, OSC_READ4INT (argv + k));
        n++; k += 4;
    }
    
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

static t_error oscparse_performArgumentsString (t_oscparse *x,
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

static t_error oscparse_performArgumentsBlob (t_oscparse *x,
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
        int blobSize = OSC_READ4INT (argv + k); 
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
            k = OSC_ROUND4 (k);
        }
    }
                
    *dataOffset = k;
    *atomOffset = n;
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error oscparse_performArguments (t_oscparse *x,
    int argc,
    t_atom *argv,
    int i,
    int *k,
    int *n,
    t_atom *a,
    int size)
{
    switch (OSC_GETCHAR (argv + i)) {
        case 'f': return oscparse_performArgumentsFloat (x, argc, argv, k, n, a, size);
        case 'i': return oscparse_performArgumentsInteger (x, argc, argv, k, n, a, size);
        case 's': return oscparse_performArgumentsString (x, argc, argv, k, n, a, size);
        case 'b': return oscparse_performArgumentsBlob (x, argc, argv, k, n, a, size);
    }
    
    PD_BUG; return PD_ERROR;
}

static int oscparse_performFetch (t_oscparse *x,
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
    
    int k = OSC_ROUND4 (typeOnset + numberOfTypeTags + 1);
    
    for (i = typeOnset; i < typeOnset + numberOfTypeTags; i++) {
        if (n >= size || oscparse_performArguments (x, argc, argv, i, &k, &n, a, size)) { return -1; }
    }
    
    return n;
}

static t_error oscparse_performBundle (t_oscparse *x, int argc, t_atom *argv)
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
    int length = OSC_READ4INT (argv + i);
        
    err = (length <= 0 || length & 3);  /* Must be a multiple of 4. */
    
    if (!err) { 
        err = oscparse_perform (x, length, argv + i + headerMessage);
        i += headerMessage + length;
    }
    //
    }
    //
    }
    
    if (err) { error_invalid (sym_oscparse, sym_bundle); }
    
    return err;
}

static t_error oscparse_perform (t_oscparse *x, int argc, t_atom *argv)
{
    if (OSC_GETCHAR (argv) == '#') { return oscparse_performBundle (x, argc, argv); }
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
    
    i = OSC_ROUND4 (i + 1);
    
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
    
    ATOMS_ALLOCA (a, size);
    
    /* Fetch adress and arguments. */
    
    n = oscparse_performFetch (x, argc, argv, typeOnset, numberOfTypeTags, a, size);
    
    if (n == -1) { err = PD_ERROR; }
    else {
        outlet_list (x->x_outlet, n, a);
    }
    
    ATOMS_FREEA (a, size);
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
#pragma mark -

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
        err = oscparse_perform (x, argc, argv);
    }
    
    if (err) { 
        error_failed (sym_oscparse);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_oscparse *oscparse_new (t_symbol *s, int argc, t_atom *argv)
{
    t_oscparse *x = (t_oscparse *)pd_new (oscparse_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void oscparse_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_oscparse,
            (t_newmethod)oscparse_new,
            NULL,
            sizeof (t_oscparse),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, oscparse_list);
    
    oscparse_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
