
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

static t_glist *canvas_newGraphOnParent (t_glist *glist,
    t_float valueStart,
    t_float valueUp,
    t_float valueEnd,
    t_float valueDown,
    t_float topLeftX,
    t_float topLeftY,
    t_float width,
    t_float height)
{
    t_glist *x = (t_glist *)pd_new (canvas_class);
    
    t_glist *current = instance_contextGetCurrent();
    t_fontsize fontSize = current ? current->gl_fontSize : font_getDefaultFontSize();

    object_setBuffer (cast_object (x), buffer_new());
    object_setX (cast_object (x), topLeftX);
    object_setY (cast_object (x), topLeftY);
    object_setType (cast_object (x), TYPE_OBJECT);
    
    x->gl_holder            = gmaster_createWithGlist (x);
    x->gl_parent            = glist;
    x->gl_name              = utils_getDefaultBindName (canvas_class, sym__graph);
    x->gl_uniqueIdentifier  = utils_unique();
    x->gl_fontSize          = fontSize;
    x->gl_isGraphOnParent   = 1;
    
    bounds_set (glist_getBounds (x), valueStart, valueUp, valueEnd, valueDown);
    rectangle_setByWidthAndHeight (glist_getGraphGeometry (x), 0, 0, width, height); 
    rectangle_set (glist_getWindowGeometry (x), 
        0,
        WINDOW_HEADER,
        WINDOW_WIDTH,
        WINDOW_HEIGHT + WINDOW_HEADER);
    
    canvas_bind (x);
    
    buffer_vAppend (object_getBuffer (cast_object (x)), "s", sym_graph);
    
    canvas_addObject (glist, cast_gobj (x));
    
    return x;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_makeIemObject (t_glist *glist, t_symbol *name)
{
    if (glist_isMapped (glist)) {                                              /* Interactive creation. */
    //
    t_buffer *b = buffer_new();
    int positionX = 0;
    int positionY = 0;
    t_atom a;
        
    instance_getDefaultCoordinates (glist, &positionX, &positionY);
    canvas_deselectAll (glist);
    SET_SYMBOL (&a, name);
    buffer_deserialize (b, 1, &a);
    canvas_makeTextObject (glist, positionX, positionY, 0, 1, b);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_fromArrayDialog (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    if (argc == 3) {
    //
    t_symbol *name = atom_getSymbol (argv + 0);
    t_float size   = atom_getFloat (argv + 1);
    t_float flags  = atom_getFloat (argv + 2);
    
    t_float n = (t_float)PD_MAX (1.0, size);
    int positionX = 0;
    int positionY = 0;
    t_glist *g = NULL;
    
    PD_ASSERT (name);
    
    instance_getDefaultCoordinates (glist, &positionX, &positionY);
    
    { 
        t_float a      = (t_float)positionX;
        t_float b      = (t_float)positionY;
        t_float start  = (t_float)0.0;
        t_float up     = (t_float)1.0;
        t_float width  = (t_float)200.0;
        t_float height = (t_float)140.0;
        
        g = canvas_newGraphOnParent (glist, start, up, n, -up, a, b, width, height);
    }
    
    garray_makeObject (g, dollar_fromHash (name), n, flags);
    
    canvas_dirty (glist, 1);
    //
    }
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_makeObject (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int positionX = 0;
    int positionY = 0;
    
    if (argc >= 2) {                                                            /* File creation. */
    
        t_buffer *b = buffer_new();     /* Will be owned by the object. */
            
        positionX = (int)atom_getFloatAtIndex (0, argc, argv);
        positionY = (int)atom_getFloatAtIndex (1, argc, argv);
        
        buffer_deserialize (b, argc - 2, argv + 2);
        canvas_makeTextObject (glist, positionX, positionY, 0, 0, b);
    
    } else if (glist_isMapped (glist)) {                                       /* Interactive creation. */
        
        t_buffer *b = buffer_new();
            
        instance_getDefaultCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll (glist);
        canvas_makeTextObject (glist, positionX, positionY, 0, 1, b);
    }
}

void canvas_makeMessage (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    message_makeObject (glist, s, argc, argv);
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
        
        canvas_addObject (glist, cast_gobj (x));
        
    } else if (glist_isMapped (glist)) {                                       /* Interactive creation. */
    
        int positionX = 0;
        int positionY = 0;

        instance_getDefaultCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll (glist);
            
        object_setX (x, positionX);
        object_setY (x, positionY);
        
        buffer_deserialize (object_getBuffer (x), 1, &a);
        canvas_addObject (glist, cast_gobj (x));
        canvas_selectObject (glist, cast_gobj (x));
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
    canvas_deserializeScalar (glist, buffer_size (t), buffer_atoms (t));
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

void canvas_makeBang (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_bng);
}

void canvas_makeToggle (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_tgl);
}

void canvas_makeVerticalSlider (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_vslider);
}

void canvas_makeHorizontalSlider (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_hslider);
}

void canvas_makeHorizontalRadio (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_hradio);
}

void canvas_makeVerticalRadio (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_vradio);
}

void canvas_makeVu (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_vu);
}

void canvas_makePanel (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_cnv);
}

void canvas_makeDial (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_nbx);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
