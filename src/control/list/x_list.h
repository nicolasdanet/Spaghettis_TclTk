
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_list_h_
#define __x_list_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _listinletelement {
    t_atom              le_atom;
    t_gpointer          le_gpointer;
    };

typedef struct _listinlet {
    t_pd                li_pd;          /* MUST be the first. */
    int                 li_size;
    int                 li_hasPointer;
    t_listinletelement  *li_vector;
    } t_listinlet;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    listinlet_init                  (t_listinlet *x);
void    listinlet_clear                 (t_listinlet *x);
void    listinlet_listSet               (t_listinlet *x, int argc, t_atom *argv);
void    listinlet_listAppend            (t_listinlet *x, int argc, t_atom *argv);
void    listinlet_listPrepend           (t_listinlet *x, int argc, t_atom *argv);
int     listinlet_getSize               (t_listinlet *x);
int     listinlet_hasPointer            (t_listinlet *x);
void    listinlet_copyAtomsUnchecked    (t_listinlet *x, t_atom *a);
void    listinlet_clone                 (t_listinlet *x, t_listinlet *newList);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    *listappend_new                 (t_symbol *s, int argc, t_atom *argv);
void    *listprepend_new                (t_symbol *s, int argc, t_atom *argv);
void    *listsplit_new                  (t_symbol *s, int argc, t_atom *argv);
void    *listtrim_new                   (t_symbol *s, int argc, t_atom *argv);
void    *listlength_new                 (t_symbol *s, int argc, t_atom *argv);
void    *liststore_new                  (t_symbol *s, int argc, t_atom *argv);
void    *listiterate_new                (t_symbol *s, int argc, t_atom *argv);
void    *listgroup_new                  (t_symbol *s, int argc, t_atom *argv);
void    *listfromsymbol_new             (t_symbol *s, int argc, t_atom *argv);
void    *listtosymbol_new               (t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_list_h_
