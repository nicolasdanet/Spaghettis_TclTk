
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Changes by Thomas Musil IEM KUG Graz Austria 2001. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"
#include "s_utf8.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void gatom_float                     (t_gatom *, t_float);
static void gatom_set                       (t_gatom *, t_symbol *, int, t_atom *);
static void gatom_motion                    (void *, t_float, t_float, t_float);
static void gatom_behaviorDisplaced         (t_gobj *, t_glist *, int, int);
static void gatom_behaviorSelected          (t_gobj *, t_glist *, int);
static void gatom_behaviorVisibilityChanged (t_gobj *, t_glist *, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *gatom_class;                                   /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _gatom {
    t_object        a_obj;                              /* MUST be the first. */
    t_atom          a_atom;
    t_float         a_lowRange;
    t_float         a_highRange;
    t_fontsize      a_fontSize;
    int             a_position;
    int             a_isSelected;
    t_glist         *a_owner;
    t_symbol        *a_send;
    t_symbol        *a_receive;
    t_symbol        *a_label;
    t_symbol        *a_unexpandedSend;
    t_symbol        *a_unexpandedReceive;
    t_symbol        *a_unexpandedLabel;
    t_outlet        *a_outlet;
    };
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior gatom_widgetBehavior =          /* Shared. */
    {
        text_behaviorGetRectangle,
        gatom_behaviorDisplaced,
        gatom_behaviorSelected,
        NULL,
        text_behaviorDeleted,
        gatom_behaviorVisibilityChanged,
        text_behaviorMouse
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define ATOM_LABEL_LEFT         0
#define ATOM_LABEL_RIGHT        1
#define ATOM_LABEL_UP           2
#define ATOM_LABEL_DOWN         3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ATOM_WIDTH_FLOAT        5
#define ATOM_WIDTH_SYMBOL       10
#define ATOM_WIDTH_MAXIMUM      80

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define ATOM_DIALOG_SIZE        8

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_drawJob (t_gobj *z, t_glist *glist)
{
    t_gatom *x = (t_gatom *)z;
    
    box_retext (x->a_owner, cast_object (x));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_symbol *gatom_parse (t_symbol *s)
{
    if (s == utils_empty() || s == utils_dash()) { return &s_; }
    else { 
        return dollar_fromHash (s);
    }
}

static void gatom_update (t_gatom *x)
{
    buffer_reset (object_getBuffer (cast_object (x)));
    buffer_appendAtom (object_getBuffer (cast_object (x)), &x->a_atom);
    defer_addJob ((void *)x, x->a_owner, gatom_drawJob);
}

static void gatom_setFloat (t_gatom *x, t_float f)
{
    if (x->a_lowRange != 0.0 || x->a_highRange != 0.0) { f = PD_CLAMP (f, x->a_lowRange, x->a_highRange); }
    
    gatom_float (x, f);
}

static void gatom_getPostion (t_gatom *x, t_glist *glist, int *positionX, int *positionY)
{
    double width  = font_getHostFontWidth (x->a_fontSize);
    double height = font_getHostFontHeight (x->a_fontSize);
    
    t_rectangle r;
    
    text_behaviorGetRectangle (cast_gobj (x), glist, &r);
    
    {
        int a = rectangle_getTopLeftX (&r);
        int b = rectangle_getTopLeftY (&r);
        int c = rectangle_getBottomRightX (&r);
        int d = rectangle_getBottomRightY (&r);
        
        if (x->a_position == ATOM_LABEL_LEFT) {
            *positionX = a - 3 - (int)(strlen (x->a_label->s_name) * width);
            *positionY = b + 2;
            
        } else if (x->a_position == ATOM_LABEL_RIGHT) {
            *positionX = c + 3;
            *positionY = b + 2;
            
        } else if (x->a_position == ATOM_LABEL_UP) {
            *positionX = a;
            *positionY = b - 3 - (int)height;
            
        } else {
            *positionX = a;
            *positionY = d + 3;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_bang (t_gatom *x)
{
    if (IS_FLOAT (&x->a_atom)) {
    
        outlet_float (x->a_outlet, GET_FLOAT (&x->a_atom));
        
        if (x->a_send != &s_ && pd_isThing (x->a_send)) {
            if (x->a_send != x->a_receive) { pd_float (pd_getThing (x->a_send), GET_FLOAT (&x->a_atom)); }
            else {
                error_sendReceiveLoop (x->a_unexpandedSend);
            }
        }
        
    } else {
    
        outlet_symbol (x->a_outlet, GET_SYMBOL (&x->a_atom));
        
        if (x->a_send != &s_ && pd_isThing (x->a_send)) {
            if (x->a_send != x->a_receive) { pd_symbol (pd_getThing (x->a_send), GET_SYMBOL (&x->a_atom)); }
            else {
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
    if (IS_FLOAT (&x->a_atom)) {
    //
    t_float a = atom_getFloatAtIndex (0, argc, argv);
    t_float b = atom_getFloatAtIndex (1, argc, argv);
    canvas_setMotionFunction (x->a_owner, cast_gobj (x), (t_motionfn)gatom_motion, a, b);
    //
    }
    
    gatom_bang (x);
}

static void gatom_set (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) {
    //
    if (IS_FLOAT (&x->a_atom)) { SET_FLOAT (&x->a_atom, atom_getFloat (argv)); }
    else {
        SET_SYMBOL (&x->a_atom, atom_getSymbol (argv));
    }

    gatom_update (x);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_motion (void *z, t_float deltaX, t_float deltaY, t_float modifier)
{
    t_gatom *x = (t_gatom *)z;
    
    PD_ASSERT (IS_FLOAT (&x->a_atom));
    
    if (deltaY != 0.0) { 
    //
    double f = GET_FLOAT (&x->a_atom);
    
    if ((int)modifier & MODIFIER_SHIFT) { f -= 0.01 * deltaY; }
    else {
        f -= deltaY;
    }
    
    gatom_setFloat (x, (t_float)f);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void gatom_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_gatom *x = (t_gatom *)z;
    
    text_behaviorDisplaced (z, glist, deltaX, deltaY);
    
    sys_vGui (".x%lx.c move %lxLABEL %d %d\n", 
                    canvas_getView (glist), 
                    x,
                    deltaX,
                    deltaY);
}

static void gatom_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_gatom *x = (t_gatom *)z;
    
    text_behaviorSelected (z, glist, isSelected);
    
    x->a_isSelected = isSelected;
    
    sys_vGui (".x%lx.c itemconfigure %lxLABEL -fill #%06x\n", 
                    canvas_getView (glist), 
                    x,
                    (isSelected ? COLOR_SELECTED : COLOR_NORMAL));
}

static void gatom_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_gatom *x = (t_gatom *)z;
    
    text_behaviorVisibilityChanged (z, glist, isVisible);
    
    if (x->a_label != &s_) {
    //
    if (!isVisible) { sys_vGui (".x%lx.c delete %lxLABEL\n", canvas_getView (glist), x); }
    else { 
        int positionX = 0;
        int positionY = 0;
        
        gatom_getPostion (x, glist, &positionX, &positionY);
        
        sys_vGui ("::ui_box::newText .x%lx.c %lxLABEL %d %d {%s} %d #%06x\n",   // --
                        canvas_getView (glist),
                        x,
                        positionX,
                        positionY,
                        x->a_label->s_name,
                        font_getHostFontSize (x->a_fontSize),
                        x->a_isSelected ? COLOR_SELECTED : COLOR_NORMAL);
    }
    //
    }
    
    if (!isVisible) { defer_removeJob ((void *)z); }
}

static void gatom_functionSave (t_gobj *z, t_buffer *b)
{
    t_gatom *x = (t_gatom *)z;
    
    buffer_vAppend (b, "ssiiifffsss;",
        sym___hash__X,
        (IS_SYMBOL (&x->a_atom) ? sym_symbolatom : sym_floatatom),
        object_getX (cast_object (x)),
        object_getY (cast_object (x)),
        object_getWidth (cast_object (x)),
        (double)x->a_lowRange,
        (double)x->a_highRange,
        (double)x->a_position,
        dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedLabel, 1)),
        dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedReceive, 1)),
        dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedSend, 1)));

    object_saveWidth (cast_object (x), b);
}

static void gatom_functionProperties (t_gobj *z, t_glist *owner)
{
    t_gatom *x  = (t_gatom *)z;
    t_error err = PD_ERROR_NONE;
    char t[PD_STRING] = { 0 };
    
    t_symbol *symSend    = dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedSend, 0));
    t_symbol *symReceive = dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedReceive, 0));
    t_symbol *symLabel   = dollar_toHash (utils_substituteIfEmpty (x->a_unexpandedLabel, 0));
    
    if (IS_FLOAT (&x->a_atom)) {
    
        err = string_sprintf (t, PD_STRING, 
                "::ui_atom::show %%s %d %g %g %s %g {%s} {%s} {%s} %d\n",       // --
                object_getWidth (cast_object (x)),
                x->a_lowRange,
                x->a_highRange,
                sym_floatatom->s_name,
                GET_FLOAT (&x->a_atom),
                symSend->s_name,
                symReceive->s_name,
                symLabel->s_name, 
                x->a_position);
            
    } else {
    
        err = string_sprintf (t, PD_STRING, 
                "::ui_atom::show %%s %d %g %g %s {%s} {%s} {%s} {%s} %d\n",     // --
                object_getWidth (cast_object (x)),
                x->a_lowRange,
                x->a_highRange,
                sym_symbolatom->s_name,
                GET_SYMBOL (&x->a_atom)->s_name,
                symSend->s_name,
                symReceive->s_name,
                symLabel->s_name, 
                x->a_position);
    }
    
    PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

static void gatom_fromDialog (t_gatom *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == ATOM_DIALOG_SIZE) {
    //
    t_float width           = atom_getFloatAtIndex (0, argc, argv);
    t_float lowRange        = atom_getFloatAtIndex (1, argc, argv);
    t_float highRange       = atom_getFloatAtIndex (2, argc, argv);
    t_symbol *symSend       = gatom_parse (atom_getSymbolAtIndex (4, argc, argv));
    t_symbol *symReceive    = gatom_parse (atom_getSymbolAtIndex (5, argc, argv));
    t_symbol *symLabel      = gatom_parse (atom_getSymbolAtIndex (6, argc, argv));
    int position            = (int)atom_getFloatAtIndex (7, argc, argv);

    gobj_visibilityChanged (cast_gobj (x), x->a_owner, 0);

    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }
    
    object_setWidth (cast_object (x), PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM));

    x->a_lowRange               = PD_MIN (lowRange, highRange);
    x->a_highRange              = PD_MAX (lowRange, highRange);
    x->a_position               = PD_CLAMP (position, ATOM_LABEL_LEFT, ATOM_LABEL_DOWN);
    x->a_unexpandedSend         = symSend;
    x->a_unexpandedReceive      = symReceive;
    x->a_unexpandedLabel        = symLabel;
    x->a_send                   = canvas_expandDollar (x->a_owner, x->a_unexpandedSend);
    x->a_receive                = canvas_expandDollar (x->a_owner, x->a_unexpandedReceive);
    x->a_label                  = canvas_expandDollar (x->a_owner, x->a_unexpandedLabel);
    
    if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }
    
    gatom_set (x, NULL, 1, argv + 3);

    gobj_visibilityChanged (cast_gobj (x), x->a_owner, 1);
    
    canvas_dirty (x->a_owner, 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gatom_makeObject (t_glist *glist, t_atomtype type, t_symbol *s, int argc, t_atom *argv)
{
    t_gatom *x = (t_gatom *)pd_new (gatom_class);
    
    object_setBuffer (cast_object (x), buffer_new());
    object_setWidth (cast_object (x), type == A_FLOAT ? ATOM_WIDTH_FLOAT : ATOM_WIDTH_SYMBOL);
    object_setType (cast_object (x), TYPE_ATOM);
    
    x->a_owner                  = glist;
    x->a_lowRange               = 0;
    x->a_highRange              = 0;
    x->a_fontSize               = canvas_getFontSize (x->a_owner);
    x->a_position               = ATOM_LABEL_RIGHT;
    x->a_send                   = &s_;
    x->a_receive                = &s_;
    x->a_label                  = &s_;
    x->a_unexpandedSend         = &s_;
    x->a_unexpandedReceive      = &s_;
    x->a_unexpandedLabel        = &s_;
    
    PD_ASSERT (type == A_FLOAT || type == A_SYMBOL);
    
    if (type == A_FLOAT) {
        t_atom a;
        SET_FLOAT (&x->a_atom, (t_float)0.0);
        SET_FLOAT (&a, (t_float)0.0);
        buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
        
    } else {
        t_atom a;
        SET_SYMBOL (&x->a_atom, &s_symbol);
        SET_SYMBOL (&a, &s_symbol);
        buffer_appendAtom (object_getBuffer (cast_object (x)), &a);
    }
    
    if (argc > 1) {                                                             /* File creation. */
    
        int width    = (int)atom_getFloatAtIndex (2, argc, argv);
        int position = (int)atom_getFloatAtIndex (5, argc, argv);
        
        object_setX (cast_object (x), atom_getFloatAtIndex (0, argc, argv));
        object_setY (cast_object (x), atom_getFloatAtIndex (1, argc, argv));
        object_setWidth (cast_object (x), PD_CLAMP (width, 0, ATOM_WIDTH_MAXIMUM));

        x->a_lowRange                       = atom_getFloatAtIndex (3, argc, argv);
        x->a_highRange                      = atom_getFloatAtIndex (4, argc, argv);
        x->a_position                       = PD_CLAMP (position, ATOM_LABEL_LEFT, ATOM_LABEL_DOWN);
        x->a_unexpandedLabel                = gatom_parse (atom_getSymbolAtIndex (6, argc, argv));
        x->a_unexpandedReceive              = gatom_parse (atom_getSymbolAtIndex (7, argc, argv));
        x->a_unexpandedSend                 = gatom_parse (atom_getSymbolAtIndex (8, argc, argv));
        x->a_send                           = canvas_expandDollar (x->a_owner, x->a_unexpandedSend);
        x->a_receive                        = canvas_expandDollar (x->a_owner, x->a_unexpandedReceive);
        x->a_label                          = canvas_expandDollar (x->a_owner, x->a_unexpandedLabel);
                
        if (x->a_receive != &s_) { pd_bind (cast_pd (x), x->a_receive); }

        x->a_outlet = outlet_new (cast_object (x), IS_FLOAT (&x->a_atom) ? &s_float : &s_symbol);
        
        canvas_addObject (glist, cast_gobj (x));
        
    } else {                                                                    /* Interactive creation. */
    
        int positionX = 0;
        int positionY = 0;

        canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll (glist);
        
        object_setX (cast_object (x), positionX);
        object_setY (cast_object (x), positionY);
        
        x->a_outlet = outlet_new (cast_object (x), IS_FLOAT (&x->a_atom) ? &s_float : &s_symbol);
                
        canvas_addObject (glist, cast_gobj (x));
        
        canvas_selectObject (glist, cast_gobj (x));
    }
}

static void gatom_free (t_gatom *x)
{
    defer_removeJob ((void *)x);
    
    if (x->a_receive != &s_) { pd_unbind (cast_pd (x), x->a_receive); }
    
    stub_destroyWithKey ((void *)x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

    class_setWidgetBehavior (c, &gatom_widgetBehavior);
    class_setSaveFunction (c, gatom_functionSave);
    class_setPropertiesFunction (c, gatom_functionProperties);
    
    gatom_class = c;
}

void gatom_destroy (void)
{
    CLASS_FREE (gatom_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
