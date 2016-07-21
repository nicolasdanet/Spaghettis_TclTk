
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

static void scalar_behaviorGetRectangle         (t_gobj *, t_glist *, int *, int *, int *, int *);
static void scalar_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void scalar_behaviorSelected             (t_gobj *, t_glist *, int);
static void scalar_behaviorActivated            (t_gobj *, t_glist *, int);
static void scalar_behaviorDeleted              (t_gobj *, t_glist *);
static void scalar_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  scalar_behaviorClicked              (t_gobj *, t_glist *, int, int, int, int, int, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_widgetbehavior scalar_widgetBehavior =         /* Shared. */
    {
        scalar_behaviorGetRectangle,
        scalar_behaviorDisplaced,
        scalar_behaviorSelected,
        scalar_behaviorActivated,
        scalar_behaviorDeleted,
        scalar_behaviorVisibilityChanged,
        scalar_behaviorClicked,
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

static t_float scalar_getCoordinateX (t_scalar *x)
{
    return template_getFloat (template_findByIdentifier (x->sc_templateIdentifier), sym_x, x->sc_vector);
}

static t_float scalar_getCoordinateY (t_scalar *x)
{
    return template_getFloat (template_findByIdentifier (x->sc_templateIdentifier), sym_y, x->sc_vector);
}

static void scalar_drawSelectRectangle (t_scalar *x, t_glist *glist, int isSelected)
{
    if (isSelected) {
    
        int a, b, c, d;
       
        scalar_behaviorGetRectangle (cast_gobj (x), glist, &a, &b, &c, &d);
        
        a -= SCALAR_SELECT_MARGIN;
        b -= SCALAR_SELECT_MARGIN;
        c += SCALAR_SELECT_MARGIN;
        d += SCALAR_SELECT_MARGIN;
        
        sys_vGui (".x%lx.c create line %d %d %d %d %d %d %d %d %d %d"
                        " -width 0"
                        " -fill #%06x"
                        " -dash {2 4}"
                        " -tags HANDLE%lx\n",
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
        sys_vGui (".x%lx.c delete HANDLE%lx\n", canvas_getView (glist), x);
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
    template_notifyInstance (template, glist, x, sym_click, 2, t);
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
    template_notifyInstance (template, glist, x, sym_displace, 2, t);
}

static void scalar_notifySelected (t_scalar *x, 
    t_glist *glist,
    t_template *template,
    int isSelected)
{
    if (isSelected) { template_notifyInstance (template, glist, x, sym_select, 0, NULL); } 
    else {
        template_notifyInstance (template, glist, x, sym_deselect, 0, NULL);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scalar_redraw (t_scalar *x, t_glist *glist)
{
    if (canvas_isMapped (glist)) {
    //
    interface_guiQueueAddIfNotAlreadyThere ((void *)x, glist, scalar_drawJob);
    //
    }
}

void scalar_redrawByPointer (t_gpointer *gp)
{
    t_glist *glist = gpointer_getView (gp);
    
    t_scalar *scalar = NULL;
        
    if (gpointer_isScalar (gp)) { scalar = gpointer_getScalar (gp); }
    else {
        scalar = gpointer_getScalar (array_getTopParentArray (gp));
    }
    
    scalar_redraw (scalar, glist);
}

void scalar_setVisibility (t_gpointer *gp, int isVisible)
{
    t_glist *glist = gpointer_getView (gp);
    
    if (canvas_isMapped (glist)) {
    //
    t_scalar *scalar = NULL;
        
    if (gpointer_isScalar (gp)) { scalar = gpointer_getScalar (gp); }
    else {
        scalar = gpointer_getScalar (array_getTopParentArray (gp));
    }
    
    scalar_behaviorVisibilityChanged (cast_gobj (scalar), glist, isVisible); 
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int scalar_performClick (t_word *w,
    t_template *template,
    t_scalar *scalar,
    t_array *array,
    t_glist *glist,
    t_float offsetX,
    t_float offsetY,
    int a,
    int b,
    int shift,
    int alt,
    int dbl,
    int clicked)
{
    t_glist *view = template_getInstanceView (template);
    
    if (view) {
    //
    t_float baseX = template_getFloat (template, sym_x, w);
    t_float baseY = template_getFloat (template, sym_y, w);
    t_gobj *y = NULL;
        
    if (clicked) { scalar_notifyClicked (scalar, glist, template, baseX + offsetX, baseY + offsetY); }
            
    for (y = view->gl_graphics; y; y = y->g_next) {
    //
    t_parentwidgetbehavior *behavior = class_getParentWidget (pd_class (y));
    
    if (behavior) { 
        int k = (*behavior->w_fnParentClicked) (y, 
                    glist,
                    w,
                    template,
                    scalar,
                    array,
                    baseX + offsetX,
                    baseY + offsetY,
                    a,
                    b,
                    shift,
                    alt,
                    dbl,
                    clicked);
                    
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

static void scalar_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_scalar *x = cast_scalar (z);
    
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    
    t_template *template = template_findByIdentifier (x->sc_templateIdentifier);
    
    PD_ASSERT (template);
    
    t_glist *view = template_getInstanceView (template);
    t_float baseX = scalar_getCoordinateX (x);
    t_float baseY = scalar_getCoordinateY (x);

    if (!view) {
    
        x1 = canvas_valueToPositionX (glist, baseX);
        y1 = canvas_valueToPositionY (glist, baseY);
        x2 = x1 + SCALAR_WRONG_SIZE;
        y2 = y1 + SCALAR_WRONG_SIZE;
        
    } else {
    
        t_gobj *y = NULL;
        
        x1 += PD_INT_MAX;
        y1 += PD_INT_MAX;
        x2 -= PD_INT_MAX;
        y2 -= PD_INT_MAX;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
        //
        t_parentwidgetbehavior *behavior = class_getParentWidget (pd_class (y));
        
        if (behavior) {
        //
        int e, f, g, h;
        
        (*behavior->w_fnParentGetRectangle) (y,
            glist,
            x->sc_vector,
            template,
            baseX,
            baseY,
            &e,
            &f,
            &g,
            &h);
            
        x1 = PD_MIN (x1, e); y1 = PD_MIN (y1, f); x2 = PD_MAX (x2, g); y2 = PD_MAX (y2, h);
        //
        }
        //
        }
        
        if (x2 < x1 || y2 < y1) { x1 = y1 = x2 = y2 = 0; PD_BUG; }
    }

    *a = x1;
    *b = y1;
    *c = x2;
    *d = y2; 
}

static void scalar_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
    t_scalar *x = cast_scalar (z);
    
    t_template *template = template_findByIdentifier (x->sc_templateIdentifier);
    
    if (!template) { PD_BUG; }
    else {
    //
    if (template_isFloat (template, sym_x)) {
    //
    t_float f = template_getFloat (template, sym_x, x->sc_vector);
    f += canvas_deltaPositionToValueX (glist, deltaX);
    template_setFloat (template, sym_x, x->sc_vector, f);
    //
    }
    
    if (template_isFloat (template, sym_y)) {
    //
    t_float f = template_getFloat (template, sym_y, x->sc_vector);
    f += canvas_deltaPositionToValueY (glist, deltaY);
    template_setFloat (template, sym_y, x->sc_vector, f);
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
    
    t_template *template = template_findByIdentifier (x->sc_templateIdentifier);
    
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
    
    t_template *template = template_findByIdentifier (x->sc_templateIdentifier);
    
    PD_ASSERT (template);
    
    t_glist *view = template_getInstanceView (template);
    t_float baseX = scalar_getCoordinateX (x);
    t_float baseY = scalar_getCoordinateY (x);

    if (!view) {
        
        if (isVisible) {
        
            int a = canvas_valueToPositionX (glist, baseX);
            int b = canvas_valueToPositionY (glist, baseY);
            
            sys_vGui (".x%lx.c create rectangle %d %d %d %d"
                            " -outline #%06x"
                            " -tags SCALAR%lx\n",
                            canvas_getView (glist),
                            a,
                            b,
                            a + SCALAR_WRONG_SIZE,
                            b + SCALAR_WRONG_SIZE,
                            SCALAR_WRONG_COLOR,
                            x);
        } else {
            sys_vGui (".x%lx.c delete SCALAR%lx\n", canvas_getView (glist), x);
        }
        
    } else {
    
        t_gobj *y = NULL;
        
        for (y = view->gl_graphics; y; y = y->g_next) {
        
            t_parentwidgetbehavior *behavior = class_getParentWidget (pd_class (y));
            
            if (behavior) {
                (*behavior->w_fnParentVisibilityChanged) (y, 
                    glist,
                    x->sc_vector,
                    template,
                    baseX,
                    baseY,
                    isVisible);
            }
        }
    }
    
    if (canvas_isObjectSelected (glist, cast_gobj (x))) {
        scalar_drawSelectRectangle (x, glist, 0);
        scalar_drawSelectRectangle (x, glist, 1);
    }
}

static int scalar_behaviorClicked (t_gobj *z,
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
    
    int k = scalar_performClick (x->sc_vector, template_findByIdentifier (x->sc_templateIdentifier),
                x,
                NULL,
                glist,
                0.0,
                0.0,
                a,
                b,
                shift,
                alt,
                dbl,
                clicked);
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scalar_functionSave (t_gobj *z, t_buffer *b)
{
    t_scalar *x = cast_scalar (z);
    t_buffer *t = buffer_new();
   
    canvas_writescalar (x->sc_templateIdentifier, x->sc_vector, t, 0);
    
    buffer_vAppend (b, "ss", sym___hash__X, sym_scalar);
    buffer_serialize (b, t);
    buffer_appendSemicolon (b);
    
    buffer_free (t);
}

static void scalar_functionProperties (t_gobj *z, t_glist *glist)
{
    t_scalar *x = cast_scalar (z);
    
    char cmd[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    
    canvas_deselectAll (glist);
    canvas_selectObject (glist, z);

    err = string_sprintf (cmd, PD_STRING, "::ui_data::show %%s {");
    
    if (!err && !(err = guistub_new (cast_pd (glist), (void *)x, cmd))) {

        int n; char *s = NULL;
        t_buffer *t = glist_writetobinbuf (glist, 0);
        
        buffer_toString (t, &s, &n);
        guistub_add (s);
        guistub_add ("}\n");
        
        buffer_free (t);
        PD_MEMORY_FREE (s);

    } else { PD_BUG; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_word *scalar_getData (t_scalar *x)
{
    return x->sc_vector;
}

t_symbol *scalar_getTemplateIdentifier (t_scalar *x)
{
    return x->sc_templateIdentifier;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar *scalar_new (t_glist *owner, t_symbol *templateIdentifier)
{
    t_scalar *x = NULL;
    
    t_template *template = template_findByIdentifier (templateIdentifier);

    if (!template_existRecursive (template)) { PD_BUG; }
    else {
    //
    {
        size_t extraForArrayOfWords = (template->tp_size - 1) * sizeof (t_word);
        x = (t_scalar *)PD_MEMORY_GET (sizeof (t_scalar) + extraForArrayOfWords); 
        pd_class (x) = scalar_class;
    }
    
    t_gpointer gp = GPOINTER_INIT;
    
    x->sc_templateIdentifier = templateIdentifier;
    
    gpointer_setAsScalarType (&gp, owner, x); word_init (x->sc_vector, template, &gp);  /* ??? */
    //
    }
    
    return x;
}

static void scalar_free (t_scalar *x)
{
    word_free (x->sc_vector, template_findByIdentifier (x->sc_templateIdentifier));
    guistub_destroyWithKey ((void *)x);

    PD_MEMORY_FREE (x);
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
        0,
        CLASS_GRAPHIC,
        A_NULL);
        
    class_setWidgetBehavior (c, &scalar_widgetBehavior);
    class_setSaveFunction (c, scalar_functionSave);
    class_setPropertiesFunction (c, scalar_functionProperties);
    
    scalar_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
