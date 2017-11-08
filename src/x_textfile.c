
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

static t_class *textfile_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void textfile_bang (t_qlist *x)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    
    int i = x->ql_indexOfMessage;
    int start, end;
    
    if (buffer_getMessageAt (b, i, &start, &end) == PD_ERROR_NONE) {
    //
    int size = end - start;
    t_atom *first = buffer_getAtomAtIndex (b, start);
        
    if (size && IS_SYMBOL (first)) {
        outlet_anything (x->ql_outletLeft, GET_SYMBOL (first), size - 1, first + 1); 
    } else {
        outlet_list (x->ql_outletLeft, size, size ? first : NULL);
    }
    
    x->ql_indexOfMessage += 1;
    //
    } else { x->ql_indexOfMessage = PD_INT_MAX; outlet_bang (x->ql_outletMiddle); }
}

static void textfile_rewind (t_qlist *x)
{
    x->ql_indexOfMessage = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void *textfile_new (t_symbol *s, int argc, t_atom *argv)
{
    t_qlist *x = (t_qlist *)pd_new (textfile_class);
    
    textbuffer_init (&x->ql_textbuffer);
    
    x->ql_indexOfMessage = 0;
    x->ql_outletLeft     = outlet_new (cast_object (x), &s_list);
    x->ql_outletMiddle   = outlet_new (cast_object (x), &s_bang);
    x->ql_outletRight    = outlet_new (cast_object (x), &s_bang);
    
    if (argc && !IS_FLOAT (argv)) { qlist_read (x, symbol_withAtoms (argc, argv)); }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textfile_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_textfile,
            (t_newmethod)textfile_new,
            (t_method)textbuffer_free,
            sizeof (t_qlist),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addBang (c, (t_method)textfile_bang);
    class_addClick (c, (t_method)textbuffer_click);
        
    class_addMethod (c, (t_method)textfile_rewind,      sym_rewind,     A_NULL);
    
    class_addMethod (c, (t_method)qlist_clear,          sym_clear,      A_NULL);
    class_addMethod (c, (t_method)qlist_set,            sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_add,            sym_add,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_append,         sym_append,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_read,           sym_read,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_write,          sym_write,      A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_modified,       sym__modified,  A_NULL);
    
    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_addLine,   sym__addline,   A_GIMME, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)qlist_append,         sym_add2,       A_GIMME, A_NULL);
        
    #endif
    
    textfile_class = c;
}

void textfile_destroy (void)
{
    class_free (textfile_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
