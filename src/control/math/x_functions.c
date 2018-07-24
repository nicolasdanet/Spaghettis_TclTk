
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_TINYEXPR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_randMT *expr_randMT;           /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

double expr_functionRandom (void)
{
    static int once = 0;                /* Static. */
    static t_rand48 seed = 0;           /* Static. */
    
    if (!once) { seed = (t_rand48)time_makeRandomSeed(); once = 1; }
    
    return PD_RAND48_DOUBLE (seed);
}

double expr_functionRandomMT (void)
{
    return randMT_getDouble (expr_randMT);
}

double expr_functionMinimum (double a, double b)
{
    return PD_MIN (a, b);
}

double expr_functionMaximum (double a, double b)
{
    return PD_MAX (a, b);
}

double expr_functionEqual (double a, double b)
{
    return (a == b);
}

double expr_functionUnequal (double a, double b)
{
    return (a != b);
}

double expr_functionLessThan (double a, double b)
{
    return (a < b);
}

double expr_functionLessEqual (double a, double b)
{
    return (a <= b);
}

double expr_functionGreaterThan (double a, double b)
{
    return (a > b);
}

double expr_functionGreaterEqual (double a, double b)
{
    return (a >= b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void expr_initialize (void)
{
    expr_randMT = randMT_new();
}

void expr_release (void)
{
    randMT_free (expr_randMT);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_WITH_TINYEXPR

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
