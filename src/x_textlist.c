
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *texttolist_class;           /* Shared. */
static t_class *textfromlist_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _texttolist {
    t_textclient    x_textclient;           /* Must be the first. */
    t_outlet        *x_outlet;
    } t_texttolist;

typedef struct _textfromlist {
    t_textclient    x_textclient;           /* Must be the first. */
    } t_textfromlist;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void *texttolist_new (t_symbol *s, int argc, t_atom *argv)
{
    t_texttolist *x = (t_texttolist *)pd_new (texttolist_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);     /* It may consume arguments. */
    
    if (!err) {
    
        x->x_outlet = outlet_new (cast_object (x), &s_list);
        
        if (argc) { warning_unusedArguments (sym_text__space__tolist, argc, argv); }
        
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
    
    } else {
        
        error_invalidArguments (sym_text__space__tolist, argc, argv);
        pd_free (x); x = NULL;
    }
    
    return x;
}

static void texttolist_bang (t_texttolist *x)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
        t_buffer *t = buffer_new();
        buffer_serialize (t, b);
        outlet_list (x->x_outlet, buffer_size (t), buffer_atoms (t));
        buffer_free (t);
        
    } else { error_undefined (sym_text__space__tolist, sym_text); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *textfromlist_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textfromlist *x = (t_textfromlist *)pd_new (textfromlist_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);     /* It may consume arguments. */
    
    if (!err) {
    
        if (argc) { warning_unusedArguments (sym_text__space__fromlist, argc, argv); }
        
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
        
    } else {
    
        error_invalidArguments (sym_text__space__fromlist, argc, argv);
        pd_free (x); x = NULL;
    }
    
    return x;
}

static void textfromlist_list (t_textfromlist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
        buffer_reset (b);
        buffer_deserialize (b, argc, argv);
        textclient_update (&x->x_textclient);
    
    } else { error_undefined (sym_text__space__fromlist, sym_text); } 
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
            sizeof (t_texttolist),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, (t_method)texttolist_bang);
    class_setHelpName (c, sym_text);

    texttolist_class = c;
    
    c = class_new (sym_text__space__fromlist,
            (t_newmethod)textfromlist_new,
            (t_method)textclient_free,
            sizeof (t_textfromlist),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)textfromlist_list);
    
    class_setHelpName (c, sym_text);
    
    textfromlist_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
