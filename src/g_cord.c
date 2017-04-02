
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

#define CORD_GRIP_SIZE      4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void cord_set (t_cord *x, t_outconnect *connection, int isSignal, int a, int b, int c, int d)
{
    x->tr_lineStartX     = a;
    x->tr_lineStartY     = b;
    x->tr_lineEndX       = c;
    x->tr_lineEndY       = d;
    x->tr_lineIsSignal   = isSignal;
    x->tr_lineConnection = connection;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void cord_init (t_cord *x, t_outconnect *connection)
{
    cord_set (x, connection, 0, 0, 0, 0, 0);
}

void cord_make (t_cord *x, t_outconnect *connection, t_object *src, int i, t_object *dest, int j, t_glist *g)
{
    int isSignal = object_isSignalOutlet (src, i);
    int m        = object_getNumberOfOutlets (src);
    int n        = object_getNumberOfInlets (dest);
    
    t_rectangle srcBox;
    t_rectangle destBox;
    
    gobj_getRectangle (cast_gobj (src), g, &srcBox);
    gobj_getRectangle (cast_gobj (dest), g, &destBox);
    
    {
    //
    int a = rectangle_getTopLeftX (&srcBox) + inlet_middle (rectangle_getWidth (&srcBox), i, m);
    int b = rectangle_getBottomRightY (&srcBox);
    int c = rectangle_getTopLeftX (&destBox) + inlet_middle (rectangle_getWidth (&destBox), j, n);
    int d = rectangle_getTopLeftY (&destBox);
    
    cord_set (x, connection, isSignal, a, b, c, d);
    //
    }
}   
                                                
int cord_hit (t_cord *x, int positionX, int positionY)
{
    t_float a = x->tr_lineStartX;
    t_float b = x->tr_lineStartY;
    t_float c = x->tr_lineEndX;
    t_float d = x->tr_lineEndY;
    
    t_rectangle r;
    
    rectangle_set (&r, a, b, c, d); rectangle_enlarge (&r, CORD_GRIP_SIZE / 2);
    
    if (rectangle_containsPoint (&r, positionX, positionY)) {
    //
    /* Area of the triangle (with extremities of the line and mouse as vertices). */
    
    t_float area = a * (positionY - d) + positionX * (d - b) + c * (b - positionY);
    
    /* Tolerance proportional to the distance between the line extremities. */
    
    t_float k = PD_MAX (PD_ABS (c - a), PD_ABS (d - b));    
    
    if (PD_ABS (area) < (k * CORD_GRIP_SIZE)) {
        return 1;
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
