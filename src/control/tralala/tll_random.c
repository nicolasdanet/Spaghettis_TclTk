
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_rand48 *tll_randomNewWithSeed (t_seed s)
{
    t_rand48 *x = (t_rand48 *)PD_MEMORY_GET (sizeof (t_rand48));
    
    (*x) = s ? s : (t_rand48)(time_makeRandomSeed() & 0xffffffffffffULL);
    
    return x;
}

t_rand48 *tll_randomNew (void)
{
    return tll_randomNewWithSeed (0);
}

void tll_randomFree (t_rand48 *x)
{
    if (x) { PD_MEMORY_FREE (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
