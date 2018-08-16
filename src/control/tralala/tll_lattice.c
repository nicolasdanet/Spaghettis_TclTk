
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_LATTICE_THRESHOLD   50      /* Must be less than TLL_ITEMS_SIZE. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tll_latticeReconnect (tll_lattice *x, int g, int n)
{
    int i, j;
    
    for (i = 0; i < x->l_intersectionCardinal; i++) {
    //
    for (j = 0; j < tll_arrayGetSize (x->l_tempMap[i]); j++) {
    //
    int q = tll_arrayGetAtIndex (x->l_tempMap[i], j);
    
    if (tll_itemsIsIncluded (&(x->l_pool[q].l_items), &(x->l_intersection))) {
    //
    int t, isParentOfGenerator = 1;
    
    for (t = 0; t < TLL_ITEMS_SIZE; t++) {
        if (tll_itemsIsSetAtIndex (&(x->l_pool[q].l_childs), t)) {
            if (tll_itemsIsIncluded (&(x->l_pool[t].l_items), &(x->l_intersection))) {
                isParentOfGenerator = 0;
                break;
            }
        }
    }
    
    if (isParentOfGenerator) {
    //
    tll_itemsUnsetAtIndex (&(x->l_pool[g].l_parents), q);
    tll_itemsUnsetAtIndex (&(x->l_pool[q].l_childs), g);
    tll_itemsSetAtIndex (&(x->l_pool[n].l_parents), q);
    tll_itemsSetAtIndex (&(x->l_pool[q].l_childs), n);
    //
    }
    //
    }
    //
    }
    //
    }
}

static void tll_latticeMakeMap (tll_lattice *x)
{
    int k = -1;
    int i, j = 0;
    
    x->l_targetedConcept = -1;
    
    if (x->l_count) {
        k = tll_randomGetInteger (x->l_seed, x->l_count);
    }
    
    for (i = 1; i <= x->l_mapPeak; i++) { tll_arrayClear (x->l_map[i]); }
    
    x->l_mapPeak = 0;
    
    for (i = 2; i < TLL_ITEMS_SIZE; i++) {
    //
    if (x->l_pool[i].l_cardinal) {
    //
    if (x->l_pool[i].l_cardinal > x->l_mapPeak) { x->l_mapPeak = x->l_pool[i].l_cardinal; }
    tll_arrayAppend (x->l_map[x->l_pool[i].l_cardinal], i);
    if (k == j) { x->l_targetedConcept = i; }
    j++;
    //
    }
    //
    }
}

static void tll_latticeKillConcept (tll_lattice *x, int n)
{
    if (n > 1) {
    //
    int i;
        
    for (i = 0; i < TLL_ITEMS_SIZE; i++) {
    //
    if (tll_itemsIsSetAtIndex (&(x->l_pool[n].l_parents), i)) {
        tll_itemsUnsetAtIndex (&(x->l_pool[i].l_childs), n);
        tll_itemsUnsetAtIndex (&(x->l_pool[n].l_parents), i);
        
        if (tll_itemsCount (&(x->l_pool[i].l_childs)) == 0) {
            tll_latticeKillConcept (x, i);
        }
    }
    
    if (tll_itemsIsSetAtIndex (&(x->l_pool[n].l_childs), i)) {
        tll_itemsUnsetAtIndex (&(x->l_pool[i].l_parents), n);
        tll_itemsUnsetAtIndex (&(x->l_pool[n].l_childs), i);
        
        if (tll_itemsCount (&(x->l_pool[i].l_parents)) == 0) {
            tll_latticeKillConcept (x, i);
        }
    }
    //
    }

    x->l_pool[n].l_cardinal = 0;
    tll_itemsClear (&(x->l_pool[n].l_items));
        
    x->l_count--;
    tll_stackAppend (x->l_tickets, n);
    tll_stackSort (x->l_tickets);         /* Sorting down to get conservative serialization. */
    
    if (x->l_shuttle == n) {
        x->l_shuttle = -1;
        x->l_previousShuttle = -1;
    }
    //
    }
}

static void tll_latticeStep (tll_lattice *x)
{
    int i, j, t;
    
    tll_arrayClear (x->l_tempMap[0]);
    tll_arrayClear (x->l_tempMap[TLL_ITEMS_SIZE]);
    
    for (i = 1; i <= x->l_tempMapPeak; i++) { tll_arrayClear (x->l_tempMap[i]); }
    
    x->l_tempMapPeak = 0;
    
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) {
    //
    for (j = 0; j < tll_arrayGetSize (x->l_map[i]); j++) {
    //
    int g = tll_arrayGetAtIndex (x->l_map[i], j);
    
    if (tll_itemsIsEqual (&(x->l_pool[g].l_items), &(x->l_toBeAdded))) {
        goto end;
        
    } else if (tll_itemsIsIncluded (&(x->l_pool[g].l_items), &(x->l_toBeAdded))) {
        tll_arrayAppend (x->l_tempMap[i], g);
        if (i > x->l_tempMapPeak) { x->l_tempMapPeak = i; }
        
    } else {
    //
    int isGenerator = 0;
    
    tll_itemsIntersection (&(x->l_pool[g].l_items), &(x->l_toBeAdded), &(x->l_intersection));
        
    if ((x->l_intersectionCardinal = tll_itemsCount (&(x->l_intersection)))) {
    //
    isGenerator = 1;
    
    for (t = 0; t < tll_arrayGetSize (x->l_tempMap[x->l_intersectionCardinal]); t++) {
        int q = tll_arrayGetAtIndex (x->l_tempMap[x->l_intersectionCardinal], t);
        if (tll_itemsIsEqual (&(x->l_pool[q].l_items), &(x->l_intersection))) {
            isGenerator = 0;
        }
    }
    //
    }
    
    if (isGenerator) {
    //
    if (!(tll_stackPop (x->l_tickets))) {
    //
    int n = tll_stackGetPopped (x->l_tickets);
    
    tll_arrayAppend (x->l_tempMap[x->l_intersectionCardinal], n);

    x->l_pool[n].l_cardinal = x->l_intersectionCardinal;
    x->l_pool[n].l_items = x->l_intersection;

    x->l_count++;
    x->l_needToMakeMap = 1;

    if (x->l_intersectionCardinal > x->l_tempMapPeak) {
        x->l_tempMapPeak = x->l_intersectionCardinal;
    }
    
    tll_itemsSetAtIndex (&(x->l_pool[n].l_childs), g);
    tll_itemsSetAtIndex (&(x->l_pool[g].l_parents), n);
    
    tll_latticeReconnect (x, g, n);
    
    if (tll_itemsIsEqual (&(x->l_intersection), &(x->l_toBeAdded))) {
        goto end;
    }
    //
    }
    //
    }
    //
    }
    //
    }
    //    
    }
        
end:
    
    if (x->l_needToMakeMap) {
    //
    tll_latticeMakeMap (x);
    
    while (x->l_count > x->l_threshold) {
        tll_latticeKillConcept (x, x->l_targetedConcept);
        tll_latticeMakeMap (x);
    }

    if (!x->l_count) {
        tll_itemsSetAtIndex (&(x->l_pool[0].l_childs), 1);
        tll_itemsSetAtIndex (&(x->l_pool[1].l_parents), 0);
    }
    
    x->l_needToMakeMap = 0;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tll_latticeFree (tll_algorithm *vPtr)
{
    tll_lattice *x = (tll_lattice *)vPtr;
    
    int i;
    
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) { tll_arrayFree (x->l_tempMap[i]); }
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) { tll_arrayFree (x->l_map[i]); }
    
    PD_MEMORY_FREE (x->l_tempMap);
    PD_MEMORY_FREE (x->l_map);
    PD_MEMORY_FREE (x->l_pool);
    
    tll_stackFree (x->l_tickets);

    PD_MEMORY_FREE (x);
}

static t_error tll_latticeAdd (tll_algorithm *vPtr, int argc, int *argv)
{
    tll_lattice *x = (tll_lattice *)vPtr;
    
    t_error err = PD_ERROR;
    
    tll_itemsClear (&(x->l_toBeAdded));
    
    if ((argc > 0) && argv) {
    //
    int t;
    
    err = PD_ERROR_NONE;
    
    for (t = 0; t < argc; t++) {
        if (!(err |= (argv[t] < 0) || (argv[t] >= TLL_ITEMS_SIZE))) {   // == TLL_ALPHABET_SIZE
            tll_itemsSetAtIndex (&(x->l_toBeAdded), argv[t]);
        }
    }
    //
    }

    if (!err) { tll_latticeStep (x); }

    return err;
}

static void tll_latticeClear (tll_algorithm *vPtr)
{
    tll_lattice *x = (tll_lattice *)vPtr;
    
    int i;
    
    for (i = 1; i <= x->l_mapPeak; i++) { tll_arrayClear (x->l_map[i]); }
    
    x->l_mapPeak = 0;
    
    tll_itemsClear (&(x->l_pool[0].l_childs));
    tll_itemsClear (&(x->l_pool[1].l_parents));
    
    tll_itemsSetAtIndex (&(x->l_pool[0].l_childs), 1);
    tll_itemsSetAtIndex (&(x->l_pool[1].l_parents), 0);
    
    for (i = 2; i < TLL_ITEMS_SIZE; i++) {
    //
    x->l_pool[i].l_cardinal = 0;
    tll_itemsClear (&(x->l_pool[i].l_items));
    tll_itemsClear (&(x->l_pool[i].l_childs));
    tll_itemsClear (&(x->l_pool[i].l_parents));
    //
    }
    
    tll_stackClear (x->l_tickets);
    
    for (i = (TLL_ITEMS_SIZE - 1); i > 1; i--) { tll_stackAppend (x->l_tickets, i); }
    
    x->l_count = 0;
    x->l_shuttle = -1;
    x->l_previousShuttle = -1;
}

static t_error tll_latticeProceed (tll_algorithm *vPtr, int argc, int *argv)
{
    tll_lattice *x = (tll_lattice *)vPtr;
    
    t_error err = PD_ERROR;
    
    if (((argc > 0) && argv) && (x->l_count > 0)) {
    //
    int k = 0;
        
    if (x->l_shuttle == -1) {
        x->l_shuttle = x->l_targetedConcept;
        x->l_previousShuttle = x->l_targetedConcept;
    }

    while (argc) {
    //
    int i, n, indexConnections = 0;
    int connections[TLL_ITEMS_SIZE];
    
    if (x->l_shuttle > 1) {
    //
    int j, t = tll_randomGetInteger (x->l_seed, TLL_ITEMS_SIZE) + TLL_ITEMS_SIZE;
    int temp, left = t % 2;
        
    for (j = 0; j < TLL_ITEMS_SIZE; j++) {
    //
    if (argc) {
    //
    if (left) {
        temp = (t - j) % TLL_ITEMS_SIZE;
    } else {
        temp = (t + j) % TLL_ITEMS_SIZE;
    }
    
    if (tll_itemsIsSetAtIndex (&(x->l_pool[x->l_shuttle].l_items), temp)) {
        argv[k] = temp;
        argc--;
        k++;
    }
    //
    } else {
        break;
    }
    //
    }
    //
    }
    
    for (i = 0; i < TLL_ITEMS_SIZE; i++) {
        int child  = tll_itemsIsSetAtIndex (&(x->l_pool[x->l_shuttle].l_childs), i);
        int parent = tll_itemsIsSetAtIndex (&(x->l_pool[x->l_shuttle].l_parents), i);
        if (child || parent) {
            connections[indexConnections] = i; indexConnections++;
        }
    }
    
    if (x->l_shuttle < 2) {
        x->l_previousShuttle = x->l_shuttle;
        x->l_shuttle = connections[tll_randomGetInteger (x->l_seed, indexConnections)];
        
    } else {
        do {
        n = connections[tll_randomGetInteger (x->l_seed, indexConnections)];
        } while (n == x->l_previousShuttle);
        
        x->l_previousShuttle = x->l_shuttle;
        x->l_shuttle = n;
    }
    //
    }
    
    err = PD_ERROR_NONE;
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tll_latticeSerialize (tll_algorithm *vPtr, tll_array *a)
{
    tll_lattice *x = (tll_lattice *)vPtr;
    
    int i, j, t, k = 0;
    
    tll_arrayAppend (a, x->l_targetedConcept);                                  /* Attribute. */
    tll_arrayAppend (a, x->l_threshold);                                        /* Attribute. */
    tll_arrayAppend (a, x->l_shuttle);                                          /* Attribute. */
    tll_arrayAppend (a, x->l_previousShuttle);                                  /* Attribute. */
    
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) {
    //
    for (j = 0; j < tll_arrayGetSize (x->l_map[i]); j++) {
    //
    k++;
    //
    }
    //
    }
    
    tll_arrayAppend (a, k);                                                     /* Number of nodes. */
    
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) {
    //
    for (j = 0; j < tll_arrayGetSize (x->l_map[i]); j++) {
    //
    int index = tll_arrayGetAtIndex (x->l_map[i], j);
    
    tll_arrayAppend (a, index);                                                 /* Index of the node. */
    tll_arrayAppend (a, tll_itemsCount (&(x->l_pool[index].l_items)));          /* Number of items. */
    tll_arrayAppend (a, tll_itemsCount (&(x->l_pool[index].l_childs)));         /* Number of childs. */
    
    for (t = 0; t < TLL_ITEMS_SIZE; t++) {
        if (tll_itemsIsSetAtIndex (&(x->l_pool[index].l_items), t))  {
            tll_arrayAppend (a, t);                                             /* Value of item. */
        }
    }
    
    for (t = 0; t < TLL_ITEMS_SIZE; ++t) {
        if (tll_itemsIsSetAtIndex (&(x->l_pool[index].l_childs), t)) {
            tll_arrayAppend (a, t);                                             /* Index of child. */
        }
    }
    //
    }
    //
    }
}

t_error tll_latticeDeserialize (tll_algorithm *vPtr, tll_array *a)
{
    tll_lattice *x = (tll_lattice *)vPtr;
    
    int j, data = 0;
    int targetedConcept = tll_arrayGetAtIndex (a, data++);
    int threshold       = tll_arrayGetAtIndex (a, data++);
    int shuttle         = tll_arrayGetAtIndex (a, data++);
    int previousShuttle = tll_arrayGetAtIndex (a, data++);
    
    t_error err = (x->l_threshold != threshold);
    
    if (!err) {
    //
    
    /* Reset. */
    
    x->l_count   = 0;
    x->l_mapPeak = 0;
    
    for (j = 1; j < TLL_ITEMS_SIZE; j++) { tll_arrayClear (x->l_map[j]); }
    
    for (j = 0; j < TLL_ITEMS_SIZE; j++) {
        x->l_pool[j].l_cardinal = 0;
        tll_itemsClear (&(x->l_pool[j].l_items));
        tll_itemsClear (&(x->l_pool[j].l_childs));
        tll_itemsClear (&(x->l_pool[j].l_parents));
    }
    
    /* Attributes. */
    
    x->l_targetedConcept = targetedConcept;
    x->l_threshold       = threshold;
    x->l_shuttle         = shuttle;
    x->l_previousShuttle = previousShuttle;
    
    /* Nodes. */
    
    {
    //
    int exists[TLL_ITEMS_SIZE] = { 0 };
    int nodes = tll_arrayGetAtIndex (a, data++);
    int t;
    
    for (j = 0; j < nodes; j++) {
    //
    int index  = tll_arrayGetAtIndex (a, data++);       /* Index of the node. */
    int items  = tll_arrayGetAtIndex (a, data++);       /* Number of items. */
    int childs = tll_arrayGetAtIndex (a, data++);       /* Number of childs. */

    x->l_pool[index].l_cardinal = items;
    
    if ((items > 0) && (items < TLL_ITEMS_SIZE)) {
        if (items > x->l_mapPeak) { x->l_mapPeak = items; }
        tll_arrayAppend (x->l_map[items], index);
        x->l_count++;
    }
    
    /* Items. */
    
    for (t = 0; t < items; t++) {
        int k = tll_arrayGetAtIndex (a, data++);
        tll_itemsSetAtIndex (&(x->l_pool[index].l_items), k);
    }
    
    /* Edges. */
    
    for (t = 0; t < childs; t++) {
        int k = tll_arrayGetAtIndex (a, data++);
        tll_itemsSetAtIndex (&(x->l_pool[index].l_childs), k);
        tll_itemsSetAtIndex (&(x->l_pool[k].l_parents), index);
    }
    
    exists[index] = 1;
    //
    }
    
    /* Tickets. */
    
    tll_stackClear (x->l_tickets);
    
    for (j = (TLL_ITEMS_SIZE - 1); j > 1; j--) {
        if (exists[j] == 0) { tll_stackAppend (x->l_tickets, j); }
    }
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

tll_algorithm *tll_latticeNew (t_rand48 *seed)
{
    tll_lattice *x = (tll_lattice *)PD_MEMORY_GET (sizeof (tll_lattice));

    int i;
    
    x->l_table.f_free        = tll_latticeFree;
    x->l_table.f_add         = tll_latticeAdd;
    x->l_table.f_clear       = tll_latticeClear;
    x->l_table.f_proceed     = tll_latticeProceed;
    x->l_table.f_serialize   = tll_latticeSerialize;
    x->l_table.f_deserialize = tll_latticeDeserialize;
    
    x->l_threshold           = TLL_LATTICE_THRESHOLD;
    x->l_tickets             = tll_stackNew ((TLL_ITEMS_SIZE - 2));
    x->l_pool                = (tll_latticeNode *)PD_MEMORY_GET (sizeof (tll_latticeNode) * TLL_ITEMS_SIZE);
    x->l_map                 = (tll_array **)PD_MEMORY_GET (sizeof (tll_array *) * (TLL_ITEMS_SIZE + 1));
    x->l_tempMap             = (tll_array **)PD_MEMORY_GET (sizeof (tll_array *) * (TLL_ITEMS_SIZE + 1));
    x->l_targetedConcept     = -1;
    x->l_shuttle             = -1;
    x->l_previousShuttle     = -1;
    x->l_seed                = seed;
    
    for (i = 0; i < TLL_ITEMS_SIZE; i++) {
        tll_itemsSetAtIndex (&(x->l_pool[1].l_items), i);
    }
    
    x->l_pool[0].l_cardinal = 0;
    x->l_pool[1].l_cardinal = TLL_ITEMS_SIZE;
    
    tll_itemsSetAtIndex (&(x->l_pool[0].l_childs), 1);
    tll_itemsSetAtIndex (&(x->l_pool[1].l_parents), 0);
    
    for (i = (TLL_ITEMS_SIZE - 1); i > 1; i--) { tll_stackAppend (x->l_tickets, i);  }
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) { x->l_map[i] = tll_arrayNew (0);     }
    for (i = 0; i < (TLL_ITEMS_SIZE + 1); i++) { x->l_tempMap[i] = tll_arrayNew (0); }

    tll_arrayAppend (x->l_map[0], 0);
    tll_arrayAppend (x->l_map[TLL_ITEMS_SIZE], 1);
    
    return (tll_algorithm *)x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
