
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

extern t_class *scalar_class;
extern t_class *canvas_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int canvas_scanbinbuf(int natoms, t_atom *vec, int *p_indexout, int *p_next)
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

static void canvas_readerror (int natoms, t_atom *vec, int message, int nline, char *s)
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
            t_buffer *z = buffer_new();
            int first = *p_nextmsg, last;
            for (last = first; last < natoms && vec[last].a_type != A_SEMICOLON;
                last++);
            buffer_deserialize(z, last-first, vec+first);
            buffer_append(w[i].w_buffer, buffer_size(z), buffer_atoms(z));
            buffer_free(z);
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
    int wasvis = canvas_isMapped(x);

    if (nextmsg >= natoms || vec[nextmsg].a_type != A_SYMBOL)
    {
        if (nextmsg < natoms)
            post("stopping early: type %d", vec[nextmsg].a_type);
        *p_nextmsg = natoms;
        return (0);
    }
    templatesym = canvas_makeBindSymbol(vec[nextmsg].a_w.w_symbol);
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
        canvas_getView(x)->gl_isMapped = 0;
    }
    canvas_addObject (x, &sc->sc_g);
    
    nline = canvas_scanbinbuf(natoms, vec, &message, p_nextmsg);
    glist_readatoms(x, natoms, vec, p_nextmsg, templatesym, sc->sc_vector, 
        nline, vec + message);
    if (wasvis)
    {
            /* reset vis flag as before */
        canvas_getView(x)->gl_isMapped = 1;
        gobj_visibilityChanged(&sc->sc_g, x, 1);
    }
    if (selectit)
    {
        canvas_selectObject(x, &sc->sc_g);
    }
    return (1);
}

void glist_readfrombinbuf(t_glist *x, t_buffer *b, char *filename, int selectem)
{
    t_glist *canvas = canvas_getView(x);
    int cr = 0, natoms, nline, message, nextmsg = 0, i, j, nitems;
    t_atom *vec;
    t_gobj *gobj;

    natoms = buffer_size(b);
    vec = buffer_atoms(b);

    
            /* check for file type */
    nline = canvas_scanbinbuf(natoms, vec, &message, &nextmsg);
    if (nline != 1 && vec[message].a_type != A_SYMBOL &&
        strcmp(vec[message].a_w.w_symbol->s_name, "data"))
    {
        post_error ("%s: file apparently of wrong type", filename);
        buffer_free(b);
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
        templatesym = canvas_makeBindSymbol(vec[message + 1].a_w.w_symbol);
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
            PD_MEMORY_FREE(templateargs);
            return;
        }
        newtemplate = template_new(templatesym, ntemplateargs, templateargs);
        PD_MEMORY_FREE(templateargs);
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
    t_buffer *b = buffer_new();
    t_glist *canvas = canvas_getView(x);
    int wasvis = canvas_isMapped(canvas);
    int cr = 0, natoms, nline, message, nextmsg = 0, i, j;
    t_atom *vec;

    /*if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name); */
    
    if (buffer_read(b, filename->s_name, canvas))
    {
        post_error ("read failed");
        buffer_free(b);
        return;
    }
    if (wasvis)
        canvas_visible(canvas, 0);
    if (clearme)
        glist_clear(x);
    glist_readfrombinbuf(x, b, filename->s_name, 0);
    if (wasvis)
        canvas_visible(canvas, 1);
    buffer_free(b);
}

void canvas_read(t_glist *x, t_symbol *filename, t_symbol *format)
{
    glist_doread(x, filename, format, 1);
}

void canvas_merge (t_glist *x, t_symbol *filename, t_symbol *format)
{
    glist_doread(x, filename, format, 0);
}

    /* read text from a "properties" window, called from a guistub set
    up in scalar_properties().  We try to restore the object; if successful
    we either copy the data from the new scalar to the old one in place
    (if their templates match) or else delete the old scalar and put the new
    thing in its place on the list. */
void canvas_dataproperties(t_glist *x, t_scalar *sc, t_buffer *b)
{
    int ntotal, nnew, scindex;
    t_gobj *y, *y2 = 0, *newone, *oldone = 0;
    t_template *template;
    
    // glist_noselect(x); /* FIX. */
    
    for (y = x->gl_graphics, ntotal = 0, scindex = -1; y; y = y->g_next)
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
        for (y = x->gl_graphics, nnew = 1; y2 = y->g_next;
            y = y2, nnew++)
                if (nnew == ntotal)
        {
            newone = y2;
            gobj_visibilityChanged(newone, x, 0);
            y->g_next = y2->g_next;
            break;    
        }
    }
    else gobj_visibilityChanged((newone = x->gl_graphics), x, 0), x->gl_graphics = newone->g_next;
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
            

        
        /* // FIX //
        int i;
        for (i = 0; i < template->t_n; i++)
        {
            t_word w = ((t_scalar *)newone)->sc_vec[i];
            ((t_scalar *)newone)->sc_vec[i] = ((t_scalar *)newone)->sc_vec[i];
            ((t_scalar *)newone)->sc_vec[i] = w;
        }
        */
        
        pd_free(&newone->g_pd);
        
        if (canvas_isMapped(x))
        {
            gobj_visibilityChanged(oldone, x, 0);
            gobj_visibilityChanged(oldone, x, 1);
        }
    }
    else
    {
            /* delete old one; put new one where the old one was on glist */
        canvas_removeObject(x, oldone);
        if (scindex > 0)
        {
            for (y = x->gl_graphics, nnew = 1; y;
                y = y->g_next, nnew++)
                    if (nnew == scindex || !y->g_next)
            {
                newone->g_next = y->g_next;
                y->g_next = newone;
                goto didit;
            }
            PD_BUG;
        }
        else newone->g_next = x->gl_graphics, x->gl_graphics = newone;
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

static void glist_writelist(t_gobj *y, t_buffer *b);

    /* save a text object to a binbuf for a file or copy buf */
static void binbuf_savetext(t_buffer *bfrom, t_buffer *bto)
{
    int k, n = buffer_size(bfrom);
    t_atom *ap = buffer_atoms(bfrom), at;
    for (k = 0; k < n; k++)
    {
        if (ap[k].a_type == A_FLOAT ||
            ap[k].a_type == A_SYMBOL &&
                !strchr(ap[k].a_w.w_symbol->s_name, ';') &&
                !strchr(ap[k].a_w.w_symbol->s_name, ',') &&
                !strchr(ap[k].a_w.w_symbol->s_name, '$'))
                    buffer_append(bto, 1, &ap[k]);
        else
        {
            char buf[PD_STRING+1];
            atom_toString(&ap[k], buf, PD_STRING);
            SET_SYMBOL(&at, gensym (buf));
            buffer_append(bto, 1, &at);
        }
    }
    buffer_appendSemicolon(bto);
}

void canvas_writescalar(t_symbol *templatesym, t_word *w, t_buffer *b,
    int amarrayelement)
{
    t_dataslot *ds;
    t_template *template = template_findbyname(templatesym);
    t_atom *a = (t_atom *)PD_MEMORY_GET(0);
    int i, n = template->t_n, natom = 0;
    if (!amarrayelement)
    {
        t_atom templatename;
        SET_SYMBOL(&templatename, gensym (templatesym->s_name + 3));
        buffer_append(b, 1, &templatename);
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
    buffer_append(b, natom, a);
    buffer_appendSemicolon(b);
    PD_MEMORY_FREE(a);
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
            buffer_appendSemicolon(b);
        }
        else if (template->t_vec[i].ds_type == DATA_TEXT)
            binbuf_savetext(w[i].w_buffer, b);
    }
}

static void glist_writelist(t_gobj *y, t_buffer *b)
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
t_buffer *glist_writetobinbuf(t_glist *x, int wholething)
{
    int i;
    t_symbol **templatevec = PD_MEMORY_GET(0);
    int ntemplates = 0;
    t_gobj *y;
    t_buffer *b = buffer_new();

    for (y = x->gl_graphics; y; y = y->g_next)
    {
        if ((pd_class(&y->g_pd) == scalar_class) &&
            (wholething || canvas_isObjectSelected(x, y)))
        {
            canvas_addtemplatesforscalar(((t_scalar *)y)->sc_template,
                ((t_scalar *)y)->sc_vector,  &ntemplates, &templatevec);
        }
    }
    buffer_vAppend(b, "s;", sym_data);
    for (i = 0; i < ntemplates; i++)
    {
        t_template *template = template_findbyname(templatevec[i]);
        int j, m = template->t_n;
            /* drop "pd-" prefix from template symbol to print it: */
        buffer_vAppend(b, "ss;", sym_template,
            gensym (templatevec[i]->s_name + 3));
        for (j = 0; j < m; j++)
        {
            t_symbol *type;
            switch (template->t_vec[j].ds_type)
            {
                case DATA_FLOAT: type = &s_float; break;
                case DATA_SYMBOL: type = &s_symbol; break;
                case DATA_ARRAY: type = sym_array; break;
                case DATA_TEXT: type = &s_list; break;
                default: type = &s_float; PD_BUG;
            }
            if (template->t_vec[j].ds_type == DATA_ARRAY)
                buffer_vAppend(b, "sss;", type, template->t_vec[j].ds_name,
                    gensym (template->t_vec[j].ds_arraytemplate->s_name + 3));
            else buffer_vAppend(b, "ss;", type, template->t_vec[j].ds_name);
        }
        buffer_appendSemicolon(b);
    }
    buffer_appendSemicolon(b);
        /* now write out the objects themselves */
    for (y = x->gl_graphics; y; y = y->g_next)
    {
        if ((pd_class(&y->g_pd) == scalar_class) &&
            (wholething || canvas_isObjectSelected(x, y)))
        {
            canvas_writescalar(((t_scalar *)y)->sc_template,
                ((t_scalar *)y)->sc_vector,  b, 0);
        }
    }
    return (b);
}

void canvas_write(t_glist *x, t_symbol *filename, t_symbol *format)
{
    int cr = 0, i;
    t_buffer *b;
    char buf[PD_STRING];
    t_symbol **templatevec = PD_MEMORY_GET(0);
    int ntemplates = 0;
    t_gobj *y;
    t_glist *canvas = canvas_getView(x);
    canvas_makeFilePath(canvas, filename->s_name, buf, PD_STRING);
    if (!strcmp(format->s_name, "cr"))
        cr = 1;
    else if (*format->s_name)
        post_error ("qlist_read: unknown flag: %s", format->s_name);
    
    b = glist_writetobinbuf(x, 1);
    if (b)
    {
        if (buffer_write(b, buf, ""))
            post_error ("%s: write failed", filename->s_name);
        buffer_free(b);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_findTemplates (t_glist *glist, int *n, t_symbol ***v)
{
    t_gobj *y = NULL;

    for (y = glist->gl_graphics; y; y = y->g_next) {
    //
    if (pd_class (y) == scalar_class) {
        canvas_addtemplatesforscalar (cast_scalar (y)->sc_template, cast_scalar (y)->sc_vector, n, v);
    } else if (pd_class (y) == canvas_class) {
        canvas_findTemplates (cast_glist (y), n, v);
    }
    //
    }
}

void canvas_serializeTemplates (t_glist *glist, t_buffer *b)
{
    t_symbol **v = PD_MEMORY_GET (0);
    int i, n = 0;
    
    canvas_findTemplates (glist, &n, &v);
    
    for (i = 0; i < n; i++) {
    //
    t_template *template = template_findbyname(v[i]);
    int j, m = template->t_n;
    if (!template)
    {
        PD_BUG;
        continue;
    }
        /* drop "pd-" prefix from template symbol to print */
    buffer_vAppend(b, "sss", sym___hash__N, sym_struct,
        gensym (v[i]->s_name + 3));
    for (j = 0; j < m; j++)
    {
        t_symbol *type;
        switch (template->t_vec[j].ds_type)
        {
            case DATA_FLOAT: type = &s_float; break;
            case DATA_SYMBOL: type = &s_symbol; break;
            case DATA_ARRAY: type = sym_array; break;
            case DATA_TEXT: type = sym_text; break;
            default: type = &s_float; PD_BUG;
        }
        if (template->t_vec[j].ds_type == DATA_ARRAY)
            buffer_vAppend(b, "sss", type, template->t_vec[j].ds_name,
                gensym (template->t_vec[j].ds_arraytemplate->s_name + 3));
        else buffer_vAppend(b, "ss", type, template->t_vec[j].ds_name);
    }
    buffer_appendSemicolon(b);
    //
    }
    
    PD_MEMORY_FREE (v);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
