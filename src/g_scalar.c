
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SCALAR_SELECT_MARGIN            5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define SCALAR_WRONG_SIZE               20
#define SCALAR_WRONG_COLOR              0xdddddd        /* Grey. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *scalar_class;                                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error word_setInternalBuffer      (t_word *, t_template *, t_symbol *, t_buffer *);
t_error word_unsetInternalBuffer    (t_word *, t_template *, t_symbol *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scalar_behaviorGetRectangle         (t_gobj *, t_glist *, int *, int *, int *, int *);
static void scalar_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void scalar_behaviorSelected             (t_gobj *, t_glist *, int);
static void scalar_behaviorActivated            (t_gobj *, t_glist *, int);
static void scalar_behaviorDeleted              (t_gobj *, t_glist *);
static void scalar_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  scalar_behaviorMouse                (t_gobj *, t_glist *, int, int, int, int, int, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior scalar_widgetBehavior =         /* Shared. */
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

static void scalar_drawJob (t_gobj *z, t_glist *glist)
{
    scalar_behaviorVisibilityChanged (z, glist, 0);
    scalar_behaviorVisibilityChanged (z, glist, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scalar_drawSelectRectangle (t_scalar *x, t_glist *glist, int isSelected)
{
    if (isSelected) {
    
        int a, b, c, d;
       
        scalar_behaviorGetRectangle (cast_gobj (x), glist, &a, &b, &c, &d);
        
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -width 0"
                        " -fill #%06x"
                        " -dash {2 4}"  // --
                        " -tags %lxHANDLE\n",
                        canvas_getView (glist),
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
                
    } else {
        sys_vGui (".x%lx.c delete %lxHANDLE\n", canvas_getView (glist), x);
    }
}

static void scalar_notifyClicked (t_scalar *x, 
    t_glist *glist,
    t_template *template,
    t_float positionX,
    t_float positionY)
{
    t_atom t[2];
    SET_FLOAT (t + 0, positionX);
    SET_FLOAT (t + 1, positionY);
    template_notify (template, glist, x, sym_click, 2, t);
}
    
static void scalar_notifyDisplaced (t_scalar *x, 
    t_glist *glist,
    t_template *template,
    t_float deltaX,
    t_float deltaY)
{
    t_atom t[2];
    SET_FLOAT (t + 0, deltaX);
    SET_FLOAT (t + 1, deltaY);
    template_notify (template, glist, x, sym_displace, 2, t);
}

static void scalar_notifySelected (t_scalar *x, 
    t_glist *glist,
    t_template *template,
    int isSelected)
{
    if (isSelected) { template_notify (template, glist, x, sym_select, 0, NULL); } 
    else {
        template_notify (template, glist, x, sym_deselect, 0, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scalar_redraw (t_scalar *x, t_glist *glist)
{
    defer_addJob ((void *)x, glist, scalar_drawJob);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scalar_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_scalar *x = cast_scalar (z);
    
    int xA = 0;
    int yA = 0;
    int xB = 0;
    int yB = 0;
    
    t_template *template = scalar_getTemplate (x);
    t_glist *view = template_getFirstInstanceView (template);
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);

    if (!view) {
    
        xA = canvas_valueToPixelX (glist, baseX);
        yA = canvas_valueToPixelY (glist, baseY);
        xB = xA + SCALAR_WRONG_SIZE;
        yB = yA + SCALAR_WRONG_SIZE;
        
    } else {
    
        t_gobj *y = NULL;
        
        area_setNowhere (&xA, &yA, &xB, &yB);
        
        for (y = view->gl_graphics; y; y = y->g_next) {
        //
        t_painterwidgetbehavior *behavior = class_getPainterWidget (pd_class (y));
        
        if (behavior) {
        //
        int e, f, g, h;
        
        t_gpointer gp; GPOINTER_INIT (&gp);
        
        gpointer_setAsScalar (&gp, glist, x);
        
        (*behavior->w_fnPainterGetRectangle) (y, &gp, baseX, baseY, &e, &f, &g, &h);
        
        gpointer_unset (&gp);
        
        xA = PD_MIN (xA, e); yA = PD_MIN (yA, f); xB = PD_MAX (xB, g); yB = PD_MAX (yB, h);
        //
        }
        //
        }
        
        if (xB < xA || yB < yA) { area_setNothing (&xA, &yA, &xB, &yB); }
        else {
            if (!area_isEverything (xA, yA, xB, yB)) {
                xA -= SCALAR_SELECT_MARGIN;
                yA -= SCALAR_SELECT_MARGIN;
                xB += SCALAR_SELECT_MARGIN;
                yB += SCALAR_SELECT_MARGIN;
            }
        }
    }

    *a = xA;
    *b = yA;
    *c = xB;
    *d = yB; 
}

static void scalar_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *template = scalar_getTemplate (x);
    
    if (!template) { PD_BUG; }
    else {
    //
    if (template_fieldIsFloat (template, sym_x)) {
    //
    t_float f = word_getFloat (x->sc_vector, template, sym_x);
    f += canvas_valueForDeltaInPixelX (glist, deltaX);
    word_setFloat (x->sc_vector, template, sym_x, f);
    //
    }
    
    if (template_fieldIsFloat (template, sym_y)) {
    //
    t_float f = word_getFloat (x->sc_vector, template, sym_y);
    f += canvas_valueForDeltaInPixelY (glist, deltaY);
    word_setFloat (x->sc_vector, template, sym_y, f);
    //
    }
    
    scalar_notifyDisplaced (x, glist, template, (t_float)deltaX, (t_float)deltaY);
    scalar_redraw (x, glist);
    //
    }
}

static void scalar_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *template = scalar_getTemplate (x);
    
    if (!template) { PD_BUG; }
    else {
        scalar_notifySelected (x, glist, template, isSelected);
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
    
    t_template *template = scalar_getTemplate (x);
    t_glist *view = template_getFirstInstanceView (template);
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);

    if (!view) {
        
        if (isVisible) {
        
            int a = canvas_valueToPixelX (glist, baseX);
            int b = canvas_valueToPixelY (glist, baseY);
            
            sys_vGui (".x%lx.c create rectangle %d %d %d %d"
                            " -outline #%06x"
                            " -tags %lxSCALAR\n",
                            canvas_getView (glist),
                            a,
                            b,
                            a + SCALAR_WRONG_SIZE,
                            b + SCALAR_WRONG_SIZE,
                            SCALAR_WRONG_COLOR,
                            x);
        } else {
            sys_vGui (".x%lx.c delete %lxSCALAR\n", canvas_getView (glist), x);
        }
        
    } else {
    
        t_gobj *y = NULL;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
        
            t_painterwidgetbehavior *behavior = class_getPainterWidget (pd_class (y));
            
            if (behavior) {
                
                t_gpointer gp; GPOINTER_INIT (&gp);
                
                gpointer_setAsScalar (&gp, glist, x);
                
                (*behavior->w_fnPainterVisibilityChanged) (y, &gp, baseX, baseY, isVisible);
                
                gpointer_unset (&gp);
            }
        }
    }
    
    if (canvas_isObjectSelected (glist, cast_gobj (x))) {
        scalar_drawSelectRectangle (x, glist, 0);
        scalar_drawSelectRectangle (x, glist, 1);
    }
}

static int scalar_behaviorMouse (t_gobj *z,
    t_glist *glist,
    int a,
    int b,
    int shift,
    int ctrl,
    int alt,
    int dbl,
    int clicked)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *template = scalar_getTemplate (x);
    t_glist *view = template_getFirstInstanceView (template);
    
    if (view) {
    //
    t_float baseX = scalar_getFloat (x, sym_x);
    t_float baseY = scalar_getFloat (x, sym_y);
    t_gobj *y = NULL;
        
    if (clicked) { scalar_notifyClicked (x, glist, template, baseX, baseY); }
            
    for (y = view->gl_graphics; y; y = y->g_next) {
    //
    t_painterwidgetbehavior *behavior = class_getPainterWidget (pd_class (y));
    
    if (behavior) { 
        
        t_gpointer gp; GPOINTER_INIT (&gp);
        
        gpointer_setAsScalar (&gp, glist, x);
        
        int k = (*behavior->w_fnPainterMouse) (y,
                    &gp,
                    baseX,
                    baseY,
                    a,
                    b,
                    shift,
                    alt,
                    dbl,
                    clicked);
        
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
#pragma mark -

void scalar_serialize (t_scalar *x, t_buffer *b)
{
    t_template *template = scalar_getTemplate (x);
    int i;
        
    buffer_vAppend (b, "s", utils_stripTemplateIdentifier (x->sc_templateIdentifier));

    for (i = 0; i < template_getSize (template); i++) {
    
        t_symbol *fieldName = template_getFieldAtIndex (template, i);
        
        if (template_fieldIsFloat (template, fieldName)) {
            t_atom t;
            SET_FLOAT (&t, word_getFloat (x->sc_vector, template, fieldName));
            buffer_appendAtom (b, &t);
            
        } else if (template_fieldIsSymbol (template, fieldName)) {
            t_atom t;
            SET_SYMBOL (&t, word_getSymbol (x->sc_vector, template, fieldName));
            buffer_appendAtom (b, &t);
        }
    }

    buffer_appendSemicolon (b);

    for (i = 0; i < template_getSize (template); i++) {
    
        t_symbol *fieldName = template_getFieldAtIndex (template, i);
            
        if (template_fieldIsArray (template, fieldName)) {
            array_serialize (word_getArray (x->sc_vector, template, fieldName), b);
            buffer_appendSemicolon (b);
            
        } else if (template_fieldIsText (template, fieldName)) {
            buffer_serialize (b, word_getText (x->sc_vector, template, fieldName));
            buffer_appendSemicolon (b);
        }
    }
}

void scalar_deserialize (t_scalar *x, t_glist *glist, int argc, t_atom *argv)
{
    t_template *template = scalar_getTemplate (x);
    
    if (!template_isValid (template)) { PD_BUG; }
    else {
    //
    t_iterator *iter = iterator_new (argc, argv);
    t_atom *atoms = NULL;
    int count = iterator_next (iter, &atoms);      
    int i;
        
    for (i = 0; i < template_getSize (template); i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (template, i);
    
    if (template_fieldIsFloat (template, fieldName)) {
        t_float f = (t_float)0.0;
        if (count) { f = atom_getFloat (atoms); atoms++; count--; }
        word_setFloat (x->sc_vector, template, fieldName, f);
        
    } else if (template_fieldIsSymbol (template, fieldName)) {
        t_symbol *s = &s_;
        if (count) { s = atom_getSymbol (atoms); atoms++; count--; }
        word_setSymbol (x->sc_vector, template, fieldName, s);
    }
    //
    }
    
    PD_ASSERT (count == 0);
    
    for (i = 0; i < template_getSize (template); i++) {
    //
    t_symbol *fieldName = template_getFieldAtIndex (template, i);
    
    if (template_fieldIsArray (template, fieldName)) {
        array_deserialize (word_getArray (x->sc_vector, template, fieldName), iter);

    } else if (template_fieldIsText (template, fieldName)) {
        t_buffer *t = buffer_new();
        count = iterator_next (iter, &atoms);
        buffer_deserialize (t, count, atoms);
        word_setText (x->sc_vector, template, fieldName, t);
        buffer_free (t);
    }
    //
    }
    
    iterator_free (iter);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scalar_functionSave (t_gobj *z, t_buffer *b)
{
    t_scalar *x = cast_scalar (z);
    t_buffer *t = buffer_new();
   
    scalar_serialize (x, t);
    buffer_vAppend (b, "ss", sym___hash__X, sym_scalar);
    buffer_serialize (b, t);
    buffer_appendSemicolon (b);
    
    buffer_free (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_word *scalar_getData (t_scalar *x)
{
    return x->sc_vector;
}

t_template *scalar_getTemplate (t_scalar *x)
{
    t_template *template = template_findByIdentifier (x->sc_templateIdentifier);
    
    PD_ASSERT (template);
    
    return template;
}

t_symbol *scalar_getTemplateIdentifier (t_scalar *x)
{
    return x->sc_templateIdentifier;
}

t_array *scalar_getArray (t_scalar *x, t_symbol *fieldName)
{
    return word_getArray (x->sc_vector, scalar_getTemplate (x), fieldName);
}

t_float scalar_getFloat (t_scalar *x, t_symbol *fieldName)
{
    return word_getFloat (x->sc_vector, scalar_getTemplate (x), fieldName);
}

void scalar_setFloat (t_scalar *x, t_symbol *fieldName, t_float f)
{
    word_setFloat (x->sc_vector, scalar_getTemplate (x), fieldName, f);  
}

int scalar_fieldIsFloat (t_scalar *x, t_symbol *fieldName)
{
    return template_fieldIsFloat (scalar_getTemplate (x), fieldName);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error scalar_setInternalBuffer (t_scalar *x, t_symbol *fieldName, t_buffer *b)
{
    return word_setInternalBuffer (x->sc_vector, scalar_getTemplate (x), fieldName, b);
}

t_error scalar_unsetInternalBuffer (t_scalar *x, t_symbol *fieldName)
{
    return word_unsetInternalBuffer (x->sc_vector, scalar_getTemplate (x), fieldName);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar *scalar_new (t_glist *owner, t_symbol *templateIdentifier)
{
    t_scalar *x = NULL;
    
    t_template *template = template_findByIdentifier (templateIdentifier);

    if (!template_isValid (template)) { PD_BUG; }
    else {

        t_gpointer gp; GPOINTER_INIT (&gp);
        
        x = (t_scalar *)pd_new (scalar_class);
        
        x->sc_templateIdentifier = templateIdentifier;
        x->sc_vector = (t_word *)PD_MEMORY_GET (template_getSize (template) * sizeof (t_word));
        
        gpointer_setAsScalar (&gp, owner, x);
        word_init (x->sc_vector, template, &gp);
        gpointer_unset (&gp);
    }
    
    return x;
}

static void scalar_free (t_scalar *x)
{
    word_free (x->sc_vector, scalar_getTemplate (x));

    PD_MEMORY_FREE (x->sc_vector);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    CLASS_FREE (scalar_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
