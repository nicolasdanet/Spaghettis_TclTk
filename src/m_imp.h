
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
    t_symbol                *c_name;
    t_symbol                *c_helpname;
    t_symbol                *c_externdir;
    size_t                  c_size;
    t_methodentry           *c_methods;
    int                     c_nmethod;
    t_method                c_freemethod;
    t_bangmethod            c_bangmethod;
    t_pointermethod         c_pointermethod;
    t_floatmethod           c_floatmethod;
    t_symbolmethod          c_symbolmethod;
    t_listmethod            c_listmethod;
    t_anymethod             c_anymethod;
    t_widgetbehavior        *c_wb;
    t_parentwidgetbehavior  *c_pwb;
    t_savefn                c_savefn;
    t_propertiesfn          c_propertiesfn;
    int                     c_floatsignalin;
    char                    c_gobj;
    char                    c_patchable;
    char                    c_firstin;
    char                    c_drawcommand;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _pdinstance {
    double      pd_systime;
    t_clock     *pd_clock_setlist;
    t_int       *pd_dspchain;
    int         pd_dspchainsize;
    t_canvas    *pd_canvaslist;
    int         pd_dspstate;
    t_signal    *pd_signals;
    t_symbol    *pd_midiin_sym;
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

extern t_pdinstance *pd_this;           /* Global to PureData. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_emptylist       (t_pd *x);
void outlet_setstacklim (void);

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
#endif // __m_imp_h_
