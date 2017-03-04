
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define FONT_LIST_SIZE  6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _fontinfo {
    int     fi_size;
    int     fi_requiredWidth;
    int     fi_requiredHeight;
    double  fi_hostSize;
    double  fi_hostWidth;
    double  fi_hostHeight;
    } t_fontinfo;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_fontsize font_defaultSize = 12;                /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_fontinfo font_fontList[FONT_LIST_SIZE] =       /* Static. */
    {
        { 8,   6, 10,   8.0,   6.0, 10.0 }, 
        { 10,  7, 13,   10.0,  7.0, 13.0 }, 
        { 12,  9, 16,   12.0,  9.0, 16.0 },
        { 16, 10, 21,   16.0, 10.0, 21.0 }, 
        { 24, 15, 25,   24.0, 15.0, 25.0 }, 
        { 36, 25, 45,   36.0, 25.0, 45.0 }
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_fontinfo *font_getNearest (int fontSize)
{
    t_fontinfo *info = font_fontList;
    int i;
        
    for (i = 0; i < (FONT_LIST_SIZE - 1); i++, info++) {
        if (fontSize < (info + 1)->fi_size) { 
            return info; 
        }
    }
    
    return (font_fontList + (FONT_LIST_SIZE - 1));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void font_setDefaultFontSize (int size)
{
    font_defaultSize = font_getNearestValidFontSize (size);
}

t_fontsize font_getDefaultFontSize (void)
{
    return font_defaultSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_fontsize font_getNearestValidFontSize (int size)
{
    return (font_getNearest (size)->fi_size);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int font_getHostFontSize (t_fontsize fontSize)
{
    int k = (int)(font_getNearest (fontSize)->fi_hostSize);
    
    return PD_MAX (k, 1);
}

double font_getHostFontWidth (t_fontsize fontSize)
{
    double k = font_getNearest (fontSize)->fi_hostWidth;
    
    return PD_MAX (k, 1.0);
}

double font_getHostFontHeight (t_fontsize fontSize)
{
    double k = font_getNearest (fontSize)->fi_hostHeight;
    
    return PD_MAX (k, 1.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void font_withHostMeasured (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int i, j;
    int n = argc / 3;
    
    PD_ASSERT (n > 0);
    PD_ASSERT (argc == n * 3);
    
    for (i = 0; i < FONT_LIST_SIZE; i++) {
    //
    int best = 0;
    double requiredWidth  = font_fontList[i].fi_requiredWidth;
    double requiredHeight = font_fontList[i].fi_requiredHeight;
    
    for (j = 0; j < n; j++) {
    //
    double w = atom_getFloatAtIndex ((3 * j) + 1, argc, argv);
    double h = atom_getFloatAtIndex ((3 * j) + 2, argc, argv);
    
    if (w <= requiredWidth && h <= requiredHeight) { 
        best = j; 
    }
    //
    }
    
    font_fontList[i].fi_hostSize   = atom_getFloatAtIndex ((3 * best) + 0, argc, argv);
    font_fontList[i].fi_hostWidth  = atom_getFloatAtIndex ((3 * best) + 1, argc, argv);
    font_fontList[i].fi_hostHeight = atom_getFloatAtIndex ((3 * best) + 2, argc, argv);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
