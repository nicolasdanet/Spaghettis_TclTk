
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
    TYPE_COMMENT            = 0,
    TYPE_OBJECT             = 1,
    TYPE_MESSAGE            = 2,
    TYPE_ATOM               = 3
    };

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
    BOXTEXT_DOWN            = 1,
    BOXTEXT_DRAG            = 2,
    BOXTEXT_DOUBLE          = 3,
    BOXTEXT_SHIFT           = 4
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _cord {
    int                 tr_lineStartX;
    int                 tr_lineStartY;
    int                 tr_lineEndX;
    int                 tr_lineEndY;
    } t_cord;
    
typedef struct _linetraverser {
    t_glist             *tr_owner;
    t_outconnect        *tr_connectionCached;
    t_object            *tr_srcObject;
    t_outlet            *tr_srcOutlet;
    t_object            *tr_destObject;
    t_inlet             *tr_destInlet;
    int                 tr_srcIndexOfOutlet;
    int                 tr_srcIndexOfNextOutlet;
    int                 tr_srcNumberOfOutlets;
    int                 tr_destIndexOfInlet;
    int                 tr_destNumberOfInlets;
    t_rectangle         tr_srcBox;
    t_rectangle         tr_destBox;
    t_cord              tr_cord;
    } t_linetraverser;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_guiconnect    *guiconnect_new                     (t_pd *owner);
char            *guiconnect_getBoundAsString        (t_guiconnect *x);

void            guiconnect_release                  (t_guiconnect *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol        *environment_getActiveFilename      (void);
t_environment   *environment_fetchActiveIfAny       (void);

void            environment_free                    (t_environment *environment);
void            environment_setActiveFile           (t_symbol *name, t_symbol *directory);
void            environment_setActiveArguments      (int argc, t_atom *argv);
void            environment_resetActiveArguments    (void);

int             environment_getDollarZero           (t_environment *environment);
int             environment_getNumberOfArguments    (t_environment *environment);

t_atom          *environment_getArguments           (t_environment *environment);
t_symbol        *environment_getDirectory           (t_environment *environment);
t_symbol        *environment_getFileName            (t_environment *environment);
char            *environment_getDirectoryAsString   (t_environment *environment);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            gobj_getRectangle                   (t_gobj *x, t_glist *owner, t_rectangle *r);
void            gobj_displaced                      (t_gobj *x, t_glist *owner, int deltaX, int deltaY);
void            gobj_selected                       (t_gobj *x, t_glist *owner, int isSelected);
void            gobj_activated                      (t_gobj *x, t_glist *owner, int isActivated);
void            gobj_deleted                        (t_gobj *x, t_glist *owner);
void            gobj_visibilityChanged              (t_gobj *x, t_glist *owner, int isVisible);
int             gobj_mouse                          (t_gobj *x, t_glist *owner, t_mouse *m);

void            gobj_save                           (t_gobj *x, t_buffer *buffer);
int             gobj_hit                            (t_gobj *x, t_glist *owner, int a, int b, t_rectangle *r);
                                                            
int             gobj_isVisible                      (t_gobj *x, t_glist *owner);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int             object_isBox                        (t_object *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_boxtext       *boxtext_new                        (t_glist *glist, t_object *object);
t_boxtext       *boxtext_fetch                      (t_glist *glist, t_object *object);
char            *boxtext_getTag                     (t_boxtext *x);

void            boxtext_retext                      (t_glist *glist, t_object *object);

void            boxtext_free                        (t_boxtext *x);
int             boxtext_getWidth                    (t_boxtext *x);
int             boxtext_getHeight                   (t_boxtext *x);
void            boxtext_getText                     (t_boxtext *x, char **p, int *size);
void            boxtext_getSelection                (t_boxtext *x, char **p, int *size);
void            boxtext_draw                        (t_boxtext *x);
void            boxtext_update                      (t_boxtext *x);
void            boxtext_erase                       (t_boxtext *x);
void            boxtext_displace                    (t_boxtext *x, int deltaX, int deltaY);
void            boxtext_select                      (t_boxtext *x, int isSelected);
void            boxtext_activate                    (t_boxtext *x, int state);
void            boxtext_mouse                       (t_boxtext *x, int a, int b, int flag);
void            boxtext_key                         (t_boxtext *x, t_keycode n, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            text_behaviorGetRectangle           (t_gobj *x, t_glist *glist, t_rectangle *r);
void            text_behaviorDisplaced              (t_gobj *x, t_glist *glist, int deltaX, int deltaY);
void            text_behaviorSelected               (t_gobj *x, t_glist *glist, int isSelected);
void            text_behaviorActivated              (t_gobj *x, t_glist *glist, int isActivated);
void            text_behaviorDeleted                (t_gobj *x, t_glist *glist);
void            text_behaviorVisibilityChanged      (t_gobj *x, t_glist *glist, int isVisible);
int             text_behaviorMouse                  (t_gobj *x, t_glist *glist, t_mouse *m);

void            text_functionSave                   (t_gobj *x, t_buffer *b);

void            text_set                            (t_object *x, t_glist *glist, char *s, int size);
int             text_getPixelX                      (t_object *x, t_glist *glist);
int             text_getPixelY                      (t_object *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            message_makeObject                  (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void            message_click                       (t_message *x, 
                                                        t_float a,
                                                        t_float b,
                                                        t_float shift,
                                                        t_float ctrl,
                                                        t_float alt);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            gatom_makeObject                    (t_glist *glist, 
                                                        t_atomtype type,
                                                        t_symbol *s,
                                                        int argc,
                                                        t_atom *argv);

void            gatom_click                         (t_gatom *x,
                                                        t_float a,
                                                        t_float b,
                                                        t_float shift,
                                                        t_float ctrl,
                                                        t_float alt);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_garray        *garray_makeObject                  (t_glist *glist,
                                                        t_symbol *name,
                                                        t_float size,
                                                        t_float flags);
                                                            
t_array         *garray_getArray                    (t_garray *x);
t_glist         *garray_getView                     (t_garray *x);
t_scalar        *garray_getScalar                   (t_garray *x);
t_symbol        *garray_getName                     (t_garray *x);

int             garray_isSingle                     (t_glist *glist);

int             garray_getSize                      (t_garray *x);         
int             garray_getData                      (t_garray *x, int *size, t_word **w);
void            garray_setDataAtIndex               (t_garray *x, int i, t_float f);
t_float         garray_getDataAtIndex               (t_garray *x, int i);
void            garray_setDataFromIndex             (t_garray *x, int i, t_float f);
t_float         garray_getAmplitude                 (t_garray *x);
void            garray_setAsUsedInDSP               (t_garray *x);
void            garray_setSaveWithParent            (t_garray *x, int savedWithParent);
void            garray_redraw                       (t_garray *x);
void            garray_resizeWithInteger            (t_garray *x, int n);
void            garray_saveContentsToBuffer         (t_garray *x, t_buffer *b);
void            garray_functionProperties           (t_garray *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet         *vinlet_getInlet                    (t_pd *x);
t_outlet        *voutlet_getOutlet                  (t_pd *x);

int             vinlet_isSignal                     (t_vinlet *x);
int             voutlet_isSignal                    (t_voutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            cord_init                           (t_cord *c);
int             cord_hit                            (t_cord *c, int positionX, int positionY);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect    *linetraverser_next                 (t_linetraverser *t);

void            linetraverser_start                 (t_linetraverser *t, t_glist *glist);
void            linetraverser_disconnect            (t_linetraverser *t);
int             linetraverser_isLineBetween         (t_linetraverser *t, 
                                                        t_object *src,
                                                        int m,
                                                        t_object *dest,
                                                        int n);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_cord *linetraverser_getCord (t_linetraverser *t)
{
    return &t->tr_cord;
}

static inline int linetraverser_getStartX (t_linetraverser *t)
{
    return t->tr_cord.tr_lineStartX;
}

static inline int linetraverser_getStartY (t_linetraverser *t)
{
    return t->tr_cord.tr_lineStartY;
}

static inline int linetraverser_getEndX (t_linetraverser *t)
{
    return t->tr_cord.tr_lineEndX;
}

static inline int linetraverser_getEndY (t_linetraverser *t)
{
    return t->tr_cord.tr_lineEndY;
}

static inline t_object *linetraverser_getSource (t_linetraverser *t)
{
    return t->tr_srcObject;
}

static inline t_object *linetraverser_getDestination (t_linetraverser *t)
{
    return t->tr_destObject;
}

static inline t_outlet *linetraverser_getOutlet (t_linetraverser *t)
{
    return t->tr_srcOutlet;
}

static inline t_inlet *linetraverser_getInlet (t_linetraverser *t)
{
    return t->tr_destInlet;
}

static inline int linetraverser_getIndexOfOutlet (t_linetraverser *t)
{
    return t->tr_srcIndexOfOutlet;
}

static inline int linetraverser_getIndexOfInlet (t_linetraverser *t)
{
    return t->tr_destIndexOfInlet;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_patch_h_
