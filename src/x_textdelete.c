
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_text.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textdelete {
    t_textclient x_textclient;                  /* Must be the first. */
    } t_textdelete;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *textdelete_class;               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void textdelete_float (t_textdelete *x, t_float f)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
    //
    int start, end;
    t_atomtype type;
    
    if (buffer_getMessageAtWithTypeOfEnd (b, (int)f, &start, &end, &type) == PD_ERROR_NONE) {
        if (type == A_COMMA || type == A_SEMICOLON) { end++; }
        buffer_expand (b, start, end , 0);
    }
        
    textclient_update (&x->x_textclient);
    //
    } else { error_undefined (sym_text__space__delete, sym_text); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *textdelete_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textdelete *x = (t_textdelete *)pd_new (textdelete_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);         /* It may consume arguments. */
    
    if (!err) {
    
        if (argc) { warning_unusedArguments (sym_text__space__delete, argc, argv); }
        
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
    
    } else {
        
        error_invalidArguments (sym_text__space__delete, argc, argv); pd_free (cast_pd (x)); x = NULL;
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textdelete_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__delete,
            (t_newmethod)textdelete_new,
            (t_method)textclient_free,
            sizeof (t_textdelete),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addFloat (c, (t_method)textdelete_float);
    
    class_setHelpName (c, sym_text);
    
    textdelete_class = c;
}

void textdelete_destroy (void)
{
    class_free (textdelete_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
