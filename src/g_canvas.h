
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_canvas_h_
#define __g_canvas_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_fromArrayDialog                  (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeArray                        (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeObject                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeMessage                      (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeFloatAtom                    (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeSymbolAtom                   (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeComment                      (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeScalar                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeBang                         (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeToggle                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeVerticalSlider               (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeHorizontalSlider             (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeHorizontalRadio              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeVerticalRadio                (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeVu                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makePanel                        (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_makeDial                         (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet         *canvas_addInlet                        (t_glist *glist, t_pd *receiver, t_symbol *s);
t_outlet        *canvas_addOutlet                       (t_glist *glist, t_symbol *s);

void            canvas_removeInlet                      (t_glist *glist, t_inlet *inlet);
void            canvas_removeOutlet                     (t_glist *glist, t_outlet *outlet);

void            canvas_resortInlets                     (t_glist *glist);
void            canvas_resortOutlets                    (t_glist *glist);

void            canvas_bounds                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
                                                            
t_float         canvas_valueToPixelX                    (t_glist *glist, t_float f);
t_float         canvas_valueToPixelY                    (t_glist *glist, t_float f);
t_float         canvas_pixelToValueX                    (t_glist *glist, t_float f);
t_float         canvas_pixelToValueY                    (t_glist *glist, t_float f);

t_float         canvas_valueForDeltaInPixelX            (t_glist *glist, t_float f);
t_float         canvas_valueForDeltaInPixelY            (t_glist *glist, t_float f);
t_float         canvas_valueForOnePixelX                (t_glist *glist);
t_float         canvas_valueForOnePixelY                (t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist         *canvas_new                             (void *dummy, t_symbol *s, int argc, t_atom *argv);

void            canvas_free                             (t_glist *glist);

void            canvas_click                            (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_motion                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_mouse                            (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_mouseUp                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_window                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_connect                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_disconnect                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_key                              (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_restore                          (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_rename                           (t_glist *glist, t_symbol *s, int argc, t_atom *argv);
void            canvas_loadbang                         (t_glist *glist);
void            canvas_editmode                         (t_glist *glist, t_float f);
void            canvas_close                            (t_glist *glist, t_float f);
void            canvas_visible                          (t_glist *glist, t_float f);
void            canvas_map                              (t_glist *glist, t_float f);

void            canvas_cut                              (t_glist *glist);
void            canvas_copy                             (t_glist *glist);
void            canvas_paste                            (t_glist *glist);
void            canvas_duplicate                        (t_glist *glist);
void            canvas_selectAll                        (t_glist *glist);

void            canvas_save                             (t_glist *glist, float destroy);
void            canvas_saveAs                           (t_glist *glist, float destroy);
void            canvas_saveToFile                       (t_glist *glist, t_symbol *s, int argc, t_atom *argv);

void            canvas_serialize                        (t_glist *glist, t_buffer *b);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_dsp                              (t_glist *glist, t_signal **sp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void            canvas_serializeTemplates               (t_glist *glist, t_buffer *b);
t_error         canvas_deserializeScalar                (t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_canvas_h_
