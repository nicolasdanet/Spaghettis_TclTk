
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __m_abstractions_h_
#define __m_abstractions_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Used to keep states of abstractions during encapsulation (live coding). */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Takes exclusive ownership of the buffer. */

t_abstractions  *abstractions_new           (void);

t_symbol        *abstractions_cache         (t_abstractions *x, t_glist *glist, t_symbol *name, t_buffer *b);
t_symbol        *abstractions_getName       (t_abstractions *x, t_symbol *key);
t_buffer        *abstractions_getSnippet    (t_abstractions *x,
                                                t_symbol *key,
                                                t_symbol **filename,
                                                t_symbol **directory);

void            abstractions_free           (t_abstractions *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_abstractions_h_
