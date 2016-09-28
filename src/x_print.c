
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *print_class;    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _print {
    t_object    x_obj;          /* Must be the first. */
    t_symbol    *x_name;
    } t_print;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void print_bang (t_print *x)
{
    post ("%s: bang", x->x_name->s_name);
}

static void print_float (t_print *x, t_float f)
{
    post ("%s: %g", x->x_name->s_name, f);
}

static void print_symbol (t_print *x, t_symbol *s)
{
    post ("%s: %s", x->x_name->s_name, s->s_name);
}

static void print_pointer (t_print *x, t_gpointer *gp)
{
    post ("%s: pointer", x->x_name->s_name);
}

static void print_list (t_print *x, t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post ("%s: [ %s ]", x->x_name->s_name, t);
    
    PD_MEMORY_FREE (t);
}

static void print_anything (t_print *x, t_symbol *s, int argc, t_atom *argv)
{
    char *t = atom_atomsToString (argc, argv);
    
    post ("%s: %s [ %s ]", x->x_name->s_name, s->s_name, t);
    
    PD_MEMORY_FREE (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *print_new (t_symbol *s, int argc, t_atom *argv)
{
    t_print *x = (t_print *)pd_new (print_class);
    
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);
    
    if (t == sym___dash__n || t == sym___dash__none) { x->x_name = &s_; }
    else {
        if (argc) { x->x_name = utils_gensymWithAtoms (argc, argv); } 
        else {
            x->x_name = sym_print;
        }
    }
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void print_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_print,
            (t_newmethod)print_new,
            NULL,
            sizeof (t_print),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addBang (c, print_bang);
    class_addFloat (c, print_float);
    class_addSymbol (c, print_symbol);
    class_addPointer (c, print_pointer);
    class_addList (c, print_list);
    class_addAnything (c, print_anything);
    
    print_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

