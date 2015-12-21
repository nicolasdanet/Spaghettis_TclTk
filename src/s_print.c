
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void doPost (const char *s)
{
    sys_vgui ("::pd_console::post {%s}\n", s);
}

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

    doPost (buf);
}

void error (const char *fmt, ...)
{
    int t;
    char buf[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    t = vsnprintf (buf, PD_STRING, fmt, ap);
    va_end (ap);
    
    PD_ASSERT (t >= 0 && t < PD_STRING);

    doPost (buf);
}

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
