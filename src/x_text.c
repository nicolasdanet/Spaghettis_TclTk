
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd         *pd_newest;
extern t_pd         pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class             *textdefine_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textdefine {
    t_textbuffer    x_textbuffer;                       /* Must be the first. */
    t_gpointer      x_gpointer;
    int             x_keep;
    t_symbol        *x_name;
    t_scalar        *x_scalar;
    t_outlet        *x_outlet;
    } t_textdefine;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error scalar_setInternalBuffer    (t_scalar *, t_symbol *, t_buffer *);
t_error scalar_unsetInternalBuffer  (t_scalar *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textdefine_initialize (void)
{
    static char *textTemplateFile = 
        "canvas 0 0 458 153 10;\n"
        "#X obj 43 31 struct text float x float y text t;\n";

    t_buffer *b = buffer_new();
    
    environment_setActiveFile (sym__texttemplate, sym___dot__);
    buffer_withStringUnzeroed (b, textTemplateFile, (int)strlen (textTemplateFile));
    buffer_eval (b, &pd_canvasMaker, 0, NULL);
    pd_message (pd_getBoundX(), sym__pop, 0, NULL);
    
    environment_setActiveFile (&s_, &s_);
    
    buffer_free (b);  
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textdefine_bang (t_textdefine *x)
{
    gpointer_setAsScalar (&x->x_gpointer, textbuffer_getView (&x->x_textbuffer), x->x_scalar);
    outlet_pointer (x->x_outlet, &x->x_gpointer);
}

void textdefine_set (t_textdefine *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_deserialize (textbuffer_getBuffer (&x->x_textbuffer), argc, argv);
    textbuffer_update (&x->x_textbuffer);
}

static void textdefine_clear (t_textdefine *x)
{
    buffer_reset (textbuffer_getBuffer (&x->x_textbuffer));
    textbuffer_update (&x->x_textbuffer);
}

static void textdefine_functionSave (t_gobj *z, t_buffer *b)
{
    t_textdefine *x = (t_textdefine *)z;
    
    buffer_vAppend (b, "ssii", 
        sym___hash__X,
        sym_obj,
        object_getX (cast_object (x)),
        object_getY (cast_object (x)));
        
    buffer_serialize (b, object_getBuffer (cast_object (x)));
    buffer_appendSemicolon (b);
    object_saveWidth (cast_object (x), b);
        
    if (x->x_keep) {
        buffer_vAppend (b, "ss", sym___hash__A, sym_set);
        buffer_serialize (b, textbuffer_getBuffer (&x->x_textbuffer));
        buffer_appendSemicolon (b);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *textdefine_new (t_symbol *s, int argc, t_atom *argv)
{
    t_textdefine *x = (t_textdefine *)pd_new (textdefine_class);

    textbuffer_init (&x->x_textbuffer);
    gpointer_init (&x->x_gpointer);
    
    x->x_keep = 0;
    x->x_name = &s_;
    
    while (argc && IS_SYMBOL (argv)) {
        if (GET_SYMBOL (argv) == sym___dash__k || GET_SYMBOL (argv) == sym___dash__keep) {
            x->x_keep = 1; argc--; argv++;
        } else {
            break;
        }
    }
    
    if (!error__options (s, argc, argv)) {
        if (argc && IS_SYMBOL (argv)) {
            pd_bind (cast_pd (x), GET_SYMBOL (argv));
            x->x_name = GET_SYMBOL (argv);
            argc--; argv++;
        }
    }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_scalar = scalar_new (canvas_getCurrent(), sym___TEMPLATE__text);
    
    {
        t_error err = scalar_setInternalBuffer (x->x_scalar, sym_t, textbuffer_getBuffer (&x->x_textbuffer));
        PD_ASSERT (!err);
    }

    x->x_outlet = outlet_new (cast_object (x), &s_pointer);
    
    pd_setBoundA (cast_pd (x));
    
    return x;
}

static void *textdefine_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = textdefine_new (s, argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_d || t == sym_define)  { pd_newest = textdefine_new (s,        argc - 1, argv + 1); }
    else if (t == sym_get)              { pd_newest = textget_new (s,           argc - 1, argv + 1); }
    else if (t == sym_set)              { pd_newest = textset_new (s,           argc - 1, argv + 1); }
    else if (t == sym_size)             { pd_newest = textsize_new (s,          argc - 1, argv + 1); }
    else if (t == sym_tolist)           { pd_newest = texttolist_new (s,        argc - 1, argv + 1); }
    else if (t == sym_fromlist)         { pd_newest = textfromlist_new (s,      argc - 1, argv + 1); }
    else if (t == sym_search)           { pd_newest = textsearch_new (s,        argc - 1, argv + 1); }
    else if (t == sym_sequence)         { pd_newest = textsequence_new (s,      argc - 1, argv + 1); }
    else {
        error_unexpected (sym_text, t);
    }
    //
    }
    
    return pd_newest;
}

static void textdefine_free (t_textdefine *x)
{
    if (x->x_name != &s_) { pd_unbind (cast_pd (x), x->x_name); }
    
    pd_setBoundA (NULL);
    
    {
        t_error err = scalar_unsetInternalBuffer (x->x_scalar, sym_t);
        PD_ASSERT (!err);
    }
    
    pd_free (cast_pd (x->x_scalar));
    gpointer_unset (&x->x_gpointer);
    textbuffer_free (&x->x_textbuffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void textdefine_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_text__space__define,
            (t_newmethod)textdefine_makeObject,
            (t_method)textdefine_free,
            sizeof (t_textdefine),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
    
    class_addCreator ((t_newmethod)textdefine_makeObject, sym_text, A_GIMME, A_NULL);
        
    class_addBang (c, (t_method)textdefine_bang);
    class_addClick (c, (t_method)textbuffer_click);
    
    class_addMethod (c, (t_method)textbuffer_close,     sym_close,      A_NULL);
    class_addMethod (c, (t_method)textbuffer_addLine,   sym__addline,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textbuffer_write,     sym_write,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textbuffer_read,      sym_read,       A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)textdefine_set,       sym_set,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)textdefine_clear,     sym_clear,      A_NULL);
        
    class_setSaveFunction (c, textdefine_functionSave);
    class_setHelpName (c, sym_text);

    textdefine_class = c;
}

void textdefine_destroy (void)
{
    CLASS_FREE (textdefine_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
