
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
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
        warning_tooManyCharacters (sym_console);
    }
}

void post_warning (const char *fmt, ...)
{
    int k;
    char t[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    k = vsnprintf (t, PD_STRING, fmt, ap);
    va_end (ap);
    
    PD_ASSERT (k >= 0 && k < PD_STRING);

    sys_vGui ("::ui_console::warning {%s}\n", t);
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

/* On Mac OS X the syslog call seems to affect the JACK server. */
/* Consequently it should be reserved for exceptional situations. */

static void post_syslog (const char *s)
{
    openlog (PD_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
    syslog (LOG_ERR, "%s", s);
    closelog();
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
