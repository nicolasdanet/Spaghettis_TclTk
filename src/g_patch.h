
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_patch_h_
#define __g_patch_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PD_GUISTUB          ".guistub"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

enum {
    ACTION_NONE             = 0,
    ACTION_MOVE             = 1,
    ACTION_CONNECT          = 2,
    ACTION_REGION           = 3,
    ACTION_PASS             = 4,
    ACTION_DRAG             = 5,
    ACTION_RESIZE           = 6
    };

enum {
    CURSOR_NOTHING          = 0,            /* Must NOT be changed. */
    CURSOR_CLICK            = 1,            /* Must NOT be changed. */
    CURSOR_THICKEN          = 2,
    CURSOR_ADD              = 3,
    CURSOR_CONNECT          = 4,
    CURSOR_RESIZE           = 5
    };

enum {
    MODIFIER_NONE           = 0,
    MODIFIER_SHIFT          = 1,
    MODIFIER_CTRL           = 2,            /* Command key on Mac OS X. */
    MODIFIER_ALT            = 4,
    MODIFIER_RIGHT          = 8,
    MODIFIER_DOUBLE         = 16
    };

enum {
    BOX_DOWN                = 1,
    BOX_DRAG                = 2,
    BOX_DOUBLE              = 3,
    BOX_SHIFT               = 4
    };

enum {
    BOX_CHECK               = 0,
    BOX_CREATE              = 1,
    BOX_UPDATE              = 2
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define INLET_WIDTH         7
#define INLET_HEIGHT        2
#define WINDOW_HEADER       22
#define WINDOW_WIDTH        450
#define WINDOW_HEIGHT       300

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define INLET_WIDTH         7
#define INLET_HEIGHT        1
#define WINDOW_HEADER       50
#define WINDOW_WIDTH        450
#define WINDOW_HEIGHT       300

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_APPLE

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int inlet_offset (int width, int i, int n)
{
    return (((width - INLET_WIDTH) * i) / ((n == 1) ? 1 : (n - 1)));
}

static inline int inlet_middle (int width, int i, int n)
{
    return (inlet_offset (width, i, n) + ((INLET_WIDTH - 1) / 2));
}

static inline int inlet_nearby (int x, int a, int b, int n)
{
    return (((x - a) * (n - 1) + ((b - a) / 2)) / (b - a));
}

static inline int inlet_closest (int x, int n, t_rectangle *r)
{
    return inlet_nearby (x, rectangle_getTopLeftX (r), rectangle_getBottomRightX (r), n);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define BOX_TAG_SIZE    50

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

struct _box {
    struct _box         *box_next;
    t_object            *box_object;
    t_glist             *box_owner;
    char                *box_string;                        /* Unzeroed string UTF-8 formatted. */
    int                 box_stringSizeInBytes;
    int                 box_selectionStart; 
    int                 box_selectionEnd;
    int                 box_draggedFrom;
    int                 box_isActivated;
    int                 box_widthInPixels;
    int                 box_heightInPixels;
    int                 box_checked;
    char                box_tag[BOX_TAG_SIZE];
    };
    
typedef struct _cord {
    int                 tr_lineStartX;
    int                 tr_lineStartY;
    int                 tr_lineEndX;
    int                 tr_lineEndY;
    int                 tr_lineIsSignal;
    t_outconnect        *tr_lineConnection;
    } t_cord;
    
typedef struct _traverser {
    t_glist             *tr_owner;
    t_outconnect        *tr_connectionCached;
    t_object            *tr_srcObject;
    t_outlet            *tr_srcOutlet;
    t_object            *tr_destObject;
    t_inlet             *tr_destInlet;
    int                 tr_srcIndexOfOutlet;
    int                 tr_srcNumberOfOutlets;
    int                 tr_destIndexOfInlet;
    int                 tr_destNumberOfInlets;
    int                 tr_srcIndexOfNextOutlet;
    t_cord              tr_cord;
    } t_traverser;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        gobj_getRectangle               (t_gobj *x, t_glist *owner, t_rectangle *r);
void        gobj_displaced                  (t_gobj *x, t_glist *owner, int deltaX, int deltaY);
void        gobj_selected                   (t_gobj *x, t_glist *owner, int isSelected);
void        gobj_activated                  (t_gobj *x, t_glist *owner, int isActivated);
void        gobj_deleted                    (t_gobj *x, t_glist *owner);
void        gobj_visibilityChanged          (t_gobj *x, t_glist *owner, int isVisible);
int         gobj_mouse                      (t_gobj *x, t_glist *owner, t_mouse *m);

void        gobj_save                       (t_gobj *x, t_buffer *buffer);
int         gobj_hit                        (t_gobj *x, t_glist *owner, int a, int b, t_rectangle *r);
                                                            
int         gobj_isVisible                  (t_gobj *x, t_glist *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_box       *box_fetch                      (t_glist *glist, t_object *object);
char        *box_getTag                     (t_box *x);

int         box_getWidth                    (t_box *x);
int         box_getHeight                   (t_box *x);
void        box_getText                     (t_box *x, char **p, int *size);
void        box_getSelection                (t_box *x, char **p, int *size);

void        box_retext                      (t_box *x);
void        box_create                      (t_box *x);
void        box_draw                        (t_box *x);
void        box_update                      (t_box *x);
void        box_erase                       (t_box *x);
void        box_displace                    (t_box *x, int deltaX, int deltaY);
void        box_select                      (t_box *x, int isSelected);
void        box_activate                    (t_box *x, int isActivated);
void        box_mouse                       (t_box *x, int a, int b, int flag);
void        box_key                         (t_box *x, t_keycode n, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        message_makeObject              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void        message_click                   (t_message *x, t_symbol *s, int argc, t_atom *argv);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        gatom_makeObjectFloat           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void        gatom_makeObjectSymbol          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void        gatom_click                     (t_gatom *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_garray    *garray_makeObject              (t_glist *glist, t_symbol *s, t_float n, t_float flags);
t_array     *garray_getArray                (t_garray *x);
t_glist     *garray_getView                 (t_garray *x);
t_scalar    *garray_getScalar               (t_garray *x);
t_symbol    *garray_getName                 (t_garray *x);

int         garray_isSingle                 (t_glist *glist);

int         garray_getSize                  (t_garray *x);         
int         garray_getData                  (t_garray *x, int *size, t_word **w);
void        garray_setDataAtIndex           (t_garray *x, int i, t_float f);
t_float     garray_getDataAtIndex           (t_garray *x, int i);
void        garray_setDataFromIndex         (t_garray *x, int i, t_float f);
t_float     garray_getAmplitude             (t_garray *x);
void        garray_setAsUsedInDSP           (t_garray *x);
void        garray_setSaveWithParent        (t_garray *x, int savedWithParent);
void        garray_redraw                   (t_garray *x);
void        garray_resizeWithInteger        (t_garray *x, int n);
void        garray_saveContentsToBuffer     (t_garray *x, t_buffer *b);
void        garray_functionProperties       (t_garray *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet     *vinlet_getInlet                (t_pd *x);
t_outlet    *voutlet_getOutlet              (t_pd *x);

int         vinlet_isSignal                 (t_vinlet *x);
int         voutlet_isSignal                (t_voutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         cord_hit                        (t_cord *x, int a, int b);
void        cord_init                       (t_cord *x, t_outconnect *connection);
void        cord_make                       (t_cord *x, t_outconnect *connection, 
                                                t_object *src,
                                                int i,
                                                t_object *dest, 
                                                int j, 
                                                t_glist *owner);
                                                
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect    *traverser_next             (t_traverser *t);

void        traverser_start                 (t_traverser *t, t_glist *glist);
void        traverser_disconnect            (t_traverser *t);
int         traverser_isItLineBetween       (t_traverser *t, t_object *src, int m, t_object *dest, int n);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
#pragma mark -

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
