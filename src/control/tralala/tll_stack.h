
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tll_stack {
    int     s_size;
    int     s_index;
    int     s_popped;
    int     *s_values;
    } tll_stack;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_stack   *tll_stackNew       (int size);

void        tll_stackFree       (tll_stack *x);
t_error     tll_stackAppend     (tll_stack *x, int value);
t_error     tll_stackPop        (tll_stack *x);
void        tll_stackSort       (tll_stack *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void tll_stackClear (tll_stack *x)
{
    x->s_index = x->s_popped = 0;
}

static inline int tll_stackGetSize (const tll_stack *x)
{
    return x->s_index;
}

static inline int tll_stackGetPopped (const tll_stack *x)
{
    return x->s_popped;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
