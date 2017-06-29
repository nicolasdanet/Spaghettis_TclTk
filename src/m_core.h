
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_core_h_
#define __m_core_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Order of inclusion matters. */

#include "m_macros.h"
#include "m_symbols.h"
#include "m_helpers.h"
#include "m_clipboard.h"
#include "m_environment.h"
#include "m_instance.h"
#include "m_class.h"
#include "m_object.h"
#include "m_inlet.h"
#include "m_outlet.h"
#include "m_error.h"
#include "m_utils.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_atom      *atom_substituteIfPointer               (t_atom *a);

t_atomtype  atom_getType                            (t_atom *a);
int         atom_typesAreEquals                     (t_atom *a, t_atom *b);
t_error     atom_withStringUnzeroed                 (t_atom *a, char *s, int size);
t_error     atom_toString                           (t_atom *a, char *dest, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        buffer_toStringUnzeroed                 (t_buffer *x, char **s, int *size);
void        buffer_withStringUnzeroed               (t_buffer *x, char *s, int size);

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
t_error     eval_fileByString                       (t_symbol *name, t_symbol *directory, char *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *dollar_expandSymbol                    (t_symbol *s, t_glist *glist);
t_symbol    *dollar_expandSymbolWithArguments       (t_symbol *s, t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void dollar_expandWithArguments (t_atom *dollar, t_atom *a, t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Avoid typing. */

extern t_class *block_class;
extern t_class *canvas_class;
extern t_class *catch_tilde_class;
extern t_class *delwrite_tilde_class;
extern t_class *floatinlet_class;
extern t_class *garray_class;
extern t_class *inlet_class;
extern t_class *panel_class;
extern t_class *pointerinlet_class;
extern t_class *scalar_class;
extern t_class *send_tilde_class;
extern t_class *struct_class;
extern t_class *symbolinlet_class;
extern t_class *text_class;
extern t_class *textdefine_class;
extern t_class *vinlet_class;
extern t_class *voutlet_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_core_h_
