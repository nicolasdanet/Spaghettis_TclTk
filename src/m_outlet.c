
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

int outlet_isSignal (t_outlet *x)
{
    return (x->o_type == &s_signal);
}

void outlet_moveFirst (t_outlet *x)
{
    t_object *owner = x->o_owner;
    
    if (owner->te_outlet != x) {
    //
    t_outlet *o = NULL;
    
    for (o = owner->te_outlet; o; o = o->o_next) {
        if (o->o_next == x) {
            o->o_next = x->o_next;
            x->o_next = owner->te_outlet;
            owner->te_outlet = x;
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
    
    for (o = x->o_owner->te_outlet; o && o != x; o = o->o_next) {
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
    
    if ((o1 = owner->te_outlet)) { while ((o2 = o1->o_next)) { o1 = o2; } o1->o_next = x; }
    else {
        owner->te_outlet = x;
    }

    return x;
}

void outlet_free (t_outlet *x)
{
    t_object *y = x->o_owner;
    t_outlet *o = NULL;
    
    if (y->te_outlet == x) { y->te_outlet = x->o_next; }
    else {
        for (o = y->te_outlet; o; o = o->o_next) {
            if (o->o_next == x) { o->o_next = x->o_next; break; }
        }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
