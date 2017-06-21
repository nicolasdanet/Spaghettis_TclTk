
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_object_h_
#define __m_object_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_outconnect    *object_connect                     (t_object *src, int m, t_object *dest, int n);

void        object_disconnect                       (t_object *src, int m, t_object *dest, int n);

void        object_setFromEntry                     (t_object *x, t_glist *glist, t_box *z);

int         object_getNumberOfInlets                (t_object *x);
int         object_getNumberOfOutlets               (t_object *x);
int         object_getNumberOfSignalInlets          (t_object *x);
int         object_getNumberOfSignalOutlets         (t_object *x);
int         object_getSignalIndexOfInlet            (t_object *x, int m);
int         object_getSignalIndexOfOutlet           (t_object *x, int m);
int         object_isSignalInlet                    (t_object *x, int m);
int         object_isSignalOutlet                   (t_object *x, int m);

void        object_saveWidth                        (t_object *x, t_buffer *b);
void        object_distributeOnInlets               (t_object *x, int argc, t_atom *argv);

t_float     *object_getSignalValueAtIndex           (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Viewed as a box (NOT an IEM and NOT a subpatch GOP). */
/* Note that it can be a comment, a message or an atom. */

int object_isViewAsBox (t_object *x);

/* Everything that is NOT a comment, a message, or an atom. */

static inline int object_isObject (t_object *x)
{
    return (x->te_type == TYPE_OBJECT);
}

static inline int object_isComment (t_object *x)
{
    return (x->te_type == TYPE_COMMENT);
}

static inline int object_isMessage (t_object *x)
{
    return (x->te_type == TYPE_MESSAGE);
}

static inline int object_isAtom (t_object *x)
{
    return (x->te_type == TYPE_ATOM);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_buffer *object_getBuffer (t_object *x)
{
    PD_ASSERT (x->te_buffer != NULL); return x->te_buffer;
}

static inline t_inlet *object_getInlets (t_object *x)
{
    return x->te_inlets;
}

static inline t_outlet *object_getOutlets (t_object *x)
{
    return x->te_outlets;
}

static inline int object_getX (t_object *x)
{
    return x->te_x;
}

static inline int object_getY (t_object *x)
{
    return x->te_y;
}

static inline int object_getWidth (t_object *x)
{
    return x->te_width;
}

static inline t_objecttype object_getType (t_object *x)
{
    return x->te_type;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void object_setBuffer (t_object *x, t_buffer *b)
{
    if (x->te_buffer) { buffer_free (x->te_buffer); } 
    
    PD_ASSERT (b);
    
    x->te_buffer = b;     /* Acquires ownership. */
}

static inline void object_setX (t_object *x, int n)
{
    x->te_x = n;
}

static inline void object_setY (t_object *x, int n)
{
    x->te_y = n;
}

static inline void object_setWidth (t_object *x, int n)
{
    x->te_width = n;
}

static inline void object_setType (t_object *x, t_objecttype n)
{
    x->te_type = n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_object_h_
