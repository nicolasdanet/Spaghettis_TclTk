
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* This file defines Text objects which traverse data contained in scalars
and arrays:

pointer - point to an object belonging to a template
get -     get numeric fields
set -     change numeric fields
element - get an array element
getsize - get the size of an array
setsize - change the size of an array
append -  add an element to a list
sublist - get a pointer into a list which is an element of another scalar

*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>      /* for read/write to files */
#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"

extern t_class *scalar_class;
extern t_class *canvas_class;

/*********  random utility function to find a binbuf in a datum */

t_buffer *pointertobinbuf(t_pd *x, t_gpointer *gp, t_symbol *s,
    const char *fname)
{
    t_symbol *templatesym = gpointer_gettemplatesym(gp), *arraytype;
    t_template *template;
    int onset, type;
    t_buffer *b;
    t_gmaster *gs = gp->gp_master;
    t_word *vec;
    if (!templatesym)
    {
        post_error ("%s: bad pointer", fname);
        return (0);
    }
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("%s: couldn't find template %s", fname,
            templatesym->s_name);
        return (0);
    }
    if (!template_find_field(template, s, &onset, &type, &arraytype))
    {
        post_error ("%s: %s.%s: no such field", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    if (type != DATA_TEXT)
    {
        post_error ("%s: %s.%s: not a list", fname,
            templatesym->s_name, s->s_name);
        return (0);
    }
    if (gs->gm_type == POINTER_ARRAY)
        vec = gp->gp_un.gp_w;
    else vec = gp->gp_un.gp_scalar->sc_vector;
    return (vec[onset].w_buffer);
}

    /* templates are named using the name-bashing by which canvases bind
    thenselves, with a leading "pd-".  LATER see if we can have templates
    occupy their real names.  Meanwhile, if a template has an empty name
    or is named "-" (as when passed as a "-" argument to "get", etc.), just
    return &s_; objects should check for this and allow it as a wild
    card when appropriate. */
static t_symbol *template_getbindsym(t_symbol *s)
{
    if (!*s->s_name || !strcmp(s->s_name, "-"))
        return (&s_);
    else return (canvas_makeBindSymbol(s));
}

/* ---------------------- pointers ----------------------------- */

static t_class *ptrobj_class;

typedef struct 
{
    t_symbol *to_type;
    t_outlet *to_outlet;
} t_typedout;

typedef struct _ptrobj
{
    t_object x_obj;
    t_gpointer x_gp;
    t_typedout *x_typedout;
    int x_ntypedout;
    t_outlet *x_otherout;
    t_outlet *x_bangout;
} t_ptrobj;

static void *ptrobj_new(t_symbol *classname, int argc, t_atom *argv)
{
    t_ptrobj *x = (t_ptrobj *)pd_new(ptrobj_class);
    t_typedout *to;
    int n;
    gpointer_initialize(&x->x_gp);
    x->x_typedout = to = (t_typedout *)PD_MEMORY_GET(argc * sizeof (*to));
    x->x_ntypedout = n = argc;
    for (; n--; to++)
    {
        to->to_outlet = outlet_new(&x->x_obj, &s_pointer);
        to->to_type = template_getbindsym(atom_getSymbol(argv++));
    }
    x->x_otherout = outlet_new(&x->x_obj, &s_pointer);
    x->x_bangout = outlet_new(&x->x_obj, &s_bang);
    inlet_newPointer(&x->x_obj, &x->x_gp);
    return (x);
}

static void ptrobj_traverse(t_ptrobj *x, t_symbol *s)
{
    t_glist *glist = (t_glist *)pd_findByClass(s, canvas_class);
    if (glist) gpointer_setScalar(&x->x_gp, glist, 0);
    else { post_error (x, "pointer: list '%s' not found", s->s_name); }
}

static void ptrobj_vnext(t_ptrobj *x, t_float f)
{
    t_gobj *gobj;
    t_gpointer *gp = &x->x_gp;
    t_gmaster *gs = gp->gp_master;
    t_glist *glist;
    int wantselected = (f != 0);

    if (!gs)
    {
        post_error ("ptrobj_next: no current pointer");
        return;
    }
    if (gs->gm_type != POINTER_GLIST)
    {
        post_error ("ptrobj_next: lists only, not arrays");
        return;
    }
    glist = gs->gm_un.gm_glist;
    if (glist->gl_magic != gp->gp_magic)
    {
        post_error ("ptrobj_next: stale pointer");
        return;
    }
    if (wantselected && !canvas_isMapped(glist))
    {
        post_error ("ptrobj_vnext: next-selected only works for a visible window");
        return;
    }
    gobj = &gp->gp_un.gp_scalar->sc_g;
    
    if (!gobj) gobj = glist->gl_graphics;
    else gobj = gobj->g_next;
    while (gobj && ((pd_class(&gobj->g_pd) != scalar_class) ||
        (wantselected && !canvas_isObjectSelected(glist, gobj))))
            gobj = gobj->g_next;
    
    if (gobj)
    {
        t_typedout *to;
        int n;
        t_scalar *sc = (t_scalar *)gobj;
        t_symbol *templatesym = sc->sc_template;

        gp->gp_un.gp_scalar = sc; 
        for (n = x->x_ntypedout, to = x->x_typedout; n--; to++)
        {
            if (to->to_type == templatesym)
            {
                outlet_pointer(to->to_outlet, &x->x_gp);
                return;
            }
        }
        outlet_pointer(x->x_otherout, &x->x_gp);
    }
    else
    {
        gpointer_unset(gp);
        outlet_bang(x->x_bangout);
    }
}

static void ptrobj_next(t_ptrobj *x)
{
    ptrobj_vnext(x, 0);
}

    /* send a message to the window containing the object pointed to */
static void ptrobj_sendwindow(t_ptrobj *x, t_symbol *s, int argc, t_atom *argv)
{
    t_scalar *sc;
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    t_glist *glist;
    t_pd *canvas;
    t_gmaster *gs;
    if (!gpointer_check(&x->x_gp, 1))
    {
        post_error ("send-window: empty pointer");
        return;
    }
    gs = x->x_gp.gp_master;
    if (gs->gm_type == POINTER_GLIST)
        glist = gs->gm_un.gm_glist;  
    else
    {
        t_array *owner_array = gs->gm_un.gm_array;
        while (owner_array->a_gpointer.gp_master->gm_type == POINTER_ARRAY)
            owner_array = owner_array->a_gpointer.gp_master->gm_un.gm_array;
        glist = owner_array->a_gpointer.gp_master->gm_un.gm_glist;  
    }
    canvas = (t_pd *)canvas_getView(glist);
    if (argc && argv->a_type == A_SYMBOL)
        pd_message(canvas, argv->a_w.w_symbol, argc-1, argv+1);
    else { post_error ("send-window: no message?"); }
}


    /* send the pointer to the named object */
static void ptrobj_send(t_ptrobj *x, t_symbol *s)
{
    if (!s->s_thing)
        post_error ("%s: no such object", s->s_name);
    else if (!gpointer_check(&x->x_gp, 1))
        post_error ("pointer_send: empty pointer");
    else pd_pointer(s->s_thing, &x->x_gp);
}

static void ptrobj_bang(t_ptrobj *x)
{
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    if (!gpointer_check(&x->x_gp, 1))
    {
        post_error ("pointer_bang: empty pointer");
        return;
    }
    templatesym = gpointer_gettemplatesym(&x->x_gp);
    for (n = x->x_ntypedout, to = x->x_typedout; n--; to++)
    {
        if (to->to_type == templatesym)
        {
            outlet_pointer(to->to_outlet, &x->x_gp);
            return;
        }
    }
    outlet_pointer(x->x_otherout, &x->x_gp);
}


static void ptrobj_pointer(t_ptrobj *x, t_gpointer *gp)
{
    gpointer_unset(&x->x_gp);
    gpointer_copy(gp, &x->x_gp);
    ptrobj_bang(x);
}


static void ptrobj_rewind(t_ptrobj *x)
{
    t_scalar *sc;
    t_symbol *templatesym;
    int n;
    t_typedout *to;
    t_glist *glist;
    t_pd *canvas;
    t_gmaster *gs;
    if (!gpointer_check(&x->x_gp, 1))
    {
        post_error ("pointer_rewind: empty pointer");
        return;
    }
    gs = x->x_gp.gp_master;
    if (gs->gm_type != POINTER_GLIST)
    {
        post_error ("pointer_rewind: sorry, unavailable for arrays");
        return;
    }
    glist = gs->gm_un.gm_glist;  
    gpointer_setScalar(&x->x_gp, glist, 0);
    ptrobj_bang(x);
}

static void ptrobj_free(t_ptrobj *x)
{
    PD_MEMORY_FREE(x->x_typedout);
    gpointer_unset(&x->x_gp);
}

static void ptrobj_setup(void)
{
    ptrobj_class = class_new(sym_pointer, (t_newmethod)ptrobj_new,
        (t_method)ptrobj_free, sizeof(t_ptrobj), 0, A_GIMME, 0);
    class_addMethod(ptrobj_class, (t_method)ptrobj_next, sym_next, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_send, sym_send, 
        A_SYMBOL, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_traverse, sym_traverse,
        A_SYMBOL, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_vnext, sym_vnext, /* LEGACY !!! */
        A_DEFFLOAT, 0); 
    class_addMethod(ptrobj_class, (t_method)ptrobj_sendwindow,
        sym_send__dash__window, A_GIMME, 0);                    /* LEGACY !!! */
    class_addMethod(ptrobj_class, (t_method)ptrobj_rewind,
        sym_rewind, 0); 
    class_addPointer(ptrobj_class, ptrobj_pointer); 
    class_addBang(ptrobj_class, ptrobj_bang); 
}

/* ---------------------- get ----------------------------- */

static t_class *get_class;

typedef struct _getvariable
{
    t_symbol *gv_sym;
    t_outlet *gv_outlet;
} t_getvariable;

typedef struct _get
{
    t_object x_obj;
    t_symbol *x_templatesym;
    int x_nout;
    t_getvariable *x_variables;
} t_get;

static void *get_new(t_symbol *why, int argc, t_atom *argv)
{
    t_get *x = (t_get *)pd_new(get_class);
    int varcount, i;
    t_atom at, *varvec;
    t_getvariable *sp;

    x->x_templatesym = template_getbindsym(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_getvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nout = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        sp->gv_outlet = outlet_new(&x->x_obj, 0);
            /* LATER connect with the template and set the outlet's type
            correctly.  We can't yet guarantee that the template is there
            before we hit this routine. */
    }
    return (x);
}

static void get_set(t_get *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nout != 1)
        post_error ("get: cannot set multiple fields.");
    else
    {
        x->x_templatesym = template_getbindsym(templatesym); 
        x->x_variables->gv_sym = field;
    }
}

static void get_pointer(t_get *x, t_gpointer *gp)
{
    int nitems = x->x_nout, i;
    t_symbol *templatesym;
    t_template *template;
    t_gmaster *gs = gp->gp_master;
    t_word *vec; 
    t_getvariable *vp;

    if (!gpointer_check(gp, 0))
    {
        post_error ("get: stale or empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) != gpointer_gettemplatesym(gp))
        {
            post_error ("get %s: got wrong template (%s)",
                templatesym->s_name, gpointer_gettemplatesym(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_gettemplatesym(gp);
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("get: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (gs->gm_type == POINTER_ARRAY) vec = gp->gp_un.gp_w;
    else vec = gp->gp_un.gp_scalar->sc_vector;
    for (i = nitems - 1, vp = x->x_variables + i; i >= 0; i--, vp--)
    {
        int onset, type;
        t_symbol *arraytype;
        if (template_find_field(template, vp->gv_sym, &onset, &type, &arraytype))
        {
            if (type == DATA_FLOAT)
                outlet_float(vp->gv_outlet,
                    *(t_float *)(((char *)vec) + onset));
            else if (type == DATA_SYMBOL)
                outlet_symbol(vp->gv_outlet,
                    *(t_symbol **)(((char *)vec) + onset));
            else post_error ("get: %s.%s is not a number or symbol",
                    template->tp_symbol->s_name, vp->gv_sym->s_name);
        }
        else post_error ("get: %s.%s: no such field",
            template->tp_symbol->s_name, vp->gv_sym->s_name);
    }
}

static void get_free(t_get *x)
{
    PD_MEMORY_FREE(x->x_variables);
}

static void get_setup(void)
{
    get_class = class_new (sym_get, (t_newmethod)get_new,
        (t_method)get_free, sizeof(t_get), 0, A_GIMME, 0);
    class_addPointer(get_class, get_pointer); 
    class_addMethod(get_class, (t_method)get_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

/* ---------------------- set ----------------------------- */

static t_class *set_class;

typedef struct _setvariable
{
    t_symbol *gv_sym;
    union word gv_w;
} t_setvariable;

typedef struct _set
{
    t_object x_obj;
    t_gpointer x_gp;
    t_symbol *x_templatesym;
    int x_nin;
    int x_issymbol;
    t_setvariable *x_variables;
} t_set;

static void *set_new(t_symbol *why, int argc, t_atom *argv)
{
    t_set *x = (t_set *)pd_new(set_class);
    int i, varcount;
    t_setvariable *sp;
    t_atom at, *varvec;
    if (argc && (argv[0].a_type == A_SYMBOL) &&
        !strcmp(argv[0].a_w.w_symbol->s_name, "-symbol"))
    {
        x->x_issymbol = 1;
        argc--;
        argv++;
    }
    else x->x_issymbol = 0;
    x->x_templatesym = template_getbindsym(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_setvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nin = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        if (x->x_issymbol)
            sp->gv_w.w_symbol = &s_;
        else sp->gv_w.w_float = 0;
        if (i)
        {
            if (x->x_issymbol)
                inlet_newSymbol(&x->x_obj, &sp->gv_w.w_symbol);
            else inlet_newFloat(&x->x_obj, &sp->gv_w.w_float);
        }
    }
    inlet_newPointer(&x->x_obj, &x->x_gp);
    gpointer_initialize(&x->x_gp);
    return (x);
}

static void set_set(t_set *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nin != 1)
        post_error ("set: cannot set multiple fields.");
    else
    {
       x->x_templatesym = template_getbindsym(templatesym); 
       x->x_variables->gv_sym = field;
       if (x->x_issymbol)
           x->x_variables->gv_w.w_symbol = &s_;
       else
           x->x_variables->gv_w.w_float = 0;
    }
}

static void set_bang(t_set *x)
{
    int nitems = x->x_nin, i;
    t_symbol *templatesym;
    t_template *template;
    t_setvariable *vp;
    t_gpointer *gp = &x->x_gp;
    t_gmaster *gs = gp->gp_master;
    t_word *vec;
    if (!gpointer_check(gp, 0))
    {
        post_error ("set: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) != gpointer_gettemplatesym(gp))
        {
            post_error ("set %s: got wrong template (%s)",
                templatesym->s_name, gpointer_gettemplatesym(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_gettemplatesym(gp);
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("set: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!nitems)
        return;
    if (gs->gm_type == POINTER_ARRAY)
        vec = gp->gp_un.gp_w;
    else vec = gp->gp_un.gp_scalar->sc_vector;
    if (x->x_issymbol)
        for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
            template_setsymbol(template, vp->gv_sym, vec, vp->gv_w.w_symbol, 1);
    else for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
        template_setfloat(template, vp->gv_sym, vec, vp->gv_w.w_float, 1);
    if (gs->gm_type == POINTER_GLIST)
        scalar_redraw(gp->gp_un.gp_scalar, gs->gm_un.gm_glist);  
    else
    {
        t_array *owner_array = gs->gm_un.gm_array;
        while (owner_array->a_gpointer.gp_master->gm_type == POINTER_ARRAY)
            owner_array = owner_array->a_gpointer.gp_master->gm_un.gm_array;
        scalar_redraw(owner_array->a_gpointer.gp_un.gp_scalar,
            owner_array->a_gpointer.gp_master->gm_un.gm_glist);  
    }
}

static void set_float(t_set *x, t_float f)
{
    if (x->x_nin && !x->x_issymbol)
    {
        x->x_variables[0].gv_w.w_float = f;
        set_bang(x);
    }
    else post_error ("type mismatch or no field specified");
}

static void set_symbol(t_set *x, t_symbol *s)
{
    if (x->x_nin && x->x_issymbol)
    {
        x->x_variables[0].gv_w.w_symbol = s;
        set_bang(x);
    }
    else post_error ("type mismatch or no field specified");
}

static void set_free(t_set *x)
{
    PD_MEMORY_FREE(x->x_variables);
    gpointer_unset(&x->x_gp);
}

static void set_setup(void)
{
    set_class = class_new(sym_set, (t_newmethod)set_new,
        (t_method)set_free, sizeof(t_set), 0, A_GIMME, 0);
    class_addFloat(set_class, set_float); 
    class_addSymbol(set_class, set_symbol); 
    class_addBang(set_class, set_bang); 
    class_addMethod(set_class, (t_method)set_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

/* ---------------------- elem ----------------------------- */

static t_class *elem_class;

typedef struct _elem
{
    t_object x_obj;
    t_symbol *x_templatesym;
    t_symbol *x_fieldsym;
    t_gpointer x_gp;
    t_gpointer x_gparent;
} t_elem;

static void *elem_new(t_symbol *templatesym, t_symbol *fieldsym)
{
    t_elem *x = (t_elem *)pd_new(elem_class);
    x->x_templatesym = template_getbindsym(templatesym);
    x->x_fieldsym = fieldsym;
    gpointer_initialize(&x->x_gp);
    gpointer_initialize(&x->x_gparent);
    inlet_newPointer(&x->x_obj, &x->x_gparent);
    outlet_new(&x->x_obj, &s_pointer);
    return (x);
}

static void elem_set(t_elem *x, t_symbol *templatesym, t_symbol *fieldsym)
{
    x->x_templatesym = template_getbindsym(templatesym);
    x->x_fieldsym = fieldsym;
}

static void elem_float(t_elem *x, t_float f)
{
    int indx = f, nitems, onset;
    t_symbol *templatesym, *fieldsym = x->x_fieldsym, *elemtemplatesym;
    t_template *template;
    t_template *elemtemplate;
    t_gpointer *gparent = &x->x_gparent;
    t_word *w;
    t_array *array;
    int elemsize, type;
    
    if (!gpointer_check(gparent, 0))
    {
        post_error ("element: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) !=
            gpointer_gettemplatesym(gparent))
        {
            post_error ("elem %s: got wrong template (%s)",
                templatesym->s_name, gpointer_gettemplatesym(gparent)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_gettemplatesym(gparent);
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("elem: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (gparent->gp_master->gm_type == POINTER_ARRAY) w = gparent->gp_un.gp_w;
    else w = gparent->gp_un.gp_scalar->sc_vector;
    if (!template)
    {
        post_error ("element: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!template_find_field(template, fieldsym,
        &onset, &type, &elemtemplatesym))
    {
        post_error ("element: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("element: field %s not of type array", fieldsym->s_name);
        return;
    }
    if (!(elemtemplate = template_findbyname(elemtemplatesym)))
    {
        post_error ("element: couldn't find field template %s",
            elemtemplatesym->s_name);
        return;
    }

    elemsize = elemtemplate->tp_size * sizeof(t_word);

    array = *(t_array **)(((char *)w) + onset);

    nitems = array->a_size;
    if (indx < 0) indx = 0;
    if (indx >= nitems) indx = nitems-1;

    gpointer_setWord(&x->x_gp, array, 
        (t_word *)((char *)(array->a_vector) + indx * elemsize));
    outlet_pointer(x->x_obj.te_outlet, &x->x_gp);
}

static void elem_free(t_elem *x, t_gpointer *gp)
{
    gpointer_unset(&x->x_gp);
    gpointer_unset(&x->x_gparent);
}

static void elem_setup(void)
{
    elem_class = class_new(sym_element, (t_newmethod)elem_new,
        (t_method)elem_free, sizeof(t_elem), 0, A_DEFSYMBOL, A_DEFSYMBOL, 0);
    class_addFloat(elem_class, elem_float); 
    class_addMethod(elem_class, (t_method)elem_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

/* ---------------------- getsize ----------------------------- */

static t_class *getsize_class;

typedef struct _getsize
{
    t_object x_obj;
    t_symbol *x_templatesym;
    t_symbol *x_fieldsym;
} t_getsize;

static void *getsize_new(t_symbol *templatesym, t_symbol *fieldsym)
{
    t_getsize *x = (t_getsize *)pd_new(getsize_class);
    x->x_templatesym = template_getbindsym(templatesym);
    x->x_fieldsym = fieldsym;
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

static void getsize_set(t_getsize *x, t_symbol *templatesym, t_symbol *fieldsym)
{
    x->x_templatesym = template_getbindsym(templatesym);
    x->x_fieldsym = fieldsym;
}

static void getsize_pointer(t_getsize *x, t_gpointer *gp)
{
    int nitems, onset, type;
    t_symbol *templatesym, *fieldsym = x->x_fieldsym, *elemtemplatesym;
    t_template *template;
    t_word *w;
    t_array *array;
    int elemsize;
    t_gmaster *gs = gp->gp_master;
    if (!gpointer_check(gp, 0))
    {
        post_error ("getsize: stale or empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) !=
            gpointer_gettemplatesym(gp))
        {
            post_error ("elem %s: got wrong template (%s)",
                templatesym->s_name, gpointer_gettemplatesym(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_gettemplatesym(gp);
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("elem: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!template_find_field(template, fieldsym,
        &onset, &type, &elemtemplatesym))
    {
        post_error ("getsize: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("getsize: field %s not of type array", fieldsym->s_name);
        return;
    }
    if (gs->gm_type == POINTER_ARRAY) w = gp->gp_un.gp_w;
    else w = gp->gp_un.gp_scalar->sc_vector;
    
    array = *(t_array **)(((char *)w) + onset);
    outlet_float(x->x_obj.te_outlet, (t_float)(array->a_size));
}

static void getsize_setup(void)
{
    getsize_class = class_new(sym_getsize, (t_newmethod)getsize_new, 0,
        sizeof(t_getsize), 0, A_DEFSYMBOL, A_DEFSYMBOL, 0);
    class_addPointer(getsize_class, getsize_pointer); 
    class_addMethod(getsize_class, (t_method)getsize_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

/* ---------------------- setsize ----------------------------- */

static t_class *setsize_class;

typedef struct _setsize
{
    t_object x_obj;
    t_symbol *x_templatesym;
    t_symbol *x_fieldsym;
    t_gpointer x_gp;
} t_setsize;

static void *setsize_new(t_symbol *templatesym, t_symbol *fieldsym,
    t_float newsize)
{
    t_setsize *x = (t_setsize *)pd_new(setsize_class);
    x->x_templatesym = template_getbindsym(templatesym);
    x->x_fieldsym = fieldsym;
    gpointer_initialize(&x->x_gp);
    
    inlet_newPointer(&x->x_obj, &x->x_gp);
    return (x);
}

static void setsize_set(t_setsize *x, t_symbol *templatesym, t_symbol *fieldsym)
{
    x->x_templatesym = template_getbindsym(templatesym);
    x->x_fieldsym = fieldsym;
}

static void setsize_float(t_setsize *x, t_float f)
{
    int nitems, onset, type;
    t_symbol *templatesym, *fieldsym = x->x_fieldsym, *elemtemplatesym;
    t_template *template;
    t_template *elemtemplate;
    t_word *w;
    t_atom at;
    t_array *array;
    int elemsize;
    int newsize = f;
    t_gpointer *gp = &x->x_gp;
    t_gmaster *gs = gp->gp_master;
    if (!gpointer_check(&x->x_gp, 0))
    {
        post_error ("setsize: empty pointer");
        return;
    }
    if (*x->x_templatesym->s_name)
    {
        if ((templatesym = x->x_templatesym) !=
            gpointer_gettemplatesym(gp))
        {
            post_error ("elem %s: got wrong template (%s)",
                templatesym->s_name, gpointer_gettemplatesym(gp)->s_name);
            return;
        } 
    }
    else templatesym = gpointer_gettemplatesym(gp);
    if (!(template = template_findbyname(templatesym)))
    {
        post_error ("elem: couldn't find template %s", templatesym->s_name);
        return;
    }

    if (!template_find_field(template, fieldsym,
        &onset, &type, &elemtemplatesym))
    {
        post_error ("setsize: couldn't find array field %s", fieldsym->s_name);
        return;
    }
    if (type != DATA_ARRAY)
    {
        post_error ("setsize: field %s not of type array", fieldsym->s_name);
        return;
    }
    if (gs->gm_type == POINTER_ARRAY) w = gp->gp_un.gp_w;
    else w = gp->gp_un.gp_scalar->sc_vector;

    if (!(elemtemplate = template_findbyname(elemtemplatesym)))
    {
        post_error ("element: couldn't find field template %s",
            elemtemplatesym->s_name);
        return;
    }

    elemsize = elemtemplate->tp_size * sizeof(t_word);

    array = *(t_array **)(((char *)w) + onset);

    if (elemsize != array->a_elementSize) { PD_BUG; }

    nitems = array->a_size;
    if (newsize < 1) newsize = 1;
    if (newsize == nitems) return;
    
        /* erase the array before resizing it.  If we belong to a
        scalar it's easy, but if we belong to an element of another
        array we have to search back until we get to a scalar to erase.
        When graphics updates become queueable this may fall apart... */


    if (gs->gm_type == POINTER_GLIST)
    {
        if (canvas_isMapped(gs->gm_un.gm_glist))
            gobj_visibilityChanged((t_gobj *)(gp->gp_un.gp_scalar), gs->gm_un.gm_glist, 0);  
    }
    else
    {
        t_array *owner_array = gs->gm_un.gm_array;
        while (owner_array->a_gpointer.gp_master->gm_type == POINTER_ARRAY)
            owner_array = owner_array->a_gpointer.gp_master->gm_un.gm_array;
        if (canvas_isMapped(owner_array->a_gpointer.gp_master->gm_un.gm_glist))
            gobj_visibilityChanged((t_gobj *)(owner_array->a_gpointer.gp_un.gp_scalar),
                owner_array->a_gpointer.gp_master->gm_un.gm_glist, 0);  
    }
        /* if shrinking, free the scalars that will disappear */
    if (newsize < nitems)
    {
        char *elem;
        int count;       
        for (elem = ((char *)array->a_vector) + newsize * elemsize,
            count = nitems - newsize; count--; elem += elemsize)
                word_free((t_word *)elem, elemtemplate);
    }
        /* resize the array  */
    array->a_vector = (char *)PD_MEMORY_RESIZE(array->a_vector,
        elemsize * nitems, elemsize * newsize);
    array->a_size = newsize;
        /* if growing, initialize new scalars */
    if (newsize > nitems)
    {
        char *elem;
        int count;       
        for (elem = ((char *)array->a_vector) + nitems * elemsize,
            count = newsize - nitems; count--; elem += elemsize)
                word_init((t_word *)elem, elemtemplate, gp);
    }
        /* invalidate all gpointers into the array */
    array->a_magic++;

    /* redraw again. */
    if (gs->gm_type == POINTER_GLIST)
    {
        if (canvas_isMapped(gs->gm_un.gm_glist))
            gobj_visibilityChanged((t_gobj *)(gp->gp_un.gp_scalar), gs->gm_un.gm_glist, 1);  
    }
    else
    {
        t_array *owner_array = gs->gm_un.gm_array;
        while (owner_array->a_gpointer.gp_master->gm_type == POINTER_ARRAY)
            owner_array = owner_array->a_gpointer.gp_master->gm_un.gm_array;
        if (canvas_isMapped(owner_array->a_gpointer.gp_master->gm_un.gm_glist))
            gobj_visibilityChanged((t_gobj *)(owner_array->a_gpointer.gp_un.gp_scalar),
                owner_array->a_gpointer.gp_master->gm_un.gm_glist, 1);  
    }
}

static void setsize_free(t_setsize *x)
{
    gpointer_unset(&x->x_gp);
}

static void setsize_setup(void)
{
    setsize_class = class_new (sym_setsize, (t_newmethod)setsize_new,
        (t_method)setsize_free, sizeof(t_setsize), 0,
        A_DEFSYMBOL, A_DEFSYMBOL, A_DEFFLOAT, 0);
    class_addFloat(setsize_class, setsize_float);
    class_addMethod(setsize_class, (t_method)setsize_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 

}

/* ---------------------- append ----------------------------- */

static t_class *append_class;

typedef struct _appendvariable
{
    t_symbol *gv_sym;
    t_float gv_f;
} t_appendvariable;

typedef struct _append
{
    t_object x_obj;
    t_gpointer x_gp;
    t_symbol *x_templatesym;
    int x_nin;
    t_appendvariable *x_variables;
} t_append;

static void *append_new(t_symbol *why, int argc, t_atom *argv)
{
    t_append *x = (t_append *)pd_new(append_class);
    int varcount, i;
    t_atom at, *varvec;
    t_appendvariable *sp;

    x->x_templatesym = template_getbindsym(atom_getSymbolAtIndex(0, argc, argv));
    if (argc < 2)
    {
        varcount = 1;
        varvec = &at;
        SET_SYMBOL(&at, &s_);
    }
    else varcount = argc - 1, varvec = argv + 1;
    x->x_variables
        = (t_appendvariable *)PD_MEMORY_GET(varcount * sizeof (*x->x_variables));
    x->x_nin = varcount;
    for (i = 0, sp = x->x_variables; i < varcount; i++, sp++)
    {
        sp->gv_sym = atom_getSymbolAtIndex(i, varcount, varvec);
        sp->gv_f = 0;
        if (i) inlet_newFloat(&x->x_obj, &sp->gv_f);
    }
    inlet_newPointer(&x->x_obj, &x->x_gp);
    outlet_new(&x->x_obj, &s_pointer);
    gpointer_initialize(&x->x_gp);
    return (x);
}

static void append_set(t_append *x, t_symbol *templatesym, t_symbol *field)
{
    if (x->x_nin != 1)
        post_error ("set: cannot set multiple fields.");
    else
    {
       x->x_templatesym = template_getbindsym(templatesym); 
       x->x_variables->gv_sym = field;
       x->x_variables->gv_f = 0;
    }
}

static void append_float(t_append *x, t_float f)
{
    int nitems = x->x_nin, i;
    t_symbol *templatesym = x->x_templatesym;
    t_template *template;
    t_appendvariable *vp;
    t_gpointer *gp = &x->x_gp;
    t_gmaster *gs = gp->gp_master;
    t_word *vec;
    t_scalar *sc, *oldsc;
    t_glist *glist;
    
    if (!templatesym->s_name)
    {
        post_error ("append: no template supplied");
        return;
    }
    template = template_findbyname(templatesym);
    if (!template)
    {
        post_error ("append: couldn't find template %s", templatesym->s_name);
        return;
    }
    if (!gs)
    {
        post_error ("append: no current pointer");
        return;
    }
    if (gs->gm_type != POINTER_GLIST)
    {
        post_error ("append: lists only, not arrays");
        return;
    }
    glist = gs->gm_un.gm_glist;
    if (glist->gl_magic != gp->gp_magic)
    {
        post_error ("append: stale pointer");
        return;
    }
    if (!nitems) return;
    x->x_variables[0].gv_f = f;

    sc = scalar_new(glist, templatesym);
    if (!sc)
    {
        post_error ("%s: couldn't create scalar", templatesym->s_name);
        return;
    }
    oldsc = gp->gp_un.gp_scalar;
    
    if (oldsc)
    {
        sc->sc_g.g_next = oldsc->sc_g.g_next;
        oldsc->sc_g.g_next = &sc->sc_g;
    }
    else
    {
        sc->sc_g.g_next = glist->gl_graphics;
        glist->gl_graphics = &sc->sc_g;
    }

    gp->gp_un.gp_scalar = sc;
    vec = sc->sc_vector;
    for (i = 0, vp = x->x_variables; i < nitems; i++, vp++)
    {
        template_setfloat(template, vp->gv_sym, vec, vp->gv_f, 1);
    }
 
    if (canvas_isMapped(canvas_getView(glist)))
        gobj_visibilityChanged(&sc->sc_g, glist, 1);
    /*  scalar_redraw(sc, glist);  ... have to do 'vis' instead here because
    redraw assumes we're already visible??? ... */

    outlet_pointer(x->x_obj.te_outlet, gp);
}

static void append_free(t_append *x)
{
    PD_MEMORY_FREE(x->x_variables);
    gpointer_unset(&x->x_gp);
}

static void append_setup(void)
{
    append_class = class_new (sym_append, (t_newmethod)append_new,
        (t_method)append_free, sizeof(t_append), 0, A_GIMME, 0);
    class_addFloat(append_class, append_float); 
    class_addMethod(append_class, (t_method)append_set, sym_set,
        A_SYMBOL, A_SYMBOL, 0); 
}

/* ----------------- setup function ------------------- */

void g_traversal_setup(void)
{
    ptrobj_setup();
    get_setup();
    set_setup();
    elem_setup();
    getsize_setup();
    setsize_setup();
    append_setup();
}
