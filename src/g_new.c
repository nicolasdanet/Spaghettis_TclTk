
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
    if (canvas_isMapped (glist)) {      /* Interactive creation. */
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

void canvas_obj (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
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

void canvas_text (t_glist *gl, t_symbol *s, int argc, t_atom *argv)
{
    t_object *x = (t_object *)pd_new(text_class);
    t_atom at;
    x->te_width = 0;                            /* don't know it yet. */
    x->te_type = TYPE_TEXT;
    x->te_buffer = buffer_new();
    if (argc > 1)
    {
        x->te_xCoordinate = atom_getFloatAtIndex(0, argc, argv);
        x->te_yCoordinate = atom_getFloatAtIndex(1, argc, argv);
        if (argc > 2) buffer_deserialize(x->te_buffer, argc-2, argv+2);
        else
        {
            SET_SYMBOL(&at, sym_comment);
            buffer_deserialize(x->te_buffer, 1, &at);
        }
        glist_add(gl, &x->te_g);
    }
    else
    {
        int xpix, ypix;
        pd_vMessage((t_pd *)canvas_getView(gl), sym_editmode, "i", 1);
        SET_SYMBOL(&at, sym_comment);
        canvas_deselectAll(gl);
        canvas_getLastMotionCoordinates(gl, &xpix, &ypix);
        x->te_xCoordinate = xpix-1;
        x->te_yCoordinate = ypix-1;
        buffer_deserialize(x->te_buffer, 1, &at);
        glist_add(gl, &x->te_g);
        canvas_deselectAll(gl);
        canvas_selectObject(gl, &x->te_g);
            /* it would be nice to "activate" here, but then the second,
            "put-me-down" click changes the text selection, which is quite
            irritating, so I took this back out.  It's OK in messages
            and objects though since there's no text in them at menu
            creation. */
            /* gobj_activate(&x->te_g, gl, 1); */
        //canvas_startmotion(canvas_getView(gl));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void canvas_bng (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_bng);
}

void canvas_tgl (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_tgl);
}

void canvas_vslider (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_vslider);
}

void canvas_hslider (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_hslider);
}

void canvas_hradio (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_hradio);
}

void canvas_vradio (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_vradio);
}

void canvas_vu (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_vu);
}

void canvas_cnv (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_cnv);
}

void canvas_nbx (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    canvas_makeIemObject (glist, sym_nbx);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
