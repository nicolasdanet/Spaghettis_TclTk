
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_core_h_
#define __m_core_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_STRUCT _widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_savefn)(t_gobj *x, t_buffer *b);
typedef void (*t_propertiesfn)(t_gobj *x, t_glist *glist);
typedef void (*t_guifn)(t_gobj *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _entry {
    t_symbol    *me_name;
    t_method    me_method;
    t_atomtype  me_arguments[PD_ARGUMENTS + 1];
    } t_entry;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_bangmethod)(t_pd *x);
typedef void (*t_pointermethod)(t_pd *x, t_gpointer *gp);
typedef void (*t_floatmethod)(t_pd *x, t_float f);
typedef void (*t_symbolmethod)(t_pd *x, t_symbol *s);
typedef void (*t_listmethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anymethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

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
    t_pointermethod         c_methodPointer;
    t_floatmethod           c_methodFloat;
    t_symbolmethod          c_methodSymbol;
    t_listmethod            c_methodList;
    t_anymethod             c_methodAny;
    t_widgetbehavior        *c_behavior;
    t_parentwidgetbehavior  *c_behaviorParent;
    t_savefn                c_fnSave;
    t_propertiesfn          c_fnProperties;
    int                     c_signalOffset;
    char                    c_isGraphic;
    char                    c_isBox;
    char                    c_hasFirstInlet;
    char                    c_hasDrawCommand;
    size_t                  c_size;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    double      pd_systime;
    int         pd_state;
    int         pd_chainSize;
    t_int       *pd_chain;
    t_clock     *pd_clocks;
    t_signal    *pd_signals;
    t_canvas    *pd_canvases;
    //
    t_symbol    *sym_midiin;
    t_symbol    *sym_sysexin;
    t_symbol    *sym_notein;
    t_symbol    *sym_ctlin;
    t_symbol    *sym_pgmin;
    t_symbol    *sym_bendin;
    t_symbol    *sym_touchin;
    t_symbol    *sym_polytouchin;
    t_symbol    *sym_midiclkin;
    t_symbol    *sym_midirealtimein;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* A dynamic array of atoms. */

struct _buffer {
    int     b_size;
    t_atom  *b_vector;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_push                            (t_pd *x);
void pd_pop                             (t_pd *x);
void pd_empty                           (t_pd *x);
void pd_vMessage                        (t_pd *x, t_symbol *s, char *fmt, ...);

void pd_performLoadbang                 (void);
int  pd_setLoadingAbstraction           (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void setup_initialize                   (void);
void object_initialize                  (void);
void global_initialize                  (void);
void message_initialize                 (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error utils_strncpy                   (char *dest, size_t size, const char *src);
t_error utils_strnadd                   (char *dest, size_t size, const char *src);
t_error utils_strncat                   (char *dest, size_t size, const char *src, int length);
t_error utils_snprintf                  (char *dest, size_t size, const char *format, ...);

int     utils_isTokenEnd                (char c);
int     utils_isTokenEscape             (char c);
int     utils_isTokenWhitespace         (char c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error path_withNameAndDirectory       (char *dest, size_t size, const char *name, const char *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol    *dollar_substituteDollarSymbol              (t_symbol *s, int argc, t_atom *argv);

void        dollar_substituteDollarNumber               (t_atom *dollar, t_atom *a, int argc, t_atom *argv);
int         dollar_isDollarNumber                       (char *s);
int         dollar_isPointingToDollarAndNumber          (char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        class_setExternalDirectory                  (t_symbol *s);
t_method    class_getMethod                             (t_class *c, t_symbol *s);
int         class_hasMethod                             (t_class *c, t_symbol *s);
int         class_hasBang                               (t_class *c);
int         class_hasDrawCommand                        (t_class *c);
int         class_hasPropertiesFunction                 (t_class *c); 
void        class_setWidget                             (t_class *c, t_widgetbehavior *w);
void        class_setParentWidget                       (t_class *c, t_parentwidgetbehavior *w);
void        class_setDrawCommand                        (t_class *c);
void        class_setHelpName                           (t_class *c, t_symbol *s);
void        class_setPropertiesFunction                 (t_class *c, t_propertiesfn f);
void        class_setSaveFunction                       (t_class *c, t_savefn f);

char        *class_getName                              (t_class *c);
char        *class_getHelpName                          (t_class *c);
char        *class_getHelpDirectory                     (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_parentwidgetbehavior  *class_getParentWidget          (t_class *c);
t_propertiesfn           class_getPropertiesFunction    (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            object_list                             (t_object *x, t_symbol *s, int argc, t_atom *argv);
void            object_saveWidth                        (t_object *x, t_buffer *b);
int             object_numberOfInlets                   (t_object *x);
int             object_numberOfOutlets                  (t_object *x);
int             object_numberOfSignalInlets             (t_object *x);
int             object_numberOfSignalOutlets            (t_object *x);
int             object_indexOfSignalInlet               (t_object *x, int m);
int             object_indexOfSignalOutlet              (t_object *x, int m);
int             object_isSignalInlet                    (t_object *x, int m);
int             object_isSignalOutlet                   (t_object *x, int m);
int             object_getIndexOfSignalInlet            (t_inlet *x);
int             object_getIndexOfSignalOutlet           (t_outlet *x);
void            object_moveInletFirst                   (t_object *x, t_inlet *i);
void            object_moveOutletFirst                  (t_object *x, t_outlet *i);
void            object_disconnect                       (t_object *src, int m, t_object *dest, int n);

t_outconnect    *object_connect                         (t_object *src, int m, t_object *dest, int n);
t_outconnect    *object_traverseOutletStart             (t_object *x, t_outlet **ptr, int n);
t_outconnect    *object_traverseOutletNext              (t_outconnect *last,
                                                            t_object **dest,
                                                            t_inlet **ptr,
                                                            int *n);

t_float         *object_getSignalValueAtIndex           (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int  outlet_isSignal                        (t_outlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void global_new                             (void *dummy, t_symbol *name, t_symbol *directory);
void global_dsp                             (void *dummy, t_symbol *s, int argc, t_atom *argv);
void global_key                             (void *dummy, t_symbol *s, int argc, t_atom *argv);
void global_quit                            (void *dummy);
void global_gui                             (void *dummy, t_symbol *s, int argc, t_atom *argv);
void global_audioProperties                 (void *dummy, t_float f);
void global_midiProperties                  (void *dummy, t_float f);
void global_audioDialog                     (void *dummy, t_symbol *s, int argc, t_atom *argv);
void global_midiDialog                      (void *dummy, t_symbol *s, int argc, t_atom *argv);
void global_audioAPI                        (void *dummy, t_float f);
void global_midiAPI                         (void *dummy, t_float f);
void global_pathDialog                      (void *dummy, t_float f);
void global_setPath                         (void *dummy, t_symbol *s, int argc, t_atom *argv);
void global_ping                            (void *dummy);
void global_watchdog                        (void *dummy);
void global_savePreferences                 (void *dummy);
void global_shouldQuit                      (void *dummy);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error atom_withStringUnzeroed             (t_atom *a, char *s, int size);
t_error atom_toString                       (t_atom *a, char *s, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    buffer_log                          (t_buffer *x);
void    buffer_post                         (t_buffer *x);
void    buffer_resize                       (t_buffer *x, int n);
void    buffer_vAppend                      (t_buffer *x, char *fmt, ...);
void    buffer_appendSemicolon              (t_buffer *x);
void    buffer_parseStringUnzeroed          (t_buffer *x, char *s, int size, int allocated);
void    buffer_toString                     (t_buffer *x, char **s, int *size);
void    buffer_toStringUnzeroed             (t_buffer *x, char **s, int *size);
void    buffer_withStringUnzeroed           (t_buffer *x, char *s, int size);
void    buffer_serialize                    (t_buffer *x, t_buffer *y);
void    buffer_deserialize                  (t_buffer *x, int argc, t_atom *argv);
void    buffer_eval                         (t_buffer *x, t_pd *target, int argc, t_atom *argv);
t_error buffer_read                         (t_buffer *x, char *name, t_canvas *canvas);
t_error buffer_write                        (t_buffer *x, char *name, char *directory);
t_error buffer_evalFile                     (t_symbol *name, t_symbol *directory);
void    buffer_openFile                     (void *dummy, t_symbol *name, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    clock_setUnitAsSamples              (t_clock *x, double samples);
void    clock_setUnitAsMilliseconds         (t_clock *x, double ms);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

double  scheduler_getSystime                (void);
double  scheduler_getSystimeAfter           (double ms);
double  scheduler_getMillisecondsSince      (double systime);
double  scheduler_getUnitsSince             (double systime, double unit, int isSamples);
void    scheduler_setAudioMode              (int flag);
void    scheduler_needToRestart             (void);
void    scheduler_needToExit                (void);
void    scheduler_lock                      (void);
void    scheduler_unlock                    (void);
void    scheduler_audioCallback             (void);
int     scheduler_main                      (void);
int     scheduler_mainForBatchProcessing    (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    sys_vgui                            (char *fmt, ...);
void    sys_gui                             (char *s);
void    sys_pretendguibytes                 (int n);
void    sys_queuegui                        (void *client, t_glist *glist, t_guifn f);
void    sys_unqueuegui                      (void *client);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    gfxstub_new                         (t_pd *owner, void *key, const char *cmd);
void    gfxstub_deleteforkey                (void *key);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_core_h_
