
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *receive_class;      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _receive {
    t_object    x_obj;              /* Must be the first. */
    t_symbol    *x_name;
    t_outlet    *x_outlet;
    } t_receive;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void receive_bang (t_receive *x)
{
    outlet_bang (x->x_outlet);
}

static void receive_float (t_receive *x, t_float f)
{
    outlet_float (x->x_outlet, f);
}

static void receive_symbol (t_receive *x, t_symbol *s)
{
    outlet_symbol (x->x_outlet, s);
}

static void receive_pointer (t_receive *x, t_gpointer *gp)
{
    outlet_pointer (x->x_outlet, gp);
}

static void receive_list (t_receive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list (x->x_outlet, argc, argv);
}

static void receive_anything (t_receive *x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_anything (x->x_outlet, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *receive_new (t_symbol *s)
{
    t_receive *x = (t_receive *)pd_new (receive_class);
    
    x->x_name = s;
    x->x_outlet = outlet_new (cast_object (x), &s_anything);
        
    pd_bind (cast_pd (x), s);

    return x;
}

static void receive_free (t_receive *x)
{
    pd_unbind (cast_pd (x), x->x_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void receive_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_receive,
            (t_newmethod)receive_new, 
            (t_method)receive_free,
            sizeof (t_receive),
            CLASS_NOINLET,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)receive_new, sym_r, A_DEFSYMBOL, A_NULL);
    
    class_addBang (c, receive_bang);
    class_addFloat (c, receive_float);
    class_addSymbol (c, receive_symbol);
    class_addPointer (c, receive_pointer);
    class_addList (c, receive_list);
    class_addAnything (c, receive_anything);
    
    receive_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
