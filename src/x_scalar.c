
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
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

static void scalardefine_send (t_glist *x, t_symbol *s)
{
    if (pd_isThing (s)) {
    //
    if (x->gl_graphics) {
    //
    t_gobj *y = x->gl_graphics;

    if (pd_class (y) == scalar_class) {
        t_gpointer gp; GPOINTER_INIT (&gp);
        gpointer_setAsScalar (&gp, x, cast_scalar (y));
        pd_pointer (pd_getThing (s), &gp);
        gpointer_unset (&gp);
    }
    //
    }
    //
    }
}

static void scalardefine_set (t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->gl_graphics) {
    //
    t_gobj *y = x->gl_graphics;
    
    if (pd_class (y) == scalar_class) {
        t_buffer *t = buffer_new();
        canvas_clear (x);
        buffer_deserialize (t, argc, argv);
        canvas_deserializeScalar (x, buffer_size (t), buffer_atoms (t));
        buffer_free (t);
    }
    //
    } else { PD_BUG; }
}

static void scalardefine_save (t_gobj *z, t_buffer *b)
{
    t_glist *x = cast_glist (z);
    
    buffer_vAppend (b, "ssii", 
        sym___hash__X,
        sym_obj,
        cast_object (x)->te_xCoordinate,
        cast_object (x)->te_yCoordinate);
        
    buffer_serialize (b, cast_object (x)->te_buffer);
    buffer_appendSemicolon (b);
    object_saveWidth (cast_object (x), b);
    
    if (x->gl_saveScalar && x->gl_graphics) {

        t_gobj *y = x->gl_graphics;
        
        if (pd_class (y) == scalar_class) {
            t_buffer *t = buffer_new();
            buffer_vAppend (b, "ss", sym___hash__A, sym_set);
            scalar_serialize (cast_scalar (y), t);
            buffer_serialize (b, t);
            buffer_appendSemicolon (b);
            buffer_free (t);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void scalardefine_anything (t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *scalardefine_new (t_symbol *s, int argc, t_atom *argv)
{
    t_glist *x = NULL;
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
    
    if (!error__options (s, argc, argv)) {
        if (argc && IS_SYMBOL (argv)) { 
            templateIdentifier = utils_makeTemplateIdentifier (GET_SYMBOL (argv)); argc--; argv++;
        }
    }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    SET_FLOAT  (a + 0, (t_float)0.0);
    SET_FLOAT  (a + 1, WINDOW_HEADER);
    SET_FLOAT  (a + 2, WINDOW_WIDTH);
    SET_FLOAT  (a + 3, WINDOW_HEIGHT);
    SET_SYMBOL (a + 4, sym_Scalar);
    SET_FLOAT  (a + 5, (t_float)0.0);
    
    x = canvas_new (NULL, NULL, 6, a);

    PD_ASSERT (x);
    
    x->gl_saveScalar = k;

    if (template_findByIdentifier (templateIdentifier)) {
    //
    t_scalar *scalar = scalar_new (x, templateIdentifier);
    
    if (!scalar) { PD_BUG; }
    else {
        canvas_addScalarNext (x, NULL, scalar);
        pd_setBoundA (cast_pd (x));
    }
    //
    }

    canvas_pop (x, (t_float)0.0);
    
    pd_class (x) = scalardefine_class;
    
    return x;
}

static void *scalardefine_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = scalardefine_new (s, argc, argv); }
    else {
        t_symbol *t = atom_getSymbol (argv);
        if (t == sym_d || t == sym_define) { 
            pd_newest = scalardefine_new (s, argc - 1, argv + 1); 
        } else {
            error_unexpected (sym_scalar, t);
        }
    }
    
    return pd_newest;
}

static void scalardefine_free (t_glist *x)
{
    pd_setBoundA (NULL); canvas_free (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void scalardefine_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_scalar__space__define,
            NULL,
            (t_method)scalardefine_free,
            sizeof (t_glist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)scalardefine_makeObject, sym_scalar, A_GIMME, A_NULL);
        
    class_addKey (c, (t_method)canvas_key);
    class_addClick (c, (t_method)canvas_click);
    class_addMotion (c, (t_method)canvas_motion);
    class_addMouse (c, (t_method)canvas_mouse);
    class_addMouseUp (c, (t_method)canvas_mouseUp);
    
    class_addMethod (c, (t_method)scalardefine_send,    sym_send,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)scalardefine_set,     sym_set,        A_GIMME, A_NULL);
    
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

    class_addAnything (c, (t_method)scalardefine_anything);

    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_close,  sym_menuclose,  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,   sym_menusave,   A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_saveAs, sym_menusaveas, A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_window, sym_setbounds,  A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    
    #endif 
        
    class_setHelpName (c, sym_scalar);
    class_setSaveFunction (c, scalardefine_save);

    scalardefine_class = c;
}

void scalardefine_destroy (void)
{
    CLASS_FREE (scalardefine_class);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
