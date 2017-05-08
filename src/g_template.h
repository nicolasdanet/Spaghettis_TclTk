
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __g_template_h_
#define __g_template_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _dataslot {
    int         ds_type;
    t_symbol    *ds_fieldName;
    t_symbol    *ds_templateIdentifier;
    } t_dataslot;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _template {
    t_pd        tp_pd;                              /* MUST be the first. */
    int         tp_size;    
    t_dataslot  *tp_slots;   
    t_symbol    *tp_templateIdentifier; 
    t_struct    *tp_instance;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_template  *template_findByIdentifier              (t_symbol *templateIdentifier);
t_template  *template_new                           (t_symbol *templateIdentifier, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        template_create                         (void *dummy, t_symbol *s, int argc, t_atom *argv);
int         template_isValid                        (t_template *x);
void        template_free                           (t_template *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         template_hasInstance                    (t_template *x);
void        template_registerInstance               (t_template *x, t_struct *o);
void        template_unregisterInstance             (t_template *x, t_struct *o);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         template_containsArray                  (t_template *x);
int         template_containsTemplate               (t_template *x, t_symbol *templateIdentifier);
int         template_hasField                       (t_template *x, t_symbol *field);
int         template_getIndexOfField                (t_template *x, t_symbol *field);
int         template_getRaw                         (t_template *x,
                                                            t_symbol *field,
                                                            int *index,
                                                            int *type,
                                                            t_symbol **templateIdentifier);

t_symbol    *template_getFieldAtIndex               (t_template *x, int n);
t_template  *template_getTemplateIfArrayAtIndex     (t_template *x, int n);   
t_glist     *template_getInstanceViewIfPainters     (t_template *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         template_fieldIsFloat                   (t_template *x, t_symbol *field);
int         template_fieldIsSymbol                  (t_template *x, t_symbol *field);
int         template_fieldIsText                    (t_template *x, t_symbol *field);
int         template_fieldIsArray                   (t_template *x, t_symbol *field);
int         template_fieldIsArrayAndValid           (t_template *x, t_symbol *field);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        template_serialize                      (t_template *x, t_buffer *b);
void        template_notify                         (t_template *x, 
                                                        t_glist *owner,
                                                        t_scalar *scalar,
                                                        t_symbol *s,
                                                        int argc,
                                                        t_atom *argv);
                                                        
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_glist     *struct_getView                         (t_struct *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void        struct_notify                           (t_struct *x, t_symbol *s, int argc, t_atom *argv);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int template_getSize (t_template *x)
{
    return x->tp_size;
}

static inline t_dataslot *template_getSlots (t_template *x)
{
    return x->tp_slots;
}

static inline t_symbol *template_getTemplateIdentifier (t_template *x)
{
    return x->tp_templateIdentifier;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int template_isPrivate (t_symbol *templateIdentifier)
{
    if (templateIdentifier == sym___TEMPLATE__float__dash__array) { return 1; }
    else if (templateIdentifier == sym___TEMPLATE__float)         { return 1; }
    else if (templateIdentifier == sym___TEMPLATE__text)          { return 1; }
    else {
        return 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_symbol *template_getWildcard (void)
{
    return &s_;
}

static inline t_symbol *template_makeIdentifierWithWildcard (t_symbol *s)
{
    if (s == &s_ || s == sym___dash__) { return template_getWildcard(); }
    else { 
        return utils_makeTemplateIdentifier (s);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __g_template_h_
