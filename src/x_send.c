
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

static t_class *send_class;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _send {
    t_object    x_obj;          /* Must be the first. */
    t_symbol    *x_name;
    } t_send;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void send_bang (t_send *x)
{
    if (pd_isThing (x->x_name)) { pd_bang (x->x_name->s_thing); }
}

static void send_float (t_send *x, t_float f)
{
    if (pd_isThing (x->x_name)) { pd_float (x->x_name->s_thing, f); }
}

static void send_symbol (t_send *x, t_symbol *s)
{
    if (pd_isThing (x->x_name)) { pd_symbol (x->x_name->s_thing, s); }
}

static void send_pointer (t_send *x, t_gpointer *gp)
{
    if (pd_isThing (x->x_name)) { pd_pointer (x->x_name->s_thing, gp); }
}

static void send_list (t_send *x, t_symbol *s, int argc, t_atom *argv)
{
    if (pd_isThing (x->x_name)) { pd_list (x->x_name->s_thing, argc, argv); }
}

static void send_anything (t_send *x, t_symbol *s, int argc, t_atom *argv)
{
    if (pd_isThing (x->x_name)) { pd_message (x->x_name->s_thing, s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *send_new (t_symbol *s)
{
    t_send *x = (t_send *)pd_new (send_class);
    
    x->x_name = s;
    
    if (s == &s_) { inlet_newSymbol (cast_object (x), &x->x_name); }

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void send_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_send,
            (t_newmethod)send_new,
            NULL,
            sizeof (t_send),
            CLASS_DEFAULT,
            A_DEFSYMBOL,
            A_NULL);
            
    class_addCreator ((t_newmethod)send_new, sym_s, A_DEFSYMBOL, A_NULL);
    
    class_addBang (c, send_bang);
    class_addFloat (c, send_float);
    class_addSymbol (c, send_symbol);
    class_addPointer (c, send_pointer);
    class_addList (c, send_list);
    class_addAnything (c, send_anything);
    
    send_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
