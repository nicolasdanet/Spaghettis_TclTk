
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

#define GARRAY_FLAG_SAVE        (1)
#define GARRAY_FLAG_PLOT        (2 + 4)
#define GARRAY_FLAG_HIDE        (8)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_getrect      (t_gobj *, t_glist *, int *, int *, int *, int *);
static void garray_displace     (t_gobj *, t_glist *, int, int);
static void garray_select       (t_gobj *, t_glist *, int);
static void garray_activate     (t_gobj *, t_glist *, int);
static void garray_delete       (t_gobj *, t_glist *);
static void garray_vis          (t_gobj *, t_glist *, int);
static int  garray_click        (t_gobj *, t_glist *, int, int, int, int, int, int, int);

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
    char        x_hideName;
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_widgetbehavior garray_widgetBehavior =             /* Shared. */
    {
        garray_getrect,
        garray_displace,
        garray_select,
        garray_activate,
        garray_delete,
        garray_vis,
        garray_click,
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define GARRAY_FETCH    int yOnset; \
                        int elementSize; \
                        t_array *array = garray_getArrayCheckedFloat (x, &yOnset, &elementSize); \
                        PD_ASSERT (array != NULL);
                        
#define GARRAY_AT(n)    *((t_float *)(array->a_vector + (elementSize * (n))) + yOnset)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_array *garray_getArrayCheckedFloat (t_garray *, int *, int *);

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
    pd_vMessage (s__X.s_thing, sym__pop, "i", 0);
    
    canvas_setActiveFileNameAndDirectory (sym__floatarraytemplate, sym___dot__);
    buffer_withStringUnzeroed (b, floatArrayTemplateFile, strlen (floatArrayTemplateFile));
    buffer_eval (b, &pd_canvasMaker, 0, NULL);
    pd_vMessage (s__X.s_thing, sym__pop, "i", 0);

    canvas_setActiveFileNameAndDirectory (&s_, &s_);
    
    buffer_free (b);  
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_drawJob (t_gobj *z, t_glist *glist)
{
    t_garray *x = cast_garray (z);
    
    if (canvas_isMapped (x->x_owner) && gobj_isVisible (z, glist)) {
    //
    garray_vis (z, x->x_owner, 0); 
    garray_vis (z, x->x_owner, 1);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_fitToGraph (t_garray *x, int size, int style)
{
    t_array *array = garray_getArray (x);
    t_glist *glist = x->x_owner;
    
    if (glist->gl_graphics == cast_gobj (x) && !cast_gobj (x)->g_next) {
    //
    pd_vMessage (cast_pd (glist), sym_bounds, "ffff",
        0.0,
        glist->gl_valueTop,
        (double)((style == PLOT_POINTS || size == 1) ? size : size - 1),
        glist->gl_valueBottom);
    
    guistub_destroyWithKey ((void *)glist);
    //
    }
}

static void garray_setWithSumOfFourierComponents (t_garray *x,
    long numberOfPoints,
    int  numberOfSineWaves,
    t_float *valuesOfSineWaves,
    int isSine)
{
    double phase, phaseIncrement;
    int i;
    
    GARRAY_FETCH;
    
    numberOfPoints = (numberOfPoints <= 0) ? 512 : numberOfPoints;
    
    if (!PD_ISPOWER2 (numberOfPoints)) { numberOfPoints = PD_NEXTPOWER2 (numberOfPoints); }
    
    garray_resize_long (x, numberOfPoints + 3);
    
    phaseIncrement = 2.0 * (double)PD_PI / numberOfPoints;
    
    for (i = 0, phase = -phaseIncrement; i < array->a_size; i++, phase += phaseIncrement) {
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
    long numberOfPoints = atom_getFloatAtIndex (0, argc, argv);
    
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

static t_array *garray_getArrayCheckedFloat (t_garray *x, int *onset, int *elementSize)
{
    t_array    *a = garray_getArray (x);
    t_template *template = template_findbyname (a->a_template);
    
    if (template) {
    //
    t_error err = PD_ERROR_NONE;
    
    int yOnset = 0;
    int yType  = -1;
    t_symbol *yArrayType = NULL;
    
    err |= !(template_find_field (template, sym_y, &yOnset, &yType, &yArrayType));
    err |= (yType != DATA_FLOAT);

    if (!err) {
        *onset = yOnset;
        *elementSize = a->a_elementSize;
        return a;
    }
    //
    }
    
    PD_BUG;
    
    return NULL;
}

t_array *garray_getArray (t_garray *x)
{
    t_template *template = template_findbyname (x->x_scalar->sc_template);
    
    if (template) {
    //
    t_error err = PD_ERROR_NONE;
    
    int zOnset = 0;
    int zType  = -1;
    t_symbol *zArrayType = NULL;
    
    err |= !(template_find_field (template, sym_z, &zOnset, &zType, &zArrayType));
    err |= (zType != DATA_ARRAY);
    
    if (!err) { return (x->x_scalar->sc_vector[zOnset].w_array); }
    //
    }
    
    PD_BUG;
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int garray_getName (t_garray *x, t_symbol **s)
{
    *s = x->x_unexpandedName; return x->x_hideName;
}

t_glist *garray_getOwner (t_garray *x)
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

int garray_getFloats (t_garray *x, int *size, t_word **w)
{
    GARRAY_FETCH;
    
    if (array && (elementSize == sizeof (t_word))) { 
        *size = array->a_size; *w = (t_word *)array->a_vector; return 1; 
    }
    
    PD_BUG;
    
    return 0;
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
    interface_guiQueueAddIfNotAlreadyThere (cast_gobj (x), x->x_owner, garray_drawJob);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_list (t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) { 
    //
    int i, j = atom_getFloat (argv);
    
    GARRAY_FETCH;
    
    argc--;
    argv++;

    if (j < 0) { argc += j; argv -= j; j = 0; }
    
    if (j + argc > array->a_size) { argc = array->a_size - j; }
    
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
    
    GARRAY_FETCH;

    for (i = 0; i < array->a_size; i++) { GARRAY_AT (i) = f; }
    
    garray_redraw (x);
}

static void garray_normalize (t_garray *x, t_float f)
{
    int i;
    double maximum  = 0.0;
    
    GARRAY_FETCH;

    if (f <= 0.0) { f = 1.0; }

    for (i = 0; i < array->a_size; i++) {
        double t = GARRAY_AT (i);
        if (PD_ABS (t) > maximum) { maximum = PD_ABS (t); }
    }
    
    if (maximum > 0.0) {
        double k = f / maximum;
        for (i = 0; i < array->a_size; i++) { GARRAY_AT (i) *= k; }
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
    x->x_name = s;
    pd_bind (cast_pd (x), x->x_name);
    garray_redraw (x);
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
        
    GARRAY_FETCH;

    for (i = 0; i < array->a_size; i++) {
        double v = 0.0; if (!fscanf (file, "%lf", &v)) { break; } else { GARRAY_AT (i) = v; }
    }
    
    while (i < array->a_size) { GARRAY_AT (i) = 0.0; i++; }
    
    fclose (file);
    
    garray_redraw (x);
    //
    }
    //
    }
    
    if (err) { post_error (PD_TRANSLATE ("%s: can't open"), name->s_name); }    // --
}

static void garray_write (t_garray *x, t_symbol *name)
{
    char t[PD_STRING] = { 0 };
    FILE *file = NULL;
        
    canvas_makeFilePath (canvas_getView (x->x_owner), name->s_name, t, PD_STRING);
    
    if (!(file = file_openWrite (t))) { post_error (PD_TRANSLATE ("%s: can't create"), t); }    // --
    else {
    //
    int i;
    
    GARRAY_FETCH;
    
    for (i = 0; i < array->a_size; i++) {
        if (fprintf (file, "%g\n", GARRAY_AT (i)) < 1) { PD_BUG; break; }
    }
    
    fclose (file);
    //
    }
}

void garray_resize_long(t_garray *x, long n)
{
    t_array *array = garray_getArray(x);
    t_glist *gl = x->x_owner;
    if (n < 1)
        n = 1;
    garray_fitToGraph(x, n, template_getfloat(
        template_findbyname(x->x_scalar->sc_template),
            sym_style, x->x_scalar->sc_vector, 1));
    array_resize_and_redraw(array, x->x_owner, n);
    if (x->x_isUsedInDSP)
        dsp_update();
}

    /* float version to use as Pd method */
void garray_resize(t_garray *x, t_float f)
{
    garray_resize_long(x, f);
}

static void garray_print(t_garray *x)
{
    t_array *array = garray_getArray(x);
    post("garray %s: template %s, length %d",
        x->x_name->s_name, array->a_template->s_name, array->a_size);
}

    /* forward a "bounds" message to the owning graph */
static void garray_bounds(t_garray *x, t_float x1, t_float y1,
    t_float x2, t_float y2)
{
    pd_vMessage(&x->x_owner->gl_obj.te_g.g_pd, sym_bounds, "ffff", x1, y1, x2, y2);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void garray_getrect(t_gobj *z, t_glist *glist, int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_garray *x = (t_garray *)z;
    gobj_getRectangle(&x->x_scalar->sc_g, glist, xp1, yp1, xp2, yp2);
}

static void garray_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    /* refuse */
}

static void garray_select(t_gobj *z, t_glist *glist, int state)
{
    t_garray *x = (t_garray *)z;
    /* fill in later */
}

static void garray_activate(t_gobj *z, t_glist *glist, int state)
{
}

static void garray_delete(t_gobj *z, t_glist *glist)
{
    /* nothing to do */
}

static void garray_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_garray *x = (t_garray *)z;
    gobj_visibilityChanged(&x->x_scalar->sc_g, glist, vis);
    // interface_guiQueueRemove
}

static int garray_click(t_gobj *z, t_glist *glist, int xpix, int ypix, int shift, int ctrl, int alt, int dbl, int doit)
{
    t_garray *x = (t_garray *)z;
    return (gobj_clicked(&x->x_scalar->sc_g, glist,
        xpix, ypix, shift, ctrl, alt, dbl, doit));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void garray_savecontentsto(t_garray *x, t_buffer *b)
{
    int ARRAYWRITECHUNKSIZE = 1000;
    
    if (x->x_saveWithParent)
    {
        t_array *array = garray_getArray(x);
        int n = array->a_size, n2 = 0;
        if (n > 200000)
            post("warning: I'm saving an array with %d points!\n", n);
        while (n2 < n)
        {
            int chunk = n - n2, i;
            if (chunk > ARRAYWRITECHUNKSIZE)
                chunk = ARRAYWRITECHUNKSIZE;
            buffer_vAppend(b, "si", sym___hash__A, n2);
            for (i = 0; i < chunk; i++)
                buffer_vAppend(b, "f", ((t_word *)(array->a_vector))[n2+i].w_float);
            buffer_vAppend(b, ";");
            n2 += chunk;
        }
    }
}

static void garray_save(t_gobj *z, t_buffer *b)
{
    int style, filestyle;
    t_garray *x = (t_garray *)z;
    t_array *array = garray_getArray(x);
    t_template *scalartemplate;
    if (x->x_scalar->sc_template != sym_pd__dash__float__dash__array)   /* ??? */
    {
            /* LATER "save" the scalar as such */ 
        post_error ("can't save arrays of type %s yet", 
            x->x_scalar->sc_template->s_name);
        return;
    }
    if (!(scalartemplate = template_findbyname(x->x_scalar->sc_template)))
    {
        post_error ("array: no template of type %s",
            x->x_scalar->sc_template->s_name);
        return;
    }
    style = template_getfloat(scalartemplate, sym_style,
            x->x_scalar->sc_vector, 0);    
    filestyle = (style == PLOT_POINTS ? 1 : 
        (style == PLOT_POLYGONS ? 0 : style)); 
    buffer_vAppend(b, "sssisi;", sym___hash__X, sym_array,
        x->x_unexpandedName, array->a_size, &s_float,
            x->x_saveWithParent + 2 * filestyle + 8*x->x_hideName);
    garray_savecontentsto(x, b);
}

    /* called from graph_dialog to set properties */
void garray_properties(t_garray *x)
{
    char cmdbuf[200];
    t_array *a = garray_getArray(x);
    t_scalar *sc = x->x_scalar;
    int style = template_getfloat(template_findbyname(sc->sc_template),
        sym_style, x->x_scalar->sc_vector, 1);
    int filestyle = (style == 0 ? PLOT_POLYGONS :
        (style == 1 ? PLOT_POINTS : style));

    if (!a)
        return;
    guistub_destroyWithKey ((void *)x);
        /* create dialog window.  LATER fix this to escape '$'
        properly; right now we just detect a leading '$' and escape
        it.  There should be a systematic way of doing this. */
    sprintf(cmdbuf, "::ui_array::show %%s %s %d %d\n",
            dollar_toHash(x->x_unexpandedName)->s_name, a->a_size, x->x_saveWithParent + 
            2 * filestyle);
    guistub_new(&x->x_gobj.g_pd, x, cmdbuf);
}

    /* this is called from the properties dialog window for an existing array */
void garray_arraydialog(t_garray *x, t_symbol *name, t_float fsize, t_float fflags)
{
    int flags = fflags;
    int saveit = ((flags & 1) != 0);
    int filestyle = ((flags & 6) >> 1);
    int style = (filestyle == 0 ? PLOT_POLYGONS :
        (filestyle == 1 ? PLOT_POINTS : filestyle));
    t_float stylewas = template_getfloat(
        template_findbyname(x->x_scalar->sc_template),
            sym_style, x->x_scalar->sc_vector, 1);
    if (0) // deleteit
    {
        int wasused = x->x_isUsedInDSP;
        canvas_removeObject(x->x_owner, &x->x_gobj);
        if (wasused)
            dsp_update();
    }
    else
    {
        long size;
        int styleonset, styletype;
        t_symbol *stylearraytype;
        t_symbol *argname = dollar_fromHash(name);
        t_array *a = garray_getArray(x);
        t_template *scalartemplate;
        if (!a)
        {
            post_error ("can't find array\n");
            return;
        }
        if (!(scalartemplate = template_findbyname(x->x_scalar->sc_template)))
        {
            post_error ("array: no template of type %s",
                x->x_scalar->sc_template->s_name);
            return;
        }
        if (argname != x->x_unexpandedName)
        {
            x->x_unexpandedName = argname;
            pd_unbind(&x->x_gobj.g_pd, x->x_name);
            x->x_name = canvas_expandDollar(x->x_owner, argname);
            pd_bind(&x->x_gobj.g_pd, x->x_name);
                /* redraw the whole glist, just so the name change shows up */
            if (x->x_owner->gl_hasWindow)
                canvas_redraw(x->x_owner);
            else if (canvas_isMapped(x->x_owner->gl_parent))
            {
                gobj_visibilityChanged(&x->x_owner->gl_obj.te_g, x->x_owner->gl_parent, 0);
                gobj_visibilityChanged(&x->x_owner->gl_obj.te_g, x->x_owner->gl_parent, 1);
            }
            dsp_update();
        }
        size = fsize;
        if (size < 1)
            size = 1;
        if (size != a->a_size)
            garray_resize_long(x, size);
        else if (style != stylewas)
            garray_fitToGraph(x, size, style);
        template_setfloat(scalartemplate, sym_style,
            x->x_scalar->sc_vector, (t_float)style, 0);

        garray_setSaveWithParent(x, (saveit != 0));
        garray_redraw(x);
        canvas_dirty(x->x_owner, 1);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_garray *garray_makeObjectWithScalar (t_glist *glist,
    t_symbol *name,
    t_symbol *templateName,
    int save, 
    int hide)
{
    t_garray *x = (t_garray *)pd_new (garray_class);
    
    x->x_scalar         = scalar_new (glist, templateName);
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

t_garray *garray_makeObject (t_glist *glist,
    t_symbol *name,
    t_symbol *templateName,
    t_float size,
    t_float flags)
{
    t_garray *x = NULL;

    if (templateName != &s_float) { PD_BUG; }
    else {
    //
    t_template *template = template_findbyname (sym_pd__dash__float__dash__array);
    
    if (template) {
    //
    t_error err = PD_ERROR_NONE;
    
    int zOnset = 0;
    int zType  = -1;
    t_symbol *zArrayType = NULL;

    err |= !(template_find_field (template, sym_z, &zOnset, &zType, &zArrayType));
    err |= !(template_findbyname (zArrayType));
    err |= (zType != DATA_ARRAY);
    
    if (!err) {
    //
    int save = (((int)flags & GARRAY_FLAG_SAVE) != 0);
    int hide = (((int)flags & GARRAY_FLAG_HIDE) != 0);
    
    int plot = PD_CLAMP ((((int)flags & GARRAY_FLAG_PLOT) >> 1), PLOT_POLYGONS, PLOT_CURVES);

    x = garray_makeObjectWithScalar (glist, name, sym_pd__dash__float__dash__array, save, hide);

    array_resize (x->x_scalar->sc_vector[zOnset].w_array, PD_MAX (1, size));

    template_setfloat (template, sym_style, x->x_scalar->sc_vector, plot, 1);
    template_setfloat (template, sym_linewidth, x->x_scalar->sc_vector, 1, 1);

    sym___hash__A->s_thing = NULL;
    pd_bind (cast_pd (x), sym___hash__A); 

    garray_redraw (x);
    dsp_update();
    //
    }
    //
    }
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
    
    pd_unbind (cast_pd (x), x->x_name);
    
    /* Just in case we're still bound to #A from above. */
    
    while (t = pd_findByClass (sym___hash__A, garray_class)) { pd_unbind (t, sym___hash__A); }
    
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
    class_addMethod (c, (t_method)garray_print,         sym_print,          A_NULL);

    class_addMethod (c, (t_method)garray_bounds,
        sym_bounds,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)garray_arraydialog,
        sym__arraydialog,
        A_SYMBOL,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)garray_constant, sym_const, A_DEFFLOAT, A_NULL);
    
    #endif
    
    class_setWidgetBehavior (c, &garray_widgetBehavior);
    class_setSaveFunction (c, garray_save);
    
    garray_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
