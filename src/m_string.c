
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

t_error string_append (char *dest, size_t size, const char *src, int n)
{
    size_t d = strlen (dest);
    size_t k = (size - 1) - d;
    size_t s = 0;
        
    PD_ASSERT (size > d);
    
    if (n < 0) { s = strlen (src); }
    else {
        const char *t = src; while (*t && s < n) { s++; t++; }
    }
    
    strncat (dest, src, PD_MIN (s, k));
    
    if (s <= k) { return PD_ERROR_NONE; }
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

t_error string_addSprintf (char *dest, size_t size, const char *format, ...)
{
    int t;
    va_list args;
    size_t d = strlen (dest);
    
    PD_ASSERT (size > d);
    
    va_start (args, format);
    t = vsnprintf (dest + d, size - d, format, args);
    va_end (args);
    
    if (t >= 0 && (size_t)t < (size - d)) { return PD_ERROR_NONE; }
    else {
        return PD_ERROR;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int string_containsAtStart (const char *s, const char *isContained)
{
    size_t n = strlen (isContained);
    
    if (strlen (s) >= n) { return (strncmp (s, isContained, n) == 0); }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int rangecolor(int n)    /* 0 to 9 in 5 steps */
{
    int n2 = (n == 9 ? 8 : n);               /* 0 to 8 */
    int ret = (n2 << 5);        /* 0 to 256 in 9 steps */
    if (ret > 255) ret = 255;
    return (ret);
}

void numbertocolor(int n, char *s)
{
    int red, blue, green;
    if (n < 0) n = 0;
    red = n / 100;
    blue = ((n / 10) % 10);
    green = n % 10;
    sprintf(s, "#%2.2x%2.2x%2.2x", rangecolor(red), rangecolor(blue),
        rangecolor(green));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int string_indexOfFirstCharUntil (char *s, char c, size_t n)
{
    char *s2 = s + n;
    
    int i = 0;
    
    while (s != s2) {
        if (*s == c) { return i; } 
        i++; 
        s++;
    }
    
    return -1;
}

static int string_indexOfFirstCharFrom (char *s, char c, size_t n)
{
    char *s2 = s + n;
    
    while (s2 != s) { 
        s2--;
        n--;
        if (*s2 == c) { return n; }
    }
    
    return -1;
}

int string_indexOfFirstOccurrenceUntil (char *s, const char *c, size_t n)
{
    int k = n;
    int t = 0;
    
    while (*c != 0) { 
        t = string_indexOfFirstCharUntil (s, *c, n);
        if ((t >= 0) && (t < k)) { k = t; }
        c++; 
    }
    
    return (k < n ? k : -1);
}

int string_indexOfFirstOccurrenceFrom (char *s, const char *c, size_t n)
{
    int k = -1;
    int t = 0;
    
    while (*c != 0) { 
        t = string_indexOfFirstCharFrom (s, *c, n);
        if (t > k) { k = t; }
        c++; 
    }
    
    return k;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
