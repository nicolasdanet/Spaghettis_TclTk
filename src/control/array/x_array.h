
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_array_h_
#define __x_array_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _arrayclient {
    t_object        ac_obj;                 /* Must be the first. */
    t_gpointer      ac_gpointer;
    t_symbol        *ac_name;
    t_symbol        *ac_templateIdentifier;
    t_symbol        *ac_fieldName;
    } t_arrayclient;

typedef struct _arrayrange {
    t_arrayclient   ar_arrayclient;         /* Must be the first. */
    t_error         ar_error;
    t_float         ar_onset;
    t_float         ar_size;
    t_symbol        *ar_fieldName;
    } t_arrayrange;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ARRAYCLIENT_HAS_POINTER(x)          ((x)->ac_templateIdentifier)
#define ARRAYCLIENT_ADDRESS_POINTER(x)      &((x)->ac_gpointer)
#define ARRAYCLIENT_ADDRESS_NAME(x)         &((x)->ac_name)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ARRAYRANGE_GOOD(x)                  (((t_arrayrange *)(x))->ar_error == PD_ERROR_NONE)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     arrayclient_init                (t_arrayclient *x, int *argc, t_atom **argv);
void        arrayclient_free                (t_arrayclient *x);
void        arrayclient_update              (t_arrayclient *x);
void        arrayclient_setName             (t_arrayclient *x, t_symbol *s);

t_symbol    *arrayclient_getName            (t_arrayclient *x);
t_glist     *arrayclient_fetchOwner         (t_arrayclient *x);
t_array     *arrayclient_fetchArray         (t_arrayclient *x);
t_garray    *arrayclient_fetchGraphicArray  (t_arrayclient *x);

void        arrayclient_restore             (t_arrayclient *x, t_arrayclient *old);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *arrayrange_new                 (t_class *c, int argc, t_atom *argv, int makeOnset, int makeSize);
t_array     *arrayrange_getRange            (t_arrayrange *x, int *i, int *n);
t_symbol    *arrayrange_getFieldName        (t_arrayrange *x);

void        arrayrange_update               (t_arrayrange *x);
int         arrayrange_isValid              (t_arrayrange *x);
void        arrayrange_setOnset             (t_arrayrange *x, t_float f);
void        arrayrange_setSize              (t_arrayrange *x, t_float f);
t_float     arrayrange_getOnset             (t_arrayrange *x);
t_float     arrayrange_getSize              (t_arrayrange *x);
t_float     arrayrange_quantile             (t_arrayrange *x, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_buffer    *arrayrange_functionData        (t_gobj *z, int flags);

void        arrayrange_restore              (t_arrayrange *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *arraysize_new                  (t_symbol *s, int argc, t_atom *argv);
void        *arraysum_new                   (t_symbol *s, int argc, t_atom *argv);
void        *arrayget_new                   (t_symbol *s, int argc, t_atom *argv);
void        *arrayset_new                   (t_symbol *s, int argc, t_atom *argv);
void        *arrayquantile_new              (t_symbol *s, int argc, t_atom *argv);
void        *arrayrandom_new                (t_symbol *s, int argc, t_atom *argv);
void        *arraymax_new                   (t_symbol *s, int argc, t_atom *argv);
void        *arraymin_new                   (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_array_h_
