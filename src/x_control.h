
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_control_h_
#define __x_control_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _arrayclient {
    t_object            ac_obj;                         /* Must be the first. */
    t_gpointer          ac_gpointer;
    t_symbol            *ac_name;
    t_symbol            *ac_templateIdentifier;
    t_symbol            *ac_fieldName;
    } t_arrayclient;

typedef struct _arrayrange {
    t_arrayclient       ar_arrayclient;                 /* Must be the first. */
    t_float             ar_first;
    t_float             ar_size;
    t_symbol            *ar_fieldName;
    } t_arrayrange;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listinlet {
    t_pd                li_pd;                          /* MUST be the first. */
    int                 li_size;
    int                 li_hasPointer;
    t_listinletelement  *li_vector;
    } t_listinlet;

typedef struct _atomoutlet {
    t_atom              ao_atom;
    t_gpointer          ao_gpointer;
    t_outlet            *ao_outlet;
    } t_atomoutlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _binop {
    t_object            bo_obj;                         /* MUST be the first. */
    t_float             bo_f1;
    t_float             bo_f2;
    t_outlet            *bo_outlet;
    } t_binop;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TIME_DEFAULT_DELAY                  1000.0
#define TIME_DEFAULT_GRAIN                  20.0

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ARRAYCLIENT_ASPOINTER(x)            ((x)->ac_templateIdentifier)
#define ARRAYCLIENT_GETPOINTER(x)           &((x)->ac_gpointer)
#define ARRAYCLIENT_GETNAME(x)              &((x)->ac_name)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define OSC_GETCHAR(x)                      (int)GET_FLOAT (x)
#define OSC_SETCHAR(x, c)                   SET_FLOAT (x, (t_float)((unsigned char)(c) & 0xff))

#define OSC_ROUND4(x)                       (((x) + 3) & (~3))

#define OSC_READ4INT(x)                     ((((unsigned int)(GET_FLOAT ((x) + 0))) & 0xff) << 24) | \
                                            ((((unsigned int)(GET_FLOAT ((x) + 1))) & 0xff) << 16) | \
                                            ((((unsigned int)(GET_FLOAT ((x) + 2))) & 0xff) << 8)  | \
                                            ((((unsigned int)(GET_FLOAT ((x) + 3))) & 0xff) << 0)

#define OSC_WRITE4INT(x, i)                 SET_FLOAT ((x) + 0, (((unsigned int)(i) >> 24) & 0xff)); \
                                            SET_FLOAT ((x) + 1, (((unsigned int)(i) >> 16) & 0xff)); \
                                            SET_FLOAT ((x) + 2, (((unsigned int)(i) >> 8)  & 0xff)); \
                                            SET_FLOAT ((x) + 3, (((unsigned int)(i))       & 0xff))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error     arrayclient_init                (t_arrayclient *x, int *argc, t_atom **argv);
void        arrayclient_free                (t_arrayclient *x);
void        arrayclient_update              (t_arrayclient *x);

t_glist     *arrayclient_fetchView          (t_arrayclient *x);
t_array     *arrayclient_fetchArray         (t_arrayclient *x);
t_garray    *arrayclient_fetchOwnerIfName   (t_arrayclient *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *arrayrange_new                 (t_class *c, int argc, t_atom *argv, int makeOnset, int makeSize);
t_array     *arrayrange_getRange            (t_arrayrange *x, int *i, int *n);
t_symbol    *arrayrange_getFieldName        (t_arrayrange *x);

void        arrayrange_update               (t_arrayrange *x);
int         arrayrange_isValid              (t_arrayrange *x);
void        arrayrange_setFirst             (t_arrayrange *x, t_float f);
t_float     arrayrange_quantile             (t_arrayrange *x, t_float f);

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
// MARK: -

void        listinlet_init                  (t_listinlet *x);
void        listinlet_clear                 (t_listinlet *x);
void        listinlet_setList               (t_listinlet *x, int argc, t_atom *argv);
int         listinlet_getSize               (t_listinlet *x);
int         listinlet_hasPointer            (t_listinlet *x);
void        listinlet_copyListUnchecked     (t_listinlet *x, t_atom *a);
void        listinlet_clone                 (t_listinlet *x, t_listinlet *newList);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *listappend_new                 (t_symbol *s, int argc, t_atom *argv);
void        *listprepend_new                (t_symbol *s, int argc, t_atom *argv);
void        *listsplit_new                  (t_symbol *s, int argc, t_atom *argv);
void        *listtrim_new                   (t_symbol *s, int argc, t_atom *argv);
void        *listlength_new                 (t_symbol *s, int argc, t_atom *argv);
void        *listfromsymbol_new             (t_symbol *s, int argc, t_atom *argv);
void        *listtosymbol_new               (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void        *binop_new                      (t_class *c, t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_atom      *atomoutlet_getAtom             (t_atomoutlet *x);
t_outlet    *atomoutlet_getOutlet           (t_atomoutlet *x);
t_gpointer  *atomoutlet_getPointer          (t_atomoutlet *x);

t_atomtype  atomoutlet_getType              (t_atomoutlet *x);
int         atomoutlet_isPointer            (t_atomoutlet *x);
void        atomoutlet_copyAtom             (t_atomoutlet *x, t_atom *a);
t_error     atomoutlet_setAtom              (t_atomoutlet *x, t_atom *a);
t_error     atomoutlet_outputAtom           (t_atomoutlet *x, t_atom *a);
int         atomoutlet_isEqualTo            (t_atomoutlet *x, t_atom *a);

void        atomoutlet_init                 (t_atomoutlet *x);
void        atomoutlet_release              (t_atomoutlet *x);

t_error     atomoutlet_makeParse            (t_atomoutlet *x, t_object *o, t_atom *a, int inlet, int outlet);
void        atomoutlet_makeFloat            (t_atomoutlet *x, t_object *o, t_float f, int inlet, int outlet);
void        atomoutlet_makeSymbol           (t_atomoutlet *x, t_object *o, int inlet, int outlet);
void        atomoutlet_makePointer          (t_atomoutlet *x, t_object *o, int inlet, int outlet);

t_error     atomoutlet_makeTypedOutletParse (t_atomoutlet *x, t_object *o, t_atom *a);
void        atomoutlet_makeTypedOutlet      (t_atomoutlet *x,
                                                t_object *o,
                                                t_symbol *type,
                                                t_atom *a,
                                                int inlet);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_control_h_
