
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void tll_itemsSetAtIndex (tll_items *items, int i)
{
    int j = i / TLL_ITEMS_ELEMENTS_SIZE;
    int t = i % TLL_ITEMS_ELEMENTS_SIZE;
    
    items->i_items[j] |= (1U << t);
}

void tll_itemsUnsetAtIndex (tll_items *items, int i) 
{
    int j = i / TLL_ITEMS_ELEMENTS_SIZE;
    int t = i % TLL_ITEMS_ELEMENTS_SIZE;
    
    items->i_items[j] &= ~(1U << t);
}

void tll_itemsClear (tll_items *items)  
{
    int j; for (j = 0; j < TLL_ITEMS_ELEMENTS; j++) { items->i_items[j] = 0U; }
}

/* < http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan > */

int tll_itemsCount (const tll_items *items)
{
    int j, k = 0;
    
    for (j = 0; j < TLL_ITEMS_ELEMENTS; j++) {
        uint32_t n = items->i_items[j];
            
        while (n != 0U) {
            k++;
            n &= n - 1U;
        }
    }
    
    return k;
}

int tll_itemsIsSetAtIndex (const tll_items *items, int i)
{
    uint32_t k = 0U;

    int j = i / TLL_ITEMS_ELEMENTS_SIZE;
    int t = i % TLL_ITEMS_ELEMENTS_SIZE;
    
    k = items->i_items[j];

    k >>= t;
    k  &= 1U;
    
    return (k != 0U);
}

void tll_itemsUnion (const tll_items *itemsA, const tll_items *itemsB, tll_items *r) 
{
    int j;
    
    for (j = 0; j < TLL_ITEMS_ELEMENTS; j++) {
        r->i_items[j] = itemsA->i_items[j] | itemsB->i_items[j];
    }
}

void tll_itemsIntersection (const tll_items *itemsA, const tll_items *itemsB, tll_items *r) 
{
    int j;
    
    for (j = 0; j < TLL_ITEMS_ELEMENTS; j++) {
        r->i_items[j] = itemsA->i_items[j] & itemsB->i_items[j];
    }
}

int tll_itemsIsIncluded (const tll_items *itemsA, const tll_items *itemsB)
{
    int j, k = 1;
            
    for (j = 0; j < TLL_ITEMS_ELEMENTS; j++) {
        if (itemsB->i_items[j] != (itemsB->i_items[j] | itemsA->i_items[j])) {
            k = 0;
            break;
        }
    }
        
    return k;
}

int tll_itemsIsEqual (const tll_items *itemsA, const tll_items *itemsB)
{
    int j, k = 1;
            
    for (j = 0; j < TLL_ITEMS_ELEMENTS; j++) {
        if (itemsA->i_items[j] != itemsB->i_items[j]) {
            k = 0;
            break;
        }
    }
        
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
