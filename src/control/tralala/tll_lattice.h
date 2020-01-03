
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < http://en.wikipedia.org/wiki/Formal_concept_analysis > */
/* < http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.39.4239&rep=rep1&type=pdf > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tll_latticeNode {
    int                     l_cardinal;
    tll_items               l_items;
    tll_items               l_parents;
    tll_items               l_childs;
    } tll_latticeNode;

typedef struct _tll_lattice {
    tll_algorithm           l_table;                    /* Must be the first. */
    tll_items               l_toBeAdded;
    tll_items               l_intersection;
    int                     l_count;
    int                     l_threshold;
    int                     l_targetedConcept;
    int                     l_shuttle;
    int                     l_previousShuttle;
    int                     l_intersectionCardinal;
    int                     l_mapPeak;
    int                     l_tempMapPeak;
    int                     l_needToMakeMap;
    tll_stack               *l_tickets;
    tll_latticeNode         *l_pool;
    tll_array               **l_map;
    tll_array               **l_tempMap;
    t_rand48                *l_seed;
    } tll_lattice;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_algorithm *tll_latticeNew (t_rand48 *seed);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
