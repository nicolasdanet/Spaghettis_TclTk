
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* To manage several objects bound to the same symbol. */

typedef struct _bindelement {
    t_pd                *e_what;            /* MUST be the first. */
    struct _bindelement *e_next;
    } t_bindelement;

typedef struct _bindlist {
    t_pd            b_pd;
    t_bindelement   *b_list;
    } t_bindlist;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* To maintain bindings for the #X symbol during nestable loads. */

typedef struct _gstack {
    t_pd            *g_what;
    t_symbol        *g_loadingAbstraction;
    struct _gstack  *g_next;
    } t_gstack;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd *pd_newest;                            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pd pd_objectMaker;                        /* Shared. */
t_pd pd_canvasMaker;                        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_pdinstance *pd_this;                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_gstack *pd_stackHead;              /* Shared. */
static t_pd     *pd_lastPopped;             /* Shared. */
static t_symbol *pd_loadingAbstraction;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *bindlist_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bindlist_bang (t_bindlist *x)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_bang (e->e_what); }
}

static void bindlist_float (t_bindlist *x, t_float f)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_float (e->e_what, f); }
}

static void bindlist_symbol (t_bindlist *x, t_symbol *s)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_symbol (e->e_what, s); }
}

static void bindlist_pointer (t_bindlist *x, t_gpointer *gp)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_pointer (e->e_what, gp); }
}

static void bindlist_list (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_list (e->e_what, s, argc, argv); }
}

static void bindlist_anything (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelement *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_typedmess (e->e_what, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_setup (void)
{
    bindlist_class = class_new (gensym ("bindlist"), NULL, NULL, sizeof (t_bindlist), CLASS_PURE, 0);
    
    class_addBang (bindlist_class, (t_method)bindlist_bang);
    class_addFloat (bindlist_class, (t_method)bindlist_float);
    class_addSymbol (bindlist_class, (t_method)bindlist_symbol);
    class_addPointer (bindlist_class, (t_method)bindlist_pointer);
    class_addList (bindlist_class, (t_method)bindlist_list);
    class_addAnything (bindlist_class, (t_method)bindlist_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_pdinstance *pdinstance_new()
{
    t_pdinstance *x = (t_pdinstance *)getbytes (sizeof (t_pdinstance));
    
    x->pd_systime           = 0;
    x->pd_clocks            = NULL;
    x->pd_dspChain          = NULL;
    x->pd_dspChainSize      = 0;
    x->pd_dspState          = 0;
    x->pd_canvases          = NULL;
    x->pd_signals           = NULL;
    x->sym_midiin           = gensym ("#midiin");
    x->sym_sysexin          = gensym ("#sysexin");
    x->sym_notein           = gensym ("#notein");
    x->sym_ctlin            = gensym ("#ctlin");
    x->sym_pgmin            = gensym ("#pgmin");
    x->sym_bendin           = gensym ("#bendin");
    x->sym_touchin          = gensym ("#touchin");
    x->sym_polytouchin      = gensym ("#polytouchin");
    x->sym_midiclkin        = gensym ("#midiclkin");
    x->sym_midirealtimein   = gensym ("#midirealtimein");
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_initialize (void)
{
    pd_this = pdinstance_new();
    
    mess_init();
    obj_init();
    conf_init();
    glob_init();
    garray_init();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_pd *pd_new (t_class *c)
{
    t_pd *x;
    
    PD_ASSERT (c != NULL);

    x = (t_pd *)getbytes (c->c_size);
    
    *x = c;
    
    if (c->c_isBox) { ((t_object *)x)->te_inlet = ((t_object *)x)->te_outlet = NULL; }
    
    return x;
}

void pd_free (t_pd *x)
{
    t_class *c = *x;

    if (c->c_methodFree) { (*(t_gotfn)(c->c_methodFree))(x); }

    if (c->c_isBox) {
        while (((t_object *)x)->te_outlet) { outlet_free (((t_object *)x)->te_outlet); }
        while (((t_object *)x)->te_inlet)  { inlet_free (((t_object *)x)->te_inlet);   }
        
        if (((t_object *)x)->te_binbuf) { 
            binbuf_free (((t_object *)x)->te_binbuf); 
        }
    }

    if (c->c_size) { freebytes (x, c->c_size); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bang (t_pd *x)
{
    (*(*x)->c_methodBang)(x);
}

void pd_float (t_pd *x, t_float f)
{
    (*(*x)->c_methodFloat)(x, f);
}

void pd_pointer (t_pd *x, t_gpointer *gp)
{
    (*(*x)->c_methodPointer)(x, gp);
}

void pd_symbol (t_pd *x, t_symbol *s)
{
    (*(*x)->c_methodSymbol)(x, s);
}

void pd_list (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    (*(*x)->c_methodList)(x, &s_list, argc, argv);
}

void pd_empty (t_pd *x)
{
    if (class_hasBangMethod (pd_class (x))) { (*(*x)->c_methodBang) (x); }
    else {
        (*(*x)->c_methodAny) (x, &s_bang, 0, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bind (t_pd *x, t_symbol *s)
{
    if (s->s_thing) {
    
        if (*s->s_thing == bindlist_class) {
            t_bindlist *b = (t_bindlist *)s->s_thing;
            t_bindelement *e = (t_bindelement *)getbytes (sizeof (t_bindelement));
            e->e_next = b->b_list;
            e->e_what = x;
            b->b_list = e;
            
        } else {
            t_bindlist *b = (t_bindlist *)pd_new (bindlist_class);
            t_bindelement *e1 = (t_bindelement *)getbytes (sizeof (t_bindelement));
            t_bindelement *e2 = (t_bindelement *)getbytes (sizeof (t_bindelement));
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
        
    } else if (s->s_thing && *s->s_thing == bindlist_class) {
    
        t_bindlist *b = (t_bindlist *)s->s_thing;
        t_bindelement *e1 = NULL;
        t_bindelement *e2 = NULL;
        
        if ((e1 = b->b_list)->e_what == x) {
            b->b_list = e1->e_next;
            freebytes (e1, sizeof (t_bindelement));
        } else {
            for (e1 = b->b_list; e2 = e1->e_next; e1 = e2) {
                if (e2->e_what == x) {
                    e1->e_next = e2->e_next;
                    freebytes (e2, sizeof (t_bindelement));
                    break;
                }
            }
        }
        
        if (!b->b_list->e_next) {                           /* Delete it if just one element remains. */
            s->s_thing = b->b_list->e_what;
            freebytes (b->b_list, sizeof (t_bindelement));
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
    if (*s->s_thing == c) { return s->s_thing; }
    
    if (*s->s_thing == bindlist_class) {
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
    t_gstack *p = (t_gstack *)getbytes (sizeof (t_gstack));
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
        freebytes (p, sizeof (t_gstack));
        pd_lastPopped = x;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_doLoadbang (void)
{
    if (pd_lastPopped) { pd_vmess (pd_lastPopped, gensym ("loadbang"), ""); }
    
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
