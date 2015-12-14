
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_imp_h_
#define __m_imp_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN_STRUCT _widgetbehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _methodentry {
    t_symbol    *me_name;
    t_gotfn     me_fun;
    t_atomtype  me_arg[MAXPDARG + 1];
    } t_methodentry;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void (*t_bangmethod)(t_pd *x);
typedef void (*t_pointermethod)(t_pd *x, t_gpointer *gp);
typedef void (*t_floatmethod)(t_pd *x, t_float f);
typedef void (*t_symbolmethod)(t_pd *x, t_symbol *s);
typedef void (*t_listmethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anymethod)(t_pd *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _class {
    t_symbol                *c_name;                /* Name (mostly for error reporting). */
    t_symbol                *c_helpname;            /* Name of the help file. */
    t_symbol                *c_externdir;           /* Directory extern was loaded from. */
    size_t                  c_size;                 /* Size of an instance. */
    t_methodentry           *c_methods;             /* Methods other than commons below. */
    int                     c_nmethod;              /* Number of methods. */
    t_method                c_freemethod;           /* Function to call before freeing. */
    t_bangmethod            c_bangmethod;           /* Common methods. */
    t_pointermethod         c_pointermethod;    
    t_floatmethod           c_floatmethod;
    t_symbolmethod          c_symbolmethod;
    t_listmethod            c_listmethod;
    t_anymethod             c_anymethod;
    t_widgetbehavior        *c_wb;                  /* For graphical objects only. */
    t_parentwidgetbehavior  *c_pwb;                 /* Widget behavior in parent. */
    t_savefn                c_savefn;               /* Function to call when saving. */
    t_propertiesfn          c_propertiesfn;         /* Function to start the property dialog. */
    int                     c_floatsignalin;        /* Onset to float for signal input. */
    char                    c_gobj;                 /* True if it is a graphical object. */
    char                    c_patchable;            /* True if it contains inlets and/or outlets. */
    char                    c_firstin;              /* If patchable, true if draw first inlet. */
    char                    c_drawcommand;          /* A drawing command for a template. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    double      pd_systime;                         /* Global time in ticks. */
    t_clock     *pd_clock_setlist;                  /* List of set clocks. */
    t_int       *pd_dspchain;                       /* DSP chain. */
    int         pd_dspchainsize;                    /* Number of elements in the DSP chain. */
    t_canvas    *pd_canvaslist;                     /* List of all root canvases. */
    int         pd_dspstate;                        /* Whether DSP is on or off. */
    t_signal    *pd_signals;                        /* List of signals used by the DSP chain. */
    t_symbol    *pd_midiin_sym;                     /* Symbols bound to incoming MIDI. */
    t_symbol    *pd_sysexin_sym;
    t_symbol    *pd_notein_sym;
    t_symbol    *pd_ctlin_sym;
    t_symbol    *pd_pgmin_sym;
    t_symbol    *pd_bendin_sym;
    t_symbol    *pd_touchin_sym;
    t_symbol    *pd_polytouchin_sym;
    t_symbol    *pd_midiclkin_sym;
    t_symbol    *pd_midirealtimein_sym;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pdinstance *pd_this;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

EXTERN void pd_emptylist        (t_pd *x);
EXTERN void outlet_setstacklim  (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_outconnect *obj_starttraverseoutlet    (t_object *x, t_outlet **op, int nout);
EXTERN t_outconnect *obj_nexttraverseoutlet     (t_outconnect *lastconnect, 
                                                    t_object **destp, 
                                                    t_inlet **inletp, 
                                                    int *whichp);
                                                    
EXTERN t_outconnect *obj_connect                (t_object *source, int outno, t_object *sink, int inno);
EXTERN void         obj_disconnect              (t_object *source, int outno, t_object *sink, int inno);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN int obj_noutlets         (t_object *x);
EXTERN int obj_ninlets          (t_object *x);
EXTERN int obj_issignalinlet    (t_object *x, int m);
EXTERN int obj_issignaloutlet   (t_object *x, int m);
EXTERN int obj_nsiginlets       (t_object *x);
EXTERN int obj_nsigoutlets      (t_object *x);
EXTERN int obj_siginletindex    (t_object *x, int m);
EXTERN int obj_sigoutletindex   (t_object *x, int m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

EXTERN t_pd *glob_evalfile      (t_pd *ignore, t_symbol *name, t_symbol *dir);
EXTERN void glob_initfromgui    (void *dummy, t_symbol *s, int argc, t_atom *argv);
EXTERN void glob_quit           (void *dummy);
EXTERN void open_via_helppath   (const char *name, const char *dir);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_imp_h_
