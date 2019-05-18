
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_list_h_
#define __x_list_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _listinletelement {
    t_atom              le_atom;
    t_gpointer          le_gpointer;
    } t_listinletelement;

typedef struct _listinlet {
    t_pd                li_pd;              /* MUST be the first. */
    int                 li_size;
    t_listinletelement  *li_vector;
    } t_listinlet;

typedef struct _listinlethelper {
    t_object            lh_obj;             /* Must be the first. */
    t_listinlet         lh_listinlet;
    } t_listinlethelper;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Trick to avoid duplication of code. */

typedef struct _listappend {
    t_listinlethelper   x_h;                /* Must be the first. */
    t_outlet            *x_outlet;
    t_outlet            *x_outletRight;
    } t_listappend;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef t_listappend t_listchange;
typedef t_listappend t_listequal;
typedef t_listappend t_listprepend;
typedef t_listappend t_liststore;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    listinlet_init                  (t_listinlet *x);
void    listinlet_initByClone           (t_listinlet *x, t_listinlet *newList);
void    listinlet_free                  (t_listinlet *x);
void    listinlet_copyAtomsUnchecked    (t_listinlet *x, t_atom *a);

void    listinlet_listClear             (t_listinlet *x);
void    listinlet_listSet               (t_listinlet *x, int argc, t_atom *argv);
void    listinlet_listAppend            (t_listinlet *x, int argc, t_atom *argv);
void    listinlet_listPrepend           (t_listinlet *x, int argc, t_atom *argv);
void    listinlet_listSetAt             (t_listinlet *x, int i, int argc, t_atom *argv);
void    listinlet_listInsert            (t_listinlet *x, int i, int argc, t_atom *argv);
void    listinlet_listRemove            (t_listinlet *x, int i, int n);
void    listinlet_listReplace           (t_listinlet *x, int i, int n, int argc, t_atom *argv);
int     listinlet_listIsEqualTo         (t_listinlet *x, int argc, t_atom *argv);
void    listinlet_listGet               (t_listinlet *x, t_buffer *b);
void    listinlet_listSetByCopy         (t_listinlet *x, t_listinlet *toCopy);
int     listinlet_listHasPointer        (t_listinlet *x);
int     listinlet_listSize              (t_listinlet *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    *listappend_new                 (t_symbol *s, int argc, t_atom *argv);
void    *listchange_new                 (t_symbol *s, int argc, t_atom *argv);
void    *listequal_new                  (t_symbol *s, int argc, t_atom *argv);
void    *listfromsymbol_new             (t_symbol *s, int argc, t_atom *argv);
void    *listgroup_new                  (t_symbol *s, int argc, t_atom *argv);
void    *listiterate_new                (t_symbol *s, int argc, t_atom *argv);
void    *listlength_new                 (t_symbol *s, int argc, t_atom *argv);
void    *listprepend_new                (t_symbol *s, int argc, t_atom *argv);
void    *listreverse_new                (t_symbol *s, int argc, t_atom *argv);
void    *listrotate_new                 (t_symbol *s, int argc, t_atom *argv);
void    *listscramble_new               (t_symbol *s, int argc, t_atom *argv);
void    *listsort_new                   (t_symbol *s, int argc, t_atom *argv);
void    *listsplit_new                  (t_symbol *s, int argc, t_atom *argv);
void    *liststore_new                  (t_symbol *s, int argc, t_atom *argv);
void    *liststream_new                 (t_symbol *s, int argc, t_atom *argv);
void    *listsum_new                    (t_symbol *s, int argc, t_atom *argv);
void    *listtosymbol_new               (t_symbol *s, int argc, t_atom *argv);
void    *listtrim_new                   (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_buffer    *listhelper_functionData    (t_gobj *z, int flags);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        listhelper_restore          (t_listinlethelper *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    listappend_list                 (t_listappend *x, t_symbol *s, int argc, t_atom *argv);
void    listappend_anything             (t_listappend *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_list_h_
