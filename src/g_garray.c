
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
#include "d_dsp.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GARRAY_FLAG_SAVE        (1)
#define GARRAY_FLAG_PLOT        (2 + 4)
#define GARRAY_FLAG_HIDE        (8)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GARRAY_MAXIMUM_CHUNK    1000

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_behaviorGetRectangle         (t_gobj *, t_glist *, int *, int *, int *, int *);
static void garray_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void garray_behaviorSelected             (t_gobj *, t_glist *, int);
static void garray_behaviorActivated            (t_gobj *, t_glist *, int);
static void garray_behaviorDeleted              (t_gobj *, t_glist *);
static void garray_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  garray_behaviorMouse                (t_gobj *, t_glist *, int, int, int, int, int, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

extern t_pd pd_canvasMaker;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *garray_class;                                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

struct _garray {
    t_gobj      x_gobj;
    t_scalar    *x_scalar;
    t_glist     *x_owner;
    t_symbol    *x_unexpandedName;
    t_symbol    *x_name;
    char        x_isUsedInDSP;
    char        x_saveWithParent;
    char        x_hideName;                                 /* Unused but kept for compatibility. */
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior garray_widgetBehavior =             /* Shared. */
    {
        garray_behaviorGetRectangle,
        garray_behaviorDisplaced,
        garray_behaviorSelected,
        garray_behaviorActivated,
        garray_behaviorDeleted,
        garray_behaviorVisibilityChanged,
        garray_behaviorMouse,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GARRAY_AT(n)    *((t_float *)(array_getData (array) + (int)n))

// *((t_float *)(array_getElementAtIndex (array, (int)n)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Create invisible, built-in canvases to supply templates for floats and float-arrays. */

void garray_initialize (void)
{
    static char *floatTemplateFile = 
        "canvas 0 0 458 153 10;\n"
        "#X obj 39 26 struct float float y;\n";

    static char *floatArrayTemplateFile = 
        "canvas 0 0 458 153 10;\n"
        "#X obj 43 31 struct float-array array z float float style float linewidth float color;\n"
        "#X obj 43 70 plot z color linewidth 0 0 1 style;\n";
        
    t_buffer *b = buffer_new();
    
    canvas_setActiveFileNameAndDirectory (sym__floattemplate, sym___dot__);
    buffer_withStringUnzeroed (b, floatTemplateFile, strlen (floatTemplateFile));
    buffer_eval (b, &pd_canvasMaker, 0, NULL);
    pd_vMessage (pd_getBoundX(), sym__pop, "i", 0);
    
    canvas_setActiveFileNameAndDirectory (sym__floatarraytemplate, sym___dot__);
    buffer_withStringUnzeroed (b, floatArrayTemplateFile, strlen (floatArrayTemplateFile));
    buffer_eval (b, &pd_canvasMaker, 0, NULL);
    pd_vMessage (pd_getBoundX(), sym__pop, "i", 0);

    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    
    buffer_free (b);
}

void garray_release (void)
{
}

static void garray_check (t_garray *x)
{
    t_array *array = NULL; t_template *template = NULL;
    
    // struct float-array array z float
        
    template = template_findByIdentifier (scalar_getTemplateIdentifier (x->x_scalar));
    
    PD_ASSERT (template);
    PD_ASSERT (template_fieldIsArrayAndValid (template, sym_z));
    
    array = scalar_getArray (x->x_scalar, sym_z);
    
    // struct float float y
     
    template = template_findByIdentifier (array_getTemplateIdentifier (array));
    
    PD_ASSERT (template);
    PD_ASSERT (template_fieldIsFloat (template, sym_y));
    PD_ASSERT (template_getSize (template) == 1);                   /* Just one field. */
    PD_ASSERT (template_getIndexOfField (template, sym_y) == 0);    /* Just one field. */
    PD_ASSERT (array_getElementSize (array) == 1);                  /* Just one field. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_drawJob (t_gobj *z, t_glist *glist)
{
    t_garray *x = (t_garray *)z;
    
    if (canvas_isMapped (x->x_owner) && gobj_isVisible (z, glist)) {
    //
    garray_behaviorVisibilityChanged (z, x->x_owner, 0); 
    garray_behaviorVisibilityChanged (z, x->x_owner, 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_updateGraphSize (t_garray *x, int size, int style)
{
    t_glist *glist = x->x_owner;
        
    if (garray_isSingle (glist)) {
    //
    if (!glist->gl_isLoading) {
    //
    pd_vMessage (cast_pd (glist), sym_bounds, "ffff",
        0.0,
        glist->gl_valueTop,
        (double)((style == PLOT_POINTS || size == 1) ? size : size - 1),
        glist->gl_valueBottom);
    //
    }
    //
    }
}

static void garray_updateGraphName (t_garray *x)
{
    canvas_redrawGraphOnParent (x->x_owner); canvas_setName (x->x_owner, x->x_name);
}

void garray_resizeWithInteger (t_garray *x, int n)
{
    int style = scalar_getFloat (x->x_scalar, sym_style);
    t_array *array = garray_getArray (x);
    
    PD_ASSERT (n > 0);
    
    garray_updateGraphSize (x, PD_MAX (1, n), style);
    array_resizeAndRedraw (array, x->x_owner, PD_MAX (1, n));
    
    if (x->x_isUsedInDSP) { dsp_update(); }
}

void garray_saveContentsToBuffer (t_garray *x, t_buffer *b)
{
    if (x->x_saveWithParent) {
    //
    int n = 0;
    t_array *array = garray_getArray (x);
    
    while (n < array_getSize (array)) {
    //
    int i, chunk = array_getSize (array) - n;
    
    if (chunk > GARRAY_MAXIMUM_CHUNK) { chunk = GARRAY_MAXIMUM_CHUNK; }
    
    buffer_vAppend (b, "si", sym___hash__A, n);
    for (i = 0; i < chunk; i++) { buffer_vAppend (b, "f", GARRAY_AT (n + i)); }
    buffer_appendSemicolon (b);
        
    n += chunk;
    //
    }
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_setWithSumOfFourierComponents (t_garray *x,
    int numberOfPoints,
    int numberOfSineWaves,
    t_float *valuesOfSineWaves,
    int isSine)
{
    double phase, phaseIncrement;
    int i;
    t_array *array = garray_getArray (x);
    
    numberOfPoints = (numberOfPoints <= 0) ? 512 : numberOfPoints;
    numberOfPoints = PD_MIN (numberOfPoints, 1 << 30);
    
    if (!PD_IS_POWER_2 (numberOfPoints)) { numberOfPoints = (int)PD_NEXT_POWER_2 (numberOfPoints); }
    
    garray_resizeWithInteger (x, numberOfPoints + 3);
    
    phaseIncrement = 2.0 * (double)PD_PI / numberOfPoints;
    
    for (i = 0, phase = -phaseIncrement; i < array_getSize (array); i++, phase += phaseIncrement) {
    //
    int j;
    double fj;
    double sum = 0.0;
    
    if (isSine) {
        for (j = 0, fj = phase; j < numberOfSineWaves; j++, fj += phase) { 
            sum += (double)valuesOfSineWaves[j] * sin (fj);
        }
    } else {
        for (j = 0, fj = 0; j < numberOfSineWaves; j++, fj += phase) {
            sum += (double)valuesOfSineWaves[j] * cos (fj);
        }
    }
    
    GARRAY_AT (i) = sum;
    //
    }
    
    garray_redraw (x);
}

static void garray_setWithSineWaves (t_garray *x, t_symbol *s, int argc, t_atom *argv, int isSine)
{    
    if (argc > 1) {
    //
    t_float *t = NULL;
    int i;
    int numberOfPoints = atom_getFloatAtIndex (0, argc, argv);
    
    argv++, argc--;
    
    t = (t_float *)PD_MEMORY_GET (sizeof (t_float) * argc);
    
    for (i = 0; i < argc; i++) { t[i] = atom_getFloatAtIndex (i, argc, argv); }
    
    garray_setWithSumOfFourierComponents (x, numberOfPoints, argc, t, isSine);
    
    PD_MEMORY_FREE (t);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_array *garray_getArray (t_garray *x)
{
    #if PD_WITH_DEBUG
    
    garray_check (x);
    
    #endif
    
    return scalar_getArray (x->x_scalar, sym_z);
}

int garray_getData (t_garray *x, int *size, t_word **w)
{
    t_array *array = garray_getArray (x);
    
    *size = array_getSize (array); *w = array_getData (array);
    
    return 1; 
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_symbol *garray_getName (t_garray *x)
{
    return x->x_name;
}

t_glist *garray_getView (t_garray *x)
{
    return x->x_owner;
}

t_scalar *garray_getScalar (t_garray *x)
{
    return x->x_scalar;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void garray_setAsUsedInDSP (t_garray *x)
{
    x->x_isUsedInDSP = 1;
}

void garray_setSaveWithParent (t_garray *x, int savedWithParent)
{
    x->x_saveWithParent = savedWithParent;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void garray_redraw (t_garray *x)
{
    if (canvas_isMapped (x->x_owner)) {
    //
    interface_guiQueueAddIfNotAlreadyThere ((void *)x, x->x_owner, garray_drawJob);
    //
    }
}

int garray_isSingle (t_glist *glist)       /* Legacy patches might contain several arrays. */
{
    if (glist->gl_graphics && !glist->gl_graphics->g_next) {
        if (pd_class (glist->gl_graphics) == garray_class) { return 1; }
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_list (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { 
    //
    int i, j = atom_getFloat (argv);
    t_array *array = garray_getArray (x);
    
    argc--;
    argv++;

    if (j < 0) { argc += j; argv -= j; j = 0; }
    
    if (j + argc > array_getSize (array)) { argc = array_getSize (array) - j; }
    
    if (argc > 0) {
    //
    for (i = 0; i < argc; i++) { GARRAY_AT (i + j) = atom_getFloat (argv + i); }
    
    garray_redraw (x);
    //
    }
    //
    }
}

static void garray_constant (t_garray *x, t_float f)
{
    int i;
    t_array *array = garray_getArray (x);

    for (i = 0; i < array_getSize (array); i++) { GARRAY_AT (i) = f; }
    
    garray_redraw (x);
}

static void garray_normalize (t_garray *x, t_float f)
{
    int i;
    double maximum  = 0.0;
    t_array *array = garray_getArray (x);

    if (f <= 0.0) { f = 1.0; }

    for (i = 0; i < array_getSize (array); i++) {
        double t = GARRAY_AT (i);
        if (PD_ABS (t) > maximum) { maximum = PD_ABS (t); }
    }
    
    if (maximum > 0.0) {
        double k = f / maximum;
        for (i = 0; i < array_getSize (array); i++) { GARRAY_AT (i) *= k; }
    }
    
    garray_redraw (x);
}

static void garray_sinesum (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{    
    garray_setWithSineWaves (x, s, argc, argv, 1);
}

static void garray_cosinesum (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{    
    garray_setWithSineWaves (x, s, argc, argv, 0);
}

static void garray_rename (t_garray *x, t_symbol *s)
{
    pd_unbind (cast_pd (x), x->x_name);
    x->x_unexpandedName = s;
    x->x_name = canvas_expandDollar (x->x_owner, s);
    pd_bind (cast_pd (x), x->x_name);
    garray_redraw (x);
    garray_updateGraphName (x);
}

static void garray_read (t_garray *x, t_symbol *name)
{
    t_error err = PD_ERROR_NONE;
    char *p = NULL;
    char t[PD_STRING] = { 0 }; 
    int f = canvas_openFile (canvas_getView (x->x_owner), name->s_name, "", t, &p, PD_STRING);
    
    if (!(err |= (f < 0))) {
    //
    FILE *file = fdopen (f, "r");
    
    if (!(err |= (file == NULL))) {
    //
    int i;
    t_array *array = garray_getArray (x);

    for (i = 0; i < array_getSize (array); i++) {
        double v = 0.0; if (!fscanf (file, "%lf", &v)) { break; } else { GARRAY_AT (i) = v; }
    }
    
    while (i < array_getSize (array)) { GARRAY_AT (i) = 0.0; i++; }
    
    fclose (file);
    
    garray_redraw (x);
    //
    }
    //
    }
    
    if (err) { error_canNotOpen (name); }
}

static void garray_write (t_garray *x, t_symbol *name)
{
    char t[PD_STRING] = { 0 };
    FILE *file = NULL;
        
    t_error err = canvas_makeFilePath (canvas_getView (x->x_owner), name->s_name, t, PD_STRING);
    
    if (err || !(file = file_openWrite (t))) { error_canNotCreate (name); }
    else {
    //
    int i;
    t_array *array = garray_getArray (x);
    
    for (i = 0; i < array_getSize (array); i++) {
        if (fprintf (file, "%g\n", GARRAY_AT (i)) < 1) { PD_BUG; break; }
    }
    
    fclose (file);
    //
    }
}

static void garray_resize (t_garray *x, t_float f)
{
    garray_resizeWithInteger (x, PD_MAX (1, (int)f));
}

static void garray_bounds (t_garray *x, t_float a, t_float b, t_float c, t_float d)
{
    pd_vMessage (cast_pd (x->x_owner), sym_bounds, "ffff", a, b, c, d);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_behaviorGetRectangle (t_gobj *z, t_glist *glist, int *a, int *b, int *c, int *d)
{
    t_garray *x = (t_garray *)z;
    
    gobj_getRectangle (cast_gobj (x->x_scalar), glist, a, b, c, d);
}

static void garray_behaviorDisplaced (t_gobj *z, t_glist *glist, int deltaX, int deltaY)
{
}

static void garray_behaviorSelected (t_gobj *z, t_glist *glist, int isSelected)
{
}

static void garray_behaviorActivated (t_gobj *z, t_glist *glist, int isActivated)
{
}

static void garray_behaviorDeleted (t_gobj *z, t_glist *glist)
{
}

static void garray_behaviorVisibilityChanged (t_gobj *z, t_glist *glist, int isVisible)
{
    t_garray *x = (t_garray *)z;
    
    gobj_visibilityChanged (cast_gobj (x->x_scalar), glist, isVisible);
}

static int garray_behaviorMouse (t_gobj *z,
    t_glist *glist,
    int a,
    int b,
    int shift,
    int ctrl,
    int alt,
    int dbl,
    int clicked)
{
    t_garray *x = (t_garray *)z;

    if (clicked) { gobj_mouse (cast_gobj (x->x_scalar), glist, a, b, shift, ctrl, alt, dbl, clicked); }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_functionSave (t_gobj *z, t_buffer *b)
{
    t_garray *x = (t_garray *)z;
    int style = scalar_getFloat (x->x_scalar, sym_style);    
    int flags = x->x_saveWithParent + (2 * style) + (8 * x->x_hideName);
    t_array *array = garray_getArray (x);
        
    buffer_vAppend (b, "sssisi;",
        sym___hash__X,
        sym_array,
        x->x_unexpandedName,
        array_getSize (array),
        &s_float,
        flags);
        
    garray_saveContentsToBuffer (x, b);
}

void garray_functionProperties (t_garray *x)
{
    char t[PD_STRING] = { 0 };
    t_error err = PD_ERROR_NONE;
    int style = scalar_getFloat (x->x_scalar, sym_style);
    int flags = x->x_saveWithParent + (2 * style);
    t_array *array = garray_getArray (x);
    
    err |= string_sprintf (t, PD_STRING,
                "::ui_array::show %%s %s %d %d\n",
                dollar_toHash (x->x_unexpandedName)->s_name, 
                array_getSize (array),
                flags);
    
    PD_ASSERT (!err);
    
    guistub_new (cast_pd (x), (void *)x, t);
}

void garray_fromDialog (t_garray *x, t_symbol *name, t_float size, t_float flags)
{
    t_symbol *newName    = dollar_fromHash (name);
    int newSize          = PD_MAX (1.0, size);
    int save             = (((int)flags & 1) != 0);
    int newStyle         = (((int)flags & 6) >> 1);
    int oldStyle         = (int)scalar_getFloat (x->x_scalar, sym_style);

    PD_ASSERT (newSize > 0);
    
    t_array *array = garray_getArray (x);
    
    if (newName != x->x_unexpandedName) {
    //
    x->x_unexpandedName = newName;
    pd_unbind (cast_pd (x), x->x_name);
    x->x_name = canvas_expandDollar (x->x_owner, newName);
    pd_bind (cast_pd (x), x->x_name);

    garray_updateGraphName (x);
    dsp_update();
    //
    }

    if (newSize == 1) { newStyle = PLOT_POINTS; }
    if (newStyle != oldStyle) { scalar_setFloat (x->x_scalar, sym_style, (t_float)newStyle); }
    if (newSize != array_getSize (array)) { garray_resizeWithInteger (x, newSize); }
        
    garray_updateGraphSize (x, newSize, newStyle); 
    garray_setSaveWithParent (x, save);
    garray_redraw (x);
    canvas_dirty (x->x_owner, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_garray *garray_makeObjectWithScalar (t_glist *glist,
    t_symbol *name,
    t_symbol *templateIdentifier,
    int save, 
    int hide)
{
    t_garray *x = (t_garray *)pd_new (garray_class);
    
    x->x_scalar         = scalar_new (glist, templateIdentifier);
    x->x_owner          = glist;
    x->x_unexpandedName = name;
    x->x_name           = canvas_expandDollar (glist, name);
    x->x_isUsedInDSP    = 0;
    x->x_saveWithParent = save;
    x->x_hideName       = hide;
    
    pd_bind (cast_pd (x), x->x_name);
    
    canvas_addObject (glist, cast_gobj (x));
    
    return x;
}

t_garray *garray_makeObject (t_glist *glist, t_symbol *name, t_float size, t_float flags)
{
    t_template *template = template_findByIdentifier (sym___TEMPLATE__float__dash__array);
    
    t_garray *x = NULL;
    
    PD_ASSERT (template);
    
    if (template_fieldIsArrayAndValid (template, sym_z)) {
    //
    int save  = (((int)flags & GARRAY_FLAG_SAVE) != 0);
    int hide  = (((int)flags & GARRAY_FLAG_HIDE) != 0);
    int style = PD_CLAMP ((((int)flags & GARRAY_FLAG_PLOT) >> 1), PLOT_POLYGONS, PLOT_CURVES);
    int n     = (int)PD_MAX (1.0, size);
    
    x = garray_makeObjectWithScalar (glist, name, sym___TEMPLATE__float__dash__array, save, hide);

    array_resize (scalar_getArray (x->x_scalar, sym_z), n);

    scalar_setFloat (x->x_scalar, sym_style, style);
    scalar_setFloat (x->x_scalar, sym_linewidth, 1);

    pd_setBoundA (cast_pd (x));
    
    garray_redraw (x);
    garray_updateGraphSize (x, n, style);
    garray_updateGraphName (x);
    dsp_update();
    //
    }
    
    PD_ASSERT (x != NULL);
    
    return x;
}

static void garray_free (t_garray *x)
{
    t_pd *t = NULL;
    
    interface_guiQueueRemove (cast_gobj (x));
    guistub_destroyWithKey ((void *)x);
    pd_setBoundA (NULL);
    pd_unbind (cast_pd (x), x->x_name);
    pd_free (cast_pd (x->x_scalar));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void garray_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array,
        NULL,
        (t_method)garray_free,
        sizeof (t_garray),
        CLASS_GRAPHIC,
        A_NULL);
        
    class_addList (c, garray_list);
    
    class_addMethod (c, (t_method)garray_constant,      sym_constant,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_normalize,     sym_normalize,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_sinesum,       sym_sinesum,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)garray_cosinesum,     sym_cosinesum,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)garray_rename,        sym_rename,         A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_read,          sym_read,           A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_write,         sym_write,          A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_resize,        sym_resize,         A_FLOAT, A_NULL);

    class_addMethod (c, (t_method)garray_bounds,
        sym_bounds,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)garray_fromDialog,
        sym__arraydialog,
        A_SYMBOL,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)garray_constant, sym_const, A_DEFFLOAT, A_NULL);
    
    #endif
    
    class_setWidgetBehavior (c, &garray_widgetBehavior);
    class_setSaveFunction (c, garray_functionSave);
    
    garray_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
