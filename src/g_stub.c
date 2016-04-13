
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Original "g_7_guis.h" written by Thomas Musil (c) IEM KUG Graz Austria 2000-2001. */

/* Thanks to Miller Puckette, Guenther Geiger and Krzystof Czaja. */

/* < http://iem.kug.ac.at/ > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "g_canvas.h"
#include "g_iem.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Trampoline fun. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void iemstub_send (void *x, t_symbol *s)
{
    iemgui_setSend (x, cast_iem (x), s);
}

void iemstub_receive (void *x, t_symbol *s)
{
    iemgui_setReceive (x, cast_iem (x), s);
}

void iemstub_label (void *x, t_symbol *s)
{
    iemgui_setLabel (x, cast_iem (x), s);
}

void iemstub_labelPosition (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelPosition (x, cast_iem (x), s, argc, argv);
}

void iemstub_labelFont (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelFont (x, cast_iem (x), s, argc, argv);
}

void iemstub_backgroundColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setBackgroundColor (x, cast_iem (x), s, argc, argv);
}

void iemstub_foregroundColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setForegroundColor (x, cast_iem (x), s, argc, argv);
}

void iemstub_labelColor (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setLabelColor (x, cast_iem (x), s, argc, argv);
}

void iemstub_position (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_setPosition (x, cast_iem (x), s, argc, argv);
}

void iemstub_move (void *x, t_symbol *s, int argc, t_atom *argv)
{
    iemgui_movePosition (x, cast_iem (x), s, argc, argv);
}

void iemstub_dummy (void *x, t_symbol *s, int argc, t_atom *argv)
{
    /* Dummy. */
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
