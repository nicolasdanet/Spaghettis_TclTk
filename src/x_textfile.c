
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

static t_class *textfile_class;         /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void textfile_bang (t_qlist *x)
{
    t_buffer *b = textbuffer_getBuffer (&x->ql_textbuffer);
    
    int i = x->ql_indexOfStart;
    int j, count;
    
    while (i < buffer_size (b) && IS_SEMICOLON_OR_COMMA (buffer_atomAtIndex (b, i))) { i++; }
    
    j = i;
    
    while (j < buffer_size (b) && !IS_SEMICOLON_OR_COMMA (buffer_atomAtIndex (b, j))) { j++; }

    count = j - i;
    
    if (count <= 0) { x->ql_indexOfStart = PD_INT_MAX; outlet_bang (x->ql_outletRight); }
    else {
    //
    t_atom *first = buffer_atomAtIndex (b, i);
    
    x->ql_indexOfStart = j;
        
    if (IS_SYMBOL (first)) { outlet_anything (x->ql_outletLeft, GET_SYMBOL (first), count - 1, first + 1); } 
    else {
        outlet_list (x->ql_outletLeft, NULL, count, first);
    }
    //
    }
}

static void textfile_rewind (t_qlist *x)
{
    x->ql_indexOfStart = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *textfile_new (void)
{
    t_qlist *x = (t_qlist *)pd_new (textfile_class);
    
    textbuffer_init (&x->ql_textbuffer);
    
    x->ql_indexOfStart  = PD_INT_MAX;
    x->ql_outletLeft    = outlet_new (cast_object (x), &s_list);
    x->ql_outletRight   = outlet_new (cast_object (x), &s_bang);

    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textfile_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_textfile,
            (t_newmethod)textfile_new,
            (t_method)textbuffer_free,
            sizeof (t_qlist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addBang (c, textfile_bang);
    
    class_addClick (c, textbuffer_click);
        
    class_addMethod (c, (t_method)textfile_rewind,      sym_rewind,     A_NULL);
    
    class_addMethod (c, (t_method)qlist_clear,          sym_clear,      A_NULL);
    class_addMethod (c, (t_method)qlist_set,            sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_add,            sym_add,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_append,         sym_append,     A_GIMME, A_NULL);
    class_addMethod (c, (t_method)qlist_read,           sym_read,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)qlist_write,          sym_write,      A_SYMBOL, A_NULL);

    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_addLine,   sym__addline,   A_GIMME, A_NULL);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)qlist_append,         sym_add2,       A_GIMME, A_NULL);
        
    #endif
    
    textfile_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
