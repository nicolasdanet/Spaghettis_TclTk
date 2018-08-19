
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_undo_h_
#define __m_undo_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef enum {
    UNDO_SEPARATOR      = 0,
    UNDO_ADD            = 1,
    UNDO_REMOVE         = 2,
    UNDO_CUT,
    UNDO_PASTE,
    UNDO_DUPLICATE,
    UNDO_SNAP,
    UNDO_ENCAPSULATE,
    UNDO_DEENCAPSULATE,
    UNDO_CONNECT,
    UNDO_DISCONNECT,
    UNDO_CREATE,
    UNDO_DELETE,
    UNDO_MOTION,
    UNDO_PROPERTIES,
    UNDO_RESIZE_BOX,
    UNDO_RESIZE_GRAPH,
    UNDO_FRONT,
    UNDO_BACK,
    UNDO_TYPING
    } t_undotype;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _undoaction {
    t_pd                ua_pd;              /* Must be the first. */
    t_id                ua_id;
    t_undotype          ua_type;
    t_symbol            *ua_label;
    struct _undoaction  *ua_next;
    struct _undoaction  *ua_previous;
    } t_undoaction;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _undomanager {
    int                 um_recursive;
    int                 um_count;
    t_clock             *um_clock;
    t_undoaction        *um_head;
    t_undoaction        *um_tail;
    } t_undomanager;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void undoaction_releaseAllFrom  (t_undoaction *a, t_undomanager *x);
void undoaction_release         (t_undoaction *a, t_undomanager *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_id undoaction_getUnique (t_undoaction *a)
{
    return a->ua_id;
}

static inline t_undotype undoaction_getType (t_undoaction *a)
{
    return a->ua_type;
}

static inline t_symbol *undoaction_getLabel (t_undoaction *a)
{
    return a->ua_label;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undomanager   *undomanager_new            (void);

void    undomanager_free                    (t_undomanager *x);

int     undomanager_hasSeparatorAtLast      (t_undomanager *x);
void    undomanager_appendSeparatorLater    (t_undomanager *x);         /* Filter consecutive separators. */
void    undomanager_appendSeparator         (t_undomanager *x);         /* Ditto. */

void    undomanager_append                  (t_undomanager *x, t_undoaction *a);
void    undomanager_undo                    (t_undomanager *x);
void    undomanager_redo                    (t_undomanager *x);

t_symbol    *undomanager_getUndoLabel       (t_undomanager *x);
t_symbol    *undomanager_getRedoLabel       (t_undomanager *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int undomanager_isNotRecursive (t_undomanager *x)
{
    return (x->um_recursive == 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _undosnippet {
    t_id        us_object;
    t_id        us_owner;
    int         us_z;
    t_buffer    *us_buffer;
    } t_undosnippet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undosnippet   *undosnippet_newCopy        (t_gobj *gobj, t_glist *owner);
t_undosnippet   *undosnippet_newSave        (t_gobj *gobj, t_glist *owner);
t_undosnippet   *undosnippet_newProperties  (t_gobj *gobj, t_glist *owner);
t_undosnippet   *undosnippet_new            (t_gobj *gobj, t_glist *owner);

void            undosnippet_free            (t_undosnippet *x);
void            undosnippet_load            (t_undosnippet *x);
void            undosnippet_paste           (t_undosnippet *x);
void            undosnippet_message         (t_undosnippet *x);

void            undosnippet_z               (t_undosnippet *x);
void            undosnippet_front           (t_undosnippet *x);
void            undosnippet_back            (t_undosnippet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_undoaction    *undoseparator_new          (void);
t_undoaction    *undoadd_new                (void);
t_undoaction    *undoremove_new             (void);
t_undoaction    *undocut_new                (void);
t_undoaction    *undopaste_new              (void);
t_undoaction    *undoduplicate_new          (void);
t_undoaction    *undosnap_new               (void);
t_undoaction    *undoencapsulate_new        (void);
t_undoaction    *undodeencapsulate_new      (void);
t_undoaction    *undoconnect_new            (t_object *src, int m, t_object *dest, int n);
t_undoaction    *undodisconnect_new         (t_object *src, int m, t_object *dest, int n);
t_undoaction    *undocreate_new             (t_gobj *o, t_undosnippet *snippet);
t_undoaction    *undodelete_new             (t_gobj *o, t_undosnippet *snippet);
t_undoaction    *undomotion_new             (t_gobj *o, int deltaX, int deltaY);
t_undoaction    *undoproperties_new         (t_gobj *o, t_undosnippet *s1, t_undosnippet *s2);
t_undoaction    *undoresizebox_new          (t_gobj *o, int m, int n);
t_undoaction    *undoresizegraph_new        (t_gobj *o, int deltaX, int deltaY);
t_undoaction    *undofront_new              (t_gobj *o, t_undosnippet *snippet);
t_undoaction    *undoback_new               (t_gobj *o, t_undosnippet *snippet);
t_undoaction    *undotyping_new             (t_gobj *o, t_undosnippet *s1, t_undosnippet *s2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_undo_h_
