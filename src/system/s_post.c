
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#include "m_spaghettis.h"
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
    
    if (k >= 0 && k < PD_STRING) { gui_vAdd ("::ui_console::post {%s}\n", t); }     // --
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
    
    PD_UNUSED (k); PD_ASSERT (k >= 0 && k < PD_STRING);

    gui_vAdd ("::ui_console::warning {%s}\n", t);   // --
}

void post_error (const char *fmt, ...)
{
    int k;
    char t[PD_STRING] = { 0 };
    va_list ap;
    
    va_start (ap, fmt);
    k = vsnprintf (t, PD_STRING, fmt, ap);
    va_end (ap);
    
    PD_UNUSED (k); PD_ASSERT (k >= 0 && k < PD_STRING);

    gui_vAdd ("::ui_console::error {%s}\n", t);     // --
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if PD_WITH_DEBUG
    
static void post_syslog (const char *s)
{
    #if PD_APPLE
        openlog (PD_NAME, LOG_CONS | LOG_PID, LOG_USER);
    #else
        openlog (PD_NAME, LOG_CONS | LOG_PID | LOG_PERROR, LOG_USER);
    #endif
    
    syslog (LOG_ERR, "%s", s);
    
    closelog();
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Note that it is NOT thread-safe. */

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
        if (PD_WITH_LOGGER && logger_isRunning()) { logger_appendString (t); }
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
