
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"

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
// MARK: -

static void send_bang (t_send *x)
{
    if (symbol_hasThingQuiet (x->x_name)) { pd_bang (symbol_getThing (x->x_name)); }
}

static void send_float (t_send *x, t_float f)
{
    if (symbol_hasThingQuiet (x->x_name)) { pd_float (symbol_getThing (x->x_name), f); }
}

static void send_symbol (t_send *x, t_symbol *s)
{
    if (symbol_hasThingQuiet (x->x_name)) { pd_symbol (symbol_getThing (x->x_name), s); }
}

static void send_pointer (t_send *x, t_gpointer *gp)
{
    if (symbol_hasThingQuiet (x->x_name)) { pd_pointer (symbol_getThing (x->x_name), gp); }
}

static void send_list (t_send *x, t_symbol *s, int argc, t_atom *argv)
{
    if (symbol_hasThingQuiet (x->x_name)) { pd_list (symbol_getThing (x->x_name), argc, argv); }
}

static void send_anything (t_send *x, t_symbol *s, int argc, t_atom *argv)
{
    if (symbol_hasThingQuiet (x->x_name)) { pd_message (symbol_getThing (x->x_name), s, argc, argv); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *send_new (t_symbol *s)
{
    t_send *x = (t_send *)pd_new (send_class);
    
    x->x_name = s;
    
    if (x->x_name == &s_) { inlet_newSymbol (cast_object (x), &x->x_name); }

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    class_addBang (c, (t_method)send_bang);
    class_addFloat (c, (t_method)send_float);
    class_addSymbol (c, (t_method)send_symbol);
    class_addPointer (c, (t_method)send_pointer);
    class_addList (c, (t_method)send_list);
    class_addAnything (c, (t_method)send_anything);
    
    send_class = c;
}

void send_destroy (void)
{
    class_free (send_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
