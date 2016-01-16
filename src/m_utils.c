
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_private.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error utils_strncpy (char *dest, size_t size, const char *src)
{
    size_t s = strlen (src);
    
    PD_ASSERT (size > 0);
    
    strncpy (dest, src, PD_MIN (s, size));
    dest[PD_MIN (size - 1, s)] = 0;
    
    if (s < size) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

t_error utils_strnadd (char *dest, size_t size, const char *src)
{
    return utils_strncat (dest, size, src, -1);
}

t_error utils_strncat (char *dest, size_t size, const char *src, int length)
{
    size_t d = strlen (dest);
    size_t s = strlen (src);
    size_t n = (size - 1) - d;
    
    PD_ASSERT (size > d);
    
    if (length >= 0) { s = PD_MIN (s, (size_t)length); }
    
    strncat (dest, src, PD_MIN (s, n));
    
    if (s <= n) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

t_error utils_snprintf (char *dest, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    
    va_start (args, format);
    t = vsnprintf (dest, size, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < size) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int utils_isTokenEnd (char c) 
{
    return (c == ',' || c == ';');
}

int utils_isTokenEscape (char c)
{
    return (c == '\\');
}

int utils_isTokenWhitespace (char c)
{
    return (c == ' ' || c == '\n' || c == '\r' || c == '\t');
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error path_withNameAndDirectory (char *dest, size_t size, const char *name, const char *directory)
{
    t_error err = PD_ERROR;
    
    if (*name) {
        err = PD_ERROR_NONE;
        err |= utils_strncpy (dest, size, directory);
        err |= utils_strnadd (dest, size, "/");
        err |= utils_strnadd (dest, size, name);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
