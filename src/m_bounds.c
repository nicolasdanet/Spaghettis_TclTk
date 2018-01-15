
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error bounds_set (t_bounds *b, t_float left, t_float top, t_float right, t_float bottom)
{
    b->b_left   = (t_float)0.0;     /* Avoid overzealous warning. */
    b->b_top    = (t_float)0.0;
    b->b_right  = (t_float)0.0;
    b->b_bottom = (t_float)0.0;
        
    if (left >= right || top == bottom) { return PD_ERROR; }
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
    t_float left    = atom_getFloatAtIndex (0, argc, argv);
    t_float top     = atom_getFloatAtIndex (1, argc, argv);
    t_float right   = atom_getFloatAtIndex (2, argc, argv);
    t_float bottom  = atom_getFloatAtIndex (3, argc, argv);
    
    return bounds_set (b, left, top, right, bottom);
}

void bounds_setCopy (t_bounds *b, t_bounds *toCopy)
{
    t_error err = bounds_set (b, toCopy->b_left, toCopy->b_top, toCopy->b_right, toCopy->b_bottom);
    
    PD_UNUSED (err); PD_ASSERT (!err);
}

int bounds_areEquals (t_bounds *b1, t_bounds *b2)
{
    if (b1->b_left        != b2->b_left)   { return 0; }
    else if (b1->b_top    != b2->b_top)    { return 0; }
    else if (b1->b_right  != b2->b_right)  { return 0; }
    else if (b1->b_bottom != b2->b_bottom) { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
