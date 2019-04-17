
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_core_h_
#define __m_core_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Order of inclusion matters. */

#include "core/m_macros.h"
#include "core/m_symbols.h"
#include "core/m_snap.h"
#include "core/m_helpers.h"
#include "core/m_clipboard.h"
#include "core/m_environment.h"
#include "core/m_clocks.h"
#include "core/m_instance.h"
#include "core/m_class.h"
#include "core/m_object.h"
#include "core/m_inlet.h"
#include "core/m_outlet.h"
#include "core/m_error.h"
#include "core/m_utils.h"
#include "core/m_unique.h"
#include "undo/m_undo.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *atom_getSymbolOrDollarSymbol           (t_atom *a);
t_symbol    *atom_getSymbolOrDollarSymbolAtIndex    (int n, int argc, t_atom *argv);

void        atom_copyAtom                           (t_atom *src, t_atom *dest);
void        atom_copyAtoms                          (t_atom *src, int m, t_atom *dest, int n);
int         atom_copyAtomsExpanded                  (t_atom *src, int m, t_atom *dest, int n, t_glist *glist);

t_atomtype  atom_getType                            (t_atom *a);
int         atom_typesAreEquals                     (t_atom *a, t_atom *b);
t_error     atom_withStringUnzeroed                 (t_atom *a, const char *s, int size);
t_error     atom_toString                           (t_atom *a, char *dest, int size);

void        atom_invalidatePointers                 (int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char        *buffer_toString                        (t_buffer *x);      /* Caller acquires ownership. */

void        buffer_toStringUnzeroed                 (t_buffer *x, char **s, int *size);
void        buffer_withStringUnzeroed               (t_buffer *x, const char *s, int size);

void        buffer_reparseIfNeeded                  (t_buffer *x);
void        buffer_reparse                          (t_buffer *x);
void        buffer_invalidatePointers               (t_buffer *x);
void        buffer_shuffle                          (t_buffer *x);

int         buffer_isLastMessageProperlyEnded       (t_buffer *x);
int         buffer_getNumberOfMessages              (t_buffer *x);
t_error     buffer_getMessageAt                     (t_buffer *x, int n, int *start, int *end);
t_error     buffer_getMessageAtWithTypeOfEnd        (t_buffer *x, int n, int *start, int *end, t_atomtype *t);

void        buffer_serialize                        (t_buffer *x, t_buffer *y);
void        buffer_deserialize                      (t_buffer *x, int argc, t_atom *argv);

t_error     buffer_fileRead                         (t_buffer *x, t_symbol *name, t_glist *glist);
t_error     buffer_fileWrite                        (t_buffer *x, t_symbol *name, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        eval_buffer                             (t_buffer *x, t_pd *object, int argc, t_atom *argv);

t_error     eval_file                               (t_symbol *name, t_symbol *directory);
t_error     eval_fileByString                       (t_symbol *name, t_symbol *directory, const char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Expand dollar symbol. */

t_symbol    *dollar_expandSymbolWithArguments       (t_symbol *s, t_glist *glist, int argc, t_atom *argv);
t_symbol    *dollar_expandSymbol                    (t_symbol *s, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Expand dollar number. */

int dollar_expandWithArguments (t_atom *dollar, t_atom *a, t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Avoid typing. */

extern t_class *bindlist_class;
extern t_class *block_class;
extern t_class *canvas_class;
extern t_class *catch_tilde_class;
extern t_class *concept_class;
extern t_class *constructor_class;
extern t_class *delwrite_tilde_class;
extern t_class *floatinlet_class;
extern t_class *garray_class;
extern t_class *inlet_class;
extern t_class *inlet_class;
extern t_class *line_class;
extern t_class *panel_class;
extern t_class *plot_class;
extern t_class *pointerinlet_class;
extern t_class *scalar_class;
extern t_class *send_tilde_class;
extern t_class *struct_class;
extern t_class *symbolinlet_class;
extern t_class *template_class;
extern t_class *text_class;
extern t_class *textdefine_class;
extern t_class *undomotion_class;
extern t_class *vinlet_class;
extern t_class *voutlet_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_core_h_
