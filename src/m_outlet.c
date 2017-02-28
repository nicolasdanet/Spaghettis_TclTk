
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define OUTLET_MAXIMUM_RECURSION            1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int outlet_stackCount;               /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int outlet_hasStackOverflow (t_outlet *x)
{
    int k = (++outlet_stackCount >= OUTLET_MAXIMUM_RECURSION);
    
    if (k) { error_stackOverflow(); }
    
    return k;
}

/* Assumed below that connection to a receiver is unique. */

t_outconnect *outlet_addConnection (t_outlet *x, t_pd *receiver)
{
    t_outconnect *oc1 = NULL;
    t_outconnect *oc2 = NULL;
    
    oc1 = (t_outconnect *)PD_MEMORY_GET (sizeof (t_outconnect));
    oc1->oc_next = NULL;
    oc1->oc_receiver = receiver;

    if ((oc2 = x->o_connections)) { while (oc2->oc_next) { oc2 = oc2->oc_next; } oc2->oc_next = oc1; }
    else {
        x->o_connections = oc1;
    }
    
    return oc1;
}

void outlet_removeConnection (t_outlet *x, t_pd *receiver)
{
    t_outconnect *oc1 = NULL;
    t_outconnect *oc2 = NULL;
    
    if ((oc1 = x->o_connections)) {

        if (oc1->oc_receiver == receiver) {
            x->o_connections = oc1->oc_next; PD_MEMORY_FREE (oc1); return;
            
        } else {
            while ((oc2 = oc1->oc_next)) {
                if (oc2->oc_receiver != receiver) { oc1 = oc2; }
                else {
                    oc1->oc_next = oc2->oc_next; PD_MEMORY_FREE (oc2); return;
                }
            }
        }
    }
}
    
int outlet_isSignal (t_outlet *x)
{
    return (x->o_type == &s_signal);
}

void outlet_moveFirst (t_outlet *x)
{
    t_object *owner = x->o_owner;
    
    if (owner->te_outlets != x) {
    //
    t_outlet *o = NULL;
    
    for (o = owner->te_outlets; o; o = o->o_next) {
        if (o->o_next == x) {
            o->o_next = x->o_next;
            x->o_next = owner->te_outlets;
            owner->te_outlets = x;
            return;
        }
    }
    
    PD_BUG;
    //
    }
}

int outlet_getSignalIndex (t_outlet *x)
{
    t_outlet *o = NULL;
    int n = 0;
        
    PD_ASSERT (outlet_isSignal (x));
    
    for (o = x->o_owner->te_outlets; o && o != x; o = o->o_next) {
        if (outlet_isSignal (o)) { n++; }
    }
    
    return n;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void outlet_bang (t_outlet *x)
{
    if (!outlet_hasStackOverflow (x)) {

        t_outconnect *oc = NULL;
    
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_bang (oc->oc_receiver); }
    }
    
    --outlet_stackCount;
}

void outlet_pointer (t_outlet *x, t_gpointer *gp)
{
    if (!outlet_hasStackOverflow (x)) {

        t_outconnect *oc = NULL;
        
        for (oc = x->o_connections; oc; oc = oc->oc_next) {
            
            t_gpointer gpointer; GPOINTER_INIT (&gpointer);     /* Use a copy cached on the stack. */
            
            gpointer_setByCopy (&gpointer, gp); 
                pd_pointer (oc->oc_receiver, &gpointer);
            gpointer_unset (&gpointer);
        }
    }
    
    --outlet_stackCount;
}

void outlet_float (t_outlet *x, t_float f)
{
    if (!outlet_hasStackOverflow (x)) {

        t_outconnect *oc = NULL;
        
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_float (oc->oc_receiver, f); }
    }
    
    --outlet_stackCount;
}

void outlet_symbol (t_outlet *x, t_symbol *s)
{
    if (!outlet_hasStackOverflow (x)) {

        t_outconnect *oc = NULL;
    
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_symbol (oc->oc_receiver, s); }
    }
    
    --outlet_stackCount;
}

void outlet_list (t_outlet *x, int argc, t_atom *argv)
{
    if (!outlet_hasStackOverflow (x)) {
    
        t_outconnect *oc = NULL;
    
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_list (oc->oc_receiver, argc, argv); }
    }
    
    --outlet_stackCount;
}

void outlet_anything (t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!outlet_hasStackOverflow (x)) {
    
        t_outconnect *oc = NULL;
    
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_message (oc->oc_receiver, s, argc, argv); }
    }
    
    --outlet_stackCount;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outlet *outlet_new (t_object *owner, t_symbol *s)
{
    t_outlet *x  = (t_outlet *)PD_MEMORY_GET (sizeof (t_outlet));
    t_outlet *o1 = NULL;
    t_outlet *o2 = NULL;
    
    x->o_next        = NULL;
    x->o_owner       = owner;
    x->o_connections = NULL;
    x->o_type        = s;
    
    if ((o1 = owner->te_outlets)) { while ((o2 = o1->o_next)) { o1 = o2; } o1->o_next = x; }
    else {
        owner->te_outlets = x;
    }

    return x;
}

void outlet_free (t_outlet *x)
{
    t_object *y = x->o_owner;
    t_outlet *o = NULL;
    
    PD_ASSERT (x->o_connections == NULL);
    
    if (y->te_outlets == x) { y->te_outlets = x->o_next; }
    else {
        for (o = y->te_outlets; o; o = o->o_next) {
            if (o->o_next == x) { o->o_next = x->o_next; break; }
        }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
