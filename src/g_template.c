
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
#include "m_alloca.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class  *template_class;                    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int template_getSize (t_template *x)
{
    return x->tp_size;
}

t_dataslot *template_getSlots (t_template *x)
{
    return x->tp_vector;
}

t_symbol *template_getTemplateIdentifier (t_template *x)
{
    return x->tp_templateIdentifier;
}

t_template *template_getTemplateIfArrayAtIndex (t_template *x, int n)
{
    PD_ASSERT (x);
    PD_ASSERT (n >= 0);
    PD_ASSERT (n < x->tp_size);
    
    if (x->tp_vector[n].ds_type == DATA_ARRAY) {
        return template_findByIdentifier (x->tp_vector[n].ds_templateIdentifier);
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int template_hasInstance (t_template *x)
{
    return (x->tp_instance != NULL);
}

void template_registerInstance (t_template *x, t_struct *o)
{
    paint_scalarsEraseAll();
    x->tp_instance = o;
    paint_scalarsDrawAll();
}

void template_unregisterInstance (t_template *x, t_struct *o)
{
    template_registerInstance (x, NULL);
}

t_glist *template_getFirstInstanceView (t_template *x)
{
    PD_ASSERT (x);
    
    if (!x->tp_instance) { return NULL; }
    else { 
        return struct_getView (x->tp_instance);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void template_serialize (t_template *x, t_buffer *b)
{
    int i;
    
    buffer_vAppend (b, "sss", sym___hash__N, sym_struct, utils_stripBindSymbol (x->tp_templateIdentifier));
    
    for (i = 0; i < x->tp_size; i++) {
    //
    t_symbol *type = &s_float;
    
    switch (x->tp_vector[i].ds_type) {
        case DATA_FLOAT     : type = &s_float;  break;
        case DATA_SYMBOL    : type = &s_symbol; break;
        case DATA_ARRAY     : type = sym_array; break;
        case DATA_TEXT      : type = sym_text;  break;
    }
    
    if (x->tp_vector[i].ds_type == DATA_ARRAY) {
        buffer_vAppend (b, "sss",
            type,
            x->tp_vector[i].ds_fieldName,
            utils_stripBindSymbol (x->tp_vector[i].ds_templateIdentifier));
            
    } else {
        buffer_vAppend (b,  "ss",
            type,
            x->tp_vector[i].ds_fieldName);
    }
    //
    }
    
    buffer_appendSemicolon (b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void template_notify (t_template *x,
    t_glist *owner,
    t_scalar *scalar,
    t_symbol *s,
    int argc,
    t_atom *argv)
{
    t_atom *a = NULL;
    int i, n = argc + 1;
    t_gpointer gp = GPOINTER_INIT;
    
    ATOMS_ALLOCA (a, n);
    
    gpointer_setAsScalar (&gp, owner, scalar);
    SET_POINTER (a, &gp);
    for (i = 0; i < argc; i++) { *(a + i + 1) = *(argv + i); }
    if (x->tp_instance) { struct_notify (x->tp_instance, s, n, a); }
    gpointer_unset (&gp);
    
    ATOMS_FREEA (a, n);
}

static void template_anything (t_template *x, t_symbol *s, int argc, t_atom *argv)
{
    #if PD_WITH_DEBUG
    
    post ("My name is %s.", utils_stripBindSymbol (x->tp_templateIdentifier)->s_name);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int template_findField (t_template *x,
    t_symbol *fieldName,
    int *onset,
    int *type,
    t_symbol **templateIdentifier)
{
    PD_ASSERT (x);
    
    if (x) {
    //
    int i;
    
    for (i = 0; i < x->tp_size; i++) {
    //
    if (x->tp_vector[i].ds_fieldName == fieldName) {
    
        *onset              = i * ARRAY_WORD;
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

int template_isValid (t_template *x)
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
    if (!elementTemplate || !template_isValid (elementTemplate)) {
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

int template_hasField (t_template *x, t_symbol *fieldName)
{
    int i, t; t_symbol *dummy = NULL;
    
    return (template_getRaw (x, fieldName, &i, &t, &dummy));
}

int template_getIndexOfField (t_template *x, t_symbol *fieldName)
{
    int i, t; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &t, &dummy)) { return i; }
    else {
        return -1;
    }
}

t_symbol *template_getFieldAtIndex (t_template *x, int n)
{
    if (n >= 0 && n < x->tp_size) { return x->tp_vector[n].ds_fieldName; }
    else {
        return NULL;
    }
}

int template_getRaw (t_template *x,
    t_symbol *fieldName,
    int *index,
    int *type,
    t_symbol **templateIdentifier)
{
    PD_ASSERT (x);
    
    if (x) {
    //
    int i;
    
    for (i = 0; i < x->tp_size; i++) {
    //
    if (x->tp_vector[i].ds_fieldName == fieldName) {
    
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

int template_fieldIsFloat (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_FLOAT); }
    
    return 0;
}

int template_fieldIsSymbol (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_SYMBOL); }
    
    return 0;
}

int template_fieldIsText (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_TEXT); }
    
    return 0;
}

int template_fieldIsArray (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *dummy = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &dummy)) { return (type == DATA_ARRAY); }
    
    return 0;
}

int template_fieldIsArrayAndValid (t_template *x, t_symbol *fieldName)
{
    int i, type; t_symbol *templateIdentifier = NULL;
    
    if (template_getRaw (x, fieldName, &i, &type, &templateIdentifier)) { 
        if (type == DATA_ARRAY) {
            return (template_findByIdentifier (templateIdentifier) != NULL);
        }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *template_makeTemplateIdentifier (t_symbol *s)
{
    PD_ASSERT (s);
    
    if (s == &s_ || s == sym___dash__) { return template_getWildcard(); }
    else { 
        return (utils_makeBindSymbol (s));
    }
}

t_symbol *template_getWildcard (void)
{
    return &s_;
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
    
    /* Empty template should be managed appropriately elsewhere. */
    
    PD_ASSERT (utils_stripBindSymbol (templateIdentifier) != &s_); 
        
    x->tp_size               = 0;
    x->tp_vector             = (t_dataslot *)PD_MEMORY_GET (0);
    x->tp_templateIdentifier = templateIdentifier;
    x->tp_instance           = NULL;
        
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
    
    class_addAnything (c, template_anything);
        
    class_addMethod (pd_canvasMaker, (t_method)template_create, sym_struct, A_GIMME, A_NULL);
    
    template_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
