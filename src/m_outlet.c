
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

#define OUTLET_MAXIMUM_ITERATION            1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int outlet_stackCount;               /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_outlet *outlet_new (t_object *owner, t_symbol *s)
{
    t_outlet *x = (t_outlet *)PD_MEMORY_GET (sizeof (t_outlet));
    t_outlet *yA = NULL;
    t_outlet *yB = NULL;
    
    x->o_next  = NULL;
    x->o_owner = owner;

    if ((yA = owner->te_outlet)) { while ((yB = yA->o_next)) { yA = yB; } yA->o_next = x; }
    else {
        owner->te_outlet = x;
    }
    
    x->o_connections = NULL;
    x->o_symbol = s;
    
    return x;
}

void outlet_free (t_outlet *x)
{
    t_object *y = x->o_owner;
    t_outlet *xB = NULL;
    
    if (y->te_outlet == x) { y->te_outlet = x->o_next; }
    else {
        for (xB = y->te_outlet; xB; xB = xB->o_next) {
            if (xB->o_next == x) { xB->o_next = x->o_next; break; }
        }
    }
    
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int outlet_isSignal (t_outlet *x)
{
    return (x->o_symbol == &s_signal);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void outlet_bang (t_outlet *x)
{
    t_outconnect *oc = NULL;
    
    if (++outlet_stackCount >= OUTLET_MAXIMUM_ITERATION)  { error_stackOverflow(); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_bang (oc->oc_to); }
    }
    
    --outlet_stackCount;
}

void outlet_pointer (t_outlet *x, t_gpointer *gp)
{
    t_outconnect *oc = NULL;
    t_gpointer gpointer; GPOINTER_INIT (&gpointer);
    
    if (++outlet_stackCount >= OUTLET_MAXIMUM_ITERATION)  { error_stackOverflow(); }
    else {
        gpointer_setByCopy (&gpointer, gp);     /* Temporary copy cached on the stack. */
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_pointer (oc->oc_to, &gpointer); }
        gpointer_unset (&gpointer);
    }
    
    --outlet_stackCount;
}

void outlet_float (t_outlet *x, t_float f)
{
    t_outconnect *oc = NULL;
    
    if (++outlet_stackCount >= OUTLET_MAXIMUM_ITERATION)  { error_stackOverflow(); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_float (oc->oc_to, f); }
    }
    
    --outlet_stackCount;
}

void outlet_symbol (t_outlet *x, t_symbol *s)
{
    t_outconnect *oc = NULL;
    
    if (++outlet_stackCount >= OUTLET_MAXIMUM_ITERATION)  { error_stackOverflow(); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_symbol (oc->oc_to, s); }
    }
    
    --outlet_stackCount;
}

void outlet_list (t_outlet *x, int argc, t_atom *argv)
{
    t_outconnect *oc = NULL;
    
    if (++outlet_stackCount >= OUTLET_MAXIMUM_ITERATION)  { error_stackOverflow(); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_list (oc->oc_to, argc, argv); }
    }
    
    --outlet_stackCount;
}

void outlet_anything (t_outlet *x, t_symbol *s, int argc, t_atom *argv)
{
    t_outconnect *oc = NULL;
    
    if (++outlet_stackCount >= OUTLET_MAXIMUM_ITERATION)  { error_stackOverflow(); }
    else {
        for (oc = x->o_connections; oc; oc = oc->oc_next) { pd_message (oc->oc_to, s, argc, argv); }
    }
    
    --outlet_stackCount;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
