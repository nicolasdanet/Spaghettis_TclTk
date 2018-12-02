
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tll_algorithm.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "tll_random.c"
#include "tll_array.c"
#include "tll_items.c"
#include "tll_stack.c"
#include "tll_lattice.c"
#include "tll_oracle.c"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_SERIALIZE_SIZE          2048
#define TLL_SERIALIZE_VERSION       1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

TLLFnAlgorithmNew tll_algorithmNew (int n)
{
    TLLFnAlgorithmNew f = NULL;
    
    switch (n) {
        case TLL_ALGORITHM_TYPE_LATTICE : f = tll_latticeNew;   break;
        case TLL_ALGORITHM_TYPE_ORACLE  : f = tll_oracleNew;    break;
    }
    
    return f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_array *tll_algorithmSerialize (tll_algorithm *q)
{
    tll_array *a = tll_arrayNew (TLL_SERIALIZE_SIZE);
    
    if (q->f_serialize) { q->f_serialize (q, a); }
    
    {
        uint32_t hash  = tll_arrayHash (a);
    
        /* Split the hash to avoid errors in case of floating-point conversions. */
    
        uint32_t hash1 = (hash >> 0)  & 0xff;
        uint32_t hash2 = (hash >> 8)  & 0xff;
        uint32_t hash3 = (hash >> 16) & 0xff;
        uint32_t hash4 = (hash >> 24) & 0xff;
    
        tll_arrayAppend (a, TLL_SERIALIZE_VERSION);
        tll_arrayAppend (a, hash1);
        tll_arrayAppend (a, hash2);
        tll_arrayAppend (a, hash3);
        tll_arrayAppend (a, hash4);
    }
    
    return a;
}

t_error tll_algorithmDeserialize (tll_algorithm *q, tll_array *a)
{
    t_error err = PD_ERROR;
    
    if (a) {
    //
    int n = tll_arrayGetSize (a);
    
    if (n > 5) {
    //
    int version    = tll_arrayGetAtIndex (a, n - 5);
    uint32_t hash1 = tll_arrayGetAtIndex (a, n - 4);
    uint32_t hash2 = tll_arrayGetAtIndex (a, n - 3);
    uint32_t hash3 = tll_arrayGetAtIndex (a, n - 2);
    uint32_t hash4 = tll_arrayGetAtIndex (a, n - 1);
    
    uint32_t hash;
    
    tll_arrayRemoveLast (a);
    tll_arrayRemoveLast (a);
    tll_arrayRemoveLast (a);
    tll_arrayRemoveLast (a);
    tll_arrayRemoveLast (a);
    
    hash = hash1 | (hash2 << 8) | (hash3 << 16) | (hash4 << 24);
    
    if (version == TLL_SERIALIZE_VERSION) {
    if (hash == tll_arrayHash (a)) {                                /* Prevent foolish not crackers. */
        if (q->f_deserialize) { err = q->f_deserialize (q, a); }
    }
    }
    
    tll_arrayAppend (a, version);
    tll_arrayAppend (a, hash1);
    tll_arrayAppend (a, hash2);
    tll_arrayAppend (a, hash3);
    tll_arrayAppend (a, hash4);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
