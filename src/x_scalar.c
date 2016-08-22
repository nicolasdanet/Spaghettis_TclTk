
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
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_class  *scalar_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class         *scalardefine_class;            /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

    /* send a pointer to the scalar to whomever is bound to the symbol */
static void scalardefine_send(t_glist *x, t_symbol *s)
{
    if (!s->s_thing)
        post_error ("scalardefine_send: %s: no such object", s->s_name);
    else if (x->gl_graphics && pd_class(&x->gl_graphics->g_pd) == scalar_class)
    {
        t_gpointer gp = GPOINTER_INIT;
        gpointer_setAsScalar(&gp, x, (t_scalar *)&x->gl_graphics->g_pd);
        pd_pointer(s->s_thing, &gp);
        gpointer_unset(&gp);
    }
    else { PD_BUG; }
}

    /* set to a list, used to restore from scalardefine_save()s below */
static void scalardefine_set(t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->gl_graphics && pd_class(&x->gl_graphics->g_pd) == scalar_class)
    {
        t_buffer *b = buffer_new();
        int natoms;
        t_atom *vec;
        canvas_clear(x);
        buffer_deserialize(b, argc, argv);
        natoms = buffer_size(b);
        vec = buffer_atoms(b);
        canvas_deserializeScalar(x, natoms, vec);
        buffer_free(b);
    }
    else { PD_BUG; }
}

    /* save to a binbuf (for file save or copy) */
static void scalardefine_save(t_gobj *z, t_buffer *bb)
{
    t_glist *x = (t_glist *)z;
    buffer_vAppend(bb, "ssff", sym___hash__X, sym_obj,
        (float)x->gl_obj.te_xCoordinate, (float)x->gl_obj.te_yCoordinate);
    buffer_serialize(bb, x->gl_obj.te_buffer);
    buffer_appendSemicolon(bb);
    if (x->gl_saveScalar && x->gl_graphics &&
        pd_class(&x->gl_graphics->g_pd) == scalar_class)
    {
        t_buffer *b2 = buffer_new();
        t_scalar *sc = (t_scalar *)(x->gl_graphics);
        buffer_vAppend(bb, "ss", sym___hash__A, sym_set);
        scalar_serialize (sc, b2);
        buffer_serialize(bb, b2);
        buffer_appendSemicolon(bb);
        buffer_free(b2);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void scalardefine_anything (t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *scalardefine_newObject (t_symbol *s, int argc, t_atom *argv)
{
    t_glist *x = NULL;
    t_glist *z = canvas_getCurrent();
    t_symbol *templateIdentifier = utils_makeTemplateIdentifier (&s_float);
    t_atom a[6];
    int k = 0;
    
    while (argc && IS_SYMBOL (argv)) {
        if (GET_SYMBOL (argv) == sym___dash__k || GET_SYMBOL (argv) == sym___dash__keep) {
            k = 1; argc--; argv++;
        } else {
            break;
        }
    }
    
    if (argc && IS_SYMBOL (argv)) { templateIdentifier = utils_makeTemplateIdentifier (GET_SYMBOL (argv)); }

    SET_FLOAT  (a + 0, 0.0);
    SET_FLOAT  (a + 1, WINDOW_HEADER);
    SET_FLOAT  (a + 2, WINDOW_WIDTH);
    SET_FLOAT  (a + 3, WINDOW_HEIGHT);
    SET_SYMBOL (a + 4, s);
    SET_FLOAT  (a + 5, 0.0);
    
    x = canvas_new (NULL, NULL, 6, a);

    x->gl_parent = z;
    x->gl_saveScalar = k;

    if (template_findByIdentifier (templateIdentifier)) {
    //
    t_scalar *scalar = scalar_new (x, templateIdentifier);
    
    if (!scalar) { PD_BUG; }
    else {
        canvas_addScalarNext (x, NULL, scalar);
        sym___hash__A->s_thing = NULL;
        pd_bind (cast_pd (x), sym___hash__A);
    }
    //
    }

    canvas_pop (x, 0.0);
    
    pd_class (x) = scalardefine_class;
    
    return x;
}

static void *scalardefine_new (t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = cast_pd (scalardefine_newObject (s, argc, argv)); }
    else {
        t_symbol *t = atom_getSymbol (argv);
        if (t == sym_d || t == sym_define) { 
            pd_newest = cast_pd (scalardefine_newObject (s, argc - 1, argv + 1)); 
        } else {
            post_error (PD_TRANSLATE ("scalar: unknown function"));
        }
    }
    
    return pd_newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scalardefine_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_scalar__space__define,
            NULL,
            (t_method)canvas_free,
            sizeof (t_glist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addKey (c, canvas_key);
    class_addClick (c, canvas_click);
    class_addMotion (c, canvas_motion);
    class_addMouse (c, canvas_mouse);
    class_addMouseUp (c, canvas_mouseUp);
    
    class_addMethod (c, (t_method)scalardefine_send,    sym_send,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)scalardefine_set,     sym_set,        A_GIMME, A_NULL);
    
    class_addMethod (c, (t_method)canvas_restore,       sym_restore,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_dsp,           sym_dsp,        A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_map,           sym__map,       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,         sym_close,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,          sym_save,       A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs,        sym_saveas,     A_DEFFLOAT, A_NULL);
    
    class_addMethod (c, (t_method)canvas_window,
        sym_window,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);

    class_addAnything (c, scalardefine_anything);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_close,  sym_menuclose,  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,   sym_menusave,   A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs, sym_menusaveas, A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_window, sym_setbounds,  A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    
    #endif 

    class_addCreator ((t_newmethod)scalardefine_new, sym_scalar, A_GIMME, A_NULL);
        
    class_setHelpName (c, sym_scalar);
    class_setSaveFunction (c, scalardefine_save);

    scalardefine_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
