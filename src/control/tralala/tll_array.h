
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tll_array {
    int     a_size;
    int     a_index;
    int     *a_values;
    } tll_array;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_array   *tll_arrayNew           (int size);

void        tll_arrayFree           (tll_array *x);
uint32_t    tll_arrayHash           (tll_array *x);
t_error     tll_arrayRemoveLast     (tll_array *x);
void        tll_arrayAppend         (tll_array *x, int value);

int         tll_arrayContainsValue  (const tll_array *x, int value);
t_error     tll_arrayIndexOfValue   (const tll_array *x, int value, int *i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void tll_arrayClear (tll_array *x)
{
    x->a_index = 0;
}

static inline void tll_arraySetAtIndex (const tll_array *x, int i, int n)
{
    x->a_values[i] = n;
}

static inline int tll_arrayGetAtIndex (const tll_array *x, int i)
{
    return x->a_values[i];
}

static inline int tll_arrayGetSize (const tll_array *x)
{
    return x->a_index;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
