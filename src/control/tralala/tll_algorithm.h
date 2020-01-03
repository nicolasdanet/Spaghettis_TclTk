
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_ALPHABET            128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tll_random.h"
#include "tll_array.h"
#include "tll_items.h"
#include "tll_stack.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    TLL_ALGORITHM_TYPE_LATTICE  = 0,
    TLL_ALGORITHM_TYPE_ORACLE   = 1,
    TLL_ALGORITHM_TYPE_SIZE
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

struct _tll_algorithm;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef void    (*TLLFnAlgorithmFree)           (struct _tll_algorithm *q);
typedef t_error (*TLLFnAlgorithmAdd)            (struct _tll_algorithm *q, int argc, int *argv);
typedef void    (*TLLFnAlgorithmClear)          (struct _tll_algorithm *q);
typedef t_error (*TLLFnAlgorithmProceed)        (struct _tll_algorithm *q, int argc, int *argv);
typedef void    (*TLLFnAlgorithmSerialize)      (struct _tll_algorithm *q, tll_array *a);
typedef t_error (*TLLFnAlgorithmDeserialize)    (struct _tll_algorithm *q, tll_array *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tll_algorithm {
    TLLFnAlgorithmFree          f_free;
    TLLFnAlgorithmAdd           f_add;
    TLLFnAlgorithmClear         f_clear;
    TLLFnAlgorithmProceed       f_proceed;
    TLLFnAlgorithmSerialize     f_serialize;
    TLLFnAlgorithmDeserialize   f_deserialize;
    } tll_algorithm;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef tll_algorithm *(*TLLFnAlgorithmNew)     (t_rand48 *seed);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

TLLFnAlgorithmNew tll_algorithmNew (int n);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_ALGORITHM_NEW(type, seed)           (*(tll_algorithmNew (type))) ((seed))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline void tll_algorithmFree (tll_algorithm *q)
{
    if (q) { q->f_free (q); }
}

static inline t_error tll_algorithmAdd (tll_algorithm *q, int argc, int *argv)
{
    return q->f_add (q, argc, argv);
}

static inline void tll_algorithmClear (tll_algorithm *q)
{
    q->f_clear (q);
}

static inline t_error tll_algorithmProceed (tll_algorithm *q, int argc, int *argv)
{
    return q->f_proceed (q, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

tll_array   *tll_algorithmSerialize     (tll_algorithm *q);

t_error     tll_algorithmDeserialize    (tll_algorithm *q, tll_array *a);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tll_lattice.h"
#include "tll_oracle.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
