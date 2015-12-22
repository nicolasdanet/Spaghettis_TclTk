
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include <stdio.h>
#include <stdarg.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void post (const char *fmt, ...)
{
    int t;
    char buf[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    t = vsnprintf (buf, PD_STRING, fmt, ap);
    va_end (ap);
    
    PD_ASSERT (t >= 0 && t < PD_STRING);
        
    sys_vgui ("::pd_console::post {%s}\n", buf);    // --
}

void post_log (const char *fmt, ...)
{
    int t;
    char buf[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    t = vsnprintf (buf, PD_STRING, fmt, ap);
    va_end (ap);
    
    if (t >= 0 && t < PD_STRING) { fprintf (stderr, "%s", buf); fputs ("\n", stderr); }
}

void post_error (const char *fmt, ...)
{
    int t;
    char buf[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    t = vsnprintf (buf, PD_STRING, fmt, ap);
    va_end (ap);
    
    PD_ASSERT (t >= 0 && t < PD_STRING);

    sys_vgui ("::pd_console::post {%s}\n", buf);    // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void post_atoms (int argc, t_atom *argv)
{
    int i;
    
    for (i = 0; i < argc; i++) {
        char buf[PD_STRING];
        atom_string (argv + i, buf, PD_STRING);
        post ("%s", buf);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
