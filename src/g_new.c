
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
#include "g_canvas.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *text_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void canvas_makeIemObject (t_glist *glist, t_symbol *name)
{
    if (canvas_isMapped (glist)) {                                              /* Interactive creation. */
    //
    t_buffer *b = buffer_new();
    int positionX = 0;
    int positionY = 0;
    t_atom a;
        
    canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
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

void canvas_makeGraphWithArray (t_glist *glist, t_symbol *name, t_float size, t_float flags)
{
    t_float n = PD_MAX (1.0, size);
    t_glist *g = canvas_addGraph (glist, &s_, 0, 1, n, -1, 0, 0, 0, 0);
    t_garray *a = garray_makeObject (g, dollar_fromHash (name), &s_float, n, (int)flags);
    garray_updateGraphBounds (a, (int)n, (((int)flags & 6) >> 1));
    canvas_dirty (glist, 1);
}

void canvas_makeArray (t_glist *glist, t_symbol *s, t_symbol *templateName, t_float size, t_float flags)
{
    garray_makeObject (glist, s, templateName, size, flags);
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
    
    } else if (canvas_isMapped (glist)) {                                       /* Interactive creation. */
        
        t_buffer *b = buffer_new();
            
        canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
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
    gatom_makeObject (glist, A_FLOAT, s, argc, argv);
}

void canvas_makeSymbolAtom (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    gatom_makeObject (glist, A_SYMBOL, s, argc, argv);
}

void canvas_makeComment (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    t_object *x = (t_object *)pd_new (text_class);
    
    t_atom a; SET_SYMBOL (&a, sym_comment);
            
    x->te_width  = 0;
    x->te_type   = TYPE_COMMENT;
    x->te_buffer = buffer_new();
    
    if (argc > 1) {                                                             /* File creation. */
    
        x->te_xCoordinate = atom_getFloatAtIndex (0, argc, argv);
        x->te_yCoordinate = atom_getFloatAtIndex (1, argc, argv);
        
        if (argc > 2) { buffer_deserialize (x->te_buffer, argc - 2, argv + 2); }
        else {
            buffer_deserialize (x->te_buffer, 1, &a);
        }
        
        canvas_addObject (glist, cast_gobj (x));
        
    } else if (canvas_isMapped (glist)) {                                       /* Interactive creation. */
    
        int positionX = 0;
        int positionY = 0;

        canvas_getLastMotionCoordinates (glist, &positionX, &positionY);
        canvas_deselectAll (glist);
                
        x->te_xCoordinate = positionX;
        x->te_yCoordinate = positionY;
        
        buffer_deserialize (x->te_buffer, 1, &a);
        canvas_addObject (glist, cast_gobj (x));
        canvas_selectObject (glist, cast_gobj (x));
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
