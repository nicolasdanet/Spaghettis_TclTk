
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
#include "g_graphics.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *texttolist_class;           /* Shared. */
static t_class *textfromlist_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *texttolist_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textclient *x = (t_textclient *)pd_new (texttolist_class);
    
    textclient_init (x, &argc, &argv);                  /* Note that it may consume arguments. */
        
    outlet_new (cast_object (x), &s_list);

    if (TEXTCLIENT_ASPOINTER (x)) {
        inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (x));
    } else {
        inlet_newSymbol (cast_object (x), TEXTCLIENT_GETNAME (x));
    }
    
    return x;
}

static void texttolist_bang (t_textclient *x)
{
    t_buffer *b = textclient_fetchBuffer (x);
    
    if (b) {
        t_buffer *t = buffer_new();
        buffer_serialize (t, b);
        outlet_list (cast_object (x)->te_outlet, NULL, buffer_size (t), buffer_atoms (t));
        buffer_free (t);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textfromlist_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textclient *x = (t_textclient *)pd_new (textfromlist_class);
    
    textclient_init (x, &argc, &argv);                  /* Note that it may consume arguments. */
    
    if (TEXTCLIENT_ASPOINTER (x)) {
        inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (x));
    } else {
        inlet_newSymbol (cast_object (x), TEXTCLIENT_GETNAME (x));
    }
    
     return x;
}

static void textfromlist_list (t_textclient *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (x);
    
    if (b) {
        buffer_reset (b);
        buffer_deserialize (b, argc, argv);
        textclient_update (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textlist_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__tolist,
            (t_newmethod)texttolist_new,
            (t_method)textclient_free,
            sizeof (t_textclient),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, texttolist_bang);
    class_setHelpName (c, sym_text);

    texttolist_class = c;
    
    c = class_new (sym_text__space__fromlist,
            (t_newmethod)textfromlist_new,
            (t_method)textclient_free,
            sizeof (t_textclient),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, textfromlist_list);
    class_setHelpName (c, sym_text);
    
    textfromlist_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
