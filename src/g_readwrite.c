/* Copyright (c) 1997-2002 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* 
Routines to read and write canvases to files:
canvas_savetofile() writes a root canvas to a "pd" file.  (Reading "pd" files
is done simply by passing the contents to the pd message interpreter.)
Alternatively, the  glist_read() and glist_write() routines read and write
"data" from and to files (reading reads into an existing canvas), using a
file format as in the dialog window for data.
*/

#include <stdlib.h>
#include <stdio.h>
#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"
#include "g_canvas.h"
#include <string.h>

extern t_class *scalar_class;
extern t_class *canvas_class;

static t_class *declare_class;
void canvas_savedeclarationsto(t_canvas *x, t_binbuf *b);

    /* the following routines read "scalars" from a file into a canvas. */

static int canvas_scanbinbuf(int natoms, t_atom *vec, int *p_indexout,
    int *p_next)
{
    int i, j;
    int indexwas = *p_next;
    *p_indexout = indexwas;
    if (indexwas >= natoms)
        return (0);
    for (i = indexwas; i < natoms && vec[i].a_type != A_SEMICOLON; i++)
        ;
    if (i >= natoms)
        *p_next = i;
    else *p_next = i + 1;
    return (i - indexwas);
}

int canvas_readscalar(t_glist *x, int natoms, t_atom *vec,
    int *p_nextmsg, int selectit);

static void canvas_readerror(int natoms, t_atom *vec, int message, 
    int nline, char *s)
{
    post_error ("%s", s);
    post("line was:");
    post_atoms(nline, vec + message);
}

    /* fill in the contents of the scalar into the vector w. */

static void glist_readatoms(t_glist *x, int natoms, t_atom *vec,
    int *p_nextmsg, t_symbol *templatesym, t_word *w, int argc, t_atom *argv)
{
    int message, nline, n, i;

    t_template *template = template_findbyname(templatesym);
    if (!template)
    {
        post_error ("%s: no such template", templatesym->s_name);
        *p_nextmsg = natoms;
        return;
    }
    word_restore(w, template, argc, argv);
    n = template->t_n;
    for (i = 0; i < n; i++)
    {
        if (template->t_vec[i].ds_type == DATA_ARRAY)
        {
            int j;
            t_array *a = w[i].w_array;
            int elemsize = a->a_elemsize, nitems = 0;
            t_symbol *arraytemplatesym = template->t_vec[i].ds_arraytemplate;
            t_template *arraytemplate =
                template_findbyname(arraytemplatesym);
            if (!arraytemplate)
            {
                post_error ("%s: no such template", arraytemplatesym->s_name);
            }
            else while (1)
            {
                t_word *element;
                int nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
                    /* empty line terminates array */
                if (!nline)
                    break;
                array_resize(a, nitems + 1);
                element = (t_word *)(((char *)a->a_vec) +
                    nitems * elemsize);
                glist_readatoms(x, natoms, vec, p_nextmsg, arraytemplatesym,
                    element, nline, vec + message);
                nitems++;
            }
        }
        else if (template->t_vec[i].ds_type == DATA_TEXT)
        {
            t_binbuf *z = binbuf_new();
            int first = *p_nextmsg, last;
            for (last = first; last < natoms && vec[last].a_type != A_SEMICOLON;
                last++);
            binbuf_restore(z, last-first, vec+first);
            binbuf_add(w[i].w_binbuf, binbuf_getnatom(z), binbuf_getvec(z));
            binbuf_free(z);
            last++;
            if (last > natoms) last = natoms;
            *p_nextmsg = last;
        }
    }
}

int canvas_readscalar(t_glist *x, int natoms, t_atom *vec,
    int *p_nextmsg, int selectit)
{
    int message, i, j, nline;
    t_template *template;
    t_symbol *templatesym;
    t_scalar *sc;
    int nextmsg = *p_nextmsg;
    int wasvis = glist_isvisible(x);

    if (nextmsg >= natoms || vec[nextmsg].a_type != A_SYMBOL)
    {
        if (nextmsg < natoms)
            post("stopping early: type %d", vec[nextmsg].a_type);
        *p_nextmsg = natoms;
        return (0);
    }
    templatesym = canvas_makebindsym(vec[nextmsg].a_w.w_symbol);
    *p_nextmsg = nextmsg + 1;
    
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("canvas_read: %s: no such template", templatesym->s_name);
        *p_nextmsg = natoms;
        return (0);
    }
    sc = scalar_new(x, templatesym);
    if (!sc)
    {
        post_error ("couldn't create scalar \"%s\"", templatesym->s_name);
        *p_nextmsg = natoms;
        return (0);
    }
    if (wasvis)
    {
            /* temporarily lie about vis flag while this is built */
        glist_getcanvas(x)->gl_mapped = 0;
    }
    glist_add(x, &sc->sc_g);
    
    nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
    glist_readatoms(x, natoms, vec, p_nextmsg, templatesym, sc->sc_vector, 
        nline, vec + message);
    if (wasvis)
    {
            /* reset vis flag as before */
        glist_getcanvas(x)->gl_mapped = 1;
        gobj_vis(&sc->sc_g, x, 1);
    }
    if (selectit)
    {
        glist_select(x, &sc->sc_g);
    }
    return (1);
}

void glist_readfrombinbuf(t_glist *x, t_binbuf *b, char *filename, int selectem)
{
    t_canvas *canvas = glist_getcanvas(x);
    int cr = 0, natoms, nline, message, nextmsg = 0, i, j, nitems;
    t_atom *vec;
    t_gobj *gobj;

    natoms = binbuf_getnatom(b);
    vec = binbuf_getvec(b);

    
            /* check for file type */
    nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
    if (nline != 1 && vec[message].a_type != A_SYMBOL &&
        strcmp(vec[message].a_w.w_symbol->s_name, "data"))
    {
        post_error ("%s: file apparently of wrong type", filename);
        binbuf_free(b);
        return;
    }
        /* read in templates and check for consistency */
    while (1)
    {
        t_template *newtemplate, *existtemplate;
        t_symbol *templatesym;
        t_atom *templateargs = PD_MEMORY_GET(0);
        int ntemplateargs = 0, newnargs;
        nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
        if (nline < 2)
            break;
        else if (nline > 2)
            canvas_readerror(natoms, vec, message, nline,
                "extra items ignored");
        else if (vec[message].a_type != A_SYMBOL ||
            strcmp(vec[message].a_w.w_symbol->s_name, "template") ||
            vec[message + 1].a_type != A_SYMBOL)
        {
            canvas_readerror(natoms, vec, message, nline,
                "bad template header");
            continue;
        }
        templatesym = canvas_makebindsym(vec[message + 1].a_w.w_symbol);
        while (1)
        {
            nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
            if (nline != 2 && nline != 3)
                break;
            newnargs = ntemplateargs + nline;
            templateargs = (t_atom *)PD_MEMORY_RESIZE(templateargs,
                sizeof(*templateargs) * ntemplateargs,
                sizeof(*templateargs) * newnargs);
            templateargs[ntemplateargs] = vec[message];
            templateargs[ntemplateargs + 1] = vec[message + 1];
            if (nline == 3)
                templateargs[ntemplateargs + 2] = vec[message + 2];
            ntemplateargs = newnargs;
        }
        if (!(existtemplate = template_findbyname(templatesym)))
        {
            post_error ("%s: template not found in current patch",
                templatesym->s_name);
            PD_MEMORY_FREE(templateargs, sizeof (*templateargs) * ntemplateargs);
            return;
        }
        newtemplate = template_new(templatesym, ntemplateargs, templateargs);
        PD_MEMORY_FREE(templateargs, sizeof (*templateargs) * ntemplateargs);
        if (!template_match(existtemplate, newtemplate))
        {
            post_error ("%s: template doesn't match current one",
                templatesym->s_name);
            template_free(newtemplate);
            return;
        }
        template_free(newtemplate);
    }
    while (nextmsg < natoms)
    {
        canvas_readscalar(x, natoms, vec, &nextmsg, selectem);
    }
}

static void glist_doread(t_glist *x, t_symbol *filename, t_symbol *format,
    int clearme)
{
    t_binbuf *b = binbuf_new();
    t_canvas *canvas = glist_getcanvas(x);
    int wasvis = glist_isvisible(canvas);
    int cr = 0, natoms, nline, message, nextmsg = 0, i, j;
    t_atom *vec;

    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);
    
    if (binbuf_read_via_canvas(b, filename->s_name, canvas, cr))
    {
        post_error ("read failed");
        binbuf_free(b);
        return;
    }
    if (wasvis)
        canvas_vis(canvas, 0);
    if (clearme)
        glist_clear(x);
    glist_readfrombinbuf(x, b, filename->s_name, 0);
    if (wasvis)
        canvas_vis(canvas, 1);
    binbuf_free(b);
}

void glist_read(t_glist *x, t_symbol *filename, t_symbol *format)
{
    glist_doread(x, filename, format, 1);
}

void glist_mergefile(t_glist *x, t_symbol *filename, t_symbol *format)
{
    glist_doread(x, filename, format, 0);
}

    /* read text from a "properties" window, called from a gfxstub set
    up in scalar_properties().  We try to restore the object; if successful
    we either copy the data from the new scalar to the old one in place
    (if their templates match) or else delete the old scalar and put the new
    thing in its place on the list. */
void canvas_dataproperties(t_canvas *x, t_scalar *sc, t_binbuf *b)
{
    int ntotal, nnew, scindex;
    t_gobj *y, *y2 = 0, *newone, *oldone = 0;
    t_template *template;
    for (y = x->gl_list, ntotal = 0, scindex = -1; y; y = y->g_next)
    {
        if (y == &sc->sc_g)
            scindex = ntotal, oldone = y;
        ntotal++;
    }
    
    if (scindex == -1)
    {
        post_error ("data_properties: scalar disappeared");
        return;
    }
    glist_readfrombinbuf(x, b, "properties dialog", 0);
    newone = 0;
        /* take the new object off the list */
    if (ntotal)
    {
        for (y = x->gl_list, nnew = 1; y2 = y->g_next;
            y = y2, nnew++)
                if (nnew == ntotal)
        {
            newone = y2;
            gobj_vis(newone, x, 0);
            y->g_next = y2->g_next;
            break;    
        }
    }
    else gobj_vis((newone = x->gl_list), x, 0), x->gl_list = newone->g_next;
    if (!newone)
        post_error ("couldn't update properties (perhaps a format problem?)");
    else if (!oldone) { PD_BUG; }
    else if (newone->g_pd == scalar_class && oldone->g_pd == scalar_class
        && ((t_scalar *)newone)->sc_template ==
            ((t_scalar *)oldone)->sc_template 
        && (template = template_findbyname(((t_scalar *)newone)->sc_template)))
    {
            /* copy new one to old one and deete new one */
        memcpy(&((t_scalar *)oldone)->sc_vector, &((t_scalar *)newone)->sc_vector,
            template->t_n * sizeof(t_word));
        pd_free(&newone->g_pd);
        if (glist_isvisible(x))
        {
            gobj_vis(oldone, x, 0);
            gobj_vis(oldone, x, 1);
        }
    }
    else
    {
            /* delete old one; put new one where the old one was on glist */
        glist_delete(x, oldone);
        if (scindex > 0)
        {
            for (y = x->gl_list, nnew = 1; y;
                y = y->g_next, nnew++)
                    if (nnew == scindex || !y->g_next)
            {
                newone->g_next = y->g_next;
                y->g_next = newone;
                goto didit;
            }
            PD_BUG;
        }
        else newone->g_next = x->gl_list, x->gl_list = newone;
    }
didit:
    ;
}

    /* ----------- routines to write data to a binbuf ----------- */

void canvas_doaddtemplate(t_symbol *templatesym, 
    int *p_ntemplates, t_symbol ***p_templatevec)
{
    int n = *p_ntemplates, i;
    t_symbol **templatevec = *p_templatevec;
    for (i = 0; i < n; i++)
        if (templatevec[i] == templatesym)
            return;
    templatevec = (t_symbol **)PD_MEMORY_RESIZE(templatevec,
        n * sizeof(*templatevec), (n+1) * sizeof(*templatevec));
    templatevec[n] = templatesym;
    *p_templatevec = templatevec;
    *p_ntemplates = n+1;
}

static void glist_writelist(t_gobj *y, t_binbuf *b);

void binbuf_savetext(t_binbuf *bfrom, t_binbuf *bto);

void canvas_writescalar(t_symbol *templatesym, t_word *w, t_binbuf *b,
    int amarrayelement)
{
    t_dataslot *ds;
    t_template *template = template_findbyname(templatesym);
    t_atom *a = (t_atom *)PD_MEMORY_GET(0);
    int i, n = template->t_n, natom = 0;
    if (!amarrayelement)
    {
        t_atom templatename;
        SET_SYMBOL(&templatename, gensym(templatesym->s_name + 3));
        binbuf_add(b, 1, &templatename);
    }
    if (!template) { PD_BUG; }
        /* write the atoms (floats and symbols) */
    for (i = 0; i < n; i++)
    {
        if (template->t_vec[i].ds_type == DATA_FLOAT ||
            template->t_vec[i].ds_type == DATA_SYMBOL)
        {
            a = (t_atom *)PD_MEMORY_RESIZE(a,
                natom * sizeof(*a), (natom + 1) * sizeof (*a));
            if (template->t_vec[i].ds_type == DATA_FLOAT)
                SET_FLOAT(a + natom, w[i].w_float);
            else SET_SYMBOL(a + natom,  w[i].w_symbol);
            natom++;
        }
    }
        /* array elements have to have at least something */
    if (natom == 0 && amarrayelement)
        SET_SYMBOL(a + natom,  &s_bang), natom++;
    binbuf_add(b, natom, a);
    binbuf_addsemi(b);
    PD_MEMORY_FREE(a, natom * sizeof(*a));
    for (i = 0; i < n; i++)
    {
        if (template->t_vec[i].ds_type == DATA_ARRAY)
        {
            int j;
            t_array *a = w[i].w_array;
            int elemsize = a->a_elemsize, nitems = a->a_n;
            t_symbol *arraytemplatesym = template->t_vec[i].ds_arraytemplate;
            for (j = 0; j < nitems; j++)
                canvas_writescalar(arraytemplatesym,
                    (t_word *)(((char *)a->a_vec) + elemsize * j), b, 1);
            binbuf_addsemi(b);
        }
        else if (template->t_vec[i].ds_type == DATA_TEXT)
            binbuf_savetext(w[i].w_binbuf, b);
    }
}

static void glist_writelist(t_gobj *y, t_binbuf *b)
{
    for (; y; y = y->g_next)
    {
        if (pd_class(&y->g_pd) == scalar_class)
        {
            canvas_writescalar(((t_scalar *)y)->sc_template,
                ((t_scalar *)y)->sc_vector, b, 0);
        }
    }
}

    /* ------------ routines to write out templates for data ------- */

static void canvas_addtemplatesforlist(t_gobj *y,
    int  *p_ntemplates, t_symbol ***p_templatevec);

static void canvas_addtemplatesforscalar(t_symbol *templatesym,
    t_word *w, int *p_ntemplates, t_symbol ***p_templatevec)
{
    t_dataslot *ds;
    int i;
    t_template *template = template_findbyname(templatesym);
    canvas_doaddtemplate(templatesym, p_ntemplates, p_templatevec);
    if (!template) { PD_BUG; }
    else for (ds = template->t_vec, i = template->t_n; i--; ds++, w++)
    {
        if (ds->ds_type == DATA_ARRAY)
        {
            int j;
            t_array *a = w->w_array;
            int elemsize = a->a_elemsize, nitems = a->a_n;
            t_symbol *arraytemplatesym = ds->ds_arraytemplate;
            canvas_doaddtemplate(arraytemplatesym, p_ntemplates, p_templatevec);
            for (j = 0; j < nitems; j++)
                canvas_addtemplatesforscalar(arraytemplatesym,
                    (t_word *)(((char *)a->a_vec) + elemsize * j), 
                        p_ntemplates, p_templatevec);
        }
    }
}

static void canvas_addtemplatesforlist(t_gobj *y,
    int  *p_ntemplates, t_symbol ***p_templatevec)
{
    for (; y; y = y->g_next)
    {
        if (pd_class(&y->g_pd) == scalar_class)
        {
            canvas_addtemplatesforscalar(((t_scalar *)y)->sc_template,
                ((t_scalar *)y)->sc_vector, p_ntemplates, p_templatevec);
        }
    }
}

    /* write all "scalars" in a glist to a binbuf. */
t_binbuf *glist_writetobinbuf(t_glist *x, int wholething)
{
    int i;
    t_symbol **templatevec = PD_MEMORY_GET(0);
    int ntemplates = 0;
    t_gobj *y;
    t_binbuf *b = binbuf_new();

    for (y = x->gl_list; y; y = y->g_next)
    {
        if ((pd_class(&y->g_pd) == scalar_class) &&
            (wholething || glist_isselected(x, y)))
        {
            canvas_addtemplatesforscalar(((t_scalar *)y)->sc_template,
                ((t_scalar *)y)->sc_vector,  &ntemplates, &templatevec);
        }
    }
    binbuf_addv(b, "s;", gensym("data"));
    for (i = 0; i < ntemplates; i++)
    {
        t_template *template = template_findbyname(templatevec[i]);
        int j, m = template->t_n;
            /* drop "pd-" prefix from template symbol to print it: */
        binbuf_addv(b, "ss;", gensym("template"),
            gensym(templatevec[i]->s_name + 3));
        for (j = 0; j < m; j++)
        {
            t_symbol *type;
            switch (template->t_vec[j].ds_type)
            {
                case DATA_FLOAT: type = &s_float; break;
                case DATA_SYMBOL: type = &s_symbol; break;
                case DATA_ARRAY: type = gensym("array"); break;
                case DATA_TEXT: type = &s_list; break;
                default: type = &s_float; PD_BUG;
            }
            if (template->t_vec[j].ds_type == DATA_ARRAY)
                binbuf_addv(b, "sss;", type, template->t_vec[j].ds_name,
                    gensym(template->t_vec[j].ds_arraytemplate->s_name + 3));
            else binbuf_addv(b, "ss;", type, template->t_vec[j].ds_name);
        }
        binbuf_addsemi(b);
    }
    binbuf_addsemi(b);
        /* now write out the objects themselves */
    for (y = x->gl_list; y; y = y->g_next)
    {
        if ((pd_class(&y->g_pd) == scalar_class) &&
            (wholething || glist_isselected(x, y)))
        {
            canvas_writescalar(((t_scalar *)y)->sc_template,
                ((t_scalar *)y)->sc_vector,  b, 0);
        }
    }
    return (b);
}

static void glist_write(t_glist *x, t_symbol *filename, t_symbol *format)
{
    int cr = 0, i;
    t_binbuf *b;
    char buf[PD_STRING];
    t_symbol **templatevec = PD_MEMORY_GET(0);
    int ntemplates = 0;
    t_gobj *y;
    t_canvas *canvas = glist_getcanvas(x);
    canvas_makefilename(canvas, filename->s_name, buf, PD_STRING);
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);
    
    b = glist_writetobinbuf(x, 1);
    if (b)
    {
        if (binbuf_write(b, buf, "", cr))
            post_error ("%s: write failed", filename->s_name);
        binbuf_free(b);
    }
}

/* ------ routines to save and restore canvases (patches) recursively. ----*/

    /* save to a binbuf, called recursively; cf. canvas_savetofile() which
    saves the document, and is only called on root canvases. */
static void canvas_saveto(t_canvas *x, t_binbuf *b)
{
    t_gobj *y;
    t_linetraverser t;
    t_outconnect *oc;
        /* subpatch */
    if (x->gl_owner && !x->gl_env)
    {
        /* have to go to original binbuf to find out how we were named. */
        t_binbuf *bz = binbuf_new();
        t_symbol *patchsym;
        binbuf_addbinbuf(bz, x->gl_obj.te_binbuf);
        patchsym = atom_getSymbolAtIndex(1, binbuf_getnatom(bz), binbuf_getvec(bz));
        binbuf_free(bz);
        binbuf_addv(b, "ssiiiisi;", gensym("#N"), gensym("canvas"),
            (int)(x->gl_screenx1),
            (int)(x->gl_screeny1),
            (int)(x->gl_screenx2 - x->gl_screenx1),
            (int)(x->gl_screeny2 - x->gl_screeny1),
            (patchsym != &s_ ? patchsym: gensym("(subpatch)")),
            x->gl_mapped);
    }
        /* root or abstraction */
    else 
    {
        binbuf_addv(b, "ssiiiii;", gensym("#N"), gensym("canvas"),
            (int)(x->gl_screenx1),
            (int)(x->gl_screeny1),
            (int)(x->gl_screenx2 - x->gl_screenx1),
            (int)(x->gl_screeny2 - x->gl_screeny1),
                (int)x->gl_font);
        canvas_savedeclarationsto(x, b);
    }
    for (y = x->gl_list; y; y = y->g_next)
        gobj_save(y, b);

    linetraverser_start(&t, x);
    while (oc = linetraverser_next(&t))
    {
        int srcno = canvas_getindex(x, &t.tr_ob->te_g);
        int sinkno = canvas_getindex(x, &t.tr_ob2->te_g);
        binbuf_addv(b, "ssiiii;", gensym("#X"), gensym("connect"),
            srcno, t.tr_outno, sinkno, t.tr_inno);
    }
        /* unless everything is the default (as in ordinary subpatches)
        print out a "coords" message to set up the coordinate systems */
    if (x->gl_isgraph || x->gl_x1 || x->gl_y1 ||
        x->gl_x2 != 1 ||  x->gl_y2 != 1 || x->gl_pixwidth || x->gl_pixheight)
    {
        if (x->gl_isgraph && x->gl_goprect)
                /* if we have a graph-on-parent rectangle, we're new style.
                The format is arranged so
                that old versions of Pd can at least do something with it. */
            binbuf_addv(b, "ssfffffffff;", gensym("#X"), gensym("coords"),
                x->gl_x1, x->gl_y1,
                x->gl_x2, x->gl_y2,
                (t_float)x->gl_pixwidth, (t_float)x->gl_pixheight,
                (t_float)((x->gl_hidetext)?2.:1.),
                (t_float)x->gl_xmargin, (t_float)x->gl_ymargin); 
                    /* otherwise write in 0.38-compatible form */
        else binbuf_addv(b, "ssfffffff;", gensym("#X"), gensym("coords"),
                x->gl_x1, x->gl_y1,
                x->gl_x2, x->gl_y2,
                (t_float)x->gl_pixwidth, (t_float)x->gl_pixheight,
                (t_float)x->gl_isgraph);
    }
}

    /* call this recursively to collect all the template names for
    a canvas or for the selection. */
static void canvas_collecttemplatesfor(t_canvas *x, int *ntemplatesp,
    t_symbol ***templatevecp, int wholething)
{
    t_gobj *y;

    for (y = x->gl_list; y; y = y->g_next)
    {
        if ((pd_class(&y->g_pd) == scalar_class) &&
            (wholething || glist_isselected(x, y)))
                canvas_addtemplatesforscalar(((t_scalar *)y)->sc_template,
                    ((t_scalar *)y)->sc_vector,  ntemplatesp, templatevecp);
        else if ((pd_class(&y->g_pd) == canvas_class) &&
            (wholething || glist_isselected(x, y)))
                canvas_collecttemplatesfor((t_canvas *)y,
                    ntemplatesp, templatevecp, 1);
    }
}

    /* save the templates needed by a canvas to a binbuf. */
static void canvas_savetemplatesto(t_canvas *x, t_binbuf *b, int wholething)
{
    t_symbol **templatevec = PD_MEMORY_GET(0);
    int i, ntemplates = 0;
    t_gobj *y;
    canvas_collecttemplatesfor(x, &ntemplates, &templatevec, wholething);
    for (i = 0; i < ntemplates; i++)
    {
        t_template *template = template_findbyname(templatevec[i]);
        int j, m = template->t_n;
        if (!template)
        {
            PD_BUG;
            continue;
        }
            /* drop "pd-" prefix from template symbol to print */
        binbuf_addv(b, "sss", &s__N, gensym("struct"),
            gensym(templatevec[i]->s_name + 3));
        for (j = 0; j < m; j++)
        {
            t_symbol *type;
            switch (template->t_vec[j].ds_type)
            {
                case DATA_FLOAT: type = &s_float; break;
                case DATA_SYMBOL: type = &s_symbol; break;
                case DATA_ARRAY: type = gensym("array"); break;
                case DATA_TEXT: type = gensym("text"); break;
                default: type = &s_float; PD_BUG;
            }
            if (template->t_vec[j].ds_type == DATA_ARRAY)
                binbuf_addv(b, "sss", type, template->t_vec[j].ds_name,
                    gensym(template->t_vec[j].ds_arraytemplate->s_name + 3));
            else binbuf_addv(b, "ss", type, template->t_vec[j].ds_name);
        }
        binbuf_addsemi(b);
    }
}

void canvas_reload(t_symbol *name, t_symbol *dir, t_gobj *except);

    /* save a "root" canvas to a file; cf. canvas_saveto() which saves the
    body (and which is called recursively.) */
static void canvas_savetofile(t_canvas *x, t_symbol *filename, t_symbol *dir,
    float fdestroy)
{
    t_binbuf *b = binbuf_new();
    canvas_savetemplatesto(x, b, 1);
    canvas_saveto(x, b);
    if (binbuf_write(b, filename->s_name, dir->s_name, 0)) { /* sys_ouch */ }
    else {
            /* if not an abstraction, reset title bar and directory */ 
        if (!x->gl_owner)
        {
            canvas_rename(x, filename, dir);
            /* update window list in case Save As changed the window name */
            canvas_updatewindowlist(); 
        }
        post("saved to: %s/%s", dir->s_name, filename->s_name);
        canvas_dirty(x, 0);
        canvas_reload(filename, dir, &x->gl_obj.te_g);
        if (fdestroy != 0)
            pd_vMessage(&x->gl_obj.te_g.g_pd, gensym("menuclose"), "f", 1.);
    }
    binbuf_free(b);
}

static void canvas_menusaveas(t_canvas *x, float fdestroy)
{
    t_canvas *x2 = canvas_getroot(x);
    sys_vgui("::ui_file::saveAs .x%lx {%s} {%s} %d\n", x2,
        x2->gl_name->s_name, canvas_getdir(x2)->s_name, (fdestroy != 0));
}

static void canvas_menusave(t_canvas *x, float fdestroy)
{
    t_canvas *x2 = canvas_getroot(x);
    char *name = x2->gl_name->s_name;
    if (*name && strncmp(name, "Untitled", 8)
            && (strlen(name) < 4 || strcmp(name + strlen(name)-4, ".pat")
                || strcmp(name + strlen(name)-4, ".mxt")))
    {
        canvas_savetofile(x2, x2->gl_name, canvas_getdir(x2), fdestroy);
    }
    else canvas_menusaveas(x2, fdestroy);
}

void g_readwrite_setup(void)
{
    class_addMethod(canvas_class, (t_method)glist_write,
        gensym("write"), A_SYMBOL, A_DEFSYMBOL, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_read,
        gensym("read"), A_SYMBOL, A_DEFSYMBOL, A_NULL);
    class_addMethod(canvas_class, (t_method)glist_mergefile,
        gensym("mergefile"), A_SYMBOL, A_DEFSYMBOL, A_NULL);
    class_addMethod(canvas_class, (t_method)canvas_savetofile,
        gensym("savetofile"), A_SYMBOL, A_SYMBOL, A_DEFFLOAT, 0);
    class_addMethod(canvas_class, (t_method)canvas_saveto,
        gensym("saveto"), A_CANT, 0);
/* ------------------ from the menu ------------------------- */
    class_addMethod(canvas_class, (t_method)canvas_menusave,
        gensym("menusave"), A_DEFFLOAT, 0);
    class_addMethod(canvas_class, (t_method)canvas_menusaveas,
        gensym("menusaveas"), A_DEFFLOAT, 0);
}

void canvas_readwrite_for_class(t_class *c)
{
    class_addMethod(c, (t_method)canvas_menusave,
        gensym("menusave"), 0);
    class_addMethod(c, (t_method)canvas_menusaveas,
        gensym("menusaveas"), 0);
}
