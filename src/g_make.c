
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

void canvas_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int positionX = 0;
    int positionY = 0;
    
    if (argc >= 2) {                                                            /* File creation. */
    
        t_buffer *b = buffer_new();     /* Will be owned by the object. */
            
        positionX = (int)atom_getFloatAtIndex (0, argc, argv);
        positionY = (int)atom_getFloatAtIndex (1, argc, argv);
        
        buffer_deserialize (b, argc - 2, argv + 2);
        glist_objectMake (glist, positionX, positionY, 0, 0, b);
    
    } else if (glist_isOnScreen (glist)) {                                      /* Interactive creation. */
        
        t_buffer *b = buffer_new();
            
        instance_getDefaultCoordinates (glist, &positionX, &positionY);
        glist_deselectAll (glist);
        glist_objectMake (glist, positionX, positionY, 0, 1, b);
    }
}

void canvas_makeMessage (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    message_makeObject (glist, s, argc, argv);
}

void canvas_makeArray (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) {
    //
    t_symbol *name = atom_getSymbol (argv + 0);
    t_float size   = atom_getFloat (argv + 1);
    t_float flags  = atom_getFloatAtIndex (3, argc, argv);
    
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
    
    int a = 0;
    int b = 0;
    
    bounds_set (&bounds, 0, 1, n, -1);
    rectangle_set (&r1,  0, 0, 200, 140);
    rectangle_set (&r2,  0, WINDOW_HEADER, WINDOW_WIDTH, WINDOW_HEIGHT);
    
    instance_getDefaultCoordinates (glist, &a, &b);
    
    PD_ASSERT (name);
    
    {
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
    t_object *x = (t_object *)pd_new (text_class);
    
    t_atom a; SET_SYMBOL (&a, sym_comment);

    object_setBuffer (x, buffer_new());
    object_setWidth (x, 0);
    object_setType (x, TYPE_COMMENT);
    
    if (argc > 1) {                                                             /* File creation. */
        
        object_setX (x, atom_getFloatAtIndex (0, argc, argv));
        object_setY (x, atom_getFloatAtIndex (1, argc, argv));
        
        if (argc > 2) { buffer_deserialize (object_getBuffer (x), argc - 2, argv + 2); }
        else {
            buffer_deserialize (object_getBuffer (x), 1, &a);
        }
        
        glist_objectAdd (glist, cast_gobj (x));
        
    } else if (glist_isOnScreen (glist)) {                                      /* Interactive creation. */
    
        int positionX = 0;
        int positionY = 0;

        instance_getDefaultCoordinates (glist, &positionX, &positionY);
        glist_deselectAll (glist);
            
        object_setX (x, positionX);
        object_setY (x, positionY);
        
        buffer_deserialize (object_getBuffer (x), 1, &a);
        glist_objectAdd (glist, cast_gobj (x));
        glist_objectSelect (glist, cast_gobj (x));
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void canvas_makeIEM (t_glist *glist, t_symbol *name)
{
    if (glist_isOnScreen (glist)) {                                         /* Interactive creation. */
    //
    t_buffer *b = buffer_new();
    int positionX = 0;
    int positionY = 0;
    t_atom a;
        
    instance_getDefaultCoordinates (glist, &positionX, &positionY);
    glist_deselectAll (glist);
    SET_SYMBOL (&a, name);
    buffer_deserialize (b, 1, &a);
    glist_objectMake (glist, positionX, positionY, 0, 1, b);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_makeBang (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_bng);
}

void canvas_makeToggle (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_tgl);
}

void canvas_makeVerticalSlider (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_vslider);
}

void canvas_makeHorizontalSlider (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_hslider);
}

void canvas_makeHorizontalRadio (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_hradio);
}

void canvas_makeVerticalRadio (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_vradio);
}

void canvas_makeVu (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_vu);
}

void canvas_makePanel (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_cnv);
}

void canvas_makeDial (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIEM (glist, sym_nbx);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
