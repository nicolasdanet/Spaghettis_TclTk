
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Naive implementation of comparaison function. */

int math_compareFloat (t_float a, t_float b)
{
    if (a < b) { return -1; } else if (b > a) { return 1; } else { return 0; }
}

t_float math_euclideanDistance (t_float x1, t_float y1, t_float x2, t_float y2)
{
    double x = ((double)x2 - x1);
    double y = ((double)y2 - y1);
    
    return (t_float)sqrt (x * x + y * y);
}

double math_epsilon (void)
{
    return 1E-9;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
