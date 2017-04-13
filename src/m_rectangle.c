
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

void rectangle_set (t_rectangle *r, int a, int b, int c, int d)
{
    r->rect_topLeftX     = PD_MIN (a, c);
    r->rect_topLeftY     = PD_MIN (b, d);
    r->rect_bottomRightX = PD_MAX (a, c);
    r->rect_bottomRightY = PD_MAX (b, d);
    r->rect_isNothing    = 0;
}

void rectangle_setByAtoms (t_rectangle *r, int argc, t_atom *argv)
{
    if (argc && argv) {
    //
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int c = (int)atom_getFloatAtIndex (2, argc, argv);
    int d = (int)atom_getFloatAtIndex (3, argc, argv);
    
    rectangle_set (r, a, b, c, d);
    //
    }
}

void rectangle_setByAtomsByWidthAndHeight (t_rectangle *r, int argc, t_atom *argv)
{
    if (argc && argv) {
    //
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int c = (int)atom_getFloatAtIndex (2, argc, argv);
    int d = (int)atom_getFloatAtIndex (3, argc, argv);
    
    rectangle_setByWidthAndHeight (r, a, b, c, d);
    //
    }
}

void rectangle_setByWidthAndHeight (t_rectangle *r, int a, int b, int w, int h)
{
    rectangle_set (r, a, b, a + PD_ABS (w), b + PD_ABS (h));
}

void rectangle_setEverything (t_rectangle *r)
{
    rectangle_set (r, -PD_INT_MAX, -PD_INT_MAX, PD_INT_MAX, PD_INT_MAX);
}

void rectangle_setNothing (t_rectangle *r)
{
    rectangle_set (r, 0, 0, 0, 0); r->rect_isNothing = 1;
}

void rectangle_setCopy (t_rectangle *r, t_rectangle *toCopy)
{
    r->rect_topLeftX     = toCopy->rect_topLeftX;
    r->rect_topLeftY     = toCopy->rect_topLeftY;
    r->rect_bottomRightX = toCopy->rect_bottomRightX;
    r->rect_bottomRightY = toCopy->rect_bottomRightY;
    r->rect_isNothing    = toCopy->rect_isNothing;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int rectangle_areEquals (t_rectangle *r1, t_rectangle *r2)
{
    if (r1->rect_isNothing         != r2->rect_isNothing)    { return 0; }
    else if (r1->rect_topLeftX     != r2->rect_topLeftX)     { return 0; }
    else if (r1->rect_topLeftY     != r2->rect_topLeftY)     { return 0; }
    else if (r1->rect_bottomRightX != r2->rect_bottomRightX) { return 0; }
    else if (r1->rect_bottomRightY != r2->rect_bottomRightY) { return 0; }
    
    return 1;
}

int rectangle_overlap (t_rectangle *r1, t_rectangle *r2)
{
    int noOverlap = r1->rect_topLeftX > r2->rect_bottomRightX ||
                    r2->rect_topLeftX > r1->rect_bottomRightX ||
                    r1->rect_topLeftY > r2->rect_bottomRightY ||
                    r2->rect_topLeftY > r1->rect_bottomRightY;
    
    return !noOverlap;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int rectangle_isEverything (t_rectangle *r)
{
    t_rectangle t; rectangle_setEverything (&t);
    
    return rectangle_areEquals (r, &t);
}

int rectangle_isNothing (t_rectangle *r)
{
    t_rectangle t; rectangle_setNothing (&t);
    
    return rectangle_areEquals (r, &t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rectangle_enlarge (t_rectangle *r, int n)
{
    if (!rectangle_isNothing (r)) {
    //
    int a = r->rect_topLeftX - n;
    int b = r->rect_topLeftY - n;
    int c = r->rect_bottomRightX + n;
    int d = r->rect_bottomRightY + n;
    
    rectangle_set (r, a, b, c, d);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void rectangle_boundingBoxAddRectangle (t_rectangle *r, t_rectangle *toAdd)
{
    if (!rectangle_isNothing (toAdd)) { 

        if (rectangle_isNothing (r)) { rectangle_setCopy (r, toAdd); }
        else {
            r->rect_topLeftX     = PD_MIN (r->rect_topLeftX,     toAdd->rect_topLeftX);
            r->rect_topLeftY     = PD_MIN (r->rect_topLeftY,     toAdd->rect_topLeftY);
            r->rect_bottomRightX = PD_MAX (r->rect_bottomRightX, toAdd->rect_bottomRightX);
            r->rect_bottomRightY = PD_MAX (r->rect_bottomRightY, toAdd->rect_bottomRightY);
        }
    }
}

void rectangle_boundingBoxAddPoint (t_rectangle *r, int x, int y)
{
    t_rectangle t; rectangle_set (&t, x, y, x, y);
    
    rectangle_boundingBoxAddRectangle (r, &t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int rectangle_containsPoint (t_rectangle *r, int x, int y)
{
    if (r->rect_isNothing)             { return 0; }
    else if (x < r->rect_topLeftX)     { return 0; }
    else if (x > r->rect_bottomRightX) { return 0; }
    else if (y < r->rect_topLeftY)     { return 0; }
    else if (y > r->rect_bottomRightY) { return 0; }
    
    return 1;
}

int rectangle_containsRectangle (t_rectangle *r, t_rectangle *c)
{
    if (rectangle_isNothing (r) || rectangle_isNothing (c))                            { return 0; }
    else if (!rectangle_containsPoint (r, c->rect_topLeftX,     c->rect_topLeftY))     { return 0; }
    else if (!rectangle_containsPoint (r, c->rect_bottomRightX, c->rect_bottomRightY)) { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
