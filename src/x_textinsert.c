
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

typedef struct _textinsert {
    t_textclient    x_textclient;               /* Must be the first. */
    t_float         x_line;
    } t_textinsert;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_class *textinsert_class;               /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void textinsert_list (t_textinsert *x, t_symbol *s, int argc, t_atom *argv)
{
    t_buffer *b = textclient_fetchBuffer (&x->x_textclient);
    
    if (b) {
    //
    int line = PD_MAX (0, x->x_line);
    int start, end, count = argc;
    int i;
    
    t_atom a; SET_SEMICOLON (&a);
    
    if (buffer_getMessageAt (b, line, &start, &end) == PD_ERROR_NONE) {
    
        buffer_resizeBetween (b, start, start, count + 1);
        buffer_setAtomAtIndex (b, start + count, &a);
        
    } else {
    
        int oldSize = buffer_getSize (b);
        int addSemi = !buffer_isLastMessageProperlyEnded (b);
        int newSize = oldSize + addSemi + count + 1;
        
        buffer_resize (b, newSize);
        buffer_setAtomAtIndex (b, newSize - 1, &a);
            
        start = oldSize;
            
        if (addSemi) { buffer_setAtomAtIndex (b, oldSize, &a); start++; }
    }
    
    for (i = 0; i < count; i++) { buffer_setAtomAtIndex (b, start + i, atom_substituteIfPointer (argv + i)); }
    
    textclient_update (&x->x_textclient);
    //
    } else { error_undefined (sym_text__space__insert, sym_text); }
}

static void textinsert_anything (t_textinsert *x, t_symbol *s, int argc, t_atom *argv)
{
    utils_anythingToList (cast_pd (x), (t_listmethod)textinsert_list, s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void *textinsert_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textinsert *x = (t_textinsert *)pd_new (textinsert_class);
    
    t_error err = textclient_init (&x->x_textclient, &argc, &argv);         /* It may consume arguments. */
    
    if (!err) {
    
        x->x_line = 0;
        
        inlet_newFloat (cast_object (x), &x->x_line);
        
        if (argc && IS_FLOAT (argv)) { x->x_line = GET_FLOAT (argv); argc--; argv++; }
        
        if (argc) { warning_unusedArguments (sym_text__space__insert, argc, argv); }
        
        if (TEXTCLIENT_ASPOINTER (&x->x_textclient)) {
            inlet_newPointer (cast_object (x), TEXTCLIENT_GETPOINTER (&x->x_textclient));
        } else {
            inlet_newSymbol (cast_object (x),  TEXTCLIENT_GETNAME    (&x->x_textclient));
        }
    
    } else {
        
        error_invalidArguments (sym_text__space__insert, argc, argv); pd_free (cast_pd (x)); x = NULL;
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textinsert_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__insert,
            (t_newmethod)textinsert_new,
            (t_method)textclient_free,
            sizeof (t_textinsert),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)textinsert_list);
    class_addAnything (c, (t_method)textinsert_anything);
    
    class_setHelpName (c, sym_text);
    
    textinsert_class = c;
}

void textinsert_destroy (void)
{
    class_free (textinsert_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
