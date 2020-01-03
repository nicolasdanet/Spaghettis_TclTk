
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_ARRAY_DEFAULT_SIZE      4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_array *tll_arrayNew (int size)
{
    tll_array *x = (tll_array *)PD_MEMORY_GET (sizeof (tll_array));
    
    x->a_size   = (size > 0) ? size : TLL_ARRAY_DEFAULT_SIZE;
    x->a_values = (int *)PD_MEMORY_GET (x->a_size * sizeof (int));
    x->a_index  = 0;

    return x;
}

void tll_arrayFree (tll_array *x)
{
    if (x) { PD_MEMORY_FREE (x->a_values); PD_MEMORY_FREE (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://www.cse.yorku.ca/~oz/hash.html > */

/* Bernstein hash. */

uint32_t tll_arrayHash (tll_array *x)
{
    unsigned int hash = 5381;
    size_t i;
    size_t size = x->a_index * sizeof (int);
    const char *s = (const char *)x->a_values;
    
    for (i = 0; i < size; i++) { hash = ((hash << 5) + hash) + s[i]; }

    return (uint32_t)hash;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tll_arrayAppend (tll_array *x, int value)
{
    if (x->a_index == x->a_size) {
        size_t oldSize = x->a_size * sizeof (int);
        size_t newSize = oldSize * 2;
        x->a_values = (int *)PD_MEMORY_RESIZE (x->a_values, oldSize, newSize);
        x->a_size  *= 2;
    }
    
    x->a_values[x->a_index] = value; x->a_index++;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error tll_arrayRemoveLast (tll_array *x)
{
    if (x->a_index) { x->a_index--; return PD_ERROR_NONE; }
    
    return PD_ERROR;
}

int tll_arrayContainsValue (const tll_array *x, int value)
{
    int j;
    
    for (j = 0; j < x->a_index; j++) { if (x->a_values[j] == value) { return 1; } }
        
    return 0;
}

t_error tll_arrayIndexOfValue (const tll_array *x, int value, int *i)
{
    int j;
        
    for (j = 0; j < x->a_index; j++) { if (x->a_values[j] == value) { (*i) = j; return PD_ERROR_NONE; } }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
