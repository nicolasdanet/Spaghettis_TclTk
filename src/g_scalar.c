
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SCALAR_SELECT_MARGIN            5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define SCALAR_WRONG_SIZE               20

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_class *scalar_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error word_setInternalBuffer                  (t_word *, t_template *, t_symbol *, t_buffer *);
t_error word_unsetInternalBuffer                (t_word *, t_template *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_behaviorGetRectangle         (t_gobj *, t_glist *, t_rectangle *);
static void scalar_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void scalar_behaviorSelected             (t_gobj *, t_glist *, int);
static void scalar_behaviorActivated            (t_gobj *, t_glist *, int);
static void scalar_behaviorDeleted              (t_gobj *, t_glist *);
static void scalar_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  scalar_behaviorMouse                (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior scalar_widgetBehavior = /* Shared. */
    {
        scalar_behaviorGetRectangle,
        scalar_behaviorDisplaced,
        scalar_behaviorSelected,
        scalar_behaviorActivated,
        scalar_behaviorDeleted,
        scalar_behaviorVisibilityChanged,
        scalar_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_drawJob (t_gobj *z, t_glist *glist)
{
    scalar_behaviorVisibilityChanged (z, glist, 0);
    scalar_behaviorVisibilityChanged (z, glist, 1);
}

static void scalar_drawSelectRectangle (t_scalar *x, t_glist *glist, int isSelected)
{
    t_glist *view = glist_getView (glist);
    
    if (isSelected) {
    
        t_rectangle r;
       
        scalar_behaviorGetRectangle (cast_gobj (x), glist, &r);
        
        PD_ASSERT (!rectangle_isNothing (&r));
        
        {
            int a = rectangle_getTopLeftX (&r);
            int b = rectangle_getTopLeftY (&r);
            int c = rectangle_getBottomRightX (&r);
            int d = rectangle_getBottomRightY (&r);
            
            gui_vAdd ("%s.c create line %d %d %d %d %d %d %d %d %d %d"
                            " -width 0"
                            " -fill #%06x"
                            " -dash {2 4}"  // --
                            " -tags %lxHANDLE\n",
                            glist_getTagAsString (view),
                            a,
                            b,
                            a,
                            d,
                            c,
                            d,
                            c,
                            b,
                            a,
                            b,
                            COLOR_SELECTED,
                            x);
        }
                
    } else {
        gui_vAdd ("%s.c delete %lxHANDLE\n", glist_getTagAsString (view), x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_notifyClicked (t_scalar *x, 
    t_glist *glist,
    t_template *tmpl,
    t_float positionX,
    t_float positionY)
{
    t_atom t[2];
    SET_FLOAT (t + 0, positionX);
    SET_FLOAT (t + 1, positionY);
    template_notify (tmpl, glist, x, sym_click, 2, t);
}
    
static void scalar_notifyDisplaced (t_scalar *x, 
    t_glist *glist,
    t_template *tmpl,
    t_float deltaX,
    t_float deltaY)
{
    t_atom t[2];
    SET_FLOAT (t + 0, deltaX);
    SET_FLOAT (t + 1, deltaY);
    template_notify (tmpl, glist, x, sym_displace, 2, t);
}

static void scalar_notifySelected (t_scalar *x, 
    t_glist *glist,
    t_template *tmpl,
    int isSelected)
{
    if (isSelected) { template_notify (tmpl, glist, x, sym_select, 0, NULL); }
    else {
        template_notify (tmpl, glist, x, sym_deselect, 0, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_redraw (t_scalar *x, t_glist *glist)
{
    gui_jobAdd ((void *)x, glist, scalar_drawJob);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_snap (t_scalar *x, t_glist *glist)
{
    t_template *tmpl = scalar_getTemplate (x);
    
    if (!tmpl) { PD_BUG; }
    else {
    //
    int deltaX = 0;
    int deltaY = 0;
    
    if (template_fieldIsFloat (tmpl, sym_x)) {
        t_float f = word_getFloat (x->sc_element, tmpl, sym_x);
        deltaX = snap_getOffset ((int)(f / glist_getValueForOnePixelX (glist)));
    }
    
    if (template_fieldIsFloat (tmpl, sym_y)) {
        t_float f = word_getFloat (x->sc_element, tmpl, sym_y);
        deltaY = snap_getOffset ((int)(f / glist_getValueForOnePixelY (glist)));
    }
    
    if (deltaX || deltaY) {
        scalar_behaviorDisplaced (cast_gobj (x), glist, deltaX, deltaY);
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void scalar_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_scalar *x = cast_scalar (z);
    t_template *tmpl = scalar_getTemplate (x);
    t_glist *view = template_getInstanceViewIfPainters (tmpl);
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);

    rectangle_setNothing (r);
    
    if (view) {
    
        t_gobj *y = NULL;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
        //
        t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
        
        if (behavior) {
        //
        t_rectangle t;
        t_gpointer gp; gpointer_init (&gp);
        
        gpointer_setAsScalar (&gp, glist, x);
        (*behavior->w_fnPainterGetRectangle) (y, &gp, baseX, baseY, &t);
        gpointer_unset (&gp);
        
        rectangle_addRectangle (r, &t);
        //
        }
        //
        }
    }
    
    if (rectangle_isNothing (r)) {
    
        int a = glist_valueToPixelX (glist, baseX);
        int b = glist_valueToPixelY (glist, baseY);
        int c = a + SCALAR_WRONG_SIZE;
        int d = b + SCALAR_WRONG_SIZE;
        
        rectangle_set (r, a, b, c, d);
        
    } else if (!rectangle_isEverything (r)) { rectangle_enlarge (r, SCALAR_SELECT_MARGIN); }
}

static void scalar_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *tmpl = scalar_getTemplate (x);
    
    if (!tmpl) { PD_BUG; }
    else {
    //
    if (template_fieldIsFloat (tmpl, sym_x)) {

        t_float f = word_getFloat (x->sc_element, tmpl, sym_x);
        f += (deltaX * glist_getValueForOnePixelX (glist));
        word_setFloat (x->sc_element, tmpl, sym_x, f);
    }
    
    if (template_fieldIsFloat (tmpl, sym_y)) {

        t_float f = word_getFloat (x->sc_element, tmpl, sym_y);
        f += (deltaY * glist_getValueForOnePixelY (glist));
        word_setFloat (x->sc_element, tmpl, sym_y, f);
    }
    
    scalar_notifyDisplaced (x, glist, tmpl, (t_float)deltaX, (t_float)deltaY);
    scalar_redraw (x, glist);
    glist_redrawRequired (glist);
    //
    }
}

static void scalar_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *tmpl = scalar_getTemplate (x);
    
    if (!tmpl) { PD_BUG; }
    else {
        scalar_notifySelected (x, glist, tmpl, isSelected);
        scalar_drawSelectRectangle (x, glist, isSelected);
    }
}

static void scalar_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
}

static void scalar_behaviorDeleted (t_gobj *z, t_glist *glist)
{
}

static void scalar_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *tmpl = scalar_getTemplate (x);
    t_glist *owner = template_getInstanceViewIfPainters (tmpl);
    t_float baseX  = scalar_getFloat (x, sym_x);
    t_float baseY  = scalar_getFloat (x, sym_y);

    if (!owner) {
        
        t_glist *view = glist_getView (glist);
        
        if (isVisible) {
        
            int a = glist_valueToPixelX (glist, baseX);
            int b = glist_valueToPixelY (glist, baseY);
            
            gui_vAdd ("%s.c create rectangle %d %d %d %d"
                            " -outline #%06x"
                            " -tags %lxSCALAR\n",
                            glist_getTagAsString (view),
                            a,
                            b,
                            a + SCALAR_WRONG_SIZE,
                            b + SCALAR_WRONG_SIZE,
                            COLOR_SCALAR_WRONG,
                            x);
        } else {
            gui_vAdd ("%s.c delete %lxSCALAR\n", glist_getTagAsString (view), x);
        }
        
    } else {
    
        t_gobj *y = NULL;
        
        for (y = owner->gl_graphics; y; y = y->g_next) {
        
            t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
            
            if (behavior) {
                
                t_gpointer gp; gpointer_init (&gp);
                
                gpointer_setAsScalar (&gp, glist, x);
                
                (*behavior->w_fnPainterVisibilityChanged) (y, &gp, baseX, baseY, isVisible);
                
                gpointer_unset (&gp);
            }
        }
    }
    
    if (glist_objectIsSelected (glist, cast_gobj (x))) {
        scalar_drawSelectRectangle (x, glist, 0);
        scalar_drawSelectRectangle (x, glist, 1);
    }
}

static int scalar_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_scalar *x = cast_scalar (z);
    t_template *tmpl = scalar_getTemplate (x);
    t_glist *view = template_getInstanceViewIfPainters (tmpl);
    
    if (view) {
    //
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);
    t_gobj *y = NULL;
        
    if (m->m_clicked) { scalar_notifyClicked (x, glist, tmpl, baseX, baseY); }
            
    for (y = view->gl_graphics; y; y = y->g_next) {
    //
    t_painterbehavior *behavior = class_getPainterBehavior (pd_class (y));
    
    if (behavior) { 
        
        t_gpointer gp; gpointer_init (&gp);
        
        gpointer_setAsScalar (&gp, glist, x);
        
        int k = (*behavior->w_fnPainterMouse) (y, &gp, baseX, baseY, m);
        
        gpointer_unset (&gp);
        
        if (k) {
            return k;
        }
    }
    //
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_serialize (t_scalar *x, t_buffer *b)
{
    t_template *tmpl = scalar_getTemplate (x);
    int i;
        
    buffer_appendSymbol (b, symbol_stripTemplateIdentifier (x->sc_templateIdentifier));

    for (i = 0; i < template_getSize (tmpl); i++) {
    
        t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
        
        if (template_fieldIsFloat (tmpl, fieldName)) {
            t_atom t;
            SET_FLOAT (&t, word_getFloat (x->sc_element, tmpl, fieldName));
            buffer_appendAtom (b, &t);
            
        } else if (template_fieldIsSymbol (tmpl, fieldName)) {
            t_atom t;
            SET_SYMBOL (&t, word_getSymbol (x->sc_element, tmpl, fieldName));
            buffer_appendAtom (b, &t);
        }
    }

    buffer_appendSemicolon (b);

    for (i = 0; i < template_getSize (tmpl); i++) {
    
        t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
            
        if (template_fieldIsArray (tmpl, fieldName)) {
            array_serialize (word_getArray (x->sc_element, tmpl, fieldName), b);
            buffer_appendSemicolon (b);
            
        } else if (template_fieldIsText (tmpl, fieldName)) {
            buffer_serialize (b, word_getText (x->sc_element, tmpl, fieldName));
            buffer_appendSemicolon (b);
        }
    }
}

void scalar_deserialize (t_scalar *x, t_glist *glist, int argc, t_atom *argv)
{
    t_template *tmpl = scalar_getTemplate (x);
    
    if (!template_isValid (tmpl)) { PD_BUG; }
    else {
    //
    t_iterator *iter = iterator_new (argc, argv);
    t_atom *atoms = NULL;
    int count = iterator_next (iter, &atoms);      
    int i;
        
    for (i = 0; i < template_getSize (tmpl); i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
    
    if (template_fieldIsFloat (tmpl, fieldName)) {
        t_float f = (t_float)0.0;
        if (count) { f = atom_getFloat (atoms); atoms++; count--; }
        word_setFloat (x->sc_element, tmpl, fieldName, f);
        
    } else if (template_fieldIsSymbol (tmpl, fieldName)) {
        t_symbol *s = &s_;
        if (count) { s = atom_getSymbol (atoms); atoms++; count--; }
        word_setSymbol (x->sc_element, tmpl, fieldName, s);
    }
    //
    }
    
    PD_ASSERT (count == 0);
    
    for (i = 0; i < template_getSize (tmpl); i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (tmpl, i);
    
    if (template_fieldIsArray (tmpl, fieldName)) {
        array_deserialize (word_getArray (x->sc_element, tmpl, fieldName), iter);

    } else if (template_fieldIsText (tmpl, fieldName)) {
        t_buffer *t = buffer_new();
        count = iterator_next (iter, &atoms);
        buffer_deserialize (t, count, atoms);
        word_setText (x->sc_element, tmpl, fieldName, t);
        buffer_free (t);
    }
    //
    }
    
    iterator_free (iter);
    //
    }
}

/* Note that scalars are NOT saved with patch. */
/* This function is required for copy and paste behavior. */

static void scalar_functionSave (t_gobj *z, t_buffer *b)
{
    t_scalar *x = cast_scalar (z);
    t_buffer *t = buffer_new();
   
    scalar_serialize (x, t);
    buffer_appendSymbol (b, sym___hash__X);
    buffer_appendSymbol (b, sym_scalar);
    buffer_serialize (b, t);
    buffer_appendSemicolon (b);
    
    buffer_free (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_word *scalar_getElement (t_scalar *x)
{
    return x->sc_element;
}

t_symbol *scalar_getTemplateIdentifier (t_scalar *x)
{
    return x->sc_templateIdentifier;
}

t_template *scalar_getTemplate (t_scalar *x)
{
    t_template *tmpl = template_findByIdentifier (x->sc_templateIdentifier);
    
    PD_ASSERT (tmpl);
    
    return tmpl;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int scalar_containsTemplate (t_scalar *x, t_symbol *templateIdentifier)
{
    return template_containsTemplate (scalar_getTemplate (x), templateIdentifier);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_array *scalar_getArray (t_scalar *x, t_symbol *fieldName)
{
    return word_getArray (x->sc_element, scalar_getTemplate (x), fieldName);
}

t_float scalar_getFloat (t_scalar *x, t_symbol *fieldName)
{
    return word_getFloat (x->sc_element, scalar_getTemplate (x), fieldName);
}

void scalar_setFloat (t_scalar *x, t_symbol *fieldName, t_float f)
{
    word_setFloat (x->sc_element, scalar_getTemplate (x), fieldName, f);  
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int scalar_fieldIsFloat (t_scalar *x, t_symbol *fieldName)
{
    return template_fieldIsFloat (scalar_getTemplate (x), fieldName);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error scalar_setInternalBuffer (t_scalar *x, t_symbol *fieldName, t_buffer *b)
{
    return word_setInternalBuffer (x->sc_element, scalar_getTemplate (x), fieldName, b);
}

t_error scalar_unsetInternalBuffer (t_scalar *x, t_symbol *fieldName)
{
    return word_unsetInternalBuffer (x->sc_element, scalar_getTemplate (x), fieldName);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_scalar *scalar_new (t_glist *owner, t_symbol *templateIdentifier)
{
    t_scalar *x = NULL;
    
    t_template *tmpl = template_findByIdentifier (templateIdentifier);

    if (template_isValid (tmpl)) {

        t_gpointer gp; gpointer_init (&gp);
        
        x = (t_scalar *)pd_new (scalar_class);
        
        x->sc_templateIdentifier = templateIdentifier;
        x->sc_element = (t_word *)PD_MEMORY_GET (template_getSize (tmpl) * sizeof (t_word));
        
        gpointer_setAsScalar (&gp, owner, x);
        word_init (x->sc_element, tmpl, &gp);
        gpointer_unset (&gp);
    }
    
    return x;
}

static void scalar_free (t_scalar *x)
{
    gui_jobRemove ((void *)x);
    
    word_free (x->sc_element, scalar_getTemplate (x));

    PD_MEMORY_FREE (x->sc_element);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void scalar_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_scalar,
        NULL,
        (t_method)scalar_free,
        sizeof (t_scalar),
        CLASS_GRAPHIC,
        A_NULL);
        
    class_setWidgetBehavior (c, &scalar_widgetBehavior);
    class_setSaveFunction (c, scalar_functionSave);
    
    scalar_class = c;
}

void scalar_destroy (void)
{
    class_free (scalar_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
