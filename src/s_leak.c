
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#if PD_WITH_DEBUG

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define LEAK_BUFFER_SIZE        65536
#define LEAK_BUFFER_STEP        64

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int              leak_maximum;                           /* Shared. */
static t_int            leak_allocated[LEAK_BUFFER_SIZE];       /* Shared. */
static const char *     leak_function[LEAK_BUFFER_SIZE];        /* Shared. */
static int              leak_line[LEAK_BUFFER_SIZE];            /* Shared. */
static pthread_mutex_t  leak_mutex;                             /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void leak_report (void)
{
    int i;
    
    pthread_mutex_lock (&leak_mutex);
    
    for (i = leak_maximum - 1; i >= 0; i--) {
        if (leak_allocated[i] != 0) { 
            post_log ("*** Leak / %s / line %d", leak_function[i], leak_line[i]);
        }
    }
    
    pthread_mutex_unlock (&leak_mutex);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void leak_initialize (void)
{
    pthread_mutex_init (&leak_mutex, NULL);
}

void leak_release (void)
{
    leak_report(); pthread_mutex_destroy (&leak_mutex);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void leak_set (int i, t_int t, const char *f, const int line)
{
    leak_allocated[i] = t;
    leak_function[i]  = f;
    leak_line[i]      = line;
}

static void leak_unset (int i)
{
    leak_allocated[i] = 0;
    leak_function[i]  = NULL;
    leak_line[i]      = 0;
}

static int leak_enlarge (void)
{
    int n = leak_maximum + LEAK_BUFFER_STEP;
    
    if (n <= LEAK_BUFFER_SIZE) { leak_maximum = n; return (leak_maximum - 1); }
    else {
        PD_BUG; return -1;
    }
}

static void leak_crop (void)
{
    while (leak_maximum > 0 && leak_allocated[leak_maximum - 1] == 0) { leak_maximum--; }
}

static int leak_slot (void)
{
    int i;
    
    for (i = leak_maximum - 1; i >= 0; i--) { if (leak_allocated[i] == 0) { return i; } }
    
    return leak_enlarge();
}

static int leak_contains (t_int t)
{
    int i;
    
    for (i = leak_maximum - 1; i >= 0; i--) { if (leak_allocated[i] == t) { return i; } }
    
    return -1;
}

static void leak_add (t_int t, const char *f, const int line)
{
    int slot = leak_slot();
    
    if (slot >= 0) { leak_set (slot, t, f, line); }
}

static void leak_update (t_int ptr, t_int t, const char *f, const int line)
{
    int slot = leak_contains (ptr);
    
    if (slot >= 0) { leak_unset (slot); leak_set (slot, t, f, line); }
}

static void leak_remove (t_int ptr, const char *f, const int line)
{
    int slot = leak_contains (ptr);
    
    if (slot >= 0) { leak_unset (slot); leak_crop(); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void *sys_getMemoryChecked (size_t n, const char *f, int line)
{
    void *t = sys_getMemory (n);
    
    pthread_mutex_lock (&leak_mutex);
    
        leak_add ((t_int)t, f, line);
        
    pthread_mutex_unlock (&leak_mutex);
    
    return t;
}

void *sys_getMemoryResizeChecked (void *ptr, size_t oldSize, size_t newSize, const char *f, int line)
{
    void *t = sys_getMemoryResize (ptr, oldSize, newSize);
    
    pthread_mutex_lock (&leak_mutex);
    
        leak_update ((t_int)ptr, (t_int)t, f, line);
    
    pthread_mutex_unlock (&leak_mutex);
    
    return t;
}

void sys_freeMemoryChecked (void *ptr, const char *f, int line)
{
    pthread_mutex_lock (&leak_mutex);
    
        leak_remove ((t_int)ptr, f, line);
    
    pthread_mutex_unlock (&leak_mutex);
    
    return sys_freeMemory (ptr);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // PD_WITH_DEBUG

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
