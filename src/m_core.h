
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_core_h_
#define __m_core_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_savefn)                (t_gobj *x, t_buffer *b);
typedef void (*t_propertiesfn)          (t_gobj *x, t_glist *glist);
typedef void (*t_drawfn)                (t_gobj *x, t_glist *glist);
typedef void (*t_motionfn)              (void *z, t_float deltaX, t_float deltaY, t_float modifier);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_getrectanglefn)        (t_gobj *x, t_glist *glist, int *a, int *b, int *c, int *d);
typedef void (*t_displacedfn)           (t_gobj *x, t_glist *glist, int deltaX, int deltaY);
typedef void (*t_selectedfn)            (t_gobj *x, t_glist *glist, int isSelected);
typedef void (*t_activatedfn)           (t_gobj *x, t_glist *glist, int isActivated);
typedef void (*t_deletedfn)             (t_gobj *x, t_glist *glist);
typedef void (*t_visibilityfn)          (t_gobj *x, t_glist *glist, int isVisible);
typedef int  (*t_clickedfn)             (t_gobj *x,
                                            t_glist *glist,
                                            int a,
                                            int b,
                                            int shift,
                                            int ctrl,
                                            int alt,
                                            int dbl,
                                            int clicked);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_parentgetrectanglefn)  (t_gobj *x, t_gpointer *gp, t_float baseX, t_float baseY,
                                            int *a,
                                            int *b,
                                            int *c,
                                            int *d);
typedef void (*t_parentvisibilityfn)    (t_gobj *x, t_gpointer *gp, t_float baseX, t_float baseY, int flag);
typedef int  (*t_parentclickedfn)       (t_gobj *x, t_gpointer *gp,
                                            t_float baseX,
                                            t_float baseY,
                                            int a,
                                            int b,
                                            int shift,
                                            int alt,
                                            int dbl,
                                            int clicked);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _widgetbehavior {
    t_getrectanglefn                w_fnGetRectangle;
    t_displacedfn                   w_fnDisplaced;
    t_selectedfn                    w_fnSelected;
    t_activatedfn                   w_fnActivated;
    t_deletedfn                     w_fnDeleted;
    t_visibilityfn                  w_fnVisibilityChanged;
    t_clickedfn                     w_fnClicked;
    };
    
struct _parentwidgetbehavior {
    t_parentgetrectanglefn          w_fnParentGetRectangle;
    t_parentvisibilityfn            w_fnParentVisibilityChanged;
    t_parentclickedfn               w_fnParentClicked;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_bangmethod)        (t_pd *x);
typedef void (*t_floatmethod)       (t_pd *x, t_float f);
typedef void (*t_symbolmethod)      (t_pd *x, t_symbol *s);
typedef void (*t_listmethod)        (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anythingmethod)    (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_pointermethod)     (t_pd *x, t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _entry {
    t_symbol                *me_name;
    t_method                me_method;
    t_atomtype              me_arguments[PD_ARGUMENTS + 1];
    } t_entry;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _class {
    t_symbol                *c_name;
    t_symbol                *c_helpName;
    t_symbol                *c_externalDirectory;
    t_entry                 *c_methods;
    int                     c_methodsSize;
    t_method                c_methodFree;
    t_bangmethod            c_methodBang;
    t_floatmethod           c_methodFloat;
    t_symbolmethod          c_methodSymbol;
    t_listmethod            c_methodList;
    t_anythingmethod        c_methodAnything;
    t_pointermethod         c_methodPointer;
    t_widgetbehavior        *c_behavior;
    t_parentwidgetbehavior  *c_behaviorParent;
    t_savefn                c_fnSave;
    t_propertiesfn          c_fnProperties;
    int                     c_signalOffset;
    char                    c_isBox;
    char                    c_hasFirstInlet;
    int                     c_type;
    size_t                  c_size;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    double                  pd_systime;
    int                     pd_dspState;
    int                     pd_dspChainSize;
    t_int                   *pd_dspChain;
    t_clock                 *pd_clocks;
    t_signal                *pd_signals;
    t_glist                 *pd_roots;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _heapstring {
    size_t                  hs_used;
    size_t                  hs_size;
    char                    *hs_raw;
    } t_heapstring;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _bindelement {
    t_pd                    *e_what;                    /* MUST be the first. */
    struct _bindelement     *e_next;
    } t_bindelement;

typedef struct _bindlist {
    t_pd                    b_pd;                       /* MUST be the first. */
    t_bindelement           *b_list;
    } t_bindlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        pd_empty                                    (t_pd *x);
void        pd_vMessage                                 (t_pd *x, t_symbol *s, char *fmt, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        stack_push                                  (t_pd *x);
void        stack_pop                                   (t_pd *x);
void        stack_performLoadbang                       (void);
int         stack_setLoadingAbstraction                 (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     string_copy                                 (char *dest, size_t size, const char *src);
t_error     string_add                                  (char *dest, size_t size, const char *src);
t_error     string_append                               (char *dest, size_t size, const char *src, int n);
t_error     string_sprintf                              (char *dest, size_t size, const char *format, ...);
t_error     string_addSprintf                           (char *dest, size_t size, const char *format, ...);
t_error     string_addAtom                              (char *dest, size_t size, t_atom *a);

int         string_containsAtStart                      (const char *s, const char *isContained);

int         string_indexOfFirstOccurrenceUntil          (char *s, const char *c, size_t n);
int         string_indexOfFirstOccurrenceFrom           (char *s, const char *c, size_t n);

void        string_getNumberOfColumnsAndLines           (char *s, int *numberOfColumns, int *numberOfLines);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_heapstring    *heapstring_new                         (int size);
char            *heapstring_getRaw                      (t_heapstring *x);

void        heapstring_free                             (t_heapstring *x);
t_error     heapstring_add                              (t_heapstring *x, const char *src);
t_error     heapstring_append                           (t_heapstring *x, const char *src, int n);
t_error     heapstring_addSprintf                       (t_heapstring *x, const char *format, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *utils_getFirstAtomOfObjectAsSymbol         (t_object *x);
t_symbol    *utils_getFirstAtomOfBufferAsSymbol         (t_buffer *x);

t_symbol    *utils_decode                               (t_symbol *s);
t_symbol    *utils_dash                                 (void);
t_symbol    *utils_empty                                (void); 
t_symbol    *utils_substituteIfEmpty                    (t_symbol *s, int asDash);

t_symbol    *utils_makeBindSymbol                       (t_symbol *s);
t_symbol    *utils_stripBindSymbol                      (t_symbol *s);

int         utils_isTokenEnd                            (char c);
int         utils_isTokenEscape                         (char c);
int         utils_isTokenWhitespace                     (char c);
int         utils_isAlphanumericOrUnderscore            (char c);
t_unique    utils_unique                                (void);
t_error     utils_version                               (char *dest, size_t size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float     math_euclideanDistance                      (t_float x1, t_float y1, t_float x2, t_float y2);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *dollar_toHash                              (t_symbol *s);
t_symbol    *dollar_fromHash                            (t_symbol *s);
t_symbol    *dollar_expandDollarSymbol                  (t_symbol *s, int argc, t_atom *argv);

void        dollar_expandDollarNumber                   (t_atom *dollar, t_atom *a, int argc, t_atom *argv);
int         dollar_isDollarNumber                       (char *s);
int         dollar_isPointingToDollarAndNumber          (char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        class_setDefaultExternalDirectory           (t_symbol *s);
t_method    class_getMethod                             (t_class *c, t_symbol *s);
int         class_hasMethod                             (t_class *c, t_symbol *s);
int         class_hasBang                               (t_class *c);
int         class_hasDrawCommand                        (t_class *c);
int         class_hasPropertiesFunction                 (t_class *c); 
void        class_setWidgetBehavior                     (t_class *c, t_widgetbehavior *w);
void        class_setParentWidgetBehavior               (t_class *c, t_parentwidgetbehavior *w);
void        class_setHelpName                           (t_class *c, t_symbol *s);
void        class_setPropertiesFunction                 (t_class *c, t_propertiesfn f);
void        class_setSaveFunction                       (t_class *c, t_savefn f);

char        *class_getName                              (t_class *c);
char        *class_getHelpName                          (t_class *c);
char        *class_getExternalDirectory                 (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_parentwidgetbehavior  *class_getParentWidget          (t_class *c);
t_propertiesfn          class_getPropertiesFunction     (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        object_list                                 (t_object *x, t_symbol *s, int argc, t_atom *argv);
void        object_saveWidth                            (t_object *x, t_buffer *b);
int         object_numberOfInlets                       (t_object *x);
int         object_numberOfOutlets                      (t_object *x);
int         object_numberOfSignalInlets                 (t_object *x);
int         object_numberOfSignalOutlets                (t_object *x);
int         object_indexOfSignalInlet                   (t_object *x, int m);
int         object_indexOfSignalOutlet                  (t_object *x, int m);
int         object_isSignalInlet                        (t_object *x, int m);
int         object_isSignalOutlet                       (t_object *x, int m);
int         object_getIndexOfSignalInlet                (t_inlet *x);
int         object_getIndexOfSignalOutlet               (t_outlet *x);
void        object_moveInletFirst                       (t_object *x, t_inlet *i);
void        object_moveOutletFirst                      (t_object *x, t_outlet *i);
void        object_disconnect                           (t_object *src, int m, t_object *dest, int n);

t_outconnect    *object_connect                         (t_object *src, int m, t_object *dest, int n);
t_outconnect    *object_traverseOutletStart             (t_object *x, t_outlet **ptr, int n);
t_outconnect    *object_traverseOutletNext              (t_outconnect *last,
                                                            t_object **dest,
                                                            t_inlet **ptr,
                                                            int *n);

t_float     *object_getSignalValueAtIndex               (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         outlet_isSignal                             (t_outlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        global_shouldQuit                           (void *dummy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     atom_withStringUnzeroed                     (t_atom *a, char *s, int size);
t_error     atom_toString                               (t_atom *a, char *s, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_iterator  *iterator_new                               (int argc, t_atom *argv);

void        iterator_free                               (t_iterator *x);
int         iterator_next                               (t_iterator *x, t_atom **a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        buffer_resize                               (t_buffer *x, int n);
void        buffer_vAppend                              (t_buffer *x, char *fmt, ...);
void        buffer_appendAtom                           (t_buffer *x, t_atom *a);
void        buffer_appendBuffer                         (t_buffer *x, t_buffer *y);
void        buffer_appendSemicolon                      (t_buffer *x);
void        buffer_parseStringUnzeroed                  (t_buffer *x, char *s, int size, int preallocated);
void        buffer_toString                             (t_buffer *x, char **s);
void        buffer_toStringUnzeroed                     (t_buffer *x, char **s, int *size);
void        buffer_withStringUnzeroed                   (t_buffer *x, char *s, int size);
void        buffer_serialize                            (t_buffer *x, t_buffer *y);
void        buffer_deserialize                          (t_buffer *x, int argc, t_atom *argv);
void        buffer_eval                                 (t_buffer *x, t_pd *object, int argc, t_atom *argv);
t_error     buffer_read                                 (t_buffer *x, char *name, t_glist *glist);
t_error     buffer_write                                (t_buffer *x, char *name, char *directory);
t_error     buffer_fileEval                             (t_symbol *name, t_symbol *directory);
void        buffer_fileOpen                             (void *dummy, t_symbol *name, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        instance_initialize                         (void);
void        instance_release                            (void);
void        instance_addToRoots                         (t_glist *glist);
void        instance_removeFromRoots                    (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        setup_initialize                            (void);
void        setup_release                               (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        message_initialize                          (void);
void        message_release                             (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        sys_gui                                     (char *s);
void        sys_vGui                                    (char *format, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error     guistub_new                                 (t_pd *owner, void *key, const char *cmd);
void        guistub_destroyWithKey                      (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_color.h"
#include "m_symbols.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_core_h_
