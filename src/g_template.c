
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *template_class;    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int template_isValid (t_template *x)
{
    if (!x) { return 0; }
    else {
    //
    t_dataslot *v = x->tp_slots;
    int i;
    
    for (i = 0; i < x->tp_size; i++, v++) {
    //
    if (v->ds_type == DATA_ARRAY) {

        t_template *y = template_findByIdentifier (v->ds_templateIdentifier);
        
        if (!y)     { return 0; }                           /* Element's template must exist. */
        if (y == x) { return 0; }                           /* Forbid circular dependencies. */
        if (template_containsArray (y)) { return 0; }       /* Forbid nested arrays. */
    }
    //
    }
    //
    }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int template_containsArray (t_template *x)
{
    t_dataslot *v = x->tp_slots;
    int i;
    
    for (i = 0; i < x->tp_size; i++, v++) { 
        if (v->ds_type == DATA_ARRAY) { 
            return 1; 
        } 
    }
    
    return 0;
}

int template_containsTemplate (t_template *x, t_symbol *templateIdentifier)
{
    if (x->tp_templateIdentifier == templateIdentifier) { return 1; }
    else {
    //
    t_dataslot *v = x->tp_slots;
    int i;

    for (i = 0; i < x->tp_size; i++, v++) { 
        if (v->ds_type == DATA_ARRAY) { 
            if (v->ds_templateIdentifier == templateIdentifier) { 
                return 1; 
            }
        } 
    }
    
    return 0;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int template_hasField (t_template *x, t_symbol *fieldName)
{
    int i, t; t_symbol *dummy = NULL;
    
    return template_getRaw (x, fieldName, &i, &t, &dummy);
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
    if (n >= 0 && n < x->tp_size) { return x->tp_slots[n].ds_fieldName; }
    else {
        return NULL;
    }
}

int template_getRaw (t_template *x,
    t_symbol *fieldName,
    int *position,
    int *type,
    t_symbol **templateIdentifier)
{
    PD_ASSERT (x);
    
    if (x) {
    //
    int i;
    
    for (i = 0; i < x->tp_size; i++) {
    //
    if (x->tp_slots[i].ds_fieldName == fieldName) {
    
        *position           = i;
        *type               = x->tp_slots[i].ds_type;
        *templateIdentifier = x->tp_slots[i].ds_templateIdentifier;
        
        return 1;
    }
    //
    }
    //
    }
    
    return 0;
}

t_template *template_getTemplateIfArrayAtIndex (t_template *x, int n)
{
    PD_ASSERT (x);
    PD_ASSERT (n >= 0);
    PD_ASSERT (x && n < x->tp_size);
    
    if (x->tp_slots[n].ds_type == DATA_ARRAY) {
        return template_findByIdentifier (x->tp_slots[n].ds_templateIdentifier);
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
            PD_ASSERT (template_isValid (template_findByIdentifier (templateIdentifier))); 
            return 1;
        }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void template_serialize (t_template *x, t_buffer *b)
{
    int i;
    
    buffer_vAppend (b, "sss",
        sym___hash__N,
        sym_struct,
        symbol_stripTemplateIdentifier (x->tp_templateIdentifier));
    
    for (i = 0; i < x->tp_size; i++) {
    //
    t_symbol *type = &s_float;
    
    switch (x->tp_slots[i].ds_type) {
        case DATA_FLOAT     : type = &s_float;  break;
        case DATA_SYMBOL    : type = &s_symbol; break;
        case DATA_ARRAY     : type = sym_array; break;
        case DATA_TEXT      : type = sym_text;  break;
    }
    
    if (x->tp_slots[i].ds_type == DATA_ARRAY) {
        buffer_vAppend (b, "sss",
            type,
            x->tp_slots[i].ds_fieldName,
            symbol_stripTemplateIdentifier (x->tp_slots[i].ds_templateIdentifier));
            
    } else {
        buffer_vAppend (b,  "ss",
            type,
            x->tp_slots[i].ds_fieldName);
    }
    //
    }
    
    buffer_appendSemicolon (b);
}

void template_notify (t_template *x,
    t_glist *owner,
    t_scalar *scalar,
    t_symbol *s,
    int argc,
    t_atom *argv)
{
    t_atom *a = NULL;
    int i, n = argc + 1;
    t_gpointer gp; gpointer_init (&gp);
    
    PD_ATOMS_ALLOCA (a, n);
    
    gpointer_setAsScalar (&gp, owner, scalar);
    SET_POINTER (a, &gp);
    for (i = 0; i < argc; i++) { *(a + i + 1) = *(argv + i); }
    if (x->tp_instance) { struct_notify (x->tp_instance, s, n, a); }
    gpointer_unset (&gp);
    
    PD_ATOMS_FREEA (a, n);
}

static void template_anything (t_template *x, t_symbol *s, int argc, t_atom *argv)
{
    #if PD_WITH_DEBUG
    
    post ("My name is %s.", symbol_stripTemplateIdentifier (x->tp_templateIdentifier)->s_name);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int template_hasInstance (t_template *x)
{
    return (x->tp_instance != NULL);
}

void template_registerInstance (t_template *x, t_struct *o)
{
    paint_erase();
    
    x->tp_instance = o; if (!o) { instance_destroyAllScalarsByTemplate (x); }
    
    paint_draw();
}

void template_unregisterInstance (t_template *x, t_struct *o)
{
    template_registerInstance (x, NULL);
    
    pd_free (cast_pd (x));
}

static t_glist *template_getInstanceView (t_template *x)
{
    PD_ASSERT (x);
    
    if (!x->tp_instance) { return NULL; } else { return struct_getView (x->tp_instance); }
}

t_glist *template_getInstanceViewIfPainters (t_template *x)
{
    t_glist *view = template_getInstanceView (x);
    
    if (view) {
    
        t_gobj *y = NULL;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
            if (class_hasPainterBehavior (pd_class (y))) { return view; }
        }
    }
    
    return NULL;
}
 
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_template *template_findByIdentifier (t_symbol *s)
{
    return ((t_template *)pd_getThingByClass (s, template_class));
}

void template_create (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc && IS_SYMBOL (argv)) {
    //
    t_symbol *templateIdentifier = symbol_makeTemplateIdentifier (atom_getSymbolAtIndex (0, argc, argv));
    
    argc--;
    argv++;
    
    if (template_findByIdentifier (templateIdentifier) == NULL) { 
        template_new (templateIdentifier, argc, argv);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error template_newParse (t_template *x, int *ac, t_atom **av)
{
    int argc = *ac; t_atom *argv = *av;
    
    while (argc > 0) {

        if ((argc >= 2) && IS_SYMBOL (argv + 0) && IS_SYMBOL (argv + 1)) {
        //
        t_symbol *type               = GET_SYMBOL (argv + 0);
        t_symbol *fieldName          = GET_SYMBOL (argv + 1);
        t_symbol *templateIdentifier = &s_;
        
        int k = -1;
        
        #if PD_WITH_LEGACY
        
        if (type == &s_list) {
            type = sym_text; 
        }
            
        #endif
        
        if (type == &s_float)        { k = DATA_FLOAT;  }
        else if (type == &s_symbol)  { k = DATA_SYMBOL; }
        else if (type == sym_text)   { k = DATA_TEXT;   }
        else if (type == sym_array)  {
            if (argc >= 3 && IS_SYMBOL (argv + 2)) {
                templateIdentifier = symbol_makeTemplateIdentifier (GET_SYMBOL (argv + 2));
                k = DATA_ARRAY;
                argc--;
                argv++;
            } else {
                return PD_ERROR;
            }
        }
        
        if (k < 0) { error_invalid (sym_template, sym_type); }
        else {
        //
        int oldSize = x->tp_size;
        int newSize = x->tp_size + 1;
        size_t m = oldSize * sizeof (t_dataslot);
        size_t n = newSize * sizeof (t_dataslot);
        
        x->tp_slots = (t_dataslot *)PD_MEMORY_RESIZE (x->tp_slots, m, n);
        x->tp_size  = newSize;
        
        x->tp_slots[newSize - 1].ds_type               = k;
        x->tp_slots[newSize - 1].ds_fieldName          = fieldName;
        x->tp_slots[newSize - 1].ds_templateIdentifier = templateIdentifier;
        //
        }
        //
        } else {
            return PD_ERROR;
        }

    argc -= 2; *ac = argc;
    argv += 2; *av = argv;
    //
    }
    
    return PD_ERROR_NONE;
}
    
t_template *template_new (t_symbol *templateIdentifier, int argc, t_atom *argv)
{
    t_template *x = (t_template *)pd_new (template_class);
    
    PD_ASSERT (templateIdentifier);
    
    /* Empty template should be managed appropriately elsewhere. */
    
    PD_ASSERT (symbol_stripTemplateIdentifier (templateIdentifier) != &s_);
        
    x->tp_size               = 0;
    x->tp_slots              = (t_dataslot *)PD_MEMORY_GET (0);
    x->tp_templateIdentifier = templateIdentifier;
    x->tp_instance           = NULL;
    
    pd_bind (cast_pd (x), x->tp_templateIdentifier);
    
    if (template_newParse (x, &argc, &argv)) {      /* It may consume arguments. */
    //
    error_invalidArguments (symbol_stripTemplateIdentifier (templateIdentifier), argc, argv);
    pd_free (cast_pd (x)); x = NULL;
    //
    }
    
    return x;
}

void template_free (t_template *x)
{
    pd_unbind (cast_pd (x), x->tp_templateIdentifier);
    
    PD_MEMORY_FREE (x->tp_slots);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void template_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_template,
        NULL, 
        (t_method)template_free,
        sizeof (t_template),
        CLASS_NOBOX,
        A_NULL);
    
    class_addAnything (c, (t_method)template_anything);
    
    template_class = c;
}

void template_destroy (void)
{
    class_free (template_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
