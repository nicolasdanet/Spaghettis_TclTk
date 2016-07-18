
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *template_class;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int template_exist (t_template *x)
{
    if (!x) { return 0; }
    else {
    //
    int i, size = x->tp_size;
    t_dataslot *v = x->tp_vector;
    
    for (i = 0; i < size; i++, v++) {
    //
    if (v->ds_type == DATA_ARRAY) {
    //
    t_template *elementTemplate = template_findByIdentifier (v->ds_templateIdentifier);
    if (!elementTemplate || !template_exist (elementTemplate)) {
        return 0;
    }
    //
    }
    //
    }
    //
    }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int template_findField (t_template *x, t_symbol *name, int *onset, int *type, t_symbol **templateIdentifier)
{
    PD_ASSERT (x);
    
    if (x) {
    //
    int i;
    
    for (i = 0; i < x->tp_size; i++) {
    //
    if (x->tp_vector[i].ds_fieldName == name) {
    
        *onset              = i * sizeof (t_word);
        *type               = x->tp_vector[i].ds_type;
        *templateIdentifier = x->tp_vector[i].ds_templateIdentifier;
        
        return 1;
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int template_getRaw (t_template *x, t_symbol *name, int *index, int *type, t_symbol **templateIdentifier)
{
    PD_ASSERT (x);
    
    if (x) {
    //
    int i;
    
    for (i = 0; i < x->tp_size; i++) {
    //
    if (x->tp_vector[i].ds_fieldName == name) {
    
        *index              = i;
        *type               = x->tp_vector[i].ds_type;
        *templateIdentifier = x->tp_vector[i].ds_templateIdentifier;
        
        return 1;
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_float template_getFloat (t_template *x, t_symbol *fieldName, t_word *w)
{
    int i, type;
    t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) {
        if (type == DATA_FLOAT) { return *(t_float *)(w + i); }
    }

    return 0.0;
}

void template_setFloat (t_template *x, t_symbol *fieldName, t_word *w, t_float f)
{
    int i, type;
    t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) {
        if (type == DATA_FLOAT) { *(t_float *)(w + i) = f; }
    }
}

t_symbol *template_getSymbol (t_template *x, t_symbol *fieldName, t_word *w)
{
    int i, type;
    t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) {
        if (type == DATA_SYMBOL) { return *(t_symbol **)(w + i); }
    }

    return &s_;
}

void template_setSymbol (t_template *x, t_symbol *fieldName, t_word *w, t_symbol *s)
{
    int i, type;
    t_symbol *dummy = NULL;
    
    if (template_findField (x, fieldName, &i, &type, &dummy)) {
        if (type == DATA_SYMBOL) { *(t_symbol **)(w + i) = s; }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_template *template_findByIdentifier (t_symbol *s)
{
    return ((t_template *)pd_findByClass (s, template_class));
}

static void template_create (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_SYMBOL (argv)) {
    //
    t_symbol *templateIdentifier = utils_makeBindSymbol (atom_getSymbolAtIndex (0, argc, argv));
    
    argc--;
    argv++;
    
    if (template_findByIdentifier (templateIdentifier) == NULL) { 
        template_new (templateIdentifier, argc, argv);
    } else {
        PD_BUG;
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void template_newParse (t_template *x, int argc, t_atom *argv)
{
    while (argc > 0) {

        if ((argc >= 2) && IS_SYMBOL (argv + 0) && IS_SYMBOL (argv + 1)) {
        //
        t_symbol *type               = GET_SYMBOL (argv + 0);
        t_symbol *fieldName          = GET_SYMBOL (argv + 1);
        t_symbol *templateIdentifier = &s_;
        
        int k = -1;
        
        #if PD_WITH_LEGACY
        
        if (type == sym_list) { type = sym_text; }
            
        #endif
        
        if (type == sym_float)       { k = DATA_FLOAT;  }
        else if (type == sym_symbol) { k = DATA_SYMBOL; }
        else if (type == sym_text)   { k = DATA_TEXT;   }
        else if (type == sym_array)  {
            if (argc >= 3 && IS_SYMBOL (argv + 2)) {
                templateIdentifier = utils_makeBindSymbol (GET_SYMBOL (argv + 2));
                k = DATA_ARRAY;
                argc--;
                argv++;
            }
        }
        
        if (k < 0) { PD_BUG; }
        else {
        //
        int oldSize = x->tp_size;
        int newSize = x->tp_size + 1;
        size_t m = oldSize * sizeof (t_dataslot);
        size_t n = newSize * sizeof (t_dataslot);
        
        x->tp_vector = (t_dataslot *)PD_MEMORY_RESIZE (x->tp_vector, m, n);
        x->tp_size   = newSize;
        
        x->tp_vector[newSize - 1].ds_type               = k;
        x->tp_vector[newSize - 1].ds_fieldName          = fieldName;
        x->tp_vector[newSize - 1].ds_templateIdentifier = templateIdentifier;
        //
        }
        //
        }

    argc -= 2;
    argv += 2;
    //
    }
}
    
t_template *template_new (t_symbol *templateIdentifier, int argc, t_atom *argv)
{
    t_template *x = (t_template *)pd_new (template_class);
    
    PD_ASSERT (templateIdentifier);
    
    x->tp_size               = 0;
    x->tp_vector             = (t_dataslot *)PD_MEMORY_GET (0);
    x->tp_list               = NULL;
    x->tp_templateIdentifier = templateIdentifier;
    
    template_newParse (x, argc, argv);
    
    pd_bind (cast_pd (x), x->tp_templateIdentifier);

    return x;
}

void template_free (t_template *x)
{
    pd_unbind (cast_pd (x), x->tp_templateIdentifier);
    
    PD_MEMORY_FREE (x->tp_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void template_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_template,
        NULL, 
        (t_method)template_free,
        sizeof (t_template),
        CLASS_NOBOX,
        A_NULL);
    
    class_addMethod (pd_canvasMaker, (t_method)template_create, sym_struct, A_GIMME, A_NULL);
        
    template_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
