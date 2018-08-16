
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://www-igm.univ-mlv.fr/~lecroq/articles/awoca2000.pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tll_oracleNode {
    int                 o_refer;
    int                 o_lrs;
    tll_array           *o_destinations;
    tll_array           *o_values;
    } tll_oracleNode;
       
typedef struct _tll_oracle {
    tll_algorithm       o_table;            /* Must be the first. */
    int                 o_size;
    int                 o_peak;
    int                 o_index;
    int                 o_shuttle;
    int                 o_backward;
    int                 o_threshold;
    int                 o_persistence;
    int                 o_straight;
    tll_array           *o_temp;
    tll_oracleNode      *o_nodes;
    t_rand48            *o_seed;
    } tll_oracle;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_algorithm *tll_oracleNew (t_rand48 *seed);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
