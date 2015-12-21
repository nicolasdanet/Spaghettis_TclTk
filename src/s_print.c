
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

/* escape characters for tcl/tk */
static char* strnescape(char *dest, const char *src, size_t len)
{
    int ptin = 0;
    unsigned ptout = 0;
    for(; ptout < len; ptin++, ptout++)
    {
        int c = src[ptin];
        if (c == '\\' || c == '{' || c == '}')
            dest[ptout++] = '\\';
        dest[ptout] = src[ptin];
        if (c==0) break;
    }

    if(ptout < len) 
        dest[ptout]=0;
    else 
        dest[len-1]=0;

    return dest;
}

static char* strnpointerid(char *dest, const void *pointer, size_t len)
{
    *dest=0;
    if (pointer) 
        snprintf(dest, len, ".x%lx", (unsigned long)pointer);
    return dest;
}

static void dopost(const char *s)
{
    if (0)
#ifdef _WIN32
        fwprintf(stderr, L"%S", s);
#else
        fprintf(stderr, "%s", s);
#endif
    else
    {
        char upbuf[PD_STRING];
        sys_vgui("::pd_console::post {%s}\n", strnescape(upbuf, s, PD_STRING));
    }
}

static void doerror(const void *object, const char *s)
{
    char upbuf[PD_STRING];
    upbuf[PD_STRING-1]=0;

    if (0) {
        fprintf(stderr, "error: %s", s);
    } else { 
        char obuf[PD_STRING];
        sys_vgui("::pd_console::post {%s}\n",
                 strnescape(upbuf, s, PD_STRING));
                 /* */
    }
}

static void dologpost(const void *object, const int level, const char *s)
{
    char upbuf[PD_STRING];
    upbuf[PD_STRING-1]=0;

    if (0) 
    {
        fprintf(stderr, "verbose(%d): %s", level, s);
    }
    else
    {
        char obuf[PD_STRING];
        sys_vgui("::pd_console::post {%s}\n",  
                 strnescape(upbuf, s, PD_STRING));
                 /* */
    }
}

void logpost(const void *object, const int level, const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    t_int arg[8];
    int i;
    va_start(ap, fmt);
    vsnprintf(buf, PD_STRING-1, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");

    dologpost(object, level, buf);
}

void post(const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    t_int arg[8];
    int i;
    va_start(ap, fmt);
    vsnprintf(buf, PD_STRING-1, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");

    dopost(buf);
}

void startpost(const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    t_int arg[8];
    int i;
    va_start(ap, fmt);
    vsnprintf(buf, PD_STRING-1, fmt, ap);
    va_end(ap);

    dopost(buf);
}

void poststring(const char *s)
{
    dopost(" ");

    dopost(s);
}

void postatom(int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        char buf[PD_STRING];
        atom_string(argv+i, buf, PD_STRING);
        poststring(buf);
    }
}

void postfloat(t_float f)
{
    char buf[80];
    t_atom a;
    SETFLOAT(&a, f);

    postatom(1, &a);
}

void endpost(void)
{
    if (0) {
        fprintf(stderr, "\n");
    } else { 
        post("");
    }
}

void error(const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    t_int arg[8];
    int i;

    va_start(ap, fmt);
    vsnprintf(buf, PD_STRING-1, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");

    doerror(NULL, buf);
}

void verbose(int level, const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    t_int arg[8];
    int i;
    int loglevel=level+3;

    //if(level>sys_verbose)return;

    va_start(ap, fmt);
    vsnprintf(buf, PD_STRING-1, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");

    dologpost(NULL, loglevel, buf);
}

    /* here's the good way to log errors -- keep a pointer to the
    offending or offended object around so the user can search for it
    later. */

static void *error_object;
static char error_string[256];
void canvas_finderror(void *object);

void pd_error(void *object, const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    t_int arg[8];
    int i;
    static int saidit;

    va_start(ap, fmt);
    vsnprintf(buf, PD_STRING-1, fmt, ap);
    va_end(ap);
    strcat(buf, "\n");

    doerror(object, buf);

    error_object = object;
    strncpy(error_string, buf, 256);
    error_string[255] = 0;

    if (!saidit)
    {
        logpost(NULL, 4,
                "... you might be able to track this down from the Find menu.");
        saidit = 1;
    }
}

void glob_finderror(t_pd *dummy)
{
    if (!error_object)
        post("no findable error yet.");
    else
    {
        post("last trackable error:");
        post("%s", error_string);
        canvas_finderror(error_object);
    }
}

void glob_findinstance(t_pd *dummy, t_symbol*s)
{
    // revert s to (potential) pointer to object
    long obj = 0;
    if (sscanf(s->s_name, ".x%lx", &obj))
    {
        if (obj)
        {
            canvas_finderror((void *)obj);
        }
    }
}

void bug (const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;
    
    va_start (ap, fmt);
    vsnprintf (buf, PD_STRING - 1, fmt, ap);
    va_end (ap);

    error("consistency check failed: %s", buf);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
