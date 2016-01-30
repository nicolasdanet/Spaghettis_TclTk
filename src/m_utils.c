
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char utils_time[] = __TIME__;    /* Shared. */
static char utils_date[] = __DATE__;    /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error string_copy (char *dest, size_t size, const char *src)
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

t_error string_add (char *dest, size_t size, const char *src)
{
    return string_append (dest, size, src, -1);
}

t_error string_append (char *dest, size_t size, const char *src, int length)
{
    size_t d = strlen (dest);
    size_t n = (size - 1) - d;
    size_t s = 0;
        
    PD_ASSERT (size > d);
    
    if (length < 0) { s = strlen (src); }
    else {
        const char *t = src; while (*t && s < length) { s++; t++; }
    }
    
    strncat (dest, src, PD_MIN (s, n));
    
    if (s <= n) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

t_error string_sprintf (char *dest, size_t size, const char *format, ...)
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

t_error utils_version (char *dest, size_t size)
{
    t_error err = string_sprintf (dest, size, "%s %s / %s / %s", PD_NAME, PD_VERSION, utils_date, utils_time);
    
    #if PD_WITH_DEBUG
        err |= string_add (dest, size, " / DEBUG");
    #endif
    
    #if PD_WITH_REALTIME
        err |= string_add (dest, size, " / RT");
    #endif
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int path_isFileExist (const char *filepath)
{
    struct stat t; return (stat (filepath, &t) == 0);
}

t_error path_withNameAndDirectory (char *dest, size_t size, const char *name, const char *directory)
{
    t_error err = PD_ERROR;
    
    if (*name) {
        err = PD_ERROR_NONE;
        err |= string_copy (dest, size, directory);
        err |= string_add (dest, size, "/");
        err |= string_add (dest, size, name);
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
