
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
#pragma mark -

struct _inlet
{
    t_pd            i_pd;
    struct _inlet   *i_next;
    t_object        *i_owner;
    t_pd            *i_destination;
    t_symbol        *i_symbolFrom;
    union {
        t_symbol    *i_symbolTo;
        t_gpointer  *i_pointer;
        t_float     *i_float;
        t_symbol    **i_symbol;
        t_float     i_signal;
    } i_un;
};

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *inlet_class;                /* Shared. */
static t_class *pointerinlet_class;         /* Shared. */
static t_class *floatinlet_class;           /* Shared. */
static t_class *symbolinlet_class;          /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void object_inletList (t_inlet *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void object_inletWrong (t_inlet *x, t_symbol *s)
{
    post_error (PD_TRANSLATE ("inlet / Unexpected \"%s\""), s->s_name);
}

static void object_inletBang (t_inlet *x)
{
    if (x->i_symbolFrom == &s_bang)         { pd_vMessage (x->i_destination, x->i_un.i_symbolTo, ""); }
    else if (x->i_symbolFrom == NULL)       { pd_bang (x->i_destination); }
    else if (x->i_symbolFrom == &s_list)    { object_inletList (x, &s_bang, 0, NULL); }
    else {
        object_inletWrong (x, &s_bang);
    }
}

static void object_inletPointer (t_inlet *x, t_gpointer *gp)
{
    if (x->i_symbolFrom == &s_pointer)      { pd_vMessage (x->i_destination, x->i_un.i_symbolTo, "p", gp); }
    else if (x->i_symbolFrom == NULL)       { pd_pointer (x->i_destination, gp); }
    else if (x->i_symbolFrom == &s_list)    {
        t_atom a;
        SET_POINTER (&a, gp);
        object_inletList (x, &s_pointer, 1, &a);

    } else {
        object_inletWrong (x, &s_pointer);
    }
}

static void object_inletFloat (t_inlet *x, t_float f)
{
    if (x->i_symbolFrom == &s_float)        { pd_vMessage (x->i_destination, x->i_un.i_symbolTo, "f", f); }
    else if (x->i_symbolFrom == &s_signal)  { x->i_un.i_signal = f; }
    else if (x->i_symbolFrom == NULL)       { pd_float (x->i_destination, f); }
    else if (x->i_symbolFrom == &s_list)    {
        t_atom a;
        SET_FLOAT (&a, f);
        object_inletList (x, &s_float, 1, &a);
    } else { 
        object_inletWrong (x, &s_float);
    }
}

static void object_inletSymbol (t_inlet *x, t_symbol *s)
{
    if (x->i_symbolFrom == &s_symbol)       { pd_vMessage (x->i_destination, x->i_un.i_symbolTo, "s", s); }
    else if (x->i_symbolFrom == NULL)       { pd_symbol (x->i_destination, s); }
    else if (x->i_symbolFrom == &s_list)    {
        t_atom a;
        SET_SYMBOL (&a, s);
        object_inletList (x, &s_symbol, 1, &a);
    } else { 
        object_inletWrong (x, &s_symbol);
    }
}

static void object_inletList (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_symbolFrom == &s_list 
        || x->i_symbolFrom == &s_float 
        || x->i_symbolFrom == &s_symbol 
        || x->i_symbolFrom == &s_pointer)   { pd_message (x->i_destination, x->i_un.i_symbolTo, argc, argv); }
    else if (x->i_symbolFrom == NULL)       { pd_list (x->i_destination, argc, argv);  }
    else if (!argc)                         { object_inletBang (x); }
    else if (argc == 1 && IS_FLOAT (argv))  { object_inletFloat (x, atom_getfloat (argv));   }
    else if (argc == 1 && IS_SYMBOL (argv)) { object_inletSymbol (x, atom_getsymbol (argv)); }
    else { 
        object_inletWrong (x, &s_list);
    }
}

static void object_inletAnything (t_inlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->i_symbolFrom == s)               { pd_message (x->i_destination, x->i_un.i_symbolTo, argc, argv); }
    else if (x->i_symbolFrom == NULL)       { pd_message (x->i_destination, s, argc, argv); }
    else {
        object_inletWrong (x, s);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void object_pointerInlet (t_inlet *x, t_gpointer *gp)
{
    gpointer_unset (x->i_un.i_pointer);
    *(x->i_un.i_pointer) = *gp;
    if (gp->gp_stub) { gp->gp_stub->gs_count++; }
}

static void object_floatInlet (t_inlet *x, t_float f)
{
    *(x->i_un.i_float) = f;
}

static void object_symbolInlet (t_inlet *x, t_symbol *s)
{
    *(x->i_un.i_symbol) = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_inlet *inlet_new (t_object *owner, t_pd *destination, t_symbol *s1, t_symbol *s2)
{
    t_inlet *x = (t_inlet *)pd_new (inlet_class);
    t_inlet *y1 = NULL;
    t_inlet *y2 = NULL;
    
    x->i_owner = owner;
    x->i_destination = destination;
    
    if (s1 == &s_signal) { x->i_un.i_signal = 0.0; }
    else { 
        x->i_un.i_symbolTo = s2;
    }
    
    x->i_symbolFrom = s1;
    x->i_next = NULL;
    
    if (y1 = owner->te_inlet) { while (y2 = y1->i_next) { y1 = y2; } y1->i_next = x; } 
    else { 
        owner->te_inlet = x;
    }
    
    return x;
}

t_inlet *signalinlet_new (t_object *owner, t_float f)
{
    t_inlet *x = inlet_new (owner, &owner->te_g.g_pd, &s_signal, &s_signal);
    
    x->i_un.i_signal = f;
    
    return x;
}

t_inlet *pointerinlet_new (t_object *owner, t_gpointer *gp)
{
    t_inlet *x = (t_inlet *)pd_new (pointerinlet_class);
    t_inlet *y1 = NULL;
    t_inlet *y2 = NULL;
    
    x->i_owner = owner;
    x->i_destination = NULL;
    
    x->i_symbolFrom = &s_pointer;
    x->i_un.i_pointer = gp;
    x->i_next = NULL;
    
    if (y1 = owner->te_inlet) { while (y2 = y1->i_next) { y1 = y2; } y1->i_next = x; }
    else {
        owner->te_inlet = x;
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void inlet_free (t_inlet *x)
{
    t_object *y = x->i_owner;
    t_inlet *x2 = NULL;
    
    if (y->te_inlet == x) { y->te_inlet = x->i_next; }
    else {
        for (x2 = y->te_inlet; x2; x2 = x2->i_next) {
            if (x2->i_next == x) {
                x2->i_next = x->i_next; break;
            }
        }
    }
    
    PD_MEMORY_FREE (x, sizeof (t_inlet));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* ----- pointerinlets, floatinlets, syminlets: optimized inlets ------- */

t_inlet *floatinlet_new(t_object *owner, t_float *fp)
{
    t_inlet *x = (t_inlet *)pd_new(floatinlet_class), *y, *y2;
    x->i_owner = owner;
    x->i_destination = 0;
    x->i_symbolFrom = &s_float;
    x->i_un.i_float = fp;
    x->i_next = 0;
    if (y = owner->te_inlet)
    {
        while (y2 = y->i_next) y = y2;
        y->i_next = x;
    }
    else owner->te_inlet = x;
    return (x);
}

t_inlet *symbolinlet_new(t_object *owner, t_symbol **sp)
{
    t_inlet *x = (t_inlet *)pd_new(symbolinlet_class), *y, *y2;
    x->i_owner = owner;
    x->i_destination = 0;
    x->i_symbolFrom = &s_symbol;
    x->i_un.i_symbol = sp;
    x->i_next = 0;
    if (y = owner->te_inlet)
    {
        while (y2 = y->i_next) y = y2;
        y->i_next = x;
    }
    else owner->te_inlet = x;
    return (x);
}

/* ---------------------- routine to handle lists ---------------------- */

    /* objects interpret lists by feeding them to the individual inlets.
    Before you call this check that the object doesn't have a more
    specific way to handle lists. */
void obj_list(t_object *x, t_symbol *s, int argc, t_atom *argv)
{
    t_atom *ap;
    int count;
    t_inlet *ip = ((t_object *)x)->te_inlet;
    if (!argc) 
    {
        pd_empty(&x->te_g.g_pd);
        return;
    }
    for (count = argc-1, ap = argv+1; ip && count--; ap++, ip = ip->i_next)
    {
        if (ap->a_type == A_POINTER) pd_pointer(&ip->i_pd, ap->a_w.w_gpointer);
        else if (ap->a_type == A_FLOAT) pd_float(&ip->i_pd, ap->a_w.w_float);
        else pd_symbol(&ip->i_pd, ap->a_w.w_symbol);
    }
    if (argv->a_type == A_POINTER) pd_pointer(&x->te_g.g_pd, argv->a_w.w_gpointer);
    else if (argv->a_type == A_FLOAT) pd_float(&x->te_g.g_pd, argv->a_w.w_float);
    else pd_symbol(&x->te_g.g_pd, argv->a_w.w_symbol);
} 

void obj_init(void)
{
    inlet_class = class_new(gensym("inlet"), 0, 0,
        sizeof(t_inlet), CLASS_PURE, 0);
    class_addBang(inlet_class, object_inletBang);
    class_addPointer(inlet_class, object_inletPointer);
    class_addFloat(inlet_class, object_inletFloat);
    class_addSymbol(inlet_class, object_inletSymbol);
    class_addList(inlet_class, object_inletList);
    class_addAnything(inlet_class, object_inletAnything);

    pointerinlet_class = class_new(gensym("inlet"), 0, 0,
        sizeof(t_inlet), CLASS_PURE, 0);
    class_addPointer(pointerinlet_class, object_pointerInlet);
    class_addAnything(pointerinlet_class, object_inletWrong);
    
    floatinlet_class = class_new(gensym("inlet"), 0, 0,
        sizeof(t_inlet), CLASS_PURE, 0);
    class_addFloat(floatinlet_class, (t_method)object_floatInlet);
    class_addAnything(floatinlet_class, object_inletWrong);

    symbolinlet_class = class_new(gensym("inlet"), 0, 0,
        sizeof(t_inlet), CLASS_PURE, 0);
    class_addSymbol(symbolinlet_class, object_symbolInlet);
    class_addAnything(symbolinlet_class, object_inletWrong);

}

/* --------------------------- outlets ------------------------------ */

static int stackcount = 0; /* iteration counter */
#define STACKITER 1000 /* maximum iterations allowed */

static int outlet_eventno;

    /* set a stack limit (on each incoming event that can set off messages)
    for the outlet functions to check to prevent stack overflow from message
    recursion */
        
void outlet_setstacklim(void)
{
    outlet_eventno++;
}

    /* get a number unique to the (clock, MIDI, GUI, etc.) event we're on */
int sched_geteventno( void)
{
    return (outlet_eventno);
}

struct _outconnect
{
    struct _outconnect *oc_next;
    t_pd *oc_to;
};

struct _outlet
{
    t_object *o_owner;
    struct _outlet *o_next;
    t_outconnect *o_connections;
    t_symbol *o_sym;
};

t_outlet *outlet_new(t_object *owner, t_symbol *s)
{
    t_outlet *x = (t_outlet *)PD_MEMORY_GET(sizeof(*x)), *y, *y2;
    x->o_owner = owner;
    x->o_next = 0;
    if (y = owner->te_outlet)
    {
        while (y2 = y->o_next) y = y2;
        y->o_next = x;
    }
    else owner->te_outlet = x;
    x->o_connections = 0;
    x->o_sym = s;
    return (x);
}

static void outlet_stackerror(t_outlet *x)
{
    post_error ("stack overflow");
}

void outlet_bang(t_outlet *x)
{
    t_outconnect *oc;
    if(++stackcount >= STACKITER)
        outlet_stackerror(x);
    else 
    for (oc = x->o_connections; oc; oc = oc->oc_next)
        pd_bang(oc->oc_to);
    --stackcount;
}

void outlet_pointer(t_outlet *x, t_gpointer *gp)
{
    t_outconnect *oc;
    t_gpointer gpointer;
    if(++stackcount >= STACKITER)
        outlet_stackerror(x);
    else
    {
        gpointer = *gp;
        for (oc = x->o_connections; oc; oc = oc->oc_next)
            pd_pointer(oc->oc_to, &gpointer);
    }
    --stackcount;
}

void outlet_float(t_outlet *x, t_float f)
{
    t_outconnect *oc;
    if(++stackcount >= STACKITER)
        outlet_stackerror(x);
    else
    for (oc = x->o_connections; oc; oc = oc->oc_next)
        pd_float(oc->oc_to, f);
    --stackcount;
}

void outlet_symbol(t_outlet *x, t_symbol *s)
{
    t_outconnect *oc;
    if(++stackcount >= STACKITER)
        outlet_stackerror(x);
    else
    for (oc = x->o_connections; oc; oc = oc->oc_next)
        pd_symbol(oc->oc_to, s);
    --stackcount;
}

void outlet_list(t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc;
    if(++stackcount >= STACKITER)
        outlet_stackerror(x);
    else
    for (oc = x->o_connections; oc; oc = oc->oc_next)
        pd_list(oc->oc_to, argc, argv);
    --stackcount;
}

void outlet_anything(t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc;
    if(++stackcount >= STACKITER)
        outlet_stackerror(x);
    else
    for (oc = x->o_connections; oc; oc = oc->oc_next)
        pd_message(oc->oc_to, s, argc, argv);
    --stackcount;
}

    /* get the outlet's declared symbol */
t_symbol *outlet_getsymbol(t_outlet *x)
{
    return (x->o_sym);
}

void outlet_free(t_outlet *x)
{
    t_object *y = x->o_owner;
    t_outlet *x2;
    if (y->te_outlet == x) y->te_outlet = x->o_next;
    else for (x2 = y->te_outlet; x2; x2 = x2->o_next)
        if (x2->o_next == x)
    {
        x2->o_next = x->o_next;
        break;
    }
    PD_MEMORY_FREE(x, sizeof(*x));
}

t_outconnect *obj_connect(t_object *source, int outno,
    t_object *sink, int inno)
{
    t_inlet *i;
    t_outlet *o;
    t_pd *to;
    t_outconnect *oc, *oc2;
    
    for (o = source->te_outlet; o && outno; o = o->o_next, outno--) ;
    if (!o) return (0);
    
    if (sink->te_g.g_pd->c_hasFirstInlet)
    {
        if (!inno)
        {
            to = &sink->te_g.g_pd;
            goto doit;
        }
        else inno--;
    }
    for (i = sink->te_inlet; i && inno; i = i->i_next, inno--) ;
    if (!i) return (0);
    to = &i->i_pd;
doit:
    oc = (t_outconnect *)PD_MEMORY_GET(sizeof(*oc));
    oc->oc_next = 0;
    oc->oc_to = to;
        /* append it to the end of the list */
        /* LATER we might cache the last "oc" to make this faster. */
    if ((oc2 = o->o_connections))
    {
        while (oc2->oc_next) oc2 = oc2->oc_next;
        oc2->oc_next = oc;
    }
    else o->o_connections = oc;
    if (o->o_sym == &s_signal) canvas_update_dsp();

    return (oc);
}

void obj_disconnect(t_object *source, int outno, t_object *sink, int inno)
{
    t_inlet *i;
    t_outlet *o;
    t_pd *to;
    t_outconnect *oc, *oc2;
    
    for (o = source->te_outlet; o && outno; o = o->o_next, outno--)
    if (!o) return;
    if (sink->te_g.g_pd->c_hasFirstInlet)
    {
        if (!inno)
        {
            to = &sink->te_g.g_pd;
            goto doit;
        }
        else inno--;
    }
    for (i = sink->te_inlet; i && inno; i = i->i_next, inno--) ;
    if (!i) return;
    to = &i->i_pd;
doit:
    if (!(oc = o->o_connections)) return;
    if (oc->oc_to == to)
    {
        o->o_connections = oc->oc_next;
        PD_MEMORY_FREE(oc, sizeof(*oc));
        goto done;
    }
    while (oc2 = oc->oc_next)
    {
        if (oc2->oc_to == to)
        {
            oc->oc_next = oc2->oc_next;
            PD_MEMORY_FREE(oc2, sizeof(*oc2));
            goto done;
        }
        oc = oc2;
    }
done:
    if (o->o_sym == &s_signal) canvas_update_dsp();
}

/* ------ traversal routines for code that can't see our structures ------ */

int obj_noutlets(t_object *x)
{
    int n;
    t_outlet *o;
    for (o = x->te_outlet, n = 0; o; o = o->o_next) n++;
    return (n);
}

int obj_ninlets(t_object *x)
{
    int n;
    t_inlet *i;
    for (i = x->te_inlet, n = 0; i; i = i->i_next) n++;
    if (x->te_g.g_pd->c_hasFirstInlet) n++;
    return (n);
}

t_outconnect *obj_starttraverseoutlet(t_object *x, t_outlet **op, int nout)
{
    t_outlet *o = x->te_outlet;
    while (nout-- && o) o = o->o_next;
    *op = o;
    if (o) return (o->o_connections);
    else return (0);
}

t_outconnect *obj_nexttraverseoutlet(t_outconnect *lastconnect,
    t_object **destp, t_inlet **inletp, int *whichp)
{
    t_pd *y;
    y = lastconnect->oc_to;
    
    t_class *c = pd_class (y);
    
    if (c == inlet_class || c == pointerinlet_class || c == floatinlet_class || c == symbolinlet_class) {
        int n;
        t_inlet *i = (t_inlet *)y, *i2;
        t_object *dest = i->i_owner;
        for (n = dest->te_g.g_pd->c_hasFirstInlet, i2 = dest->te_inlet;
            i2 && i2 != i; i2 = i2->i_next) n++;
        *whichp = n;
        *destp = dest;
        *inletp = i;
    }
    else
    {
        *whichp = 0;
        *inletp = 0;
        *destp = ((t_object *)y);
    }
    return (lastconnect->oc_next);
}

    /* this one checks that a pd is indeed a patchable object, and returns
    it, correctly typed, or zero if the check failed. */
t_object *pd_checkobject(t_pd *x)
{
    if ((*x)->c_isBox) return ((t_object *)x);
    else return (0);
}

    /* move an inlet or outlet to the head of the list */
void obj_moveinletfirst(t_object *x, t_inlet *i)
{
    t_inlet *i2;
    if (x->te_inlet == i) return;
    else for (i2 = x->te_inlet; i2; i2 = i2->i_next)
        if (i2->i_next == i)
    {
        i2->i_next = i->i_next;
        i->i_next = x->te_inlet;
        x->te_inlet = i;
        return;
    }
}

void obj_moveoutletfirst(t_object *x, t_outlet *o)
{
    t_outlet *o2;
    if (x->te_outlet == o) return;
    else for (o2 = x->te_outlet; o2; o2 = o2->o_next)
        if (o2->o_next == o)
    {
        o2->o_next = o->o_next;
        o->o_next = x->te_outlet;
        x->te_outlet = o;
        return;
    }
}

    /* routines for DSP sorting, which are used in d_ugen.c and g_canvas.c */
    /* LATER try to consolidate all the slightly different routines. */

int obj_nsiginlets(t_object *x)
{
    int n;
    t_inlet *i;
    for (i = x->te_inlet, n = 0; i; i = i->i_next)
        if (i->i_symbolFrom == &s_signal) n++;
    if (x->te_g.g_pd->c_hasFirstInlet && x->te_g.g_pd->c_floatSignalIn) n++;
    return (n);
}

    /* get the index, among signal inlets, of the mth inlet overall */
int obj_siginletindex(t_object *x, int m)
{
    int n = 0;
    t_inlet *i;
    if (x->te_g.g_pd->c_hasFirstInlet && x->te_g.g_pd->c_floatSignalIn)
    {
        if (!m--) return (0);
        n++;
    }
    for (i = x->te_inlet; i; i = i->i_next, m--)
        if (i->i_symbolFrom == &s_signal)
    {
        if (m == 0) return (n);
        n++;
    }
    return (-1);
}

int obj_issignalinlet(t_object *x, int m)
{
    t_inlet *i;
    if (x->te_g.g_pd->c_hasFirstInlet)
    {
        if (!m)
            return (x->te_g.g_pd->c_hasFirstInlet && x->te_g.g_pd->c_floatSignalIn);
        else m--;
    }
    for (i = x->te_inlet; i && m; i = i->i_next, m--)
        ;
    return (i && (i->i_symbolFrom == &s_signal));
}

int obj_nsigoutlets(t_object *x)
{
    int n;
    t_outlet *o;
    for (o = x->te_outlet, n = 0; o; o = o->o_next)
        if (o->o_sym == &s_signal) n++;
    return (n);
}

int obj_sigoutletindex(t_object *x, int m)
{
    int n;
    t_outlet *o2;
    for (o2 = x->te_outlet, n = 0; o2; o2 = o2->o_next, m--)
        if (o2->o_sym == &s_signal)
    {
        if (m == 0) return (n);
        n++;
    }
    return (-1);
}

int obj_issignaloutlet(t_object *x, int m)
{
    int n;
    t_outlet *o2;
    for (o2 = x->te_outlet, n = 0; o2 && m--; o2 = o2->o_next);
    return (o2 && (o2->o_sym == &s_signal));
}

t_float *obj_findsignalscalar(t_object *x, int m)
{
    int n = 0;
    t_inlet *i;
    if (x->te_g.g_pd->c_hasFirstInlet && x->te_g.g_pd->c_floatSignalIn)
    {
        if (!m--)
            return (x->te_g.g_pd->c_floatSignalIn > 0 ?
                (t_float *)(((char *)x) + x->te_g.g_pd->c_floatSignalIn) : 0);
        n++;
    }
    for (i = x->te_inlet; i; i = i->i_next, m--)
        if (i->i_symbolFrom == &s_signal)
    {
        if (m == 0)
            return (&i->i_un.i_signal);
        n++;
    }
    return (0);
}

/* and these are only used in g_io.c... */

int inlet_getsignalindex(t_inlet *x)
{
    int n = 0;
    t_inlet *i;
    
    if (x->i_symbolFrom != &s_signal) { PD_BUG; }
    
    for (i = x->i_owner->te_inlet, n = 0; i && i != x; i = i->i_next)
        if (i->i_symbolFrom == &s_signal) n++;
    return (n);
}

int outlet_getsignalindex(t_outlet *x)
{
    int n = 0;
    t_outlet *o;
    for (o = x->o_owner->te_outlet, n = 0; o && o != x; o = o->o_next) 
        if (o->o_sym == &s_signal) n++;
    return (n);
}

void obj_saveformat(t_object *x, t_binbuf *bb)
{
    if (x->te_width)
        binbuf_addv(bb, "ssf;", &s__X, gensym("f"), (float)x->te_width);
}

