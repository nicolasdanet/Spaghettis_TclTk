
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define FONT_LIST_SIZE  6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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

static t_fontinfo font_fontList[FONT_LIST_SIZE] =       /* Static. */
    {
        { 8,   6, 10,   8.0,   6.0, 10.0 },
        { 10,  7, 13,   10.0,  7.0, 13.0 },
        { 12,  8, 16,   12.0,  8.0, 16.0 },
        { 16, 12, 20,   16.0, 12.0, 20.0 },
        { 24, 16, 32,   24.0, 16.0, 32.0 },
        { 36, 26, 46,   36.0, 26.0, 46.0 }
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

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
// MARK: -

int font_getHostFontSize (int fontSize)
{
    int k = (int)(font_getNearest (fontSize)->fi_hostSize);
    
    return PD_MAX (k, 1);
}

double font_getHostFontWidth (int fontSize)
{
    double k = font_getNearest (fontSize)->fi_hostWidth;
    
    return PD_MAX (k, 1.0);
}

double font_getHostFontHeight (int fontSize)
{
    double k = font_getNearest (fontSize)->fi_hostHeight;
    
    return PD_MAX (k, 1.0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void font_withHostMeasured (int argc, t_atom *argv)
{
    int i, j;
    int n = argc / 3;
    
    PD_ASSERT (n > 0);
    PD_ASSERT (argc == n * 3);
    
    for (i = 0; i < FONT_LIST_SIZE; i++) {
    //
    int best = 0;
    int    required       = font_fontList[i].fi_size;
    double requiredWidth  = font_fontList[i].fi_requiredWidth;
    double requiredHeight = font_fontList[i].fi_requiredHeight;
        
    for (j = 0; j < n; j++) {
    //
    int    k = atom_getFloatAtIndex ((3 * j) + 0, argc, argv);
    double w = atom_getFloatAtIndex ((3 * j) + 1, argc, argv);
    double h = atom_getFloatAtIndex ((3 * j) + 2, argc, argv);
    
    if (k == required) { best = j; break; }                         /* Always prefers exact match. */
    else if (w <= requiredWidth && h <= requiredHeight) {
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
