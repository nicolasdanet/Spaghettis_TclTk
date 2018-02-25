
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_helpers_h_
#define __m_helpers_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _mouse {
    int     m_x;
    int     m_y;
    int     m_shift;
    int     m_ctrl;
    int     m_alt;
    int     m_dbl;
    int     m_clicked;
    t_atom  m_atoms[7];
    } t_mouse;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int mouse_argc (t_mouse *m)
{
    return 7;
}

static inline t_atom *mouse_argv (t_mouse *m)
{
    SET_FLOAT (m->m_atoms + 0, m->m_x);
    SET_FLOAT (m->m_atoms + 1, m->m_y);
    SET_FLOAT (m->m_atoms + 2, m->m_shift);
    SET_FLOAT (m->m_atoms + 3, m->m_ctrl);
    SET_FLOAT (m->m_atoms + 4, m->m_alt);
    SET_FLOAT (m->m_atoms + 5, m->m_dbl);
    SET_FLOAT (m->m_atoms + 6, m->m_clicked);
    
    return m->m_atoms;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _drag {
    int d_originX;
    int d_originY;
    int d_startX;
    int d_startY;
    int d_endX;
    int d_endY;
    int d_accumulateX;
    int d_accumulateY;
    int d_movedOnce;
    t_gobj *d_object;
    } t_drag;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void drag_begin (t_drag *x, int a, int b)
{
    x->d_originX = x->d_startX = x->d_endX = a;
    x->d_originY = x->d_startY = x->d_endY = b;
    
    x->d_accumulateX = 0;
    x->d_accumulateY = 0;
    x->d_movedOnce   = 0;
    x->d_object      = NULL;
}

static inline void drag_setEnd (t_drag *x, int a, int b)
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

static inline int drag_hasMoved (t_drag *x)
{
    return (x->d_startX != x->d_originX || x->d_startY != x->d_originY);
}

static inline int drag_hasMovedOnce (t_drag *x)
{
    if (drag_hasMoved (x) && !x->d_movedOnce) { x->d_movedOnce = 1; return 1; }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_gobj *drag_getObject (t_drag *x)
{
    return x->d_object;
}

static inline void drag_setObject (t_drag *x, t_gobj *y)
{
    x->d_object = y;
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
    int n = (x->d_endX - x->d_startX) + x->d_accumulateX;
    
    if (snap_hasSnapToGrid()) {
    //
    int k = snap_getStep();
    int t = 0;
    
    while (n >= k)  { t += k; n -= k; }
    while (n <= -k) { t -= k; n += k; }
    
    x->d_accumulateX = n;
    
    return t;
    //
    }
    
    return n;
}

static inline int drag_getMoveY (t_drag *x)
{
    int n = (x->d_endY - x->d_startY) + x->d_accumulateY;
    
    if (snap_hasSnapToGrid()) {
    //
    int k = snap_getStep();
    int t = 0;
    
    while (n >= k)  { t += k; n -= k; }
    while (n <= -k) { t -= k; n += k; }
    
    x->d_accumulateY = n;
    
    return t;
    //
    }
    
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _rectangle {
    int rect_topLeftX;
    int rect_topLeftY;
    int rect_bottomRightX;
    int rect_bottomRightY;
    int rect_isNothing;
    } t_rectangle;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    rectangle_set                           (t_rectangle *r, int a, int b, int c, int d);
void    rectangle_setByWidthAndHeight           (t_rectangle *r, int a, int b, int width, int height);
void    rectangle_setByAtoms                    (t_rectangle *r, int argc, t_atom *argv);
void    rectangle_setByAtomsByWidthAndHeight    (t_rectangle *r, int argc, t_atom *argv);
void    rectangle_setEverything                 (t_rectangle *r);
void    rectangle_setNothing                    (t_rectangle *r);
void    rectangle_setCopy                       (t_rectangle *r, t_rectangle *toCopy);

int     rectangle_isEverything                  (t_rectangle *r);
int     rectangle_isNothing                     (t_rectangle *r);
int     rectangle_areEquals                     (t_rectangle *r1, t_rectangle *r2);
int     rectangle_overlap                       (t_rectangle *r1, t_rectangle *r2);

void    rectangle_addRectangle                  (t_rectangle *r, t_rectangle *toAdd);
void    rectangle_addPoint                      (t_rectangle *r, int x, int y);
void    rectangle_enlarge                       (t_rectangle *r, int n);
void    rectangle_enlargeHeight                 (t_rectangle *r, int n);
int     rectangle_containsX                     (t_rectangle *r, int x);
int     rectangle_containsY                     (t_rectangle *r, int y);
int     rectangle_containsPoint                 (t_rectangle *r, int x, int y);
int     rectangle_containsRectangle             (t_rectangle *r, t_rectangle *isContained);

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

static inline int rectangle_getWidth (t_rectangle *r)
{
    return r->rect_bottomRightX - r->rect_topLeftX;
}

static inline int rectangle_getHeight (t_rectangle *r)
{
    return r->rect_bottomRightY - r->rect_topLeftY;
}

static inline int rectangle_getMiddleX (t_rectangle *r)
{
    return (r->rect_topLeftX + r->rect_bottomRightX) / 2;
}

static inline int rectangle_getMiddleY (t_rectangle *r)
{
    return (r->rect_topLeftY + r->rect_bottomRightY) / 2;
}

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

typedef struct _bounds {
    t_float b_left;
    t_float b_top;
    t_float b_right;
    t_float b_bottom;
    } t_bounds;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error bounds_set          (t_bounds *b, t_float left, t_float top, t_float right, t_float bottom);
t_error bounds_setByAtoms   (t_bounds *b, int argc, t_atom *argv);
void    bounds_setCopy      (t_bounds *b, t_bounds *toCopy);
int     bounds_areEquals    (t_bounds *m, t_bounds *n);

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
    return b->b_right - b->b_left;
}

static inline t_float bounds_getRangeY (t_bounds *b)
{
    return b->b_bottom - b->b_top;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _fileproperties { char f_directory[PD_STRING]; char *f_name; } t_fileproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline char *fileproperties_getDirectory (t_fileproperties *p)
{
    return p->f_directory;
}

static inline char *fileproperties_getName (t_fileproperties *p)
{
    return p->f_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _iterator {
    int                 iter_argc;
    int                 iter_index;
    t_atom              *iter_argv;
    } t_iterator;

typedef struct _pathlist {
    struct _pathlist    *pl_next;
    char                *pl_string;
    } t_pathlist;

typedef struct _heapstring {
    size_t              hs_allocated;
    size_t              hs_size;
    char                *hs_raw;
    } t_heapstring;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_iterator  *iterator_new                       (int argc, t_atom *argv);

void        iterator_free                       (t_iterator *x);
int         iterator_next                       (t_iterator *x, t_atom **a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist  *pathlist_newAppend                 (t_pathlist *x, const char *s);
t_pathlist  *pathlist_newAppendEncoded          (t_pathlist *x, t_symbol *s);
char        *pathlist_getPath                   (t_pathlist *x);
t_pathlist  *pathlist_getNext                   (t_pathlist *x);

void        pathlist_free                       (t_pathlist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_heapstring    *heapstring_new                 (int size);
char            *heapstring_getRaw              (t_heapstring *x);

char        *heapstring_freeBorrowUnzeroed      (t_heapstring *x);  /* Caller acquires string ownership. */

void        heapstring_free                     (t_heapstring *x);
int         heapstring_getSize                  (t_heapstring *x);

t_error     heapstring_add                      (t_heapstring *x, const char *src);
t_error     heapstring_append                   (t_heapstring *x, const char *src, int n);
t_error     heapstring_addSprintf               (t_heapstring *x, const char *format, ...);

void        heapstring_removeIfContainsAtEnd    (t_heapstring *x, char c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_helpers_h_
