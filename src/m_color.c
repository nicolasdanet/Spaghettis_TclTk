
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static unsigned int color_digitTo8BitsComponent (int n)
{
    unsigned int c = PD_CLAMP (n, 0, 8); return PD_MIN ((c << 5), 0xff);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_color color_withRGB (int argc, t_atom *argv)
{
    unsigned int r = (unsigned int)atom_getFloatAtIndex (0, argc, argv);
    unsigned int g = (unsigned int)atom_getFloatAtIndex (1, argc, argv);
    unsigned int b = (unsigned int)atom_getFloatAtIndex (2, argc, argv);
    
    r = r & 0xff;
    g = g & 0xff;
    b = b & 0xff;
        
    return (t_color)(COLOR_MASK & ((r << 16) | (g << 8) | b));
}

t_color color_withDigits (int c)
{
    int n = PD_CLAMP (c, 0, 999);
    
    unsigned int r = color_digitTo8BitsComponent (n / 100);
    unsigned int g = color_digitTo8BitsComponent ((n / 10) % 10);
    unsigned int b = color_digitTo8BitsComponent (n % 10);
    
    return (t_color)(COLOR_MASK & ((r << 16) | (g << 8) | b));
}

t_color color_withEncoded (t_symbol *s)
{
    if (s->s_name[0] == '#') { return color_checked ((t_color)strtol (s->s_name + 1, NULL, 16)); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static t_error color_toEncodedString (char *dest, size_t size, t_color color)
{
    return string_sprintf (dest, size, "#%06x", color_checked (color));
}

t_symbol *color_toEncoded (t_color color)
{
    char t[PD_STRING] = { 0 }; color_toEncodedString (t, PD_STRING, color);
    
    return gensym (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
