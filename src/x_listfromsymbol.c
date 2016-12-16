
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "m_alloca.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *listfromsymbol_class;       /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _listfromsymbol {
    t_object    x_obj;                      /* Must be the first. */
    t_outlet    *x_outlet;
    } t_listfromsymbol;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void listfromsymbol_symbol (t_listfromsymbol *x, t_symbol *s)
{
    t_atom *t = NULL;
    int count = strlen (s->s_name);
    int n;
        
    ATOMS_ALLOCA (t, count);
    
    for (n = 0; n < count; n++) { SET_FLOAT (t + n, (unsigned char)s->s_name[n]); }
    
    outlet_list (x->x_outlet, count, t);
    
    ATOMS_FREEA (t, count);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *listfromsymbol_new (t_symbol *s, int argc, t_atom *argv)
{
    t_listfromsymbol *x = (t_listfromsymbol *)pd_new (listfromsymbol_class);
    
    x->x_outlet = outlet_new (cast_object (x), &s_list);
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void listfromsymbol_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_list__space__fromsymbol,
            (t_newmethod)listfromsymbol_new,
            NULL,
            sizeof (t_listfromsymbol),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addSymbol (c, (t_method)listfromsymbol_symbol);
    
    class_setHelpName (c, &s_list);
    
    listfromsymbol_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

