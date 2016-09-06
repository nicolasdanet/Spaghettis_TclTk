
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

static void textfile_bang(t_qlist *x)
{
    int argc = buffer_size(textbuffer_getBuffer (&x->x_textbuf)),
        count, onset = x->x_onset, onset2;
    t_atom *argv = buffer_atoms(textbuffer_getBuffer (&x->x_textbuf));
    t_atom *ap = argv + onset, *ap2;
    while (onset < argc &&
        (ap->a_type == A_SEMICOLON || ap->a_type == A_COMMA))
            onset++, ap++;
    onset2 = onset;
    ap2 = ap;
    while (onset2 < argc &&
        (ap2->a_type != A_SEMICOLON && ap2->a_type != A_COMMA))
            onset2++, ap2++;
    if (onset2 > onset)
    {
        x->x_onset = onset2;
        if (ap->a_type == A_SYMBOL)
            outlet_anything(cast_object (x)->te_outlet, ap->a_w.w_symbol,
                onset2-onset-1, ap+1);
        else outlet_list(cast_object (x)->te_outlet, 0, onset2-onset, ap);
    }
    else
    {
        x->x_onset = PD_INT_MAX;
        outlet_bang(x->x_bangout);
    }
}

static void textfile_rewind(t_qlist *x)
{
    x->x_onset = 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *textfile_new( void)
{
    t_qlist *x = (t_qlist *)pd_new(textfile_class);
    textbuffer_init (&x->x_textbuf);
    outlet_new(cast_object (x), &s_list);
    x->x_bangout = outlet_new(cast_object (x), &s_bang);
    x->x_onset = PD_INT_MAX;
    x->x_rewound = 0;
    x->x_tempo = 1;
    x->x_whenclockset = 0;
    x->x_clockdelay = 0;
    x->x_clock = NULL;
    return (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textfile_setup(void )
{
    textfile_class = class_new(sym_textfile, (t_newmethod)textfile_new,
        (t_method)textbuffer_free, sizeof(t_qlist), 0, 0);
    class_addMethod(textfile_class, (t_method)textfile_rewind, sym_rewind,
        0);
    class_addMethod(textfile_class, (t_method)qlist_set, sym_set,
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_clear, sym_clear, 0);
    class_addMethod(textfile_class, (t_method)qlist_add, sym_add,
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_add2, sym_add2, /* LEGACY !!! */
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_add, sym_append, /* LEGACY !!! */
        A_GIMME, 0);
    class_addMethod(textfile_class, (t_method)qlist_read, sym_read, 
        A_SYMBOL, A_DEFSYMBOL, 0);
    class_addMethod(textfile_class, (t_method)qlist_write, sym_write, 
        A_SYMBOL, A_DEFSYMBOL, 0);
    class_addClick (textfile_class, textbuffer_click);
    //class_addMethod(textfile_class, (t_method)textbuffer_open, sym_click, 0);
    class_addMethod(textfile_class, (t_method)textbuffer_close, sym_close, 
        0);
    class_addMethod(textfile_class, (t_method)textbuffer_add, 
        sym__addline, A_GIMME, 0);
    /*class_addMethod(textfile_class, (t_method)qlist_print, gen_sym ("print"),
        A_DEFSYMBOL, 0);*/
    class_addBang(textfile_class, textfile_bang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
