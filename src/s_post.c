
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
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
    
    if (t >= 0 && t < PD_STRING) { sys_vGui ("::ui_console::post {%s}\n", buf); }   // --
    else {
        post_error (PD_TRANSLATE ("console: too many characters per line"));   // --
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

    sys_vGui ("::ui_console::post {%s}\n", buf);    // --
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
        if (logger_isRunning()) { logger_appendStringNative (buf); }
        else {
            openlog (PD_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
            syslog (LOG_ERR, "%s", buf);
            closelog();
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void post_atoms (int argc, t_atom *argv)
{
    t_buffer *t = buffer_new();
    
    buffer_append (t, argc, argv);
    
    if (buffer_size (t)) {
    //
    char *s = NULL;
    int size = 0;
    
    buffer_toString (t, &s, &size);
    post ("%s", s);
    
    PD_MEMORY_FREE (s);
    //
    }
    
    buffer_free (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
