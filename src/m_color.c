
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define COLOR_MASK      0xffffff

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_color color_checked (t_color color)
{
    return (color & COLOR_MASK);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_color color_withRGB (int argc, t_atom *argv)
{
    unsigned int r = (unsigned int)atom_getFloatAtIndex (0, argc, argv);
    unsigned int g = (unsigned int)atom_getFloatAtIndex (1, argc, argv);
    unsigned int b = (unsigned int)atom_getFloatAtIndex (2, argc, argv);
    
    r = PD_CLAMP (r, 0, 255);
    g = PD_CLAMP (g, 0, 255);
    b = PD_CLAMP (b, 0, 255);
        
    return (int)(COLOR_MASK & ((r << 16) | (g << 8) | b));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_color color_withEncodedSymbol (t_symbol *s)
{
    if (s->s_name[0] == '#') { return color_checked ((t_color)strtol (s->s_name + 1, NULL, 16)); }
    
    return 0;
}

t_symbol *color_toEncodedSymbol (t_color color)
{
    char t[PD_STRING] = { 0 }; string_sprintf (t, PD_STRING, "#%06x", color);
    
    return gensym (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int rangecolor (int n)    /* 0 to 9 in 5 steps */
{
    int n2 = (n == 9 ? 8 : n);               /* 0 to 8 */
    int ret = (n2 << 5);        /* 0 to 256 in 9 steps */
    if (ret > 255) ret = 255;
    return (ret);
}

void numbertocolor (int n, char *s)
{
    int red, blue, green;
    if (n < 0) n = 0;
    red = n / 100;
    blue = ((n / 10) % 10);
    green = n % 10;
    sprintf (s, "#%2.2x%2.2x%2.2x", rangecolor(red), rangecolor(blue), rangecolor(green));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
