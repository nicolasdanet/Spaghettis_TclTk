
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

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
    
    if (t >= 0 && t < PD_STRING) { sys_vgui ("::ui_console::post {%s}\n", buf); }   // --
    else {
        post_error (PD_TRANSLATE ("console: too many characters per line"));   // --
    }
}

void post_log (const char *fmt, ...)
{
    int t;
    char buf[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    t = vsnprintf (buf, PD_STRING, fmt, ap);
    va_end (ap);
    
    if (t >= 0 && t < PD_STRING) { 
        openlog (PD_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
        syslog (LOG_ERR, "%s", buf);
        closelog();
    }
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

    sys_vgui ("::ui_console::post {%s}\n", buf);    // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void post_atoms (int argc, t_atom *argv)
{
    int i;
    
    for (i = 0; i < argc; i++) {
        char buf[PD_STRING];
        atom_toString (argv + i, buf, PD_STRING);
        post ("%s", buf);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
