
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define MESSAGE_HASH_SIZE   1024                            /* Must be a power of two. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *message_hashTable[MESSAGE_HASH_SIZE];      /* Global. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void bindlist_setup     (void);
void bindlist_destroy   (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* < http://www.cse.yorku.ca/~oz/hash.html > */

t_symbol *generateSymbol (const char *s, t_symbol *alreadyAllocatedSymbol)
{
    t_symbol *sym   = NULL;
    t_symbol **next = NULL;
    unsigned int hash = 5381;
    size_t length = 0;
    const char *s2 = s;
    
    PD_ASSERT (s != NULL);
    
    while (*s2) {
        hash = ((hash << 5) + hash) + *s2;      /* Bernstein djb2 hash algorithm. */
        length++;
        s2++;
    }
    
    PD_ASSERT (length < PD_STRING);             /* For efficiency should be prohibited. */
    
    next = message_hashTable + (hash & (MESSAGE_HASH_SIZE - 1));
    
    while ((sym = *next)) {
        if (!strcmp (sym->s_name, s)) { return sym; }
        next = &sym->s_next;
    }
    
    if (alreadyAllocatedSymbol) { sym = alreadyAllocatedSymbol; }
    else {
        sym = (t_symbol *)PD_MEMORY_GET (sizeof (t_symbol));
        sym->s_name  = (char *)PD_MEMORY_GET (length + 1);
        sym->s_next  = NULL;
        sym->s_thing = NULL;
        strcpy (sym->s_name, s);
    }
    
    *next = sym; 
    
    return sym;
}

t_symbol *gensym (const char *s)
{
    return generateSymbol (s, NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void message_initialize (void)
{
    t_symbol *symbols[11] =
        { 
            &s_pointer,
            &s_float,
            &s_symbol,
            &s_bang,
            &s_list,
            &s_anything,
            &s_signal,
            &s__N,
            &s__X,
            &s__A,
            &s_
        };
    
    int i;
    for (i = 0; i < MESSAGE_HASH_SIZE; i++) { PD_ASSERT (message_hashTable[i] == NULL); }
    for (i = 0; i < 11; i++) { generateSymbol (symbols[i]->s_name, symbols[i]); }
    
    symbols_initialize();
    
    PD_ASSERT (sym___hash__N == &s__N);
    PD_ASSERT (sym___hash__X == &s__X);
    PD_ASSERT (sym___hash__A == &s__A);
    
    bindlist_setup();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static int message_isStaticSymbol (t_symbol *s)
{
    static t_symbol *symbols[11] =
        { 
            &s_pointer,
            &s_float,
            &s_symbol,
            &s_bang,
            &s_list,
            &s_anything,
            &s_signal,
            &s__N,
            &s__X,
            &s__A,
            &s_
        };
    
    int i;
    for (i = 0; i < 11; i++) { if (s == symbols[i]) { return 1; } }
    
    return 0;
}

void message_release (void)
{
    t_symbol *sym1 = NULL;
    t_symbol *sym2 = NULL;
    
    int i;
    
    for (i = 0; i < MESSAGE_HASH_SIZE; i++) {
    //
    sym1 = message_hashTable[i];
    
    while ((sym2 = sym1)) {
    //
    sym1 = sym2->s_next;
    
    if (sym2->s_thing) {
        t_class *c = pd_class (sym2->s_thing);
        if (c == bindlist_class || c == template_class) { pd_free (sym2->s_thing); }
        else {
            PD_BUG; post_log ("%s", sym2->s_name);
        }
    }
    //
    }
    //
    }
    
    for (i = 0; i < MESSAGE_HASH_SIZE; i++) {
    //
    sym1 = message_hashTable[i];
    
    while ((sym2 = sym1)) {
    //
    sym1 = sym2->s_next;
    
    if (!message_isStaticSymbol (sym2)) { PD_MEMORY_FREE (sym2->s_name); PD_MEMORY_FREE (sym2); }
    //
    }
    //
    }
    
    bindlist_destroy();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
