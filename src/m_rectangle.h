
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_rectangle_h_
#define __m_rectangle_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _bounds {
    t_float     b_left;
    t_float     b_top;
    t_float     b_right;
    t_float     b_bottom;
    } t_bounds;

typedef struct _rectangle {
    int         rect_topLeftX;
    int         rect_topLeftY;
    int         rect_bottomRightX;
    int         rect_bottomRightY;
    int         rect_isNothing;
    } t_rectangle;

typedef struct _drag {
    int         d_originX;
    int         d_originY;
    int         d_startX;
    int         d_startY;
    int         d_endX;
    int         d_endY;
    } t_drag;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error bounds_set          (t_bounds *b, t_float left, t_float top, t_float right, t_float bottom);
t_error bounds_setByAtoms   (t_bounds *b, int argc, t_atom *argv);
void    bounds_setCopy      (t_bounds *b, t_bounds *toCopy);
int     bounds_areEquals    (t_bounds *b1, t_bounds *b2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_float bounds_getLeft (t_bounds *b)
{
    return b->b_left;
}

static inline t_float bounds_getTop (t_bounds *b)
{
    return b->b_top;
}

static inline t_float bounds_getRight (t_bounds *b)
{
    return b->b_right;
}

static inline t_float bounds_getBottom (t_bounds *b)
{
    return b->b_bottom;
}

static inline t_float bounds_getRangeX (t_bounds *b)
{
    return (b->b_right - b->b_left);
}

static inline t_float bounds_getRangeY (t_bounds *b)
{
    return (b->b_bottom - b->b_top);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        rectangle_set                               (t_rectangle *r, int a, int b, int c, int d);
void        rectangle_setByWidthAndHeight               (t_rectangle *r, int a, int b, int width, int height);
void        rectangle_setByAtoms                        (t_rectangle *r, int argc, t_atom *argv);
void        rectangle_setByAtomsByWidthAndHeight        (t_rectangle *r, int argc, t_atom *argv);
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
void        rectangle_enlargeY                          (t_rectangle *r, int n);
int         rectangle_containsPoint                     (t_rectangle *r, int x, int y);
int         rectangle_containsRectangle                 (t_rectangle *r, t_rectangle *isContained);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

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
// MARK: -

static inline void rectangle_setWidth (t_rectangle *r, int width)
{
    r->rect_bottomRightX = r->rect_topLeftX + PD_ABS (width);
}

static inline void rectangle_setHeight (t_rectangle *r, int heigth)
{
    r->rect_bottomRightY = r->rect_topLeftY + PD_ABS (heigth);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void drag_begin (t_drag *x, int a, int b)
{
    x->d_originX = x->d_startX = x->d_endX = a;
    x->d_originY = x->d_startY = x->d_endY = b;
}

static inline void drag_set (t_drag *x, int a, int b)
{
    x->d_endX = a;
    x->d_endY = b;
}

static inline void drag_close (t_drag *x)
{
    x->d_startX = x->d_endX;
    x->d_startY = x->d_endY;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int drag_getStartX (t_drag *x)
{
    return x->d_startX;
}

static inline int drag_getStartY (t_drag *x)
{
    return x->d_startY;
}

static inline int drag_getEndX (t_drag *x)
{
    return x->d_endX;
}

static inline int drag_getEndY (t_drag *x)
{
    return x->d_endY;
}

static inline int drag_getMoveX (t_drag *x)
{
    return (drag_getEndX (x) - drag_getStartX (x));
}

static inline int drag_getMoveY (t_drag *x)
{
    return (drag_getEndY (x) - drag_getStartY (x));
}

static inline int drag_hasMoved (t_drag *x)
{
    return (x->d_startX != x->d_originX || x->d_startY != x->d_originY);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_rectangle_h_
