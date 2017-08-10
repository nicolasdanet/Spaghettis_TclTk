
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define HEAPSTRING_DEFAULT_SIZE     4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void heapstring_reserve (t_heapstring *x, size_t size)
{
    if (size > x->hs_size) {
    //
    size_t newSize = (size_t)PD_NEXT_POWER_2 (size);
    size_t oldSize = x->hs_size;

    x->hs_raw  = (char *)PD_MEMORY_RESIZE (x->hs_raw, oldSize, newSize);
    x->hs_size = newSize;
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

char *heapstring_getRaw (t_heapstring *x)
{
    return x->hs_raw;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_error heapstring_add (t_heapstring *x, const char *src)
{
    return heapstring_append (x, src, -1);
}

t_error heapstring_append (t_heapstring *x, const char *src, int n)
{
    size_t size = 0;
        
    if (n < 0) { size = strlen (src); }
    else {
        const char *t = src; while (*t && size < (size_t)n) { size++; t++; }
    }
    
    heapstring_reserve (x, x->hs_used + size + 1);
    
    strncat (x->hs_raw + x->hs_used, src, size);
    x->hs_used += size;
    
    return PD_ERROR_NONE;
}

t_error heapstring_addSprintf (t_heapstring *x, const char *format, ...)
{
    size_t required = x->hs_used + (x->hs_size / 4);
    t_error err = PD_ERROR_NONE;
    va_list args;
    
    do {
    //
    heapstring_reserve (x, required);
    
    {
        size_t remains = x->hs_size - x->hs_used;
        int t;
        
        va_start (args, format);
        t = vsnprintf (x->hs_raw + x->hs_used, remains, format, args);
        va_end (args);
    
        if (t < 0) { PD_BUG; return PD_ERROR; }
        else {
            if ((err = ((size_t)t >= remains))) { required = x->hs_used + (size_t)t + 1; }
            else { 
                x->hs_used += (size_t)t;
            }
        }
    }
    //
    } while (err);
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

t_heapstring *heapstring_new (int size)
{
    t_heapstring *x = (t_heapstring *)PD_MEMORY_GET (sizeof (t_heapstring));
 
    x->hs_used = 0;
    x->hs_size = (size <= 0) ? HEAPSTRING_DEFAULT_SIZE : size;
    x->hs_raw  = (char *)PD_MEMORY_GET (x->hs_size * sizeof (char));
    
    return x;
}

void heapstring_free (t_heapstring *x)
{
    PD_MEMORY_FREE (x->hs_raw);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
