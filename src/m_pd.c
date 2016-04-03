
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _gstack {
    t_pd            *g_what;                    /* MUST be the first. */
    t_symbol        *g_loadingAbstraction;
    struct _gstack  *g_next;
    } t_gstack;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class  *bindlist_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd *pd_newest;                                /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd pd_objectMaker;                            /* Shared. */
t_pd pd_canvasMaker;                            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_gstack *pd_stackHead;                  /* Shared. */
static t_pd     *pd_lastPopped;                 /* Shared. */
static t_symbol *pd_loadingAbstraction;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd *pd_new (t_class *c)
{
    t_pd *x;
    
    PD_ASSERT (c != NULL);

    x = (t_pd *)PD_MEMORY_GET (c->c_size);
    
    *x = c;
    
    if (c->c_isBox) { cast_object (x)->te_inlet = cast_object (x)->te_outlet = NULL; }
    
    return x;
}

void pd_free (t_pd *x)
{
    t_class *c = pd_class (x);

    if (c->c_methodFree) { (*(c->c_methodFree))(x); }

    if (c->c_isBox) {
        while (cast_object (x)->te_outlet) { outlet_free (cast_object (x)->te_outlet); }
        while (cast_object (x)->te_inlet)  { inlet_free (cast_object (x)->te_inlet);   }
        
        if (cast_object (x)->te_buffer) { 
            buffer_free (cast_object (x)->te_buffer); 
        }
    }

    if (c->c_size) { PD_MEMORY_FREE (x); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bang (t_pd *x)
{
    (*(pd_class (x)->c_methodBang)) (x);
}

void pd_float (t_pd *x, t_float f)
{
    (*(pd_class (x)->c_methodFloat)) (x, f);
}

void pd_symbol (t_pd *x, t_symbol *s)
{
    (*(pd_class (x)->c_methodSymbol)) (x, s);
}

void pd_list (t_pd *x, int argc, t_atom *argv)
{
    (*(pd_class (x)->c_methodList)) (x, &s_list, argc, argv);
}

void pd_pointer (t_pd *x, t_gpointer *gp)
{
    (*(pd_class (x)->c_methodPointer)) (x, gp);
}

void pd_empty (t_pd *x)
{
    if (class_hasBang (pd_class (x))) { (*(pd_class (x)->c_methodBang)) (x); }
    else {
        (*(pd_class (x)->c_methodAnything)) (x, &s_bang, 0, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bind (t_pd *x, t_symbol *s)
{
    if (s->s_thing) {
    
        if (pd_class (s->s_thing) == bindlist_class) {
            t_bindlist *b = (t_bindlist *)s->s_thing;
            t_bindelement *e = (t_bindelement *)PD_MEMORY_GET (sizeof (t_bindelement));
            e->e_next = b->b_list;
            e->e_what = x;
            b->b_list = e;
            
        } else {
            t_bindlist *b = (t_bindlist *)pd_new (bindlist_class);
            t_bindelement *e1 = (t_bindelement *)PD_MEMORY_GET (sizeof (t_bindelement));
            t_bindelement *e2 = (t_bindelement *)PD_MEMORY_GET (sizeof (t_bindelement));
            b->b_list  = e1;
            e1->e_what = x;
            e1->e_next = e2;
            e2->e_what = s->s_thing;
            e2->e_next = NULL;
            s->s_thing = &b->b_pd;
        }
        
    } else {
        s->s_thing = x;
    }
}

void pd_unbind (t_pd *x, t_symbol *s)
{
    if (s->s_thing == x) { 
        s->s_thing = NULL; 
        
    } else if (s->s_thing && pd_class (s->s_thing) == bindlist_class) {
        
        t_bindlist *b = (t_bindlist *)s->s_thing;
        t_bindelement *e1 = NULL;
        t_bindelement *e2 = NULL;
        
        if ((e1 = b->b_list)->e_what == x) {
            b->b_list = e1->e_next;
            PD_MEMORY_FREE (e1);
        } else {
            for (e1 = b->b_list; e2 = e1->e_next; e1 = e2) {
                if (e2->e_what == x) {
                    e1->e_next = e2->e_next;
                    PD_MEMORY_FREE (e2);
                    break;
                }
            }
        }
        
        if (!b->b_list->e_next) {                           /* Delete it if just one element remains. */
            s->s_thing = b->b_list->e_what;
            PD_MEMORY_FREE (b->b_list);
            pd_free (&b->b_pd);
        }
        
    } else { PD_BUG; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd *pd_findByClass (t_symbol *s, t_class *c)
{
    t_pd *x = NULL;
    
    if (!s->s_thing) { return NULL; }
    if (pd_class (s->s_thing) == c) { return s->s_thing; }
    
    if (pd_class (s->s_thing) == bindlist_class) {
        t_bindlist *b = (t_bindlist *)s->s_thing;
        t_bindelement *e = NULL;
        
        for (e = b->b_list; e; e = e->e_next) {
            if (*e->e_what == c) { PD_ASSERT (x == NULL); x = e->e_what; }
        }
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_push (t_pd *x)
{
    t_gstack *p = (t_gstack *)PD_MEMORY_GET (sizeof (t_gstack));
    p->g_what = s__X.s_thing;
    p->g_next = pd_stackHead;
    p->g_loadingAbstraction = pd_loadingAbstraction;
    pd_loadingAbstraction = NULL;
    pd_stackHead = p;
    s__X.s_thing = x;
}

void pd_pop (t_pd *x)
{
    if (!pd_stackHead || s__X.s_thing != x) { PD_BUG; }
    else {
        t_gstack *p = pd_stackHead;
        s__X.s_thing = p->g_what;
        pd_stackHead = p->g_next;
        PD_MEMORY_FREE (p);
        pd_lastPopped = x;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_performLoadbang (void)
{
    if (pd_lastPopped) { pd_vMessage (pd_lastPopped, gensym ("loadbang"), ""); }
    
    pd_lastPopped = NULL;
}

int pd_setLoadingAbstraction (t_symbol *s)
{
    t_gstack *p = pd_stackHead;
    
    for (p = pd_stackHead; p; p = p->g_next) {
        if (p->g_loadingAbstraction == s) { return 1; }
    }
    
    pd_loadingAbstraction = s;
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
