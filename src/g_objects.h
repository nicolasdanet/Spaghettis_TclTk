
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_objects_h_
#define __g_objects_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        message_makeObject          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void        message_click               (t_message *x, t_symbol *s, int argc, t_atom *argv);
                                                            
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        gatom_makeObjectFloat       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void        gatom_makeObjectSymbol      (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void        gatom_click                 (t_gatom *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_garray    *garray_makeObject          (t_glist *glist, t_symbol *s, t_float n, t_float flags);
t_array     *garray_getArray            (t_garray *x);
t_glist     *garray_getView             (t_garray *x);
t_scalar    *garray_getScalar           (t_garray *x);
t_symbol    *garray_getName             (t_garray *x);
t_symbol    *garray_getUnexpandedName   (t_garray *x);

int         garray_getSize              (t_garray *x);
int         garray_getData              (t_garray *x, int *size, t_word **w);
void        garray_setDataAtIndex       (t_garray *x, int i, t_float f);
t_float     garray_getDataAtIndex       (t_garray *x, int i);
void        garray_setDataFromIndex     (t_garray *x, int i, t_float f);
t_float     garray_getAmplitude         (t_garray *x);
void        garray_setAsUsedInDSP       (t_garray *x);
void        garray_setSaveWithParent    (t_garray *x, int savedWithParent);
void        garray_redraw               (t_garray *x);
void        garray_resize               (t_garray *x, t_float f);
void        garray_functionProperties   (t_garray *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_inlet     *vinlet_getInlet            (t_pd *x);
t_outlet    *voutlet_getOutlet          (t_pd *x);

int         vinlet_isSignal             (t_vinlet *x);
int         voutlet_isSignal            (t_voutlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_objects_h_
