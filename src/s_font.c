
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define FONT_LIST_SIZE  6

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

typedef struct _fontinfo {
    int fi_size;
    int fi_width;
    int fi_height;
    int fi_hostSize;
    int fi_hostWidth;
    int fi_hostHeight;
    } t_fontinfo;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int font_defaultSize = 12;                                     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_fontinfo font_fontList[FONT_LIST_SIZE] =       /* Shared. */
    {
        { 8,   6, 10, 0, 0, 0 }, 
        { 10,  7, 13, 0, 0, 0 }, 
        { 12,  9, 16, 0, 0, 0 },
        { 16, 10, 21, 0, 0, 0 }, 
        { 24, 15, 25, 0, 0, 0 }, 
        { 36, 25, 45, 0, 0, 0 }
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

int font_getNearestFontSize (int fontSize)
{
    return (font_getNearest (fontSize)->fi_size);
}

int font_getHostFontSize (int fontSize)
{
    int k = font_getNearest (fontSize)->fi_hostSize;
    
    PD_ASSERT (k);
    
    return PD_MAX (k, 1);
}

int font_getHostFontWidth (int fontSize)
{
    int k = font_getNearest (fontSize)->fi_hostWidth;
    
    PD_ASSERT (k);
    
    return PD_MAX (k, 1);
}

int font_getHostFontHeight (int fontSize)
{
    int k = font_getNearest (fontSize)->fi_hostHeight;
    
    PD_ASSERT (k);
    
    return PD_MAX (k, 1);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void font_withMeasured (void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int i, j;
    int n = argc / 3;
    
    PD_ASSERT (n > 0);
    PD_ASSERT (argc == n * 3);
    
    for (i = 0; i < FONT_LIST_SIZE; i++) {
    //
    int best = 0;
    int requiredWidth  = font_fontList[i].fi_width;
    int requiredHeight = font_fontList[i].fi_height;
    
    for (j = 0; j < n; j++) {
        int w = (int)atom_getFloatAtIndex ((3 * j) + 1, argc, argv);
        int h = (int)atom_getFloatAtIndex ((3 * j) + 2, argc, argv);
        if (w <= requiredWidth && h <= requiredHeight) { best = j; }
    }
    
    font_fontList[i].fi_hostSize   = (int)atom_getFloatAtIndex ((3 * best) + 0, argc, argv);
    font_fontList[i].fi_hostWidth  = (int)atom_getFloatAtIndex ((3 * best) + 1, argc, argv);
    font_fontList[i].fi_hostHeight = (int)atom_getFloatAtIndex ((3 * best) + 2, argc, argv);
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
