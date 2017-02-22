
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rectangle_set (t_rectangle *r, int xA, int yA, int xB, int yB)
{
    r->rect_topLeftX     = xA;
    r->rect_topLeftY     = yA;
    r->rect_bottomRightX = xB;
    r->rect_bottomRightY = yB;
}

void rectangle_setEverything (t_rectangle *r)
{
    rectangle_set (r, -PD_INT_MAX, -PD_INT_MAX, PD_INT_MAX, PD_INT_MAX);
}

void rectangle_setNothing (t_rectangle *r)
{
    rectangle_set (r, 0, 0, 0, 0);
}

void rectangle_setNowhere (t_rectangle *r)
{
    r->rect_topLeftX     = PD_INT_MAX;
    r->rect_topLeftY     = PD_INT_MAX;
    r->rect_bottomRightX = -PD_INT_MAX;
    r->rect_bottomRightY = -PD_INT_MAX;
}

void rectangle_setByCopy (t_rectangle *r, t_rectangle *toCopy)
{
    rectangle_set (r,
        toCopy->rect_topLeftX,
        toCopy->rect_topLeftY,
        toCopy->rect_bottomRightX,
        toCopy->rect_bottomRightY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rectangle_enlarge (t_rectangle *r, int n)
{
    r->rect_topLeftX     -= n;
    r->rect_topLeftY     -= n;
    r->rect_bottomRightX += n;
    r->rect_bottomRightY += n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rectangle_boundingBoxAddRectangle (t_rectangle *r, t_rectangle *toAdd)
{
    r->rect_topLeftX     = PD_MIN (r->rect_topLeftX,     toAdd->rect_topLeftX);
    r->rect_topLeftY     = PD_MIN (r->rect_topLeftY,     toAdd->rect_topLeftY);
    r->rect_bottomRightX = PD_MAX (r->rect_bottomRightX, toAdd->rect_bottomRightX);
    r->rect_bottomRightY = PD_MAX (r->rect_bottomRightY, toAdd->rect_bottomRightY);
}

void rectangle_boundingBoxAddPoint (t_rectangle *r, int x, int y)
{
    t_rectangle t; rectangle_set (&t, x, y, x, y);
    
    rectangle_boundingBoxAddRectangle (r, &t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int rectangle_areEquals (t_rectangle *r1, t_rectangle *r2)
{
    if (r1->rect_topLeftX != r2->rect_topLeftX)         { return 0; }
    if (r1->rect_topLeftY != r2->rect_topLeftY)         { return 0; }
    if (r1->rect_bottomRightX != r2->rect_bottomRightX) { return 0; }
    if (r1->rect_bottomRightY != r2->rect_bottomRightY) { return 0; }
    
    return 1;
}

int rectangle_isEverything (t_rectangle *r)
{
    t_rectangle t; rectangle_setEverything (&t);
    
    return rectangle_areEquals (r, &t);
}

int rectangle_isNowhere (t_rectangle *r)
{
    return ((r->rect_topLeftX > r->rect_bottomRightX) || (r->rect_topLeftY > r->rect_bottomRightY));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int rectangle_containsPoint (t_rectangle *r, int x, int y)
{
    if (x < r->rect_topLeftX)     { return 0; }
    if (x > r->rect_bottomRightX) { return 0; }
    if (y < r->rect_topLeftY)     { return 0; }
    if (y > r->rect_bottomRightY) { return 0; }
    
    return 1;
}

int rectangle_containsRectangle (t_rectangle *r, t_rectangle *c)
{
    if (rectangle_isNowhere (c))                                                       { return 0; }
    else if (!rectangle_containsPoint (r, c->rect_topLeftX, c->rect_topLeftY))         { return 0; }
    else if (!rectangle_containsPoint (r, c->rect_bottomRightX, c->rect_bottomRightY)) { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int rectangle_getTopLeftX (t_rectangle *r)
{
    return r->rect_topLeftX;
}

int rectangle_getTopLeftY (t_rectangle *r)
{
    return r->rect_topLeftY;
}

int rectangle_getBottomRightX (t_rectangle *r)
{
    return r->rect_bottomRightX;
}

int rectangle_getBottomRightY (t_rectangle *r)
{
    return r->rect_bottomRightY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
