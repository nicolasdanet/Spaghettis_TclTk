
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_makeObjectFile (t_glist *glist, int argc, t_atom *argv)
{
    int a = (int)atom_getFloat (argv + 0);
    int b = (int)atom_getFloat (argv + 1);
    
    t_buffer *t = buffer_new();
    
    buffer_deserialize (t, argc - 2, argv + 2); 
    
    glist_objectMake (glist, a, b, 0, 0, t);
}

static void canvas_makeObjectMenu (t_glist *glist)
{
    if (glist_isOnScreen (glist)) {
    //
    int a = instance_getDefaultX (glist);
    int b = instance_getDefaultY (glist);
    
    t_buffer *t = buffer_new();
    
    glist_deselectAll (glist); 
    
    glist_objectMake (glist, a, b, 0, 1, t);
    //
    }
}

static void canvas_makeCommentFile (t_glist *glist, int argc, t_atom *argv)
{
    t_object *x = (t_object *)pd_new (text_class);
    
    int a = (int)atom_getFloat (argv + 0);
    int b = (int)atom_getFloat (argv + 1);
    
    t_buffer *t = buffer_new();

    if (argc > 2) { buffer_deserialize (t, argc - 2, argv + 2); }
    else {
        buffer_appendSymbol (t, sym_comment);
    }
    
    object_setBuffer (x, t);
    object_setX (x, a);
    object_setY (x, b);
    object_setType (x, TYPE_COMMENT);
    
    glist_objectAdd (glist, cast_gobj (x));
}

void canvas_makeCommentMenu (t_glist *glist)
{
    if (glist_isOnScreen (glist)) {
    //
    t_object *x = (t_object *)pd_new (text_class);
    
    int a = instance_getDefaultX (glist);
    int b = instance_getDefaultY (glist);
    
    t_buffer *t = buffer_new();

    buffer_appendSymbol (t, sym_comment);
    
    object_setBuffer (x, t);
    object_setX (x, a);
    object_setY (x, b);
    object_setType (x, TYPE_COMMENT);

    glist_deselectAll (glist);
    glist_objectAdd (glist, cast_gobj (x));
    glist_objectSelect (glist, cast_gobj (x));
    //
    }
}

static void canvas_makeIemMenu (t_glist *glist, t_symbol *name)
{
    if (glist_isOnScreen (glist)) {
    //
    int a = instance_getDefaultX (glist);
    int b = instance_getDefaultY (glist);
    
    t_buffer *t = buffer_new();
    
    glist_deselectAll (glist); buffer_appendSymbol (t, name); 
    
    glist_objectMake (glist, a, b, 0, 1, t);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc >= 2) { canvas_makeObjectFile (glist, argc, argv); }
    else {
        canvas_makeObjectMenu (glist);
    }
}

void canvas_makeMessage (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    message_makeObject (glist, s, argc, argv);
}

/* Note that the third argument is not used. */

void canvas_makeArray (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    t_symbol *name = atom_getSymbol (argv + 0);
    t_float size   = atom_getFloat (argv + 1);
    t_float flags  = atom_getFloatAtIndex (3, argc, argv);
    
    PD_ASSERT (argc < 3 || atom_getSymbolAtIndex (2, argc, argv) == sym_float);
    
    if (name != &s_) {
        garray_makeObject (glist, name, size, flags); 
    }
    //
    }
}

void canvas_makeArrayFromDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *name = atom_getSymbolAtIndex (0, argc, argv);
    t_float size   = atom_getFloatAtIndex (1, argc, argv);
    t_float flags  = atom_getFloatAtIndex (2, argc, argv);
    
    t_float n = PD_MAX (1, size);
    
    t_rectangle r1, r2; t_bounds bounds;
    
    int a = instance_getDefaultX (glist);
    int b = instance_getDefaultY (glist);
    
    t_error err = bounds_set (&bounds, 0, 1, n, -1);
    rectangle_set (&r1,  0, 0, 200, 140);
    rectangle_set (&r2,  0, WINDOW_HEADER, WINDOW_WIDTH, WINDOW_HEIGHT);
        
    PD_ASSERT (name);
    
    if (!err) {
    //
    t_glist *x  = glist_new (glist, utils_getDefaultBindName (canvas_class, sym__graph), &bounds, &r1, &r2);
    
    t_buffer *t = buffer_new(); buffer_appendSymbol (t, sym_graph);
    
    object_setBuffer (cast_object (x), t);
    object_setX (cast_object (x), a);
    object_setY (cast_object (x), b);
    object_setType (cast_object (x), TYPE_OBJECT);
    
    glist_setGraphOnParent (x, 1);
    glist_bind (x);
    
    glist_objectAdd (glist, cast_gobj (x));
    garray_makeObject (x, utils_hashToDollar (name), n, flags);
    glist_setDirty (glist, 1);
    //
    }
}

void canvas_makeFloatAtom (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    gatom_makeObjectFloat (glist, s, argc, argv);
}

void canvas_makeSymbolAtom (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    gatom_makeObjectSymbol (glist, s, argc, argv);
}

void canvas_makeComment (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc >= 2) { canvas_makeCommentFile (glist, argc, argv); }
    else {
        canvas_makeCommentMenu (glist);
    }
}

void canvas_makeScalar (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *templateName = atom_getSymbolAtIndex (0, argc, argv);
    
    if (templateName != &s_) {
    //
    t_symbol *templateIdentifier = utils_makeTemplateIdentifier (templateName);
    
    if (template_findByIdentifier (templateIdentifier)) {
    //
    t_buffer *t = buffer_new();
    
    buffer_deserialize (t, argc, argv);
    glist_objectMakeScalar (glist, buffer_size (t), buffer_atoms (t));
    buffer_free (t);
    
    return;
    //
    }
    
    error_noSuch (templateName, sym_template);
    //
    }
}

void canvas_makeBang (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_bng);
}

void canvas_makeToggle (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_tgl);
}

void canvas_makeSliderVertical (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_vslider);
}

void canvas_makeSliderHorizontal (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_hslider);
}

void canvas_makeRadioVertical (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_vradio);
}

void canvas_makeRadioHorizontal (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_hradio);
}

void canvas_makeVu (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_vu);
}

void canvas_makePanel (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_cnv);
}

void canvas_makeDial (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    PD_ASSERT (!argc); canvas_makeIemMenu (glist, sym_nbx);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
