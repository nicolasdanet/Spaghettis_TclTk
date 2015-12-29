
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_private_h_
#define __m_private_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define TYPE_TEXT               0
#define TYPE_OBJECT             1
#define TYPE_MESSAGE            2
#define TYPE_ATOM               3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define DATA_FLOAT              0
#define DATA_SYMBOL             1
#define DATA_TEXT               2
#define DATA_ARRAY              3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define POINTER_NONE            0
#define POINTER_GLIST           1
#define POINTER_ARRAY           2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

PD_STRUCT _widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_gotfn)(void *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _methodentry {
    t_symbol    *me_name;
    t_gotfn     me_function;
    t_atomtype  me_arguments[PD_ARGUMENTS + 1];
    } t_methodentry;

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
    size_t                  c_size;
    t_methodentry           *c_methods;
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
    int                     c_floatSignalIn;
    char                    c_isGraphic;
    char                    c_isBox;
    char                    c_hasFirstInlet;
    char                    c_hasDrawCommand;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    double      pd_systime;
    t_clock     *pd_clocks;
    t_int       *pd_dspChain;
    int         pd_dspChainSize;
    int         pd_dspState;
    t_canvas    *pd_canvases;
    t_signal    *pd_signals;
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
#pragma mark -

void pd_emptylist       (t_pd *x);
void outlet_setstacklim (void);

void text_save          (t_gobj *z, t_binbuf *b);
void obj_list           (t_object *x, t_symbol *s, int argc, t_atom *argv);

void pd_push            (t_pd *x);
void pd_pop             (t_pd *x);

t_gotfn  getfn          (t_pd *x, t_symbol *s);
t_gotfn  zgetfn         (t_pd *x, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outconnect *obj_starttraverseoutlet   (t_object *x, t_outlet **op, int nout);
t_outconnect *obj_nexttraverseoutlet    (t_outconnect *lastconnect, 
                                            t_object **destp, 
                                            t_inlet **inletp, 
                                            int *whichp);

t_outconnect *obj_connect               (t_object *source, int outno, t_object *sink, int inno);
void         obj_disconnect             (t_object *source, int outno, t_object *sink, int inno);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int obj_noutlets        (t_object *x);
int obj_ninlets         (t_object *x);
int obj_issignalinlet   (t_object *x, int m);
int obj_issignaloutlet  (t_object *x, int m);
int obj_nsiginlets      (t_object *x);
int obj_nsigoutlets     (t_object *x);
int obj_siginletindex   (t_object *x, int m);
int obj_sigoutletindex  (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd *glob_evalfile     (t_pd *ignore, t_symbol *name, t_symbol *dir);
void glob_initfromgui   (void *dummy, t_symbol *s, int argc, t_atom *argv);
void glob_quit          (void *dummy);
void open_via_helppath  (const char *name, const char *dir);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void mess_init          (void);
void obj_init           (void);
void conf_init          (void);
void glob_init          (void);
void garray_init        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_private_h_
