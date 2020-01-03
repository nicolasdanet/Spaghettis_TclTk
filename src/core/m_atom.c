
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int dollar_getDollarZero (t_glist *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array *gpointer_getParentForWord (t_gpointer *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int atom_sortCompare (const void *, const void *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error atom_symbolToBackslashedString (t_atom *a, char *s, int size)
{
    const char *p = NULL;
    int quote = 0;
    t_error err = PD_ERROR_NONE;
    
    if (size < 2) { err = PD_ERROR; }
    else {
    //
    for (p = GET_SYMBOL (a)->s_name; *p; p++) {
        if (char_isEnd (*p) || char_isEscape (*p) || string_startWithOneDollarAndOneNumber (p)) {
            quote = 1; break; 
        }
    }
            
    if (quote) {
    
        char *base = s;
        char *last = s + (size - 2);
        
        p = GET_SYMBOL (a)->s_name;
        
        while (base < last && *p) {
        //
        if (char_isEnd (*p) || char_isEscape (*p) || string_startWithOneDollarAndOneNumber (p)) {
            *base++ = '\\';
        }
        *base++ = *p++;
        //
        }
        
        if (*p) { err = PD_ERROR; }
        
        *base = 0;

    } else {
        err = string_copy (s, size, GET_SYMBOL (a)->s_name);
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atom_setSymbol (t_atom *a, t_symbol *s)
{
    SET_SYMBOL (a, s);
}

void atom_setFloat (t_atom *a, t_float f)
{
    SET_FLOAT (a, f);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_float atom_getFloat (t_atom *a)
{
    if (IS_FLOAT (a)) { return GET_FLOAT (a); }
    else {
        return 0.0;
    }
}

t_float atom_getFloatAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getFloat (argv + n); }
    else {
        return 0.0;
    }
}

t_symbol *atom_getSymbol (t_atom *a)
{
    if (IS_SYMBOL (a)) { return GET_SYMBOL (a); }
    else { 
        return (&s_);
    }
}

t_symbol *atom_getSymbolAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getSymbol (argv + n); }
    else {
        return (&s_);
    }
}

t_symbol *atom_getSymbolOrDollarSymbol (t_atom *a)
{
    if ((IS_SYMBOL (a) || IS_DOLLARSYMBOL (a))) { return GET_SYMBOL (a); }
    else { 
        return (&s_);
    }
}

t_symbol *atom_getSymbolOrDollarSymbolAtIndex (int n, int argc, t_atom *argv)
{
    if (n >= 0 && n < argc) { return atom_getSymbolOrDollarSymbol (argv + n); }
    else {
        return (&s_);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_atomtype atom_getType (t_atom *a)
{
    return a->a_type;
}

int atom_typesAreEquals (t_atom *a, t_atom *b)
{
    return (atom_getType (a) == atom_getType (b));
}

int atom_areEquals (t_atom *a, t_atom *b)
{
    return (atom_sortCompare ((const void *)a, (const void *)b) == 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atom_copyAtom (t_atom *src, t_atom *dest)
{
    (*dest) = (*src);
}

void atom_copyAtoms (t_atom *src, int m, t_atom *dest, int n)
{
    int size = PD_MIN (m, n); int i; for (i = 0; i < size; i++) { dest[i] = src[i]; }
}

int atom_copyAtomsExpandedWithArguments (t_atom *src,
    int m,
    t_atom *dest,
    int n,
    t_glist *glist,
    int argc,
    t_atom *argv)
{
    int i;
    int size = PD_MIN (m, n);
    int expanded = 0;
    
    for (i = 0; i < size; i++) {
    //
    t_atom *a = src + i; t_atom *b = dest + i;
    
    if (IS_DOLLARSYMBOL (a)) {
        t_symbol *s = dollar_expandSymbolWithArguments (GET_SYMBOL (a), glist, argc, argv);
        if (s) { SET_SYMBOL (b, s); expanded++; }
        else {
            SET_DOLLARSYMBOL (b, GET_SYMBOL (a));
        }
    } else if (IS_DOLLAR (a)) {
        expanded += dollar_expandWithArguments (a, b, glist, argc, argv);
    } else {
        *b = *a;
    }
    //
    }
    
    return expanded;
}

int atom_copyAtomsExpanded (t_atom *src, int m, t_atom *dest, int n, t_glist *glist)
{
    t_environment *e = NULL;
    
    if (glist) { e = glist_getEnvironment (glist); }

    if (!e) {
        return atom_copyAtomsExpandedWithArguments (src, m, dest, n, glist, 0, NULL);
    } else {
        return atom_copyAtomsExpandedWithArguments (src,
            m,
            dest,
            n,
            glist,
            environment_getNumberOfArguments (e),
            environment_getArguments (e));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error atom_withStringUnzeroed (t_atom *a, const char *s, int size)
{
    t_buffer *t = buffer_new();
    
    buffer_withStringUnzeroed (t, s, size);
    
    if (buffer_getSize (t) != 1) { SET_NULL (a); }
    else {
        *a = *(buffer_getAtoms (t));
    }
    
    buffer_free (t);
    
    if (IS_NULL (a)) { return PD_ERROR; } else { return PD_ERROR_NONE; }
}

t_error atom_toString (t_atom *a, char *dest, int size)
{
    t_error err = PD_ERROR;
    
    PD_ASSERT (size > 0);
    
    switch (a->a_type) {
        case A_SYMBOL       : err = atom_symbolToBackslashedString (a, dest, size);             break;
        case A_FLOAT        : err = string_sprintf (dest, (size_t)size, "%.9g", GET_FLOAT (a)); break;
        case A_DOLLAR       : err = string_sprintf (dest, (size_t)size, "$%d", GET_DOLLAR (a)); break;
        case A_DOLLARSYMBOL : err = string_copy (dest,  (size_t)size, GET_SYMBOL (a)->s_name);  break;
        case A_SEMICOLON    : err = string_copy (dest,  (size_t)size, ";");                     break;
        case A_COMMA        : err = string_copy (dest,  (size_t)size, ",");                     break;
        case A_POINTER      :
            err = string_copy (dest, (size_t)size, gpointer_asRepresentation (GET_POINTER (a))->s_name);
            break;
        default             : err = string_copy (dest,  (size_t)size, "?"); PD_BUG;
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char *atom_atomsToString (int argc, t_atom *argv)
{
    char *s = NULL;
    t_buffer *t = buffer_new();
    
    buffer_append (t, argc, argv);
    s = buffer_toString (t);
    buffer_free (t);
    
    string_removeCharacter (s, '\\');
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void atom_invalidatePointers (int argc, t_atom *argv)
{
    int i;
    
    for (i = 0; i < argc; i++) {
    //
    if (IS_POINTER (argv + i)) { t_gpointer *gp = gpointer_getEmpty(); SET_POINTER (argv + i, gp); }
    //
    }
}

/* < https://en.wikipedia.org/wiki/Fisher-Yates_shuffle > */

void atom_shuffle (int argc, t_atom *argv)
{
    static int once; static t_rand48 seed;   /* Static. */

    if (!once) { once = 1; PD_RAND48_INIT (seed); }
    
    int i;
    
    for (i = (argc - 1); i > 0; i--) {
        int rnd   = (int)(PD_RAND48_DOUBLE (seed) * (i + 1));
        t_atom t  = argv[rnd];
        argv[rnd] = argv[i];
        argv[i]   = t;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int atom_sortComparePointersElement (t_gpointer *gp1, t_gpointer *gp2)
{
    int index1 = gpointer_getIndex (gp1);
    int index2 = gpointer_getIndex (gp2);
    
    if (index1 < index2) { return -1; } else if (index1 > index2) { return 1; }
    
    return 0;
}

static int atom_sortComparePointersArray (t_gpointer *gp1, t_gpointer *gp2)
{
    t_id id1 = array_getIdentifier (gpointer_getParentForWord (gp1));
    t_id id2 = array_getIdentifier (gpointer_getParentForWord (gp2));
    
    if (id1 == id2) { return atom_sortComparePointersElement (gp1, gp2); }
    
    return (id1 < id2 ? -1 : 1);
}

static int atom_sortComparePointersValid (t_gpointer *gp1, t_gpointer *gp2)
{
    t_scalar *b1 = gpointer_getBase (gp1);
    t_scalar *b2 = gpointer_getBase (gp2);
    t_id u1      = gobj_getUnique (cast_gobj (b1));
    t_id u2      = gobj_getUnique (cast_gobj (b2));

    if (u1 != u2) { return (u1 < u2 ? -1 : 1); }
    else {
    //
    int isArray1 = gpointer_isWord (gp1);
    int isArray2 = gpointer_isWord (gp2);
    
    if (isArray1 || isArray2) {
    //
    if (isArray1 == isArray2) { return atom_sortComparePointersArray (gp1, gp2); }
    else {
        return (isArray1 < isArray2 ? -1 : 1);
    }
    //
    }
    //
    }
    
    return 0;
}

static int atom_sortComparePointersValidOrNull (t_gpointer *gp1, t_gpointer *gp2)
{
    int null1 = !gpointer_isValid (gp1);
    int null2 = !gpointer_isValid (gp2);

    if (null1 || null2) {
    //
    if (null1 == null2) { return 0; } else { return (null1 > null2 ? -1 : 1);
    }
    //
    }
    
    return atom_sortComparePointersValid (gp1, gp2);
}

static int atom_sortComparePointers (t_gpointer *gp1, t_gpointer *gp2)
{
    int invalid1 = !gpointer_isValidOrNull (gp1);
    int invalid2 = !gpointer_isValidOrNull (gp2);
    
    if (invalid1 || invalid2) {
    //
    if (invalid1 == invalid2) { return 0; } else { return (invalid1 > invalid2 ? -1 : 1);
    }
    //
    }
    
    return atom_sortComparePointersValidOrNull (gp1, gp2);
}

static int atom_sortCompareFloats (t_float f1, t_float f2)
{
    if (f1 < f2) { return -1; } else if (f1 > f2) { return 1; } return 0;
}

static int atom_sortCompareDollars (int i1, int i2)
{
    if (i1 < i2) { return -1; } else if (i1 > i2) { return 1; } return 0;
}

static int atom_sortCompare (const void *p1, const void *p2)
{
    t_atom *a1 = (t_atom *)p1;
    t_atom *a2 = (t_atom *)p2;
    t_atomtype t1 = atom_getType (a1);
    t_atomtype t2 = atom_getType (a2);
    
    if (!atom_typesAreEquals (a1, a2)) { return (t1 < t2 ? -1 : 1); }
    else {
    //
    if (t1 == A_SYMBOL)             { return strcmp (GET_SYMBOL (a1)->s_name, GET_SYMBOL (a2)->s_name);     }
    else if (t1 == A_FLOAT)         { return atom_sortCompareFloats (GET_FLOAT (a1), GET_FLOAT (a2));       }
    else if (t1 == A_POINTER)       { return atom_sortComparePointers (GET_POINTER (a1), GET_POINTER (a2)); }
    else if (t1 == A_DOLLAR)        { return atom_sortCompareDollars (GET_DOLLAR (a1), GET_DOLLAR (a2));    }
    else if (t1 == A_DOLLARSYMBOL)  { return strcmp (GET_SYMBOL (a1)->s_name, GET_SYMBOL (a2)->s_name);     }
    //
    }
    
    return 0;
}

void atom_sort (int argc, t_atom *argv)
{
    if (argc) { qsort (argv, argc, sizeof (t_atom), atom_sortCompare); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
