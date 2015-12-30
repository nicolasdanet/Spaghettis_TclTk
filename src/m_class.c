
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdarg.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pd pd_objectMaker;
extern t_pd pd_canvasMaker;

extern t_widgetbehavior text_widgetBehavior;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_symbol *class_externalDirectory = &s_;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void class_defaultFloat      (t_pd *x, t_float f);
static void class_defaultList       (t_pd *x, t_symbol *s, int argc, t_atom *argv);
static void class_defaultAnything   (t_pd *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void class_floatForSignal (t_pd *x, t_float f)
{
    int offset = (*x)->c_floatSignalIn;
    PD_ASSERT (offset > 0);
    *(t_float *)(((char *)x) + offset) = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void class_defaultSave (t_gobj *z, t_binbuf *b)
{
    PD_BUG;
}

static void class_defaultProperties (t_gobj *z, t_glist *glist)
{
    PD_BUG;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void class_defaultBang (t_pd *x)
{
    if ((*(*x)->c_methodList) != class_defaultList) { (*(*x)->c_methodList) (x, NULL, 0, NULL); }
    else { 
        (*(*x)->c_methodAny) (x, &s_bang, 0, NULL);
    }
}

static void class_defaultPointer (t_pd *x, t_gpointer *gp)
{
    t_atom a;
    SET_POINTER (&a, gp);
        
    if ((*(*x)->c_methodList) != class_defaultList) { (*(*x)->c_methodList) (x, NULL, 1, &a); }
    else {
        (*(*x)->c_methodAny) (x, &s_pointer, 1, &a);
    }
}

static void class_defaultFloat (t_pd *x, t_float f)
{
    t_atom a;
    SET_FLOAT (&a, f);
        
    if ((*(*x)->c_methodList) != class_defaultList) { (*(*x)->c_methodList) (x, NULL, 1, &a); }
    else {
        (*(*x)->c_methodAny) (x, &s_float, 1, &a);
    }
}

static void class_defaultSymbol (t_pd *x, t_symbol *s)
{
    t_atom a;
    SET_SYMBOL (&a, s);
        
    if ((*(*x)->c_methodList) != class_defaultList) { (*(*x)->c_methodList) (x, NULL, 1, &a); }
    else {
        (*(*x)->c_methodAny) (x, &s_symbol, 1, &a);
    }
}

static void class_defaultList (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 0) {
        if ((*(*x)->c_methodBang) != class_defaultBang) { (*(*x)->c_methodBang) (x); return; }
    }
    
    if (argc == 1) {
    
        if (IS_FLOAT (argv)) {
            if ((*(*x)->c_methodFloat) != class_defaultFloat) {
                (*(*x)->c_methodFloat) (x, GET_FLOAT (argv));
                return;
            }
        }
            
        if (IS_SYMBOL (argv)) { 
            if ((*(*x)->c_methodSymbol) != class_defaultSymbol) {
                (*(*x)->c_methodSymbol) (x, GET_SYMBOL (argv));
                return;
            }
        }
            
        if (IS_POINTER (argv)) {
            if ((*(*x)->c_methodPointer) != class_defaultPointer) {
                (*(*x)->c_methodPointer) (x, GET_POINTER (argv)); 
                return;
            }
        }
    }

    if ((*(*x)->c_methodAny) != class_defaultAnything) { (*(*x)->c_methodAny) (x, &s_list, argc, argv); }
    else if ((*x)->c_isBox) { obj_list ((t_object *)x, s, argc, argv); }
    else { 
        class_defaultAnything (x, &s_list, argc, argv); 
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void class_defaultAnything (t_pd *x, t_symbol *s, int argc, t_atom *argv)
{
    post_error (PD_TRANSLATE ("%s / Unknown method \"%s\"."), (*x)->c_name->s_name, s->s_name);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void class_setExternalDirectory (t_symbol *s)
{
    class_externalDirectory = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *class_new (t_symbol *s, 
    t_newmethod newMethod, 
    t_method freeMethod, 
    size_t size, 
    int flags, 
    t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype arg[PD_ARGUMENTS + 1] = { 0 };
    t_atomtype *vp = arg;
    int count = 0;
    t_class *c = NULL;
    int typeflag = flags & (CLASS_PURE | CLASS_GRAPHIC | CLASS_BOX);
    if (!typeflag) { typeflag = CLASS_BOX; }
    
    *vp = type1;

    va_start (ap, type1);
    
    while (*vp) {
        if (count == PD_ARGUMENTS) { PD_BUG; break; }
        vp++; count++;
        *vp = va_arg (ap, t_atomtype);
    }
    
    va_end (ap);
    
    if (pd_objectMaker && newMethod) {
    //
    class_addMethod (pd_objectMaker, (t_method)newMethod, s, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
    //
    }
    
    c = (t_class *)getbytes (sizeof (t_class));
    c->c_name               = s;
    c->c_helpName           = s;
    c->c_externalDirectory  = class_externalDirectory;
    c->c_size               = size;
    c->c_methods            = getbytes (0);                 /* Allocate 1 byte of memory. */
    c->c_methodsSize        = 0;
    c->c_methodFree         = freeMethod;
    c->c_methodBang         = class_defaultBang;
    c->c_methodPointer      = class_defaultPointer;
    c->c_methodFloat        = class_defaultFloat;
    c->c_methodSymbol       = class_defaultSymbol;
    c->c_methodList         = class_defaultList;
    c->c_methodAny          = class_defaultAnything;
    c->c_behavior           = (typeflag == CLASS_BOX ? &text_widgetBehavior : NULL);
    c->c_behaviorParent     = NULL;
    c->c_fnSave             = (typeflag == CLASS_BOX ? text_save : class_defaultSave);
    c->c_fnProperties       = class_defaultProperties;
    c->c_floatSignalIn      = 0;
    c->c_isGraphic          = (typeflag >= CLASS_GRAPHIC);
    c->c_isBox              = (typeflag == CLASS_BOX);
    c->c_hasFirstInlet      = ((flags & CLASS_NOINLET) == 0);
    c->c_hasDrawCommand     = 0;

    return c;
}

void class_addSignal (t_class *c, int offset)
{
    if (offset <= 0) { PD_BUG; }
    else {
        c->c_floatSignalIn = offset;
        c->c_methodFloat = class_floatForSignal;
    }
}

void class_addCreator (t_newmethod newmethod, t_symbol *s, t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype arg[PD_ARGUMENTS + 1] = { 0 };
    t_atomtype *vp = arg;
    int count = 0;
    *vp = type1;

    va_start (ap, type1);
    
    while (*vp) {
        if (count == PD_ARGUMENTS) { PD_BUG; break; }
        vp++; count++;
        *vp = va_arg (ap, t_atomtype);
    } 
    
    va_end (ap);
    
    class_addMethod (pd_objectMaker, (t_method)newmethod, s, arg[0], arg[1], arg[2], arg[3], arg[4], arg[5]);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void class_addMethod (t_class *c, t_method fn, t_symbol *s, t_atomtype type1, ...)
{
    va_list ap;
    t_atomtype argtype = type1;
    t_methodentry *m = NULL;
    size_t oldSize, newSize;
    int i, n = 0;
        
    va_start (ap, type1);
    
    if (s == &s_signal) { PD_OBSOLETE; return; }
    
    /* Note that "pointer" is not catched (related to pointer object creation).*/
    
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
    
    for (i = 0; i < c->c_methodsSize; i++) {
        if (c->c_methods[i].me_name == s) { PD_OBSOLETE; return; }
    }
    
    oldSize = c->c_methodsSize * sizeof (t_methodentry);
    newSize = (c->c_methodsSize + 1) * sizeof (t_methodentry);
    
    c->c_methods = resizebytes (c->c_methods, oldSize, newSize);
    
    m = c->c_methods + c->c_methodsSize;
    c->c_methodsSize++;
    m->me_name = s;
    m->me_function = (t_gotfn)fn;

    while (argtype != A_NULL && n < PD_ARGUMENTS) {
        m->me_arguments[n] = argtype;
        n++;
        argtype = va_arg (ap, t_atomtype);
    }
    
    PD_ASSERT (argtype == A_NULL);

    va_end (ap);
    
    m->me_arguments[n] = A_NULL;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void class_addBang (t_class *c, t_method fn)
{
    c->c_methodBang = (t_bangmethod)fn;
}

void class_addPointer (t_class *c, t_method fn)
{
    c->c_methodPointer = (t_pointermethod)fn;
}

void class_addFloat (t_class *c, t_method fn)
{
    c->c_methodFloat = (t_floatmethod)fn;
}

void class_addSymbol (t_class *c, t_method fn)
{
    c->c_methodSymbol = (t_symbolmethod)fn;
}

void class_addList (t_class *c, t_method fn)
{
    c->c_methodList = (t_listmethod)fn;
}

void class_addAnything (t_class *c, t_method fn)
{
    c->c_methodAny = (t_anymethod)fn;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_gotfn class_getMethod (t_class *c, t_symbol *s)
{
    t_methodentry *m;
    int i;

    for (i = c->c_methodsSize, m = c->c_methods; i--; m++) { 
        if (m->me_name == s) { return (m->me_function); }
    }
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int class_hasBang (t_class *c)
{
    return (c->c_methodBang != class_defaultBang);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int class_hasDrawCommand (t_class *c)
{
    return c->c_hasDrawCommand;
}

int class_hasPropertiesFunction (t_class *c)
{
    return (c->c_fnProperties != class_defaultProperties);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void class_setHelpName (t_class *c, t_symbol *s)
{
    c->c_helpName = s;
}

void class_setWidget (t_class *c, t_widgetbehavior *w)
{
    c->c_behavior = w;
}

void class_setParentWidget (t_class *c, t_parentwidgetbehavior *pw)
{
    c->c_behaviorParent = pw;
}

void class_setDrawCommand (t_class *c)
{
    c->c_hasDrawCommand = 1;
}

void class_setSaveFunction (t_class *c, t_savefn f)
{
    c->c_fnSave = f;
}

void class_setPropertiesFunction (t_class *c, t_propertiesfn f)
{
    c->c_fnProperties = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

char *class_getName (t_class *c)
{
    return c->c_name->s_name;
}

char *class_getHelpName (t_class *c)
{
    return c->c_helpName->s_name;
}

char *class_getHelpDirectory (t_class *c)
{
    return (c->c_externalDirectory->s_name);
}

t_parentwidgetbehavior *class_getParentWidget (t_class *c)
{
    return c->c_behaviorParent;
}

t_propertiesfn class_getPropertiesFunction (t_class *c)
{
    return c->c_fnProperties;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
