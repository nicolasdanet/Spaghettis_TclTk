
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

int utils_strncpy (char *dest, size_t size, const char *src)
{
    size_t s = strlen (src);
    
    strncpy (dest, src, PD_MIN (s, size));
    dest[PD_MIN (size - 1, s)] = 0;
    
    if (s < size) { return 0; }
    else {
        return 1;
    }
}

int utils_strncat (char *dest, size_t size, const char *src)
{
    size_t d = strlen (dest);
    size_t s = strlen (src);
    size_t n = (size - 1) - d;
    
    strncat (dest, src, PD_MIN (s, n));
    
    if (s <= n) { return 0; }
    else {
        return 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_snprintf (char *dest, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    
    va_start (args, format);
    t = vsnprintf (dest, size, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < size) { return 0; }
    else {
        return 1;
    }
}

int utils_snprintfCat (char *dest, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    size_t d = strlen (dest);
    
    va_start (args, format);
    t = vsnprintf (dest + d, size - d, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < (size - d)) { return 0; }
    else {
        return 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_isStatementEnd (char c) 
{
    return (c == ',' || c == ';');
}

int utils_isEscape (char c)
{
    return (c == '\\');
}

int utils_isWhitespace (char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_isDollarNumber (char *s)
{
    if (*s != '$') { return 0; } while (*(++s)) { if (*s < '0' || *s > '9') { return 0; } }
    
    return 1;
}

int utils_startsWithDollarNumber (char *s)
{
    PD_ASSERT (s[0] != 0);
    PD_ASSERT (s[1] != 0);
    
    if (s[0] != '$' || s[1] < '0' || s[1] > '9') { return 0; }
    
    return 1;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
