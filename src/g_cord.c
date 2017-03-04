
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define LINE_GRIP_SIZE      4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void cord_init (t_cord *x)
{
    x->tr_lineStartX = 0;
    x->tr_lineStartY = 0;
    x->tr_lineEndX   = 0;
    x->tr_lineEndY   = 0;
}

int cord_hit (t_cord *x, int positionX, int positionY)
{
    t_float a = x->tr_lineStartX;
    t_float b = x->tr_lineStartY;
    t_float c = x->tr_lineEndX;
    t_float d = x->tr_lineEndY;
    
    t_rectangle r;
    
    rectangle_set (&r, a, b, c, d); rectangle_enlarge (&r, LINE_GRIP_SIZE / 2);
    
    if (rectangle_containsPoint (&r, positionX, positionY)) {
    //
    /* Area of the triangle (with extremities of the line and mouse as vertices). */
    
    t_float area = a * (positionY - d) + positionX * (d - b) + c * (b - positionY);
    
    /* Tolerance proportional to the distance between the line extremities. */
    
    t_float k = PD_MAX (PD_ABS (c - a), PD_ABS (d - b));    
    
    if (PD_ABS (area) < (k * LINE_GRIP_SIZE)) {
        return 1;
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
