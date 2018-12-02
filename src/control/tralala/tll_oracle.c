
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tll_oracleClear (tll_algorithm *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_ORACLE_STRAIGHT         25
#define TLL_ORACLE_BACKWARD         2
#define TLL_ORACLE_THRESHOLD        255
#define TLL_ORACLE_PERSISTENCE      128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tll_oracleStep (tll_oracle *x, int n)
{
    int q = x->o_index;
    
    if (q == x->o_size) {
    //
    size_t oldSize = x->o_size * sizeof (tll_oracleNode);
    size_t newSize = oldSize * 2;
    x->o_nodes = (tll_oracleNode *)PD_MEMORY_RESIZE (x->o_nodes, oldSize, newSize);
    x->o_size *= 2;
    //
    }
    
    x->o_nodes[q].o_refer = -1;
    x->o_nodes[q].o_lrs   = 0;
    
    if (q == x->o_peak) {
        x->o_nodes[q].o_values       = tll_arrayNew (0);
        x->o_nodes[q].o_destinations = tll_arrayNew (0);
        x->o_peak++;
        
    } else {
        tll_arrayClear (x->o_nodes[q].o_values);
        tll_arrayClear (x->o_nodes[q].o_destinations);
    }
                
    {
    //
    int t, j, w = 0;
    int suffix = 0;

    tll_arrayAppend (x->o_nodes[(q - 1)].o_values, n);
    tll_arrayAppend (x->o_nodes[(q - 1)].o_destinations, q);
    
    t = q - 1;
    j = x->o_nodes[(q - 1)].o_refer;
    
    while ((j > -1) && !(tll_arrayContainsValue (x->o_nodes[j].o_values, n))) {
        tll_arrayAppend (x->o_nodes[j].o_values, n);
        tll_arrayAppend (x->o_nodes[j].o_destinations, q);
        t = j;
        j = x->o_nodes[j].o_refer;
    }
    
    if (j != -1) {   
        int destIndex;
        if (!(tll_arrayIndexOfValue (x->o_nodes[j].o_values, n, &destIndex))) {
            w = tll_arrayGetAtIndex (x->o_nodes[j].o_destinations, destIndex);
        }
    }
        
    x->o_nodes[q].o_refer = w;
        
    if (w != 0) {
    //
    tll_oracleNode *ptrA = x->o_nodes + t;
    tll_oracleNode *ptrB = x->o_nodes + (w - 1);
    
    if ((w - 1) == ptrA->o_refer) {
        suffix = ptrA->o_lrs + 1;
        
    } else {
        while (ptrB->o_refer != ptrA->o_refer) {
            ptrB = x->o_nodes + ptrB->o_refer;
        }
        suffix = PD_MIN (ptrA->o_lrs, ptrB->o_lrs) + 1;
    }
    //
    }
        
    x->o_nodes[q].o_lrs = suffix;
    x->o_index++;
    //
    }
}

static void tll_oracleSkip (tll_oracle *x)
{
    int i;
    int m = x->o_index - (x->o_persistence + 1);
    int n = (x->o_index - 1);
    
    tll_arrayClear (x->o_temp);
    for (i = m; i < n; i++) { tll_arrayAppend (x->o_temp, tll_arrayGetAtIndex (x->o_nodes[i].o_values, 0)); }
    
    tll_oracleClear ((tll_algorithm *)x);

    for (i = 0; i < tll_arrayGetSize (x->o_temp); i++) {
        tll_oracleStep (x, tll_arrayGetAtIndex (x->o_temp, i));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void tll_oracleFree (tll_algorithm *vPtr)
{
    tll_oracle *x = (tll_oracle *)vPtr;
    
    int i;
    
    for (i = 0; i < x->o_peak; i++) {
        tll_arrayFree (x->o_nodes[i].o_values);
        tll_arrayFree (x->o_nodes[i].o_destinations);
    }

    PD_MEMORY_FREE (x->o_nodes);
    
    tll_arrayFree (x->o_temp);
    
    PD_MEMORY_FREE (x);
}

static t_error tll_oracleAdd (tll_algorithm *vPtr, int argc, int *argv)
{
    tll_oracle *x = (tll_oracle *)vPtr;
    
    int t;
    t_error err = PD_ERROR;
    
    if ((argc > 0) && argv) {
    //
    err = PD_ERROR_NONE;
    
    for (t = 0; t < argc; t++) { err |= (argv[t] < 0) || (argv[t] >= TLL_ALPHABET); }
    
    if (!err) {
    //
    for (t = 0; t < argc; t++) {
        if (x->o_threshold && (x->o_index > x->o_threshold)) { tll_oracleSkip (x); }
        tll_oracleStep (x, argv[t]);
    }
    //
    }
    //
    }
    
    return err;
}

static void tll_oracleClear (tll_algorithm *vPtr)
{
    tll_oracle *x = (tll_oracle *)vPtr;
    
    x->o_index   = 1;
    x->o_shuttle = 0;
    
    x->o_nodes[0].o_refer = -1;
    x->o_nodes[0].o_lrs   = 0;
    
    tll_arrayClear (x->o_nodes[0].o_values);
    tll_arrayClear (x->o_nodes[0].o_destinations);
}

static t_error tll_oracleProceed (tll_algorithm *vPtr, int argc, int *argv)
{
    tll_oracle *x = (tll_oracle *)vPtr;
    
    t_error err = PD_ERROR;
    
    if (((argc > 0) && argv) && (x->o_index > 1)) {
    //
    int k = 0;
    err = PD_ERROR_NONE;
    
    while (argc) {
    //
    int t = 0;
    double h = tll_randomGetDouble (x->o_seed);
    tll_oracleNode *q = NULL;
    
    if (x->o_shuttle == (x->o_index - 1)) {
        x->o_shuttle = 0;
    }
    
    q = x->o_nodes + x->o_shuttle;
    
    if (h > (x->o_straight / 100.0)) {
    //
    if ((h > (((x->o_straight / 100.0) + 1.0) / 2.0)) && (q->o_lrs > x->o_backward)) {
        x->o_shuttle = q->o_refer;
        q = x->o_nodes + x->o_shuttle;
        
    } else if (tll_arrayGetSize (q->o_values) > 1) {
        int i = tll_randomGetInteger (x->o_seed, (tll_arrayGetSize (q->o_values) - 1));
        argv[k] = tll_arrayGetAtIndex (q->o_values, (i + 1));
        x->o_shuttle = tll_arrayGetAtIndex (q->o_destinations, (i + 1));
        t = 1;
    }
    //
    }
    
    if (!t) {
        argv[k] = tll_arrayGetAtIndex (q->o_values, 0);
        x->o_shuttle++;
    }
    
    argc--;
    k++;
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tll_oracleSerialize (tll_algorithm *vPtr, tll_array *a)
{
    tll_oracle *x = (tll_oracle *)vPtr;
    
    int i, j;
    
    tll_arrayAppend (a, x->o_shuttle);                                              /* Attribute. */
    tll_arrayAppend (a, x->o_backward);                                             /* Attribute. */
    tll_arrayAppend (a, x->o_threshold);                                            /* Attribute. */
    tll_arrayAppend (a, x->o_persistence);                                          /* Attribute. */
    tll_arrayAppend (a, x->o_straight);                                             /* Attribute. */
    
    tll_arrayAppend (a, x->o_index);                                                /* Number of nodes. */
    
    for (i = 0; i < x->o_index; i++) {
    //
    int refer = x->o_nodes[i].o_refer;                                              /* Attribute of node. */
    int lrs   = x->o_nodes[i].o_lrs;                                                /* Attribute of node. */
    int arcs  = tll_arrayGetSize (x->o_nodes[i].o_destinations);                    /* Number of arcs. */
    
    tll_arrayAppend (a, refer);
    tll_arrayAppend (a, lrs);
    tll_arrayAppend (a, arcs);
    
    for (j = 0; j < arcs; j++) {
        int destination = tll_arrayGetAtIndex (x->o_nodes[i].o_destinations, j);    /* Destination of arc. */
        int value       = tll_arrayGetAtIndex (x->o_nodes[i].o_values, j);          /* Value of arc. */
        tll_arrayAppend (a, destination);
        tll_arrayAppend (a, value);
    }
    //
    }
}

t_error tll_oracleDeserialize (tll_algorithm *vPtr, tll_array *a)
{
    tll_oracle *x = (tll_oracle *)vPtr;
    
    t_error err = PD_ERROR_NONE;
    
    int i, j, data = 0;
    
    /* Attributes. */
    
    int shuttle     = tll_arrayGetAtIndex (a, data++);
    int backward    = tll_arrayGetAtIndex (a, data++);
    int threshold   = tll_arrayGetAtIndex (a, data++);
    int persistence = tll_arrayGetAtIndex (a, data++);
    int straight    = tll_arrayGetAtIndex (a, data++);
    int nodes       = tll_arrayGetAtIndex (a, data++);
    
    err |= (x->o_threshold != threshold);
    err |= (x->o_persistence != persistence);
    
    if (!err) {
    //
    
    /* Reset. */
    
    tll_oracleClear ((tll_algorithm *)x);

    /* Attributes. */
    
    x->o_shuttle  = shuttle;
    x->o_backward = backward;
    x->o_straight = straight;
    
    /* Nodes. */
    
    for (i = 0; i < nodes; i++) {
    //
    int n, v;
    
    data++;
    data++;
    
    n = tll_arrayGetAtIndex (a, data++);
    
    for (j = 0; j < n; j++) {
        data++; v = tll_arrayGetAtIndex (a, data++); if (j == 0) { tll_oracleStep (x, v); }
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

tll_algorithm *tll_oracleNew (t_rand48 *seed)
{
    tll_oracle *x = (tll_oracle *)PD_MEMORY_GET (sizeof (tll_oracle));

    x->o_table.f_free               = tll_oracleFree;
    x->o_table.f_add                = tll_oracleAdd;
    x->o_table.f_clear              = tll_oracleClear;
    x->o_table.f_proceed            = tll_oracleProceed;
    x->o_table.f_serialize          = tll_oracleSerialize;
    x->o_table.f_deserialize        = tll_oracleDeserialize;
    
    x->o_size                       = TLL_ORACLE_THRESHOLD + 1;
    x->o_peak                       = 0;
    x->o_threshold                  = TLL_ORACLE_THRESHOLD;
    x->o_persistence                = TLL_ORACLE_PERSISTENCE;
    x->o_temp                       = tll_arrayNew (x->o_persistence);
    x->o_nodes                      = (tll_oracleNode *)PD_MEMORY_GET (x->o_size * sizeof (tll_oracleNode));
    x->o_peak                       = 1;
    x->o_index                      = 1;
    x->o_shuttle                    = 0;
    x->o_backward                   = TLL_ORACLE_BACKWARD;
    x->o_straight                   = TLL_ORACLE_STRAIGHT;
    x->o_seed                       = seed;
    
    x->o_nodes[0].o_values          = tll_arrayNew (0);
    x->o_nodes[0].o_destinations    = tll_arrayNew (0);
    x->o_nodes[0].o_refer           = -1;
    x->o_nodes[0].o_lrs             = 0;
    
    return (tll_algorithm *)x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
