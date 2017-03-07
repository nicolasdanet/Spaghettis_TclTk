
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __m_class_h_
#define __m_class_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_freemethod)                (t_pd *x);
typedef void (*t_bangmethod)                (t_pd *x);
typedef void (*t_floatmethod)               (t_pd *x, t_float f);
typedef void (*t_symbolmethod)              (t_pd *x, t_symbol *s);
typedef void (*t_listmethod)                (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_anythingmethod)            (t_pd *x, t_symbol *s, int argc, t_atom *argv);
typedef void (*t_pointermethod)             (t_pd *x, t_gpointer *gp);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_savefn)                    (t_gobj *x, t_buffer *b);
typedef void (*t_propertiesfn)              (t_gobj *x, t_glist *glist);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_getrectanglefn)            (t_gobj *x, t_glist *glist, t_rectangle *r);
typedef void (*t_displacedfn)               (t_gobj *x, t_glist *glist, int deltaX, int deltaY);
typedef void (*t_selectedfn)                (t_gobj *x, t_glist *glist, int isSelected);
typedef void (*t_activatedfn)               (t_gobj *x, t_glist *glist, int isActivated);
typedef void (*t_deletedfn)                 (t_gobj *x, t_glist *glist);
typedef void (*t_visibilityfn)              (t_gobj *x, t_glist *glist, int isVisible);
typedef int  (*t_mousefn)                   (t_gobj *x, t_glist *glist, t_mouse *m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef void (*t_paintergetrectanglefn)     (t_gobj *x, t_gpointer *gp, t_float a, t_float b, t_rectangle *r);
typedef void (*t_paintervisibilityfn)       (t_gobj *x, t_gpointer *gp, t_float a, t_float b, int flag);
typedef int  (*t_paintermousefn)            (t_gobj *x, t_gpointer *gp, t_float a, t_float b, t_mouse *m);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _widgetbehavior {
    t_getrectanglefn            w_fnGetRectangle;
    t_displacedfn               w_fnDisplaced;
    t_selectedfn                w_fnSelected;
    t_activatedfn               w_fnActivated;
    t_deletedfn                 w_fnDeleted;
    t_visibilityfn              w_fnVisibilityChanged;
    t_mousefn                   w_fnMouse;
    };
    
struct _painterwidgetbehavior {
    t_paintergetrectanglefn     w_fnPainterGetRectangle;
    t_paintervisibilityfn       w_fnPainterVisibilityChanged;
    t_paintermousefn            w_fnPainterMouse;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _entry {
    t_symbol                    *me_name;
    t_method                    me_method;
    t_atomtype                  me_arguments[PD_ARGUMENTS + 1];
    } t_entry;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _class {
    t_symbol                    *c_name;
    t_symbol                    *c_helpName;
    t_symbol                    *c_externalDirectory;
    t_entry                     *c_methods;
    int                         c_methodsSize;
    t_freemethod                c_methodFree;
    t_bangmethod                c_methodBang;
    t_floatmethod               c_methodFloat;
    t_symbolmethod              c_methodSymbol;
    t_listmethod                c_methodList;
    t_anythingmethod            c_methodAnything;
    t_pointermethod             c_methodPointer;
    t_widgetbehavior            *c_behavior;
    t_painterwidgetbehavior     *c_behaviorPainter;
    t_savefn                    c_fnSave;
    t_propertiesfn              c_fnProperties;
    int                         c_signalOffset;
    char                        c_hasFirstInlet;
    char                        c_isBox;
    int                         c_type;
    size_t                      c_size;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void        class_setCurrentExternalDirectory   (t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_method    class_getMethod                     (t_class *c, t_symbol *s);
int         class_hasMethod                     (t_class *c, t_symbol *s);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int         class_hasDSP                        (t_class *c);
int         class_hasOverrideBangMethod         (t_class *c);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline int class_hasFirstInlet (t_class *c)
{
    return (c->c_hasFirstInlet != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int class_hasFreeMethod (t_class *c)
{
    return (c->c_methodFree != NULL);
}

static inline t_freemethod class_getFreeMethod (t_class *c)
{
    return c->c_methodFree;
}

static inline t_bangmethod class_getBangMethod (t_class *c)
{
    return c->c_methodBang;
}

static inline t_floatmethod class_getFloatMethod (t_class *c)
{
    return c->c_methodFloat;
}

static inline t_symbolmethod class_getSymbolMethod (t_class *c)
{
    return c->c_methodSymbol;
}

static inline t_listmethod class_getListMethod (t_class *c)
{
    return c->c_methodList;
}

static inline t_anythingmethod class_getAnythingMethod (t_class *c)
{
    return c->c_methodAnything;
}

static inline t_pointermethod class_getPointerMethod (t_class *c)
{
    return c->c_methodPointer;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_symbol *class_getName (t_class *c)
{
    return c->c_name;
}

static inline char *class_getNameAsString (t_class *c)
{
    return c->c_name->s_name;
}

static inline char *class_getHelpNameAsString (t_class *c)
{
    return c->c_helpName->s_name;
}

static inline char *class_getExternalDirectoryAsString (t_class *c)
{
    return c->c_externalDirectory->s_name;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void class_setHelpName (t_class *c, t_symbol *s)
{
    c->c_helpName = s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int class_hasWidgetBehavior (t_class *c)
{
    return (c->c_behavior != NULL);
}

static inline int class_hasPainterWidgetBehavior (t_class *c)
{
    return (c->c_behaviorPainter != NULL);
}

static inline int class_hasSaveFunction (t_class *c)
{
    return (c->c_fnSave != NULL);
}

static inline int class_hasPropertiesFunction (t_class *c)
{
    return (c->c_fnProperties != NULL);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline t_widgetbehavior *class_getWidgetBehavior (t_class *c)
{
    return c->c_behavior;
}

static inline t_painterwidgetbehavior *class_getPainterWidgetBehavior (t_class *c)
{
    return c->c_behaviorPainter;
}

static inline t_savefn class_getSaveFunction (t_class *c)
{
    return c->c_fnSave;
}

static inline t_propertiesfn class_getPropertiesFunction (t_class *c)
{
    return c->c_fnProperties;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline void class_setWidgetBehavior (t_class *c, t_widgetbehavior *w)
{
    c->c_behavior = w;
}

static inline void class_setPainterWidgetBehavior (t_class *c, t_painterwidgetbehavior *pw)
{
    c->c_behaviorPainter = pw;
}

static inline void class_setSaveFunction (t_class *c, t_savefn f)
{
    c->c_fnSave = f;
}

static inline void class_setPropertiesFunction (t_class *c, t_propertiesfn f)
{
    c->c_fnProperties = f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __m_class_h_
