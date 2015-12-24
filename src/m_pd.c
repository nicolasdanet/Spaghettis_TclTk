
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* To manage several objects bound to the same symbol. */

typedef struct _bindelem {
    t_pd             *e_what;               /* MUST be the first. */
    struct _bindelem *e_next;
    } t_bindelem;

typedef struct _bindlist {
    t_pd        b_pd;
    t_bindelem  *b_list;
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

t_pdinstance *pd_this;                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *bindlist_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_gstack *pd_stackHead;              /* Shared. */
static t_pd     *pd_lastPopped;             /* Shared. */
static t_symbol *pd_loadingAbstraction;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void bindlist_bang (t_bindlist *x)
{
    t_bindelem *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_bang (e->e_what); }
}

static void bindlist_float (t_bindlist *x, t_float f)
{
    t_bindelem *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_float (e->e_what, f); }
}

static void bindlist_symbol (t_bindlist *x, t_symbol *s)
{
    t_bindelem *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_symbol (e->e_what, s); }
}

static void bindlist_pointer (t_bindlist *x, t_gpointer *gp)
{
    t_bindelem *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_pointer (e->e_what, gp); }
}

static void bindlist_list (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelem *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_list (e->e_what, s, argc, argv); }
}

static void bindlist_anything (t_bindlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_bindelem *e = NULL;
    for (e = x->b_list; e; e = e->e_next) { pd_typedmess (e->e_what, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_setup (void)
{
    bindlist_class = class_new (gensym ("bindlist"), NULL, NULL, sizeof (t_bindlist), CLASS_PURE, 0);
    
    class_addbang (bindlist_class, (t_method)bindlist_bang);
    class_addfloat (bindlist_class, (t_method)bindlist_float);
    class_addsymbol (bindlist_class, (t_method)bindlist_symbol);
    class_addpointer (bindlist_class, (t_method)bindlist_pointer);
    class_addlist (bindlist_class, (t_method)bindlist_list);
    class_addanything (bindlist_class, (t_method)bindlist_anything);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_pdinstance *pdinstance_new()
{
    t_pdinstance *x = (t_pdinstance *)getbytes (sizeof (t_pdinstance));
    
    x->pd_systime               = 0;
    x->pd_clock_setlist         = 0;
    x->pd_dspchain              = 0;
    x->pd_dspchainsize          = 0;
    x->pd_canvaslist            = 0;
    x->pd_dspstate              = 0;
    x->pd_midiin_sym            = gensym ("#midiin");
    x->pd_sysexin_sym           = gensym ("#sysexin");
    x->pd_notein_sym            = gensym ("#notein");
    x->pd_ctlin_sym             = gensym ("#ctlin");
    x->pd_pgmin_sym             = gensym ("#pgmin");
    x->pd_bendin_sym            = gensym ("#bendin");
    x->pd_touchin_sym           = gensym ("#touchin");
    x->pd_polytouchin_sym       = gensym ("#polytouchin");
    x->pd_midiclkin_sym         = gensym ("#midiclkin");
    x->pd_midirealtimein_sym    = gensym ("#midirealtimein");
    
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
    
    if (c->c_methodFree) (*(t_gotfn)(c->c_methodFree))(x);
    
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void pd_bind (t_pd *x, t_symbol *s)
{
    if (s->s_thing) {
    
        if (*s->s_thing == bindlist_class) {
            t_bindlist *b = (t_bindlist *)s->s_thing;
            t_bindelem *e = (t_bindelem *)getbytes (sizeof (t_bindelem));
            e->e_next = b->b_list;
            e->e_what = x;
            b->b_list = e;
            
        } else {
            t_bindlist *b  = (t_bindlist *)pd_new (bindlist_class);
            t_bindelem *e1 = (t_bindelem *)getbytes (sizeof (t_bindelem));
            t_bindelem *e2 = (t_bindelem *)getbytes (sizeof (t_bindelem));
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
    
        t_bindlist *b  = (t_bindlist *)s->s_thing;
        t_bindelem *e1 = NULL;
        t_bindelem *e2 = NULL;
        
        if ((e1 = b->b_list)->e_what == x) {
            b->b_list = e1->e_next;
            freebytes (e1, sizeof (t_bindelem));
        } else {
            for (e1 = b->b_list; e2 = e1->e_next; e1 = e2) {
                if (e2->e_what == x) {
                    e1->e_next = e2->e_next;
                    freebytes (e2, sizeof (t_bindelem));
                    break;
                }
            }
        }
        
        if (!b->b_list->e_next) {                           /* Delete it if just one element remains. */
            s->s_thing = b->b_list->e_what;
            freebytes (b->b_list, sizeof (t_bindelem));
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
    
    if (!s->s_thing) return (NULL);
    if (*s->s_thing == c) return (s->s_thing);
    
    if (*s->s_thing == bindlist_class) {
        t_bindlist *b = (t_bindlist *)s->s_thing;
        t_bindelem *e = NULL;
        
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
