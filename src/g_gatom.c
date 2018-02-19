
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void gatom_float                     (t_gatom *, t_float);
static void gatom_set                       (t_gatom *, t_symbol *, int, t_atom *);
static void gatom_motion                    (void *, t_float, t_float, t_float);
static void gatom_behaviorVisibilityChanged (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *gatom_class;                                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _gatom {
    t_object        a_obj;                              /* MUST be the first. */
    t_atom          a_atom;
    t_float         a_lowRange;
    t_float         a_highRange;
    int             a_position;                         /* Unused but kept for compatibility. */
    t_glist         *a_owner;
    t_symbol        *a_send;
    t_symbol        *a_receive;
    t_symbol        *a_label;                           /* Unused but kept for compatibility. */
    t_symbol        *a_unexpandedSend;
    t_symbol        *a_unexpandedReceive;
    t_symbol        *a_unexpandedLabel;                 /* Unused but kept for compatibility. */
    t_outlet        *a_outlet;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void text_behaviorGetRectangle      (t_gobj *, t_glist *, t_rectangle *);
void text_behaviorDisplaced         (t_gobj *, t_glist *, int, int);
void text_behaviorSelected          (t_gobj *, t_glist *, int);
void text_behaviorDeleted           (t_gobj *, t_glist *);
void text_behaviorVisibilityChanged (t_gobj *, t_glist *, int);
int  text_behaviorMouse             (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_widgetbehavior gatom_widgetBehavior =          /* Shared. */
    {
        text_behaviorGetRectangle,
        text_behaviorDisplaced,
        text_behaviorSelected,
        NULL,
        text_behaviorDeleted,
        gatom_behaviorVisibilityChanged,
        text_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define ATOM_WIDTH_FLOAT        5
#define ATOM_WIDTH_SYMBOL       10
#define ATOM_WIDTH_MAXIMUM      80

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_drawJob (t_gobj *z, t_glist *glist)
{
    t_gatom *x = (t_gatom *)z;
    
    box_retext (box_fetch (x->a_owner, cast_object (x)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_symbol *gatom_parse (t_symbol *s)
{
    if (symbol_isNilOrDash (s)) { return &s_; } else { return symbol_hashToDollar (s); }
}

static void gatom_update (t_gatom *x)
{
    buffer_clear (object_getBuffer (cast_object (x)));
    buffer_appendAtom (object_getBuffer (cast_object (x)), &x->a_atom);
    gui_jobAdd ((void *)x, x->a_owner, gatom_drawJob);
}

static int gatom_isFloat (t_gatom *x)
{
    return IS_FLOAT (&x->a_atom);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_bang (t_gatom *x)
{
    if (gatom_isFloat (x)) {
    
        outlet_float (x->a_outlet, GET_FLOAT (&x->a_atom));
        
        if (x->a_send != &s_ && symbol_hasThing (x->a_send)) {
            if (x->a_send != x->a_receive) {
                pd_float (symbol_getThing (x->a_send), GET_FLOAT (&x->a_atom));
            } else {
                error_sendReceiveLoop (x->a_unexpandedSend);
            }
        }
        
    } else {
    
        outlet_symbol (x->a_outlet, GET_SYMBOL (&x->a_atom));
        
        if (x->a_send != &s_ && symbol_hasThing (x->a_send)) {
            if (x->a_send != x->a_receive) {
                pd_symbol (symbol_getThing (x->a_send), GET_SYMBOL (&x->a_atom));
            } else {
                error_sendReceiveLoop (x->a_unexpandedSend);
            }
        }
    }
}

static void gatom_float (t_gatom *x, t_float f)
{
    t_atom a;
    SET_FLOAT (&a, f);
    gatom_set (x, NULL, 1, &a);
    gatom_bang (x);
}

static void gatom_symbol (t_gatom *x, t_symbol *s)
{
    t_atom a;
    SET_SYMBOL (&a, s);
    gatom_set (x, NULL, 1, &a);
    gatom_bang (x);
}

void gatom_click (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (gatom_isFloat (x)) {
    //
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    glist_setMotion (x->a_owner, cast_gobj (x), (t_motionfn)gatom_motion, a, b);
    //
    }
    
    gatom_bang (x);
}

static void gatom_set (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    if (!gatom_isFloat (x)) { SET_SYMBOL (&x->a_atom, atom_getSymbol (argv)); }
    else {
    //
    t_float f = atom_getFloat (argv);
    if (x->a_lowRange != 0.0 || x->a_highRange != 0.0) {
        f = PD_CLAMP (f, x->a_lowRange, x->a_highRange);
    }
    SET_FLOAT (&x->a_atom, f);
    //
    }

    gatom_update (x);
    //
    }
}

static void gatom_range (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (gatom_isFloat (x)) {
    //
    t_float minimum = atom_getFloatAtIndex (0, argc, argv);
    t_float maximum = atom_getFloatAtIndex (1, argc, argv);
    
    x->a_lowRange   = PD_MIN (minimum, maximum);
    x->a_highRange  = PD_MAX (minimum, maximum);
    
    gatom_set (x, NULL, 1, &x->a_atom);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_gatom *x = (t_gatom *)z;
    
    PD_ASSERT (gatom_isFloat (x));
    
    if (deltaY != 0.0) { 
    //
    double f = GET_FLOAT (&x->a_atom);
    
    if ((int)modifier & MODIFIER_SHIFT) { f -= 0.01 * deltaY; }
    else {
        f -= deltaY;
    }
    
    gatom_float (x, (t_float)f);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    text_behaviorVisibilityChanged (z, glist, isVisible);
    
    if (!isVisible) { gui_jobRemove ((void *)z); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_functionSave (t_gobj *z, t_buffer *b)
{
    t_gatom *x = (t_gatom *)z;
    
    buffer_vAppend (b, "ssiiifffsss;",
        sym___hash__X,
        (gatom_isFloat (x) ? sym_floatatom : sym_symbolatom),
        object_getX (cast_object (x)),
        object_getY (cast_object (x)),
        object_getWidth (cast_object (x)),
        (double)x->a_lowRange,
        (double)x->a_highRange,
        (double)x->a_position,
        symbol_dollarToHash (symbol_emptyAsDash (x->a_unexpandedLabel)),
        symbol_dollarToHash (symbol_emptyAsDash (x->a_unexpandedReceive)),
        symbol_dollarToHash (symbol_emptyAsDash (x->a_unexpandedSend)));

    object_serializeWidth (cast_object (x), b);
}

static void gatom_functionProperties (t_gobj *z, t_glist *owner)
{
    t_gatom *x  = (t_gatom *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    
    t_symbol *symSend    = symbol_dollarToHash (symbol_emptyAsNil (x->a_unexpandedSend));
    t_symbol *symReceive = symbol_dollarToHash (symbol_emptyAsNil (x->a_unexpandedReceive));
    
    if (gatom_isFloat (x)) {
    
        err = string_sprintf (t, PD_STRING, 
                "::ui_atom::show %%s %d %g %g %s %g {%s} {%s}\n",               // --
                object_getWidth (cast_object (x)),
                x->a_lowRange,
                x->a_highRange,
                sym_floatatom->s_name,
                GET_FLOAT (&x->a_atom),
                symSend->s_name,
                symReceive->s_name);
            
    } else {
    
        err = string_sprintf (t, PD_STRING, 
                "::ui_atom::show %%s %d %g %g %s {%s} {%s} {%s}\n",             // --
                object_getWidth (cast_object (x)),
                x->a_lowRange,
                x->a_highRange,
                sym_symbolatom->s_name,
                GET_SYMBOL (&x->a_atom)->s_name,
                symSend->s_name,
                symReceive->s_name);
    }
    
    PD_UNUSED (err); PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void gatom_fromDialog (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    int isDirty  = 0;
    
    t_float t1   = x->a_lowRange;
    t_float t2   = x->a_highRange;
    int t3       = object_getWidth (cast_object (x));
    t_symbol *t4 = x->a_send;
    t_symbol *t5 = x->a_receive;
    
    PD_ASSERT (argc == 6);
    
    gobj_visibilityChanged (cast_gobj (x), x->a_owner, 0);
    
    {
    //
    t_float width           = atom_getFloatAtIndex (0, argc, argv);
    t_float lowRange        = atom_getFloatAtIndex (1, argc, argv);
    t_float highRange       = atom_getFloatAtIndex (2, argc, argv);
    t_symbol *symSend       = gatom_parse (atom_getSymbolAtIndex (4, argc, argv));
    t_symbol *symReceive    = gatom_parse (atom_getSymbolAtIndex (5, argc, argv));

    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }
    
    object_setWidth (cast_object (x), PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM));

    x->a_lowRange           = PD_MIN (lowRange, highRange);
    x->a_highRange          = PD_MAX (lowRange, highRange);
    x->a_unexpandedSend     = symSend;
    x->a_unexpandedReceive  = symReceive;
    x->a_send               = dollar_expandSymbol (x->a_unexpandedSend, x->a_owner);
    x->a_receive            = dollar_expandSymbol (x->a_unexpandedReceive, x->a_owner);
    
    if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }
    
    gatom_set (x, NULL, 1, argv + 3);
    //
    }
    
    gobj_visibilityChanged (cast_gobj (x), x->a_owner, 1);
    
    isDirty |= (t1 != x->a_lowRange);
    isDirty |= (t2 != x->a_highRange);
    isDirty |= (t3 != object_getWidth (cast_object (x)));
    isDirty |= (t4 != x->a_send);
    isDirty |= (t5 != x->a_receive);
    
    if (isDirty) { glist_setDirty (x->a_owner, 1); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gatom_makeObjectFile (t_gatom *x, int argc, t_atom *argv)
{
    int width    = (int)atom_getFloatAtIndex (2, argc, argv);
    int position = (int)atom_getFloatAtIndex (5, argc, argv);
    
    width        = PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM);
    
    object_setX (cast_object (x), atom_getFloatAtIndex (0, argc, argv));
    object_setY (cast_object (x), atom_getFloatAtIndex (1, argc, argv));
    object_setWidth (cast_object (x), width);

    x->a_lowRange           = atom_getFloatAtIndex (3, argc, argv);
    x->a_highRange          = atom_getFloatAtIndex (4, argc, argv);
    x->a_position           = position;
    x->a_unexpandedLabel    = gatom_parse (atom_getSymbolAtIndex (6, argc, argv));
    x->a_unexpandedReceive  = gatom_parse (atom_getSymbolAtIndex (7, argc, argv));
    x->a_unexpandedSend     = gatom_parse (atom_getSymbolAtIndex (8, argc, argv));
    x->a_send               = dollar_expandSymbol (x->a_unexpandedSend, x->a_owner);
    x->a_receive            = dollar_expandSymbol (x->a_unexpandedReceive, x->a_owner);
    x->a_label              = dollar_expandSymbol (x->a_unexpandedLabel, x->a_owner);
            
    if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }

    x->a_outlet = outlet_new (cast_object (x), gatom_isFloat (x) ? &s_float : &s_symbol);
    
    glist_objectAdd (x->a_owner, cast_gobj (x));
}

static void gatom_makeObjectMenu (t_gatom *x, int argc, t_atom *argv)
{
    glist_deselectAll (x->a_owner);
        
    object_setSnappedX (cast_object (x), instance_getDefaultX (x->a_owner));
    object_setSnappedY (cast_object (x), instance_getDefaultY (x->a_owner));
        
    x->a_outlet = outlet_new (cast_object (x), gatom_isFloat (x) ? &s_float : &s_symbol);
                
    glist_objectAdd (x->a_owner, cast_gobj (x));
    glist_objectSelect (x->a_owner, cast_gobj (x));
}

static void gatom_makeObjectProceed (t_glist *glist, t_atomtype type, int argc, t_atom *argv)
{
    t_gatom *x  = (t_gatom *)pd_new (gatom_class);
    
    t_buffer *t = buffer_new();
    
    x->a_owner              = glist;
    x->a_lowRange           = 0;
    x->a_highRange          = 0;
    x->a_position           = 1;
    x->a_send               = &s_;
    x->a_receive            = &s_;
    x->a_label              = &s_;
    x->a_unexpandedSend     = &s_;
    x->a_unexpandedReceive  = &s_;
    x->a_unexpandedLabel    = &s_;
    
    if (type == A_FLOAT) {
        t_atom a;
        SET_FLOAT (&x->a_atom, (t_float)0.0);
        SET_FLOAT (&a, (t_float)0.0);
        buffer_appendAtom (t, &a);
        
    } else {
        t_atom a;
        SET_SYMBOL (&x->a_atom, &s_symbol);
        SET_SYMBOL (&a, &s_symbol);
        buffer_appendAtom (t, &a);
    }
    
    object_setBuffer (cast_object (x), t);
    object_setWidth (cast_object (x),  type == A_FLOAT ? ATOM_WIDTH_FLOAT : ATOM_WIDTH_SYMBOL);
    object_setType (cast_object (x),   TYPE_ATOM);
    
    if (argc <= 1) { gatom_makeObjectMenu (x, argc, argv); }
    else {
        t_atom a; SET_SYMBOL (&a, &s_symbol);
        gatom_makeObjectFile (x, argc, argv);
        gatom_set (x, NULL, 1, &a);
    }
}

void gatom_makeObjectFloat (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    gatom_makeObjectProceed (glist, A_FLOAT, argc, argv);
}

void gatom_makeObjectSymbol (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    gatom_makeObjectProceed (glist, A_SYMBOL, argc, argv);
}

static void gatom_free (t_gatom *x)
{
    gui_jobRemove ((void *)x);
    
    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gatom_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_gatom,
            NULL,
            (t_method)gatom_free,
            sizeof (t_gatom),
            CLASS_DEFAULT,
            A_NULL);
            
    class_addBang (c, (t_method)gatom_bang);
    class_addFloat (c, (t_method)gatom_float);
    class_addSymbol (c, (t_method)gatom_symbol);
    class_addClick (c, (t_method)gatom_click);
        
    class_addMethod (c, (t_method)gatom_set,        sym_set,            A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_fromDialog, sym__gatomdialog,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)gatom_range,      sym_range,          A_GIMME, A_NULL);

    class_setWidgetBehavior (c, &gatom_widgetBehavior);
    class_setSaveFunction (c, gatom_functionSave);
    class_setPropertiesFunction (c, gatom_functionProperties);
    
    gatom_class = c;
}

void gatom_destroy (void)
{
    class_free (gatom_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
