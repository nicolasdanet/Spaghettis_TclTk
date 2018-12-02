
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define TLL_ITEMS_SIZE              128
#define TLL_ITEMS_ELEMENTS          4
#define TLL_ITEMS_ELEMENTS_SIZE     32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _tll_items { uint32_t i_items[TLL_ITEMS_ELEMENTS]; } tll_items;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void    tll_itemsSetAtIndex         (tll_items *items, int i);
void    tll_itemsUnsetAtIndex       (tll_items *items, int i);
void    tll_itemsClear              (tll_items *items);
int     tll_itemsCount              (const tll_items *items);
int     tll_itemsIsSetAtIndex       (const tll_items *items, int i);

void    tll_itemsUnion              (const tll_items *itemsA, const tll_items *itemsB, tll_items *r);
void    tll_itemsIntersection       (const tll_items *itemsA, const tll_items *itemsB, tll_items *r);
int     tll_itemsIsIncluded         (const tll_items *itemsA, const tll_items *itemsB);
int     tll_itemsIsEqual            (const tll_items *itemsA, const tll_items *itemsB);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
