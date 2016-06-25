
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

t_class *garray_class;                  /* Shared. */

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

static t_widgetbehavior garray_widgetBehavior = 
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    err |= !(template_findbyname (zArrayType));
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

    /* get the "array" structure and furthermore check it's float */
static t_array *garray_getarray_floatonly(t_garray *x,
    int *yonsetp, int *elemsizep)
{
    t_array *a = garray_getArray(x);
    int yonset, type;
    t_symbol *arraytype;
    t_template *template = template_findbyname(a->a_template);
    if (!template_find_field(template, sym_y, &yonset,
        &type, &arraytype) || type != DATA_FLOAT)
            return (0);
    *yonsetp = yonset;
    *elemsizep = a->a_elementSize;
    return (a);
}

    /* get the array's name.  Return nonzero if it should be hidden */
int garray_getname(t_garray *x, t_symbol **namep)
{
    *namep = x->x_unexpandedName;
    return (x->x_hideName);
}

    /* get a garray's containing glist */
t_glist *garray_getglist(t_garray *x)
{
    return (x->x_owner);
}

    /* get a garray's associated scalar */
t_scalar *garray_getscalar(t_garray *x)
{
    return (x->x_scalar);
}

void garray_usedindsp(t_garray *x)
{
    x->x_isUsedInDSP = 1;
}

static void garray_drawJob(t_gobj *client, t_glist *glist)
{
    t_garray *x = (t_garray *)client;
    if (canvas_isMapped(x->x_owner) && gobj_isVisible(client, glist))
    {
        garray_vis(&x->x_gobj, x->x_owner, 0); 
        garray_vis(&x->x_gobj, x->x_owner, 1);
    }
}

void garray_redraw(t_garray *x)
{
    if (canvas_isMapped(x->x_owner))
        interface_guiQueueAddIfNotAlreadyThere(&x->x_gobj, x->x_owner, garray_drawJob);
    /* jsarlo { */
    /* this happens in garray_vis() when array is visible for
       performance reasons */
    //else
    //{
      /* if (x->x_listviewing)
        sys_vGui("::ui_array::pdtk_array_listview_fillpage %s\n",
                 x->x_name->s_name);*/
    //}
    /* } jsarlo */
}

   /* This functiopn gets the template of an array; if we can't figure
   out what template an array's elements belong to we're in grave trouble
   when it's time to free or resize it.  */
t_template *garray_template(t_garray *x)
{
    t_array *array = garray_getArray(x);
    t_template *template = 
        (array ? template_findbyname(array->a_template) : 0);
    if (!template) { PD_BUG; }
    return (template);
}

static int garray_npoints(t_garray *x) /* get the length */
{
    t_array *array = garray_getArray(x);
    return (array->a_size);
}

static char *garray_vec(t_garray *x) /* get the contents */
{
    t_array *array = garray_getArray(x);
    return ((char *)(array->a_vector));
}

    /* routine that checks if we're just an array of floats and if
    so returns the goods */

int garray_getfloatwords(t_garray *x, int *size, t_word **vec)
{
    int yonset, type, elemsize;
    t_array *a = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!a)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
        return (0);
    }
    else if (elemsize != sizeof(t_word))
    {
        post_error ("%s: has more than one field", x->x_name->s_name);
        return (0);
    }
    *size = garray_npoints(x);
    *vec =  (t_word *)garray_vec(x);
    return (1);
}
    /* older, non-64-bit safe version, supplied for older externs */

static int garray_getfloatarray(t_garray *x, int *size, t_float **vec)
{
    if (sizeof(t_word) != sizeof(t_float))
    {
        t_symbol *patchname;
        if (x->x_owner->gl_parent)
            patchname = x->x_owner->gl_parent->gl_name;
        else
            patchname = x->x_owner->gl_name;
        post_error ("An operation on the array '%s' in the patch '%s'",
              x->x_unexpandedName->s_name, patchname->s_name);
        post_error ("failed since it uses garray_getfloatarray while running 64-bit!");
    }
    return (garray_getfloatwords(x, size, (t_word **)vec));
}

    /* set the "saveit" flag */
void garray_setsaveit(t_garray *x, int saveit)
{
    if (x->x_saveWithParent && !saveit)
        post("warning: array %s: clearing save-in-patch flag",
            x->x_unexpandedName->s_name);
    x->x_saveWithParent = saveit;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/*------------------- Pd messages ------------------------ */
static void garray_const(t_garray *x, t_float g)
{
    int yonset, i, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
    else for (i = 0; i < array->a_size; i++)
        *((t_float *)((char *)array->a_vector
            + elemsize * i) + yonset) = g;
    garray_redraw(x);
}

    /* sum of Fourier components; called from routines below */
static void garray_dofo(t_garray *x, long npoints, t_float dcval,
    int nsin, t_float *vsin, int sineflag)
{
    double phase, phaseincr, fj;
    int yonset, i, j, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
        return;
    }
    if (npoints == 0)
        npoints = 512;  /* dunno what a good default would be... */
    if (npoints != (1 << ilog2(npoints)))
        post("%s: rounnding to %d points", array->a_template->s_name,
            (npoints = (1<<ilog2(npoints))));
    garray_resize_long(x, npoints + 3);
    phaseincr = 2. * 3.14159 / npoints;
    for (i = 0, phase = -phaseincr; i < array->a_size; i++, phase += phaseincr)
    {
        double sum = dcval;
        if (sineflag)
            for (j = 0, fj = phase; j < nsin; j++, fj += phase)
                sum += vsin[j] * sin(fj);
        else
            for (j = 0, fj = 0; j < nsin; j++, fj += phase)
                sum += vsin[j] * cos(fj);
        *((t_float *)((array->a_vector + elemsize * i)) + yonset)
            = sum;
    }
    garray_redraw(x);
}

static void garray_sinesum(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{    
    t_float *svec;
    long npoints;
    int i;
    if (argc < 2)
    {
        post_error ("sinesum: %s: need number of points and partial strengths",
            x->x_name->s_name);
        return;
    }

    npoints = atom_getFloatAtIndex(0, argc, argv);
    argv++, argc--;
    
    svec = (t_float *)PD_MEMORY_GET(sizeof(t_float) * argc);
    if (!svec) return;
    
    for (i = 0; i < argc; i++)
        svec[i] = atom_getFloatAtIndex(i, argc, argv);
    garray_dofo(x, npoints, 0, argc, svec, 1);
    PD_MEMORY_FREE(svec);
}

static void garray_cosinesum(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    t_float *svec;
    long npoints;
    int i;
    if (argc < 2)
    {
        post_error ("sinesum: %s: need number of points and partial strengths",
            x->x_name->s_name);
        return;
    }

    npoints = atom_getFloatAtIndex(0, argc, argv);
    argv++, argc--;
    
    svec = (t_float *)PD_MEMORY_GET(sizeof(t_float) * argc);
    if (!svec) return;

    for (i = 0; i < argc; i++)
        svec[i] = atom_getFloatAtIndex(i, argc, argv);
    garray_dofo(x, npoints, 0, argc, svec, 0);
    PD_MEMORY_FREE(svec);
}

static void garray_normalize(t_garray *x, t_float f)
{
    int type, i;
    double maxv, renormer;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
        return;
    }

    if (f <= 0)
        f = 1;

    for (i = 0, maxv = 0; i < array->a_size; i++)
    {
        double v = *((t_float *)(array->a_vector + elemsize * i)
            + yonset);
        if (v > maxv)
            maxv = v;
        if (-v > maxv)
            maxv = -v;
    }
    if (maxv > 0)
    {
        renormer = f / maxv;
        for (i = 0; i < array->a_size; i++)
            *((t_float *)(array->a_vector + elemsize * i) + yonset)
                *= renormer;
    }
    garray_redraw(x);
}

    /* list -- the first value is an index; subsequent values are put in
    the "y" slot of the array.  This generalizes Max's "table", sort of. */
static void garray_list(t_garray *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
        return;
    }
    if (argc < 2) return;
    else
    {
        int firstindex = atom_getFloat(argv);
        argc--;
        argv++;
            /* drop negative x values */
        if (firstindex < 0)
        {
            argc += firstindex;
            argv -= firstindex;
            firstindex = 0;
            if (argc <= 0) return;
        }
        if (argc + firstindex > array->a_size)
        {
            argc = array->a_size - firstindex;
            if (argc <= 0) return;
        }
        for (i = 0; i < argc; i++)
            *((t_float *)(array->a_vector + elemsize * (i + firstindex)) + yonset)
                = atom_getFloat(argv + i);
    }
    garray_redraw(x);
}

    /* forward a "bounds" message to the owning graph */
static void garray_bounds(t_garray *x, t_float x1, t_float y1,
    t_float x2, t_float y2)
{
    pd_vMessage(&x->x_owner->gl_obj.te_g.g_pd, sym_bounds, "ffff", x1, y1, x2, y2);
}

    /* change the name of a garray. */
static void garray_rename(t_garray *x, t_symbol *s)
{
    pd_unbind(&x->x_gobj.g_pd, x->x_name);
    pd_bind(&x->x_gobj.g_pd, x->x_name = x->x_unexpandedName = s);
    garray_redraw(x);
}

static void garray_read(t_garray *x, t_symbol *filename)
{
    int nelem, filedesc, i;
    FILE *fd;
    char buf[PD_STRING], *bufptr;
    int yonset, elemsize;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
        return;
    }
    nelem = array->a_size;
    if ((filedesc = canvas_openFile(canvas_getView(x->x_owner),
            filename->s_name, "", buf, &bufptr, PD_STRING)) < 0 
                || !(fd = fdopen(filedesc, "r")))
    {
        post_error ("%s: can't open", filename->s_name);
        return;
    }
    for (i = 0; i < nelem; i++)
    {
        double f;
        if (!fscanf(fd, "%lf", &f))
        {
            post("%s: read %d elements into table of size %d",
                filename->s_name, i, nelem);
            break;
        }
        else *((t_float *)(array->a_vector + elemsize * i) + yonset) = f;
    }
    while (i < nelem)
        *((t_float *)(array->a_vector +
            elemsize * i) + yonset) = 0, i++;
    fclose(fd);
    garray_redraw(x);
}

static void garray_write(t_garray *x, t_symbol *filename)
{
    FILE *fd;
    char buf[PD_STRING];
    int yonset, elemsize, i;
    t_array *array = garray_getarray_floatonly(x, &yonset, &elemsize);
    if (!array)
    {
        post_error ("%s: needs floating-point 'y' field", x->x_name->s_name);
        return;
    }
    canvas_makeFilePath(canvas_getView(x->x_owner), filename->s_name,
        buf, PD_STRING);
    if (!(fd = file_openWrite(buf)))
    {
        post_error ("%s: can't create", buf);
        return;
    }
    for (i = 0; i < array->a_size; i++)
    {
        if (fprintf(fd, "%g\n",
            *(t_float *)(((array->a_vector + sizeof(t_word) * i)) + yonset)) < 1)
        {
            post("%s: write error", filename->s_name);
            break;
        }
    }
    fclose(fd);
}


    /* this should be renamed and moved... */
int garray_ambigendian(void)
{
    unsigned short s = 1;
    unsigned char c = *(char *)(&s);
    return (c==0);
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

        garray_setsaveit(x, (saveit != 0));
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
    
    x->x_scalar             = scalar_new (glist, templateName);
    x->x_owner              = glist;
    x->x_unexpandedName     = name;
    x->x_name               = canvas_expandDollar (glist, name);
    x->x_isUsedInDSP        = 0;
    x->x_saveWithParent     = save;
    x->x_hideName           = hide;
    
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
    
    class_addMethod (c, (t_method)garray_const,         sym_constant,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_normalize,     sym_normalize,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_rename,        sym_rename,         A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_read,          sym_read,           A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_write,         sym_write,          A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)garray_resize,        sym_resize,         A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)garray_print,         sym_print,          A_NULL);
    class_addMethod (c, (t_method)garray_sinesum,       sym_sinesum,        A_GIMME, A_NULL);
    class_addMethod (c, (t_method)garray_cosinesum,     sym_cosinesum,      A_GIMME, A_NULL);

    class_addMethod (c, (t_method)garray_arraydialog,
        sym__arraydialog,
        A_SYMBOL,
        A_FLOAT,
        A_FLOAT,
        A_NULL);

    class_addMethod (c, (t_method)garray_bounds,
        sym_bounds,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)garray_const, sym_const, A_DEFFLOAT, A_NULL);
    
    #endif
    
    class_setWidgetBehavior (c, &garray_widgetBehavior);
    class_setSaveFunction (c, garray_save);
    
    garray_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
