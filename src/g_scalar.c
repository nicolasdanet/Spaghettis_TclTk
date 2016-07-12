
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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SCALAR_SELECT_MARGIN            5

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_class *scalar_class;                  /* Shared. */

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

static t_widgetbehavior scalar_widgetBehavior =
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

static int scalar_isTemplateExistRecursive (t_template *template)
{
    if (!template) { return 0; }
    else {
    //
    int i, size = template->tp_size;
    t_dataslot *v = template->tp_vector;
    
    for (i = 0; i < size; i++, v++) {
    //
    if (v->ds_type == DATA_ARRAY) {
    //
    t_template *elementTemplate = template_findbyname (v->ds_templateIdentifier);
    if (!elementTemplate || !scalar_isTemplateExistRecursive (elementTemplate)) {
        return 0;
    }
    //
    }
    //
    }
    //
    }
    
    return 1;
}

static t_float scalar_getCoordinateX (t_scalar *x)
{
    return template_getfloat (template_findbyname (x->sc_templateIdentifier), sym_x, x->sc_vector);
}

static t_float scalar_getCoordinateY (t_scalar *x)
{
    return template_getfloat (template_findbyname (x->sc_templateIdentifier), sym_y, x->sc_vector);
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
    t_atom t[3];
    SET_FLOAT (t + 1, positionX);
    SET_FLOAT (t + 2, positionY);
    template_notifyforscalar (template, glist, x, sym_click, 3, t);
}
    
static void scalar_notifyDisplaced (t_scalar *x, 
    t_glist *glist,
    t_template *template,
    t_float deltaX,
    t_float deltaY)
{
    t_atom t[3];
    SET_FLOAT (t + 1, deltaX);
    SET_FLOAT (t + 2, deltaY);
    template_notifyforscalar (template, glist, x, sym_displace, 3, t);
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
    t_glist *view = template_findcanvas (template);
    t_float baseX = template_getfloat (template, sym_x, w);
    t_float baseY = template_getfloat (template, sym_y, w);
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
    
    t_template *template = template_findbyname (x->sc_templateIdentifier);
    
    PD_ASSERT (template);
    
    t_glist *view = template_findcanvas (template);
    t_float baseX = scalar_getCoordinateX (x);
    t_float baseY = scalar_getCoordinateY (x);

    if (!view) {
    
        PD_ASSERT (!template);
        
        x1 = x2 = canvas_valueToPositionX (glist, baseX);
        y1 = y2 = canvas_valueToPositionY (glist, baseY);
        
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
    
    t_template *template = template_findbyname (x->sc_templateIdentifier);
    
    if (!template) { PD_BUG; }
    else {
    //
    int onsetX, onsetY, typeX, typeY;
    t_symbol *s = NULL;
        
    int hasFieldX = template_find_field (template, sym_x, &onsetX, &typeX, &s);
    int hasFieldY = template_find_field (template, sym_y, &onsetY, &typeY, &s);
        
    if (hasFieldX && (typeX != DATA_FLOAT)) { hasFieldX = 0; }
    if (hasFieldY && (typeY != DATA_FLOAT)) { hasFieldY = 0; }
        
    if (hasFieldX) {
        *(t_float *)(((char *)(x->sc_vector)) + onsetX) += canvas_deltaPositionToValueX (glist, deltaX);
    }
    
    if (hasFieldY) {
        *(t_float *)(((char *)(x->sc_vector)) + onsetY) += canvas_deltaPositionToValueY (glist, deltaY);
    }
    
    scalar_notifyDisplaced (x, glist, template, (t_float)deltaX, (t_float)deltaY);
    scalar_redraw (x, glist);
    //
    }
}

static void scalar_behaviorSelected (t_gobj *z, t_glist *owner, int state)
{
    t_scalar *x = cast_scalar (z);
    t_template *tmpl;
    t_symbol *templatesym = x->sc_templateIdentifier;
    t_atom at;
    t_gpointer gp = GPOINTER_INIT;
    gpointer_setAsScalarType(&gp, owner, x);
    SET_POINTER(&at, &gp);
    if (tmpl = template_findbyname(templatesym))
        template_notify(tmpl, (state ? sym_select : sym_deselect),
            1, &at);
    gpointer_unset(&gp);
    scalar_drawSelectRectangle(x, owner, state);
}

static void scalar_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
}

static void scalar_behaviorDeleted (t_gobj *z, t_glist *glist)
{
}

static void scalar_behaviorVisibilityChanged(t_gobj *z, t_glist *owner, int vis)
{
    t_scalar *x = cast_scalar (z);
    t_template *template = template_findbyname(x->sc_templateIdentifier);
    t_glist *templatecanvas = template_findcanvas(template);
    t_gobj *y;
    t_float basex = scalar_getCoordinateX (x);
    t_float basey = scalar_getCoordinateY (x);
        /* if we don't know how to draw it, make a small rectangle */
    if (!templatecanvas)
    {
        if (vis)
        {
            int x1 = canvas_valueToPositionX(owner, basex);
            int y1 = canvas_valueToPositionY(owner, basey);
            sys_vGui(".x%lx.c create rectangle %d %d %d %d -tags scalar%lx\n",
                canvas_getView(owner), x1-1, y1-1, x1+1, y1+1, x);
        }
        else sys_vGui(".x%lx.c delete scalar%lx\n", canvas_getView(owner), x);
        return;
    }

    for (y = templatecanvas->gl_graphics; y; y = y->g_next)
    {
        t_parentwidgetbehavior *wb = class_getParentWidget (pd_class (&y->g_pd));
        if (!wb) continue;
        (*wb->w_fnParentVisibilityChanged)(y, owner, x->sc_vector, template, basex, basey, vis);
    }
    if (canvas_isObjectSelected(owner, &x->sc_g))
    {
        scalar_drawSelectRectangle(x, owner, 0);
        scalar_drawSelectRectangle(x, owner, 1);
    }
    interface_guiQueueRemove(x);
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
    
    int k = scalar_performClick (x->sc_vector, template_findbyname (x->sc_templateIdentifier),
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

static void scalar_functionSave(t_gobj *z, t_buffer *b)
{
    t_scalar *x = cast_scalar (z);
    t_buffer *b2 = buffer_new();
    t_atom a, *argv;
    int i, argc;
    canvas_writescalar(x->sc_templateIdentifier, x->sc_vector, b2, 0);
    buffer_vAppend(b, "ss", sym___hash__X, sym_scalar);
    buffer_serialize(b, b2);
    buffer_appendSemicolon(b);
    buffer_free(b2);
}

static void scalar_functionProperties(t_gobj *z, struct _glist *owner)
{
    t_scalar *x = cast_scalar (z);
    char *buf, buf2[80];
    int bufsize;
    t_buffer *b;
    canvas_deselectAll(owner);
    canvas_selectObject(owner, z);
    b = glist_writetobinbuf(owner, 0);
    buffer_toStringUnzeroed(b, &buf, &bufsize);
    buffer_free(b);
    buf = PD_MEMORY_RESIZE(buf, bufsize, bufsize+1);
    buf[bufsize] = 0;
    sprintf(buf2, "::ui_data::show %%s {");
    guistub_new((t_pd *)owner, x, buf2);
    sys_gui(buf);
    sys_gui("}\n");
    PD_MEMORY_FREE(buf);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_scalar *scalar_new (t_glist *owner, t_symbol *templateIdentifier)
{
    t_scalar *x = NULL;
    
    t_template *template = template_findbyname (templateIdentifier);

    if (!scalar_isTemplateExistRecursive (template)) { PD_BUG; }
    else {
    //
    {
        size_t extraForArrayOfWords = (template->tp_size - 1) * sizeof (t_word);
        x = (t_scalar *)PD_MEMORY_GET (sizeof (t_scalar) + extraForArrayOfWords); 
        pd_class (x) = scalar_class;
    }
    
    t_gpointer gp = GPOINTER_INIT;
    
    x->sc_templateIdentifier = templateIdentifier;
    
    /* Note that ownership of the gpointer is grabbed in later call. */
    /* Thus it doesn't need to be released at the end of the function. */
    
    gpointer_setAsScalarType (&gp, owner, x); word_init (x->sc_vector, template, &gp);
    //
    }
    
    return x;
}

static void scalar_free (t_scalar *x)
{
    word_free (x->sc_vector, template_findbyname (x->sc_templateIdentifier));
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
