
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
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

static void garray_behaviorGetRectangle         (t_gobj *, t_glist *, t_rectangle *);
static void garray_behaviorDisplaced            (t_gobj *, t_glist *, int, int);
static void garray_behaviorSelected             (t_gobj *, t_glist *, int);
static void garray_behaviorActivated            (t_gobj *, t_glist *, int);
static void garray_behaviorDeleted              (t_gobj *, t_glist *);
static void garray_behaviorVisibilityChanged    (t_gobj *, t_glist *, int);
static int  garray_behaviorMouse                (t_gobj *, t_glist *, t_mouse *);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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

/* Assume that a pointer to a union object points to each of its members. */

/* < http://c0x.coding-guidelines.com/6.7.2.1.html > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GARRAY_AT(n)    *((t_float *)(array_getElements (array) + (int)n))

// *((t_float *)(array_getElementAtIndex (array, (int)n)))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Create invisible, built-in canvases to supply templates for floats and float-arrays. */

void garray_initialize (void)
{
    static char *floatTemplate = 
        "#N canvas 0 22 450 300 12;\n"
        "#X obj 39 26 struct float float y;\n";

    static char *floatArrayTemplate = 
        "#N canvas 0 22 950 300 12;\n"
        "#X obj 43 31 struct float-array array z float float style float linewidth float color;\n"
        "#X f 74;"
        "#X obj 43 70 plot z color linewidth 0 0 1 style;\n";
    
    instance_loadInvisible (sym__floattemplate, sym___dot__, floatTemplate);
    instance_loadInvisible (sym__floatarraytemplate, sym___dot__, floatArrayTemplate);
}

#if PD_WITH_DEBUG
    
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

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_drawJob (t_gobj *z, t_glist *glist)
{
    t_garray *x = (t_garray *)z;
    
    if (glist_isOnScreen (x->x_owner) && gobj_isVisible (z, glist)) {
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
        
    if (glist_isArray (glist))  {
    //
    if (!glist_isLoading (glist)) {
    //
    t_float a = (t_float)0.0;
    t_float b = bounds_getTop (glist_getBounds (glist));
    t_float c = (t_float)((style == PLOT_POINTS || size == 1) ? size : size - 1);
    t_float d = bounds_getBottom (glist_getBounds (glist));
    
    t_bounds bounds; t_error err = bounds_set (&bounds, a, b, c, d);
    
    if (!err) { glist_setBounds (glist, &bounds); glist_updateGraphOnParent (glist); }
    else {
        PD_BUG;
    }
    //
    }
    //
    }
}

static void garray_updateGraphName (t_garray *x)
{
    glist_updateGraphOnParent (x->x_owner); glist_setName (x->x_owner, x->x_name);
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
    
    phaseIncrement = PD_TWO_PI / numberOfPoints;
    
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
    
    GARRAY_AT (i) = (t_float)sum;
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
    
    static int once = 0; if (!once) { garray_check (x); once = 1; }
    
    #endif
    
    return scalar_getArray (x->x_scalar, sym_z);
}

int garray_getSize (t_garray *x)
{
    return array_getSize (garray_getArray (x));
}

int garray_getData (t_garray *x, int *size, t_word **w)
{
    t_array *array = garray_getArray (x);
    
    *size = array_getSize (array); *w = array_getElements (array);
    
    return 1; 
}

void garray_setDataAtIndex (t_garray *x, int i, t_float f)
{
    t_array *array = garray_getArray (x);
    
    int size = array_getSize (array);
    int n = PD_CLAMP (i, 0, size - 1);
    
    GARRAY_AT (n) = f;
}

t_float garray_getDataAtIndex (t_garray *x, int i)
{
    t_array *array = garray_getArray (x);
    
    int size = array_getSize (array);
    int n = PD_CLAMP (i, 0, size - 1);
    t_float f = GARRAY_AT (n);
    
    return f;
}

void garray_setDataFromIndex (t_garray *x, int i, t_float f)
{
    t_array *array = garray_getArray (x);
    
    int n, size = array_getSize (array);
    
    for (n = PD_CLAMP (i, 0, size - 1); n < size; n++) { GARRAY_AT (n) = f; }
}

t_float garray_getAmplitude (t_garray *x)
{
    t_array *array = garray_getArray (x);
    
    int i, size = array_getSize (array);
    
    t_float f = (t_float)0.0;
    
    for (i = 0; i < size; i++) { t_float t = GARRAY_AT (i); f = PD_MAX (f, PD_ABS (t)); }

    return f;
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
    defer_addJob ((void *)x, x->x_owner, garray_drawJob);
}

int garray_isSingle (t_glist *glist)       /* Note that legacy patches might contain several arrays. */
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
    garray_setDataFromIndex (x, 0, f);
    garray_redraw (x);
}

static void garray_normalize (t_garray *x, t_float f)
{
    int i;
    double maximum  = 0.0;
    t_array *array = garray_getArray (x);

    if (f <= 0.0) { f = (t_float)1.0; }

    for (i = 0; i < array_getSize (array); i++) {
        double t = GARRAY_AT (i);
        if (PD_ABS (t) > maximum) { maximum = PD_ABS (t); }
    }
    
    if (maximum > 0.0) {
        double k = f / maximum;
        for (i = 0; i < array_getSize (array); i++) { GARRAY_AT (i) *= (t_float)k; }
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
    x->x_name = dollar_expandDollarSymbolByEnvironment (s, x->x_owner);
    pd_bind (cast_pd (x), x->x_name);
    garray_redraw (x);
    garray_updateGraphName (x);
}

static void garray_read (t_garray *x, t_symbol *name)
{
    t_error err = PD_ERROR_NONE;
    
    t_fileproperties p;
    
    int f = glist_fileOpen (glist_getView (x->x_owner), name->s_name, "", &p);
    
    if (!(err |= (f < 0))) {
    //
    FILE *file = fdopen (f, "r");
    
    if (!(err |= (file == NULL))) {

        int i;
        t_array *array = garray_getArray (x);

        for (i = 0; i < array_getSize (array); i++) {
        //
        double v = 0.0;
        if (!fscanf (file, "%lf", &v)) { break; }
        else {
            GARRAY_AT (i) = (t_float)v; 
        }
        //
        }
        
        while (i < array_getSize (array)) { GARRAY_AT (i) = (t_float)0.0; i++; }
        
        fclose (file);      /* < http://stackoverflow.com/a/13691168 > */
        
        garray_redraw (x);
    }
    //
    }
    
    if (err) { error_canNotOpen (name); }
}

static void garray_write (t_garray *x, t_symbol *name)
{
    char t[PD_STRING] = { 0 };
    FILE *file = NULL;
    char *directory = environment_getDirectoryAsString (glist_getEnvironment (glist_getView (x->x_owner)));
    
    t_error err = path_withDirectoryAndName (t, PD_STRING, directory, name->s_name);
    
    if (err || !(file = file_openWrite (t))) { error_canNotCreate (name); }
    else {

        int i;
        t_array *array = garray_getArray (x);
        
        for (i = 0; i < array_getSize (array); i++) {
            if (fprintf (file, "%g\n", GARRAY_AT (i)) < 1) {
                PD_BUG; break;
            }
        }
        
        fclose (file);
    }
}

static void garray_resize (t_garray *x, t_float f)
{
    garray_resizeWithInteger (x, PD_MAX (1, (int)f));
}

static void garray_bounds (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    pd_message (cast_pd (x->x_owner), s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_behaviorGetRectangle (t_gobj *z, t_glist *glist, t_rectangle *r)
{
    t_garray *x = (t_garray *)z;
    
    gobj_getRectangle (cast_gobj (x->x_scalar), glist, r);
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
    
    if (!isVisible) { defer_removeJob ((void *)z); }
}

static int garray_behaviorMouse (t_gobj *z, t_glist *glist, t_mouse *m)
{
    t_garray *x = (t_garray *)z;

    if (m->m_clicked) { gobj_mouse (cast_gobj (x->x_scalar), glist, m); }
    
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
    t_array *array = garray_getArray (x);
    t_bounds *bounds = glist_getBounds (x->x_owner);
    
    PD_ASSERT (glist_isArray (x->x_owner));
    
    err |= string_sprintf (t, PD_STRING,
                "::ui_array::show %%s %s %d %g %g %d %d\n",
                utils_dollarToHash (x->x_unexpandedName)->s_name,
                array_getSize (array),
                bounds_getTop (bounds),
                bounds_getBottom (bounds),
                x->x_saveWithParent,
                PD_CLAMP (style, PLOT_POLYGONS, PLOT_CURVES));
    
    PD_ASSERT (!err);
    
    stub_new (cast_pd (x), (void *)x, t);
}

void garray_fromDialog (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 4) {
    //
    t_symbol *name = utils_hashToDollar (atom_getSymbol (argv + 0));
    int size       = (int)atom_getFloat (argv + 1);
    int save       = (int)atom_getFloat (argv + 2);
    int style      = (int)atom_getFloat (argv + 3);

    PD_ASSERT (size > 0);
    
    t_array *array = garray_getArray (x);
    
    if (name != x->x_unexpandedName) {
    //
    x->x_unexpandedName = name;
    pd_unbind (cast_pd (x), x->x_name);
    x->x_name = dollar_expandDollarSymbolByEnvironment (name, x->x_owner);
    pd_bind (cast_pd (x), x->x_name);

    garray_updateGraphName (x);
    dsp_update();
    //
    }

    if (size == 1) { style = PLOT_POINTS; }
    
    scalar_setFloat (x->x_scalar, sym_style, (t_float)style);
    
    if (size != array_getSize (array)) { garray_resizeWithInteger (x, size); }
        
    garray_updateGraphSize (x, size, style); 
    garray_setSaveWithParent (x, save);
    garray_redraw (x);
    glist_setDirty (x->x_owner, 1);
    //
    }
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
    x->x_name           = dollar_expandDollarSymbolByEnvironment (name, glist);
    x->x_isUsedInDSP    = 0;
    x->x_saveWithParent = save;
    x->x_hideName       = hide;
    
    pd_bind (cast_pd (x), x->x_name);
    
    glist_objectAdd (glist, cast_gobj (x));
    
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

    instance_setBoundA (cast_pd (x));
    
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
    defer_removeJob ((void *)x);
    stub_destroyWithKey ((void *)x);
    instance_setBoundA (NULL);
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
        
    class_addList (c, (t_method)garray_list);
    
    class_addMethod (c, (t_method)garray_constant,      sym_constant,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_normalize,     sym_normalize,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_sinesum,       sym_sinesum,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)garray_cosinesum,     sym_cosinesum,      A_GIMME, A_NULL);
    class_addMethod (c, (t_method)garray_rename,        sym_rename,         A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_read,          sym_read,           A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_write,         sym_write,          A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_resize,        sym_resize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_bounds,        sym_bounds,         A_GIMME, A_NULL);
    class_addMethod (c, (t_method)garray_fromDialog,    sym__arraydialog,   A_GIMME, A_NULL);
        
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)garray_constant, sym_const, A_DEFFLOAT, A_NULL);
    
    #endif
    
    class_setWidgetBehavior (c, &garray_widgetBehavior);
    class_setSaveFunction (c, garray_functionSave);
    
    garray_class = c;
}

void garray_destroy (void)
{
    CLASS_FREE (garray_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
