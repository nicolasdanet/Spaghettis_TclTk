
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_strncpy (char *destination, size_t size, const char *source)
{
    size_t s = strlen (source);
    
    strncpy (destination, source, PD_MIN (s, size));
    destination[PD_MIN (size - 1, s)] = 0;
    
    if (s < size) { return 1; }
    else {
        return 0;
    }
}

int utils_strncat (char *destination, size_t size, const char *source)
{
    size_t d = strlen (destination);
    size_t s = strlen (source);
    size_t n = (size - 1) - d;
    
    strncat (destination, source, PD_MIN (s, n));
    
    if (s <= n) { return 1; }
    else {
        return 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_snprintf (char *destination, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    
    va_start (args, format);
    t = vsnprintf (destination, size, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < size) { return 1; }
    else {
        return 0;
    }
}

int utils_snprintfCat (char *destination, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    size_t d = strlen (destination);
    
    va_start (args, format);
    t = vsnprintf (destination + d, size - d, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < (size - d)) { return 1; }
    else {
        return 0;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
