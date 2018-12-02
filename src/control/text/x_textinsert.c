
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "../../m_spaghettis.h"
#include "../../m_core.h"
#include "../../g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

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
    
        buffer_extend (b, start, start, count + 1);
        buffer_setAtIndex (b, start + count, &a);
        
    } else {
    
        int oldSize = buffer_getSize (b);
        int addSemi = !buffer_isLastMessageProperlyEnded (b);
        int newSize = oldSize + addSemi + count + 1;
        
        buffer_resize (b, newSize);
        buffer_setAtIndex (b, newSize - 1, &a);
            
        start = oldSize;
            
        if (addSemi) { buffer_setAtIndex (b, oldSize, &a); start++; }
    }
    
    for (i = 0; i < count; i++) { buffer_setAtIndex (b, start + i, argv + i); }
    
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

static t_buffer *textinsert_functionData (t_gobj *z, int flags)
{
    if (SAVED_DEEP (flags)) {
    //
    t_textinsert *x = (t_textinsert *)z;
    t_buffer *b = buffer_new();
    
    buffer_appendSymbol (b, sym__restore);
    buffer_appendFloat (b, x->x_line);
    
    return b;
    //
    }
    
    return NULL;
}

static void textinsert_restore (t_textinsert *x, t_float f)
{
    x->x_line = f;
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
        
        inlet_newSymbol (cast_object (x), TEXTCLIENT_NAME (&x->x_textclient));
    
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
            NULL,
            sizeof (t_textinsert),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addList (c, (t_method)textinsert_list);
    class_addAnything (c, (t_method)textinsert_anything);
    
    class_addMethod (c, (t_method)textinsert_restore, sym__restore, A_FLOAT, A_NULL);
    
    class_setDataFunction (c, textinsert_functionData);
    class_setHelpName (c, sym_text);
    
    textinsert_class = c;
}

void textinsert_destroy (void)
{
    class_free (textinsert_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
