
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_rectangle_h_
#define __m_rectangle_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        rectangle_set                               (t_rectangle *r, int a, int b, int c, int d);
void        rectangle_setEverything                     (t_rectangle *r);
void        rectangle_setNothing                        (t_rectangle *r);
void        rectangle_setCopy                           (t_rectangle *r, t_rectangle *toCopy);

int         rectangle_isEverything                      (t_rectangle *r);
int         rectangle_isNothing                         (t_rectangle *r);
int         rectangle_areEquals                         (t_rectangle *r1, t_rectangle *r2);
int         rectangle_overlap                           (t_rectangle *r1, t_rectangle *r2);

void        rectangle_boundingBoxAddRectangle           (t_rectangle *r, t_rectangle *toAdd);
void        rectangle_boundingBoxAddPoint               (t_rectangle *r, int x, int y);
void        rectangle_enlarge                           (t_rectangle *r, int n);
int         rectangle_containsPoint                     (t_rectangle *r, int x, int y);
int         rectangle_containsRectangle                 (t_rectangle *r, t_rectangle *isContained);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int rectangle_getTopLeftX (t_rectangle *r)
{
    return r->rect_topLeftX;
}

static inline int rectangle_getTopLeftY (t_rectangle *r)
{
    return r->rect_topLeftY;
}

static inline int rectangle_getBottomRightX (t_rectangle *r)
{
    return r->rect_bottomRightX;
}

static inline int rectangle_getBottomRightY (t_rectangle *r)
{
    return r->rect_bottomRightY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int rectangle_getWidth (t_rectangle *r)
{
    return (rectangle_getBottomRightX (r) - rectangle_getTopLeftX (r));
}

static inline int rectangle_getHeight (t_rectangle *r)
{
    return (rectangle_getBottomRightY (r) - rectangle_getTopLeftY (r));
}

static inline int rectangle_getMiddleX (t_rectangle *r)
{
    return ((rectangle_getTopLeftX (r) + rectangle_getBottomRightX (r)) / 2);
}

static inline int rectangle_getMiddleY (t_rectangle *r)
{
    return ((rectangle_getTopLeftY (r) + rectangle_getBottomRightY (r)) / 2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_rectangle_h_
