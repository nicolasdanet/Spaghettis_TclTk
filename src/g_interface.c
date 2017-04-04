
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
#include "s_utf8.h"
#include "g_graphics.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void glist_actionEnd        (t_glist *, int, int);
void glist_action           (t_glist *, int, int, int);
void glist_mouse            (t_glist *, int, int, int, int);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void canvas_key (t_glist *glist, t_symbol *dummy, int argc, t_atom *argv)
{
    PD_ASSERT (sizeof (t_keycode) == sizeof (UCS4_CODE_POINT));
    
    if (argc > 1) { 
    //
    int isDown = ((int)(atom_getFloat (argv + 0)) != 0);
    
    t_symbol *s = sym__dummy;
    
    /* Assume key number is UTF-32. */
    /* All 32-bit integers can NOT be represented with a single precision float. */

    t_keycode n = (UCS4_CODE_POINT)(atom_getFloat (argv + 1));
    
    PD_ASSERT (n < (1 << 24));
    
    if (utils_isKeyCodeAllowed ((t_keycode)n)) {
    //
    if (IS_FLOAT (argv + 1))  { s = utils_getSymbolWithKeyCode ((t_keycode)n); }
    if (IS_SYMBOL (argv + 1)) { s = GET_SYMBOL (argv + 1); utils_parseSymbolToKeyCode (s, &n); }

    /* Report keystrokes to bounded objects. */
    
    if (n && pd_isThingQuiet (sym__key) && isDown)    { pd_float (pd_getThing (sym__key),   (t_float)n); }
    if (n && pd_isThingQuiet (sym__keyup) && !isDown) { pd_float (pd_getThing (sym__keyup), (t_float)n); }
    
    if (pd_isThingQuiet (sym__keyname)) {
        t_atom a[2];
        SET_FLOAT  (a + 0, isDown);
        SET_SYMBOL (a + 1, s);
        pd_list (pd_getThing (sym__keyname), 2, a);
    }
    
    /* Handle the event. */
    
    if (glist && isDown) { glist_key (glist, n, s); }
    
    return;
    //
    }
    
    error_ignored (sym_key);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Click down. */

void canvas_mouseDown (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    int m = (int)atom_getFloatAtIndex (2, argc, argv);

    glist_mouse (glist, a, b, m, 1);
}

/* Click up.*/

void canvas_mouseUp (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = (int)atom_getFloatAtIndex (0, argc, argv);
    int b = (int)atom_getFloatAtIndex (1, argc, argv);
    
    glist_actionEnd (glist, a, b);
}

/* Moving (drag included). */

void canvas_motion (t_glist *glist, t_symbol *s, int argc, t_atom *argv)
{
    int a = atom_getFloatAtIndex (0, argc, argv);
    int b = atom_getFloatAtIndex (1, argc, argv);
    int m = atom_getFloatAtIndex (2, argc, argv);
    
    instance_setDefaultCoordinates (glist, a, b);
    
    if (editor_getAction (glist_getEditor (glist))) { glist_action (glist, a, b, m); }
    else {
        glist_mouse (glist, a, b, m, 0);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
