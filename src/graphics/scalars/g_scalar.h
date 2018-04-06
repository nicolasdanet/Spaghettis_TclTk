
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __g_scalar_h_
#define __g_scalar_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/*

    An element is a tuple of words.
    A word can be a float, a symbol, a text or an array.
    The content of an element is defined and labelled by its template.
    
    A template is identify and fetch by its unique name.
    A template cannot be changed during its lifetime. 
    A template is instantiated by the struct object.
    For now only one instance by template at time is allowed.
    
    An array is a vector of elements.
    For now an array can NOT contain another array.
    
    A scalar is an object that wrap an element.
    A scalar is rendering by painter objects (e.g. plot).
    To be considered painters must be disposed in the instance's window.
    
    A gpointer can maintain a pointer onto a scalar.
    A gpointer can maintain a pointer onto one element of an array.
    A gpointer can be query about the validity of its target.
    A gpointer can modify the pointed element (indifferently of its type).
    
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define DATA_FLOAT          0
#define DATA_SYMBOL         1
#define DATA_TEXT           2
#define DATA_ARRAY          3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define PLOT_POLYGONS       0
#define PLOT_POINTS         1
#define PLOT_CURVES         2

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _gpointer {
    union {   
        struct _scalar      *gp_scalar;
        t_word              *gp_w;
    } gp_un;
    t_gmaster               *gp_refer;
    t_unique                gp_uniqueIdentifier;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _array {
    int                     a_elementSize;
    int                     a_size;
    t_word                  *a_elements;
    t_symbol                *a_templateIdentifier;
    t_gmaster               *a_holder;
    t_unique                a_uniqueIdentifier;
    t_gpointer              a_parent;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _fielddescriptor {
    int                     fd_type;
    int                     fd_isVariable;
    union {
        t_float             fd_float;
        t_symbol            *fd_variableName;
    } fd_un;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Element accessors (prefixed by word to avoid collision with equally named object). */

void        word_init                           (t_word *w, t_template *tmpl, t_gpointer *gp);
void        word_free                           (t_word *w, t_template *tmpl);

t_float     word_getFloat                       (t_word *w, t_template *tmpl, t_symbol *field);
t_symbol    *word_getSymbol                     (t_word *w, t_template *tmpl, t_symbol *field);
t_buffer    *word_getText                       (t_word *w, t_template *tmpl, t_symbol *field);
t_array     *word_getArray                      (t_word *w, t_template *tmpl, t_symbol *field);

void        word_setFloat                       (t_word *w, t_template *tmpl, t_symbol *field, t_float f);
void        word_setSymbol                      (t_word *w, t_template *tmpl, t_symbol *field, t_symbol *s);
void        word_setText                        (t_word *w, t_template *tmpl, t_symbol *field, t_buffer *b);

t_float     word_getFloatByDescriptor           (t_word *w, t_template *tmpl, t_fielddescriptor *fd);
void        word_setFloatByDescriptor           (t_word *w,
                                                    t_template *tmpl,
                                                    t_fielddescriptor *fd,
                                                    t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        field_setAsFloatConstant            (t_fielddescriptor *fd, t_float f);
void        field_setAsFloatVariable            (t_fielddescriptor *fd, t_symbol *s);
void        field_setAsFloat                    (t_fielddescriptor *fd, int argc, t_atom *argv);
void        field_setAsArray                    (t_fielddescriptor *fd, int argc, t_atom *argv);

int         field_isFloat                       (t_fielddescriptor *fd);
int         field_isFloatConstant               (t_fielddescriptor *fd);
int         field_isArray                       (t_fielddescriptor *fd);
int         field_isVariable                    (t_fielddescriptor *fd);

t_float     field_getFloatConstant              (t_fielddescriptor *fd);

t_symbol    *field_getVariableName              (t_fielddescriptor *fd);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array     *array_new                          (t_symbol *templateIdentifier, t_gpointer *parent);
t_symbol    *array_getTemplateIdentifier        (t_array *x);
t_template  *array_getTemplate                  (t_array *x);
t_word      *array_getElements                  (t_array *x);
t_word      *array_getElementAtIndex            (t_array *x, int n);
t_gpointer  *array_getTopParent                 (t_array *x);

int         array_getSize                       (t_array *x);
int         array_getElementSize                (t_array *x);
t_float     array_getFloatAtIndex               (t_array *x, int n, t_symbol *field);
void        array_setFloatAtIndex               (t_array *x, int n, t_symbol *field, t_float f);

t_float     array_getFloatAtIndexByDescriptor   (t_array *x, int n, t_fielddescriptor *fd);
void        array_setFloatAtIndexByDescriptor   (t_array *x, int n, t_fielddescriptor *fd, t_float f);
                                                            
void        array_free                          (t_array *x);
void        array_resize                        (t_array *x, int n);
void        array_redraw                        (t_array *x, t_glist *glist);
void        array_resizeAndRedraw               (t_array *x, t_glist *glist, int n);
void        array_serialize                     (t_array *x, t_buffer *b);
void        array_deserialize                   (t_array *x, t_iterator *iter);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_scalar    *scalar_new                         (t_glist *owner, t_symbol *templateIdentifier);
t_word      *scalar_getElement                  (t_scalar *x);
t_template  *scalar_getTemplate                 (t_scalar *x);
t_symbol    *scalar_getTemplateIdentifier       (t_scalar *x);
t_array     *scalar_getArray                    (t_scalar *x, t_symbol *field);

int         scalar_containsTemplate             (t_scalar *x, t_symbol *templateIdentifier);
int         scalar_fieldIsFloat                 (t_scalar *x, t_symbol *field);
t_float     scalar_getFloat                     (t_scalar *x, t_symbol *field);
void        scalar_setFloat                     (t_scalar *x, t_symbol *field, t_float f);

void        scalar_serialize                    (t_scalar *x, t_buffer *b);
void        scalar_deserialize                  (t_scalar *x, t_glist *glist, int argc, t_atom *argv);
void        scalar_redraw                       (t_scalar *x, t_glist *glist);
void        scalar_snap                         (t_scalar *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        paint_erase                         (void);
void        paint_draw                          (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "g_template.h"
#include "g_gmaster.h"
#include "g_gpointer.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_scalar_h_
