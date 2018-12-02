
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_STACK_DEFAULT_SIZE      4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_stack *tll_stackNew (int size)
{
    tll_stack *x = (tll_stack *)PD_MEMORY_GET (sizeof (tll_stack));

    x->s_size   = (size > 0) ? size : TLL_STACK_DEFAULT_SIZE;
    x->s_values = (int *)PD_MEMORY_GET (x->s_size * sizeof (int));
    x->s_index  = 0;
    x->s_popped = 0;
    
    return x;
}

void tll_stackFree (tll_stack *x)
{
    if (x) { PD_MEMORY_FREE (x->s_values); PD_MEMORY_FREE (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error tll_stackAppend (tll_stack *x, int value)
{   
    t_error err = PD_ERROR;
    
    if (x->s_index < x->s_size)  {
        err = PD_ERROR_NONE;
        x->s_values[x->s_index] = value;
        x->s_index++;
    }
    
    return err;
}

t_error tll_stackPop (tll_stack *x)
{
    t_error err = PD_ERROR;
    
    if (x->s_index) {
        err = PD_ERROR_NONE;
        x->s_popped = x->s_values[x->s_index - 1];
        x->s_index--;
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < https://en.wikipedia.org/wiki/Insertion_sort > */

void tll_stackSort (tll_stack *x)
{
    int i, j, t;
    
    for (i = 1; i < x->s_index; i++) {
    //
    t = x->s_values[i];
    for (j = i; (j > 0 && t > x->s_values[j - 1]); j--) { x->s_values[j] = x->s_values[j - 1]; }
    x->s_values[j] = t;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
