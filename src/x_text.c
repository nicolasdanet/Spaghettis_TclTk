
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

t_class             *textdefine_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _textdefine {
    t_textbuffer    x_textbuffer;                       /* Must be the first. */
    t_gpointer      x_gpointer;
    int             x_keep;
    t_symbol        *x_name;
    t_scalar        *x_scalar;
    t_outlet        *x_outletLeft;
    t_outlet        *x_outletRight;
    } t_textdefine;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error scalar_setInternalBuffer    (t_scalar *, t_symbol *, t_buffer *);
t_error scalar_unsetInternalBuffer  (t_scalar *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textdefine_initialize (void)
{
    static const char *textTemplateFile =   /* Static. */
        "#N canvas 0 22 450 300 12;\n"
        "#X obj 43 31 struct text float x float y text t;\n";

    instance_loadInvisible (sym__texttemplate, textTemplateFile); 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void textdefine_bang (t_textdefine *x)
{
    gpointer_setAsScalar (&x->x_gpointer, textbuffer_getView (&x->x_textbuffer), x->x_scalar);
    outlet_pointer (x->x_outletLeft, &x->x_gpointer);
}

void textdefine_set (t_textdefine *x, t_symbol *s, int argc, t_atom *argv)
{
    buffer_deserialize (textbuffer_getBuffer (&x->x_textbuffer), argc, argv);
    textbuffer_update (&x->x_textbuffer);
}

static void textdefine_clear (t_textdefine *x)
{
    buffer_clear (textbuffer_getBuffer (&x->x_textbuffer));
    textbuffer_update (&x->x_textbuffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void textdefine_modified (t_textdefine *x)
{
    outlet_symbol (x->x_outletRight, sym_updated);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void textdefine_functionSave (t_gobj *z, t_buffer *b)
{
    t_textdefine *x = (t_textdefine *)z;
    
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_obj);
    buffer_appendFloat (b,  object_getX (cast_object (x)));
    buffer_appendFloat (b,  object_getY (cast_object (x)));
    buffer_serialize (b,    object_getBuffer (cast_object (x)));
    buffer_appendSemicolon (b);
    object_serializeWidth (cast_object (x), b);
}

static t_error textdefine_functionData (t_gobj *z, t_buffer *b)
{
    t_textdefine *x = (t_textdefine *)z;
    
    if (x->x_keep) {
    //
    buffer_appendSymbol (b, sym_set);
    buffer_serialize (b, textbuffer_getBuffer (&x->x_textbuffer));
    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    /* Dollar expansion is zero in abstraction opened as patch. */
    
    if (argc && IS_FLOAT (argv) && (GET_FLOAT (argv) == 0.0)) { argc--; argv++; }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x->x_scalar = scalar_new (instance_contextGetCurrent(), sym___TEMPLATE__text);
    
    {
        t_error err = scalar_setInternalBuffer (x->x_scalar, sym_t, textbuffer_getBuffer (&x->x_textbuffer));
        PD_UNUSED (err); PD_ASSERT (!err);
    }

    x->x_outletLeft  = outlet_newPointer (cast_object (x));
    x->x_outletRight = outlet_newSymbol (cast_object (x));
    
    return x;
}

static void *textdefine_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    t_pd *newest = NULL;
    
    instance_setNewestObject (NULL);
    
    if (!argc || !IS_SYMBOL (argv))     { newest = (t_pd *)textdefine_new (s,   argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_d || t == sym_define)  { newest = (t_pd *)textdefine_new (s,   argc - 1, argv + 1); }
    else if (t == sym_get)              { newest = (t_pd *)textget_new (s,      argc - 1, argv + 1); }
    else if (t == sym_set)              { newest = (t_pd *)textset_new (s,      argc - 1, argv + 1); }
    else if (t == sym_insert)           { newest = (t_pd *)textinsert_new (s,   argc - 1, argv + 1); }
    else if (t == sym_delete)           { newest = (t_pd *)textdelete_new (s,   argc - 1, argv + 1); }
    else if (t == sym_size)             { newest = (t_pd *)textsize_new (s,     argc - 1, argv + 1); }
    else if (t == sym_tolist)           { newest = (t_pd *)texttolist_new (s,   argc - 1, argv + 1); }
    else if (t == sym_fromlist)         { newest = (t_pd *)textfromlist_new (s, argc - 1, argv + 1); }
    else if (t == sym_search)           { newest = (t_pd *)textsearch_new (s,   argc - 1, argv + 1); }
    else if (t == sym_sequence)         { newest = (t_pd *)textsequence_new (s, argc - 1, argv + 1); }
    else {
        error_unexpected (sym_text, t);
    }
    //
    }
    
    instance_setNewestObject (newest);
    
    return newest;
}

static void textdefine_free (t_textdefine *x)
{
    if (x->x_name != &s_) { pd_unbind (cast_pd (x), x->x_name); }
    
    {
        t_error err = scalar_unsetInternalBuffer (x->x_scalar, sym_t);
        PD_UNUSED (err); PD_ASSERT (!err);
    }
    
    pd_free (cast_pd (x->x_scalar));
    gpointer_unset (&x->x_gpointer);
    textbuffer_free (&x->x_textbuffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    class_addMethod (c, (t_method)textdefine_modified,  sym__modified,  A_NULL);
    
    class_setSaveFunction (c, textdefine_functionSave);
    class_setDataFunction (c, textdefine_functionData);
    class_setHelpName (c, sym_text);

    textdefine_class = c;
}

void textdefine_destroy (void)
{
    class_free (textdefine_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
