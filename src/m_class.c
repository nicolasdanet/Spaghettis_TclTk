
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *class_currentExternalDirectory = &s_;      /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void class_defaultFloat      (t_pd *, t_float);
static void class_defaultList       (t_pd *, t_symbol *, int, t_atom *);
static void class_defaultAnything   (t_pd *, t_symbol *, int, t_atom *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void class_floatForSignal (t_pd *x, t_float f)
{
    *(class_getFirstInletSignalValue (x)) = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void class_defaultBang (t_pd *x)
{
    t_class *c = pd_class (x);
    
    if ((*c->c_methodList) != class_defaultList) { (*c->c_methodList) (x, &s_bang, 0, NULL); }
    else { 
        (*c->c_methodAnything) (x, &s_bang, 0, NULL);
    }
}

static void class_defaultPointer (t_pd *x, t_gpointer *gp)
{
    t_class *c = pd_class (x);
        
    t_atom a;
    SET_POINTER (&a, gp);
        
    if ((*c->c_methodList) != class_defaultList) { (*c->c_methodList) (x, &s_pointer, 1, &a); }
    else {
        (*c->c_methodAnything) (x, &s_pointer, 1, &a);
    }
}

static void class_defaultFloat (t_pd *x, t_float f)
{
    t_class *c = pd_class (x);
    
    t_atom a;
    SET_FLOAT (&a, f);
        
    if ((*c->c_methodList) != class_defaultList) { (*c->c_methodList) (x, &s_float, 1, &a); }
    else {
        (*c->c_methodAnything) (x, &s_float, 1, &a);
    }
}

static void class_defaultSymbol (t_pd *x, t_symbol *s)
{
    t_class *c = pd_class (x);
    
    t_atom a;
    SET_SYMBOL (&a, s);
        
    if ((*c->c_methodList) != class_defaultList) { (*c->c_methodList) (x, &s_symbol, 1, &a); }
    else {
        (*c->c_methodAnything) (x, &s_symbol, 1, &a);
    }
}

static void class_defaultList (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    t_class *c = pd_class (x);
    
    if (argc == 0) {
        if ((*c->c_methodBang) != class_defaultBang) { (*c->c_methodBang) (x); return; }
    }
    
    if (argc == 1) {
    
        if (IS_FLOAT (argv)) {
            if ((*c->c_methodFloat) != class_defaultFloat) {
                (*c->c_methodFloat) (x, GET_FLOAT (argv));
                return;
            }
        }
            
        if (IS_SYMBOL (argv)) { 
            if ((*c->c_methodSymbol) != class_defaultSymbol) {
                (*c->c_methodSymbol) (x, GET_SYMBOL (argv));
                return;
            }
        }
            
        if (IS_POINTER (argv)) {
            if ((*c->c_methodPointer) != class_defaultPointer) {
                (*c->c_methodPointer) (x, GET_POINTER (argv)); 
                return;
            }
        }
    }

    if ((*c->c_methodAnything) != class_defaultAnything) { (*c->c_methodAnything) (x, &s_list, argc, argv); }
    else if (class_isBox (c)) { object_distributeOnInlets (cast_object (x), argc, argv); }
    else { 
        class_defaultAnything (x, &s_list, argc, argv); 
    }
}

static void class_defaultAnything (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    error_unknownMethod (class_getName (pd_class (x)), s);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Default save function for all boxes object. */

static void class_defaultSave (t_gobj *z, t_buffer *b)
{
    t_object *x = cast_object (z);
    
    if (object_isComment (x)) {
        buffer_vAppend (b, "ssii", sym___hash__X, sym_text, object_getX (x), object_getY (x));
        
    } else if (object_isObject (x)) {
        buffer_vAppend (b, "ssii", sym___hash__X, sym_obj,  object_getX (x), object_getY (x));
        
    } else if (object_isMessage (x)) {
        buffer_vAppend (b, "ssii", sym___hash__X, sym_msg,  object_getX (x), object_getY (x));
        
    } else { 
        PD_BUG;
    }
    
    buffer_serialize (b, object_getBuffer (x));
    buffer_appendSemicolon (b);
    object_saveWidth (x, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void class_setCurrentExternalDirectory (t_symbol *s)
{
    class_currentExternalDirectory = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* For now the class name needs to be unique only if it contains a constructor. */
/* For now it is valid to add inlets with CLASS_NOINLET set at first. */
/* Probably best to make that more restrictive in the future. */

t_class *class_new (t_symbol *s, 
    t_newmethod newMethod, 
    t_method freeMethod, 
    size_t size, 
    int flags, 
    t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype args[PD_ARGUMENTS + 1] = { 0 };
    t_atomtype *vp = args;
    int count = 0;
    t_class *c = NULL;
    int type = flags & (CLASS_ABSTRACT | CLASS_NOBOX | CLASS_GRAPHIC | CLASS_BOX);
    if (!type) { type = CLASS_BOX; }
    
    *vp = type1;

    va_start (ap, type1);
    
    while (*vp) {
        if (count == PD_ARGUMENTS) { PD_BUG; break; }
        vp++; count++;
        *vp = va_arg (ap, t_atomtype);
    }
    
    va_end (ap);
    
    if (newMethod) {
    //
    class_addMethod (instance_getMakerObjectClass(), (t_method)newMethod, s, args[0], args[1], args[2]);
    //
    }
    
    c = (t_class *)PD_MEMORY_GET (sizeof (t_class));
    c->c_name               = s;
    c->c_helpName           = s;
    c->c_externalDirectory  = class_currentExternalDirectory;
    c->c_methods            = (t_entry *)PD_MEMORY_GET (0);             /* Allocate 1 byte of memory. */
    c->c_methodsSize        = 0;
    c->c_methodFree         = (t_freemethod)freeMethod;
    c->c_methodBang         = class_defaultBang;
    c->c_methodPointer      = class_defaultPointer;
    c->c_methodFloat        = class_defaultFloat;
    c->c_methodSymbol       = class_defaultSymbol;
    c->c_methodList         = class_defaultList;
    c->c_methodAnything     = class_defaultAnything;
    c->c_behavior           = (type == CLASS_BOX ? &text_widgetBehavior : NULL);
    c->c_behaviorPainter    = NULL;
    c->c_fnSave             = (type == CLASS_BOX ? class_defaultSave : NULL);
    c->c_fnProperties       = NULL;
    c->c_signalOffset       = 0;
    c->c_hasFirstInlet      = ((flags & CLASS_NOINLET) == 0);
    c->c_type               = type;
    c->c_size               = size;

    return c;
}

void class_addSignal (t_class *c, t_int offset)
{
    PD_ASSERT (c->c_methodFloat == class_defaultFloat);
    
    if (offset <= 0) { PD_BUG; }
    else {
        c->c_signalOffset = offset;
        c->c_methodFloat  = class_floatForSignal;
    }
}

void class_addCreator (t_newmethod newMethod, t_symbol *s, t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype args[PD_ARGUMENTS + 1] = { 0 };
    t_atomtype *vp = args;
    int count = 0;
    *vp = type1;

    va_start (ap, type1);
    
    while (*vp) {
        if (count == PD_ARGUMENTS) { PD_BUG; break; }
        vp++; count++;
        *vp = va_arg (ap, t_atomtype);
    } 
    
    va_end (ap);
    
    class_addMethod (instance_getMakerObjectClass(), (t_method)newMethod, s, args[0], args[1], args[2]);
}

void class_free (t_class *c)
{
    PD_MEMORY_FREE (c->c_methods);
    PD_MEMORY_FREE (c);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void class_addMethod (t_class *c, t_method fn, t_symbol *s, t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype argtype = type1;
    
    va_start (ap, type1);
    
    if (s == &s_signal) { PD_BUG; return;  }            /* Deprecated. */
    if (s == sym_dsp)   { c->c_hasDSP = 1; }
    
    /* Note that "pointer" is not catched. */
    /* It aims to let the pointer object be A_GIMME initialized. */
    
    PD_ASSERT (s != &s_pointer || class_getName (c) == sym_objectmaker);
    
    if (s == &s_bang) {
        if (argtype) { PD_BUG; return; }
        class_addBang (c, fn);
        
    } else if (s == &s_float) {
        if (argtype != A_FLOAT || va_arg (ap, t_atomtype))  { PD_BUG; return; }
        class_addFloat (c, fn);
        
    } else if (s == &s_symbol) {
        if (argtype != A_SYMBOL || va_arg (ap, t_atomtype)) { PD_BUG; return; }
        class_addSymbol (c, fn);
        
    } else if (s == &s_list) {
        if (argtype != A_GIMME) { PD_BUG; return; }
        class_addList (c, fn);
        
    } else if (s == &s_anything) {
        if (argtype != A_GIMME) { PD_BUG; return; }
        class_addAnything (c, fn);
        
    } else {
    //
    int i;
    
    /* Method name must be unique. */
    
    for (i = 0; i < c->c_methodsSize; i++) {
        if (c->c_methods[i].me_name == s) { PD_BUG; return; }
    }
    
    {
        t_entry *m = NULL;
        int n = 0;
                
        size_t oldSize = sizeof (t_entry) * (c->c_methodsSize);
        size_t newSize = sizeof (t_entry) * (c->c_methodsSize + 1);
        
        c->c_methods = PD_MEMORY_RESIZE (c->c_methods, oldSize, newSize);
        
        m = c->c_methods + c->c_methodsSize;
        m->me_name = s;
        m->me_method = fn;

        c->c_methodsSize++;
        
        while (argtype != A_NULL && n < PD_ARGUMENTS) {
            m->me_arguments[n] = argtype;
            n++;
            argtype = va_arg (ap, t_atomtype);
        }
        
        PD_ASSERT (argtype == A_NULL);

        va_end (ap);
        
        m->me_arguments[n] = A_NULL;
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void class_addBang (t_class *c, t_method fn)
{
    PD_ASSERT (c->c_methodBang == class_defaultBang);
    
    c->c_methodBang = (t_bangmethod)fn;
}

void class_addFloat (t_class *c, t_method fn)
{
    PD_ASSERT (c->c_methodFloat == class_defaultFloat);
    
    c->c_methodFloat = (t_floatmethod)fn;
}

void class_addSymbol (t_class *c, t_method fn)
{
    PD_ASSERT (c->c_methodSymbol == class_defaultSymbol);
    
    c->c_methodSymbol = (t_symbolmethod)fn;
}

void class_addPointer (t_class *c, t_method fn)
{
    PD_ASSERT (c->c_methodPointer == class_defaultPointer);
    
    c->c_methodPointer = (t_pointermethod)fn;
}

void class_addList (t_class *c, t_method fn)
{
    PD_ASSERT (c->c_methodList == class_defaultList);
    
    c->c_methodList = (t_listmethod)fn;
}

void class_addAnything (t_class *c, t_method fn)
{
    PD_ASSERT (c->c_methodAnything == class_defaultAnything);
    
    c->c_methodAnything = (t_anythingmethod)fn;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_method class_getMethod (t_class *c, t_symbol *s)
{
    t_entry *m = NULL;
    int i;

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++) {
        if (m->me_name == s) { return m->me_method; }
    }
    
    return NULL;
}

int class_hasMethod (t_class *c, t_symbol *s)
{
    return (class_getMethod (c, s) != NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int class_hasOverrideBangMethod (t_class *c)
{
    return (c->c_methodBang != class_defaultBang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
