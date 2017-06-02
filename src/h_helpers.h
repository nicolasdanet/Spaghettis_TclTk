
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __h_helpers_h_
#define __h_helpers_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _pathlist {
    struct _pathlist    *pl_next;
    char                *pl_string;
    } t_pathlist;

typedef struct _heapstring {
    size_t              hs_used;
    size_t              hs_size;
    char                *hs_raw;
    } t_heapstring;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_iterator      *iterator_new                           (int argc, t_atom *argv);

void            iterator_free                           (t_iterator *x);
int             iterator_next                           (t_iterator *x, t_atom **a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_pathlist      *pathlist_newAppend                     (t_pathlist *x, const char *s);
t_pathlist      *pathlist_newAppendEncoded              (t_pathlist *x, t_symbol *s);
char            *pathlist_getPath                       (t_pathlist *x);
t_pathlist      *pathlist_getNext                       (t_pathlist *x);

void            pathlist_free                           (t_pathlist *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_heapstring    *heapstring_new                         (int size);
char            *heapstring_getRaw                      (t_heapstring *x);

void            heapstring_free                         (t_heapstring *x);
t_error         heapstring_add                          (t_heapstring *x, const char *src);
t_error         heapstring_append                       (t_heapstring *x, const char *src, int n);
t_error         heapstring_addSprintf                   (t_heapstring *x, const char *format, ...);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void            snippet_addOffsetToLine                 (t_buffer *x, int i);
void            snippet_substractOffsetToLine           (t_buffer *x, int i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __h_helpers_h_
