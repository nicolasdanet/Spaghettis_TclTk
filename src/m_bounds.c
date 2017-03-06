
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error bounds_set (t_bounds *b, t_float left, t_float top, t_float right, t_float bottom)
{
    if (left == right || top == bottom) { return PD_ERROR; }
    else {
        b->b_left   = left;
        b->b_top    = top;
        b->b_right  = right;
        b->b_bottom = bottom;
    }
    
    return PD_ERROR_NONE;
}

t_error bounds_setByAtoms (t_bounds *b, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    t_float left    = atom_getFloat (argv + 0);
    t_float top     = atom_getFloat (argv + 1);
    t_float right   = atom_getFloat (argv + 2);
    t_float bottom  = atom_getFloat (argv + 3);
    
    return bounds_set (b, left, top, right, bottom);
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
