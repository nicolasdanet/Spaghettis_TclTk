
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
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
// MARK: -

/* Few characters are forbidden to avoid mislead interpretations at script level. */

static int canvas_isKeyCodeAllowed (t_keycode n)
{
    if (n == '{')       { return 0; }   // --
    else if (n == '}')  { return 0; }   // --
    else if (n == '\\') { return 0; }
     
    return 1;
}

static int canvas_parseSymbolToKeyCode (t_symbol *s, t_keycode *n)
{
    if (s == sym_Enter)          { *n = 3;   return 1; }
    else if (s == sym_BackSpace) { *n = 8;   return 1; }
    else if (s == sym_Tab)       { *n = 9;   return 1; }
    else if (s == sym_Return)    { *n = 10;  return 1; }
    else if (s == sym_Escape)    { *n = 27;  return 1; }
    else if (s == sym_Space)     { *n = 32;  return 1; }
    else if (s == sym_Delete)    { *n = 127; return 1; }
    
    return 0;
}

static t_symbol *canvas_getSymbolWithKeyCode (t_keycode n)
{
    switch (n) {
    //
    case 3   : return sym_Enter;
    case 8   : return sym_BackSpace;
    case 9   : return sym_Tab;
    case 10  : return sym_Return;
    case 13  : return sym_Return;
    case 27  : return sym_Escape;
    case 32  : return sym_Space;
    case 127 : return sym_Delete;
    //
    }
    
    {
    
    /* Encode UTF-32 as UTF-8. */
    
    char t[UTF8_MAXIMUM_BYTES + 1] = { 0 };
    int size = u8_wc_toutf8 (t, n); 
    t[size] = 0;
    
    return gensym (t);
        
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
    
    if (canvas_isKeyCodeAllowed ((t_keycode)n)) {
    //
    if (IS_FLOAT (argv + 1))  { s = canvas_getSymbolWithKeyCode ((t_keycode)n); }
    if (IS_SYMBOL (argv + 1)) { s = GET_SYMBOL (argv + 1); canvas_parseSymbolToKeyCode (s, &n); }

    /* Report keystrokes to bounded objects. */
    
    if (n && symbol_hasThingQuiet (sym__key) && isDown)    {
        pd_float (symbol_getThing (sym__key), (t_float)n);
    }
    if (n && symbol_hasThingQuiet (sym__keyup) && !isDown) {
        pd_float (symbol_getThing (sym__keyup), (t_float)n);
    }
    
    if (symbol_hasThingQuiet (sym__keyname)) {
        t_atom a[2];
        SET_FLOAT  (a + 0, isDown);
        SET_SYMBOL (a + 1, s);
        pd_list (symbol_getThing (sym__keyname), 2, a);
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
// MARK: -

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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
