
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "g_graphics.h"
#include "x_text.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textset {
    t_textclient    x_textclient;           /* Must be the first. */
    t_float         x_line;
    t_float         x_field;
    } t_textset;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *textset_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void textset_list (t_textset *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
    //
    int line  = x->x_line;
    int field = x->x_field;
    
    if (line >= 0) {
    //
    int start, end, count = argc; 
    int i;
        
    if (buffer_getMessageAt (b, line, &start, &end) == PD_ERROR_NONE) {
    
        int size = end - start;
        
        if (field < 0) {
            if (size != count) { buffer_resizeBetween (b, start, end, count); }
            
        } else {
            if (field >= size) { return; }
            else { 
                count = PD_MIN (count, size - field); start += field;
            }
        }
        
    } else { 
    
        if (field < 0) {

            int oldSize = buffer_getSize (b);
            int addSemi = !buffer_isLastMessageProperlyEnded (b);
            int newSize = oldSize + addSemi + count + 1;
            
            t_atom a; SET_SEMICOLON (&a);
            
            buffer_resize (b, newSize);
            buffer_setAtomAtIndex (b, newSize - 1, &a);
            
            start = oldSize;
            
            if (addSemi) { buffer_setAtomAtIndex (b, oldSize, &a); start++; }
            
        } else {
            return;
        }
    }
    
    for (i = 0; i < count; i++) { buffer_setAtomAtIndex (b, start + i, atom_substituteIfPointer (argv + i)); }
    
    textclient_update (&x->x_textclient);
    //
    }
    //
    } else { error_undefined (sym_text__space__set, sym_text); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *textset_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textset *x = (t_textset *)pd_new (textset_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);         /* It may consume arguments. */
    
    if (!err) {
    
        x->x_line  = 0;
        x->x_field = -1;
        
        inlet_newFloat (cast_object (x), &x->x_line);
        inlet_newFloat (cast_object (x), &x->x_field);
        
        if (argc && IS_FLOAT (argv)) { x->x_line  = GET_FLOAT (argv); argc--; argv++; }
        if (argc && IS_FLOAT (argv)) { x->x_field = GET_FLOAT (argv); argc--; argv++; }
        
        if (argc) { warning_unusedArguments (sym_text__space__set, argc, argv); }
        
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
    
    } else {
        
        error_invalidArguments (sym_text__space__set, argc, argv); pd_free (cast_pd (x)); x = NULL;
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textset_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__set,
            (t_newmethod)textset_new,
            (t_method)textclient_free,
            sizeof (t_textset),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)textset_list);
    
    class_setHelpName (c, sym_text);
    
    textset_class = c;
}

void textset_destroy (void)
{
    class_free (textset_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
