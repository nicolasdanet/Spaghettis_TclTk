
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_core_h_
#define __m_core_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _mouse {
    int         m_x;
    int         m_y;
    int         m_shift;
    int         m_ctrl;
    int         m_alt;
    int         m_dbl;
    int         m_clicked;
    t_atom      m_atoms[7];
    } t_mouse;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int mouse_argc (t_mouse *m)
{
    return 7;
}

static inline t_atom *mouse_argv (t_mouse *m)
{
    SET_FLOAT (m->m_atoms + 0, m->m_x);
    SET_FLOAT (m->m_atoms + 1, m->m_y);
    SET_FLOAT (m->m_atoms + 2, m->m_shift);
    SET_FLOAT (m->m_atoms + 3, m->m_ctrl);
    SET_FLOAT (m->m_atoms + 4, m->m_alt);
    SET_FLOAT (m->m_atoms + 5, m->m_dbl);
    SET_FLOAT (m->m_atoms + 6, m->m_clicked);
    
    return m->m_atoms;      /* Lazily convert its members to an array of atoms. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* For example [ P a t h / t o / t h a t 0 f i l e 0 ]. */

struct _fileproperties { char f_directory[PD_STRING]; char *f_name; };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _fileproperties t_fileproperties;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline char *fileproperties_getDirectory (t_fileproperties *p)
{
    return p->f_directory;
}

static inline char *fileproperties_getName (t_fileproperties *p)
{
    return p->f_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Below core functions currently kept private. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_atom      *atom_substituteIfPointer               (t_atom *a);

t_atomtype  atom_getType                            (t_atom *a);
int         atom_typesAreEqual                      (t_atom *a, t_atom *b);
t_error     atom_setByStringUnzeroed                (t_atom *a, char *s, int size);
t_error     atom_toString                           (t_atom *a, char *dest, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        buffer_resize                           (t_buffer *x, int n);
void        buffer_vAppend                          (t_buffer *x, char *fmt, ...);
void        buffer_appendAtom                       (t_buffer *x, t_atom *a);
void        buffer_appendBuffer                     (t_buffer *x, t_buffer *y);
void        buffer_appendFloat                      (t_buffer *x, t_float f);
void        buffer_appendSymbol                     (t_buffer *x, t_symbol *s);
void        buffer_appendSemicolon                  (t_buffer *x);
t_error     buffer_resizeAtBetween                  (t_buffer *x, int n, int start, int end);
t_error     buffer_getAtomAtIndex                   (t_buffer *x, int n, t_atom *a);
t_error     buffer_setAtomAtIndex                   (t_buffer *x, int n, t_atom *a);
t_atom      *buffer_atomAtIndex                     (t_buffer *x, int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        buffer_toString                         (t_buffer *x, char **s);
void        buffer_toStringUnzeroed                 (t_buffer *x, char **s, int *size);
void        buffer_withStringUnzeroed               (t_buffer *x, char *s, int size);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Could use iterator helper instead. */

int         buffer_isLastMessageProperlyEnded       (t_buffer *x);
int         buffer_getNumberOfMessages              (t_buffer *x);
int         buffer_getMessageAt                     (t_buffer *x, int n, int *start, int *end);
int         buffer_getMessageAtWithTypeOfEnd        (t_buffer *x, int n, int *start, int *end, t_atomtype *t);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        buffer_serialize                        (t_buffer *x, t_buffer *y);
void        buffer_deserialize                      (t_buffer *x, int argc, t_atom *argv);
void        buffer_eval                             (t_buffer *x, t_pd *object, int argc, t_atom *argv);
t_error     buffer_read                             (t_buffer *x, t_symbol *name, t_glist *glist);
t_error     buffer_write                            (t_buffer *x, t_symbol *name, t_symbol *directory);
t_error     buffer_fileEval                         (t_symbol *name, t_symbol *directory);
t_error     buffer_fileEvalByString                 (t_symbol *name, t_symbol *directory, char *s);
void        buffer_fileOpen                         (t_symbol *name, t_symbol *directory);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_symbol    *dollar_expandDollarSymbol              (t_symbol *s, t_glist *glist);
t_symbol    *dollar_expandDollarSymbolWithArguments (t_symbol *s, t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void dollar_expandDollarWithArguments (t_atom *dollar, t_atom *a, t_glist *glist, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "h_helpers.h"
#include "m_rectangle.h"
#include "m_extern.h"
#include "m_symbols.h"
#include "m_instance.h"
#include "m_class.h"
#include "m_object.h"
#include "m_error.h"
#include "m_utils.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_core_h_
