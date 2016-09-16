
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
#include "x_control.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_pd     *pd_newest;
extern t_class  *garray_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *arraydefine_class;             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_garray *arraydefine_getContentChecked (t_glist *x)
{
    t_gobj *y = x->gl_graphics;
    
    if (y) {
        t_glist *glist = canvas_castToGlistChecked (cast_pd (y));
        if (glist && garray_isSingle (glist)) { return (t_garray *)glist->gl_graphics; }
    }
    
    PD_BUG; 
    
    return NULL;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void arraydefine_send (t_glist *x, t_symbol *s)
{
    if (pd_isThing (s)) {
    //
    t_garray *t = arraydefine_getContentChecked (x);
    t_gpointer gp = GPOINTER_INIT;
    gpointer_setAsScalar (&gp, garray_getView (t), garray_getScalar (t));
    pd_pointer (s->s_thing, &gp);
    gpointer_unset (&gp);
    //
    }
}

static void arraydefine_dummy (t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
}

static void arraydefine_anything (t_glist *x, t_symbol *s, int argc, t_atom *argv)
{
    t_garray *t = arraydefine_getContentChecked (x);
    pd_message (cast_pd (t), s, argc, argv);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraydefine_save (t_gobj *z, t_buffer *b)
{
    t_glist *x  = cast_glist (z);
    t_garray *t = arraydefine_getContentChecked (x);
        
    buffer_vAppend (b, "ssii", 
        sym___hash__X, 
        sym_obj,
        cast_object (x)->te_xCoordinate, 
        cast_object (x)->te_yCoordinate);
        
    buffer_serialize (b, cast_object (x)->te_buffer);
    object_saveAppendWidth (cast_object (x), b);
    buffer_appendSemicolon (b);

    garray_saveContentsToBuffer (t, b);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *arraydefine_new (t_symbol *s, int argc, t_atom *argv)
{
    t_glist *x = NULL;
    
    t_symbol *name = &s_;
    t_float size   = GRAPH_DEFAULT_END;
    t_float down   = 0.0;
    t_float up     = 0.0;
    int keep = 0;
    
    while (argc && IS_SYMBOL (argv)) {
    
        t_symbol *t = GET_SYMBOL (argv);
        
        if (t == sym___dash__k || t == sym___dash__keep) {
            keep = 1; argc--; argv++;
            
        } else if (t == sym___dash__yrange && (argc >= 3) && IS_FLOAT (argv + 1) && IS_FLOAT (argv + 2)) {
            down = atom_getFloatAtIndex (1, argc, argv);
            up   = atom_getFloatAtIndex (2, argc, argv);
            argc -= 3; argv += 3;
            
        } else {
            break;
        }
    }
    
    if (down == up) { down = GRAPH_DEFAULT_DOWN; up = GRAPH_DEFAULT_UP; }
    
    if (!error__options (s, argc, argv)) {
    //
    if (argc && IS_SYMBOL (argv)) { name = GET_SYMBOL (argv); argc--; argv++; }
    if (argc && IS_FLOAT (argv))  { size = GET_FLOAT (argv);  argc--; argv++; }
    //
    }
    
    if (argc) { warning_unusedArguments (s, argc, argv); }
    
    x = cast_glist (table_makeObject (name, down, up, size, keep));
    
    pd_class (x) = arraydefine_class;
    
    return x;
}

static void *arraydefine_makeObject (t_symbol *s, int argc, t_atom *argv)
{
    pd_newest = NULL;
    
    if (!argc || !IS_SYMBOL (argv)) { pd_newest = arraydefine_new (s, argc, argv); }
    else {
    //
    t_symbol *t = atom_getSymbol (argv);
    
    if (t == sym_d || t == sym_define)  { pd_newest = arraydefine_new (s,   argc - 1, argv + 1); }
    else if (t == sym_size)             { pd_newest = arraysize_new (s,     argc - 1, argv + 1); }
    else if (t == sym_sum)              { pd_newest = arraysum_new (s,      argc - 1, argv + 1); }
    else if (t == sym_get)              { pd_newest = arrayget_new (s,      argc - 1, argv + 1); }
    else if (t == sym_set)              { pd_newest = arrayset_new (s,      argc - 1, argv + 1); }
    else if (t == sym_quantile)         { pd_newest = arrayquantile_new (s, argc - 1, argv + 1); }
    else if (t == sym_random)           { pd_newest = arrayrandom_new (s,   argc - 1, argv + 1); }
    else if (t == sym_max)              { pd_newest = arraymax_new (s,      argc - 1, argv + 1); }
    else if (t == sym_min)              { pd_newest = arraymin_new (s,      argc - 1, argv + 1); }
    else {
        error_unexpected (sym_array, t);
    }
    //
    }
    
    return pd_newest;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void arraydefine_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_array__space__define,
            NULL,
            (t_method)canvas_free,
            sizeof (t_glist),
            CLASS_DEFAULT,
            A_NULL);
    
    class_addCreator ((t_newmethod)arraydefine_makeObject, sym_array, A_GIMME, A_NULL);
    
    class_addKey (c, canvas_key);
    class_addMouse (c, canvas_mouse);
    class_addMouseUp (c, canvas_mouseUp);
    class_addClick (c, canvas_click);
    class_addMotion (c, canvas_motion);
    
    class_addMethod (c, (t_method)canvas_dsp,           sym_dsp,        A_CANT, A_NULL);
    class_addMethod (c, (t_method)canvas_restore,       sym_restore,    A_GIMME, A_NULL);
    class_addMethod (c, (t_method)canvas_map,           sym__map,       A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_close,         sym_close,      A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,          sym_save,       A_NULL); 
    class_addMethod (c, (t_method)canvas_saveAs,        sym_saveas,     A_NULL);
    
    class_addMethod (c, (t_method)canvas_window,
        sym_window,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addMethod (c, (t_method)arraydefine_send,     sym_send,       A_SYMBOL, A_NULL);
    class_addMethod (c, (t_method)arraydefine_dummy,    sym_editmode,   A_GIMME, A_NULL);
    class_addMethod (c, (t_method)arraydefine_dummy,
        sym__popupdialog,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    class_addAnything (c, arraydefine_anything);
    
    #if PD_WITH_LEGACY
    
    class_addMethod (c, (t_method)canvas_close,         sym_menuclose,  A_DEFFLOAT, A_NULL);
    class_addMethod (c, (t_method)canvas_save,          sym_menusave,   A_NULL); 
    class_addMethod (c, (t_method)canvas_saveAs,        sym_menusaveas, A_NULL);
    
    class_addMethod (c, (t_method)canvas_window, 
        sym_setbounds,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_FLOAT,
        A_NULL);
        
    #endif
    
    class_setHelpName (c, sym_array);
    class_setSaveFunction (c, arraydefine_save);
    
    arraydefine_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
