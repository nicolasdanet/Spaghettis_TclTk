
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_patch_h_
#define __g_patch_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PD_GUISTUB      ".guistub"
#define PD_GUIPROXY     ".x"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    ACTION_NONE         = 0,
    ACTION_MOVE         = 1,        /* Move object. */
    ACTION_LINE         = 2,        /* Make connection. */
    ACTION_SIGNAL       = 3,        /* Ditto. */
    ACTION_REGION       = 4,        /* Select with lasso. */
    ACTION_PASS         = 5,        /* Pass motion to GUI object. */
    ACTION_DRAG         = 6,        /* Drag text in box. */
    ACTION_RESIZE       = 7,        /* Resize object. */
    ACTION_GRAPH_MOVE   = 8,        /* Move GOP. */
    ACTION_GRAPH_RESIZE = 9         /* Resize GOP. */
    };

enum {
    CURSOR_NOTHING      = 0,        /* Must NOT be changed. */
    CURSOR_CLICK        = 1,        /* Must NOT be changed. */
    CURSOR_OVER         = 2,
    CURSOR_CONNECT      = 3,
    CURSOR_RESIZE_X     = 4,
    CURSOR_RESIZE_Y     = 5,
    CURSOR_ELEMENT_1    = 6,
    CURSOR_ELEMENT_2    = 7
    };

enum {
    MODIFIER_NONE       = 0,
    MODIFIER_SHIFT      = 1,
    MODIFIER_CTRL       = 2,        /* On macOS, the Command key. */
    MODIFIER_ALT        = 4,        /* On macOS, the Option key. */
    MODIFIER_RIGHT      = 8,
    MODIFIER_DOUBLE     = 16,
    MODIFIER_EXTENDED   = 32,       /* On macOS, the Control key. */
    MODIFIER_INSIDE_X   = 64,
    MODIFIER_INSIDE_Y   = 128
    };

enum {
    BOX_DOWN            = 1,
    BOX_DRAG            = 2,
    BOX_DOUBLE          = 3,
    BOX_SHIFT           = 4
    };

enum {
    BOX_CHECK           = 0,
    BOX_CREATE          = 1,
    BOX_UPDATE          = 2
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define BOX_TAG_SIZE    50

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _box {
    struct _box     *box_next;
    t_object        *box_object;
    t_glist         *box_owner;
    char            *box_string;                    /* Unzeroed string UTF-8 formatted. */
    int             box_stringSizeInBytes;
    int             box_selectionStart;
    int             box_selectionEnd;
    int             box_draggedFrom;
    int             box_isActivated;
    int             box_widthInPixels;
    int             box_heightInPixels;
    int             box_checked;
    char            box_tag[BOX_TAG_SIZE];
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _cord {
    int             tr_lineStartX;
    int             tr_lineStartY;
    int             tr_lineEndX;
    int             tr_lineEndY;
    int             tr_lineIsSignal;
    t_outconnect    *tr_lineConnection;
    } t_cord;
    
typedef struct _traverser {
    t_glist         *tr_owner;
    t_outconnect    *tr_connectionCached;
    t_object        *tr_srcObject;
    t_outlet        *tr_srcOutlet;
    t_object        *tr_destObject;
    t_inlet         *tr_destInlet;
    int             tr_srcIndexOfOutlet;
    int             tr_srcNumberOfOutlets;
    int             tr_destIndexOfInlet;
    int             tr_destNumberOfInlets;
    int             tr_srcIndexOfNextOutlet;
    t_cord          tr_cord;
    } t_traverser;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int gobj_isGraphicArray (t_gobj *x)
{
    return (pd_class (x) == garray_class);
}

static inline int gobj_isScalar (t_gobj *x)
{
    return (pd_class (x) == scalar_class);
}

static inline int gobj_isCanvas (t_gobj *x)
{
    return (pd_class (x) == canvas_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     gobj_hasDSP                 (t_gobj *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    gobj_getRectangle           (t_gobj *x, t_glist *owner, t_rectangle *r);
void    gobj_displaced              (t_gobj *x, t_glist *owner, int deltaX, int deltaY);
void    gobj_selected               (t_gobj *x, t_glist *owner, int isSelected);
void    gobj_activated              (t_gobj *x, t_glist *owner, int isActivated);
void    gobj_deleted                (t_gobj *x, t_glist *owner);
void    gobj_visibilityChanged      (t_gobj *x, t_glist *owner, int isVisible);
int     gobj_mouse                  (t_gobj *x, t_glist *owner, t_mouse *m);

t_id    gobj_getUnique              (t_gobj *x);
void    gobj_serializeUnique        (t_gobj *x, t_symbol *s, t_buffer *b);
t_id    gobj_getSource              (t_gobj *x);
void    gobj_serializeSource        (t_gobj *x, t_symbol *s, t_buffer *b);
t_id    gobj_getNative              (t_gobj *x);
void    gobj_changeIdentifiers      (t_gobj *x, t_id u);
int     gobj_identifiersHaveChanged (t_gobj *x);

void    gobj_save                   (t_gobj *x, t_buffer *buffer, int flags);
int     gobj_hit                    (t_gobj *x, t_glist *owner, int a, int b, int n, t_rectangle *r);
                                                            
int     gobj_isViewable             (t_gobj *x, t_glist *owner);
int     gobj_isVisible              (t_gobj *x, t_glist *owner);
void    gobj_help                   (t_gobj *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_box       *box_fetch              (t_glist *glist, t_object *object);
char        *box_getTag             (t_box *x);
t_object    *box_getObject          (t_box *x);

int     box_getWidth                (t_box *x);
int     box_getHeight               (t_box *x);
void    box_getText                 (t_box *x, char **p, int *size);
void    box_getSelection            (t_box *x, char **p, int *size);

void    box_retext                  (t_box *x);
void    box_create                  (t_box *x);
void    box_draw                    (t_box *x);
void    box_update                  (t_box *x);
void    box_erase                   (t_box *x);
void    box_displace                (t_box *x, int deltaX, int deltaY);
void    box_select                  (t_box *x, int isSelected);
void    box_activate                (t_box *x, int isActivated);
void    box_mouse                   (t_box *x, int a, int b, int flag);
void    box_key                     (t_box *x, t_keycode n, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int     cord_hit                    (t_cord *x, int a, int b);
void    cord_init                   (t_cord *x, t_outconnect *connection);
void    cord_make                   (t_cord *x, t_outconnect *connection,
                                        t_object *src,
                                        int i,
                                        t_object *dest,
                                        int j,
                                        t_glist *owner);
                                                
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_outconnect    *traverser_next     (t_traverser *t);

void    traverser_start             (t_traverser *t, t_glist *glist);
t_error traverser_disconnect        (t_traverser *t, t_glist *glist);
int     traverser_isLineBetween     (t_traverser *t, t_object *src, int m, t_object *dest, int n);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int cord_getStartX (t_cord *c)
{
    return c->tr_lineStartX;
}

static inline int cord_getStartY (t_cord *c)
{
    return c->tr_lineStartY;
}

static inline int cord_getEndX (t_cord *c)
{
    return c->tr_lineEndX;
}

static inline int cord_getEndY (t_cord *c)
{
    return c->tr_lineEndY;
}

static inline t_outconnect *cord_getConnection (t_cord *c)
{
    return c->tr_lineConnection;
}

static inline int cord_isSignal (t_cord *c)
{
    return c->tr_lineIsSignal;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_cord *traverser_getCord (t_traverser *t)
{
    return &t->tr_cord;
}

static inline t_object *traverser_getSource (t_traverser *t)
{
    return t->tr_srcObject;
}

static inline t_object *traverser_getDestination (t_traverser *t)
{
    return t->tr_destObject;
}

static inline t_outlet *traverser_getOutlet (t_traverser *t)
{
    return t->tr_srcOutlet;
}

/* Can be NULL (first inlet). */

static inline t_inlet *traverser_getInlet (t_traverser *t)
{
    return t->tr_destInlet;
}

static inline int traverser_getIndexOfOutlet (t_traverser *t)
{
    return t->tr_srcIndexOfOutlet;
}

static inline int traverser_getIndexOfInlet (t_traverser *t)
{
    return t->tr_destIndexOfInlet;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_patch_h_
