
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
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
    int k;
    char t[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    k = vsnprintf (t, PD_STRING, fmt, ap);
    va_end (ap);
    
    if (k >= 0 && k < PD_STRING) { sys_vGui ("::ui_console::post {%s}\n", t); }
    else {
        post_error (PD_TRANSLATE ("console: too many characters per line"));
    }
}

void post_error (const char *fmt, ...)
{
    int k;
    char t[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    k = vsnprintf (t, PD_STRING, fmt, ap);
    va_end (ap);
    
    PD_ASSERT (k >= 0 && k < PD_STRING);

    sys_vGui ("::ui_console::error {%s}\n", t);
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
    
    buffer_toString (t, &s);
    post_error ("%s", s);
    
    PD_MEMORY_FREE (s);
    //
    }
    
    buffer_free (t);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_DEBUG

void post_log (const char *fmt, ...)
{
    int k;
    char t[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    k = vsnprintf (t, PD_STRING, fmt, ap);
    va_end (ap);
    
    if (k >= 0 && k < PD_STRING) {
        if (PD_WITH_LOGGER && logger_isRunning()) { logger_appendStringNative (t); }
        else {
            post_syslog (t);
        }
    }
}

#else

void post_log (const char *fmt, ...)
{
}

#endif

/* On Mac OS X the syslog call seems to affect the JACK server. */
/* Consequently it should be reserved for exceptional situations. */

void post_syslog (const char *s)
{
    openlog (PD_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
    syslog (LOG_ERR, "%s", s);
    closelog();
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
