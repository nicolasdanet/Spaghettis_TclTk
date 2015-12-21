
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

void error (const char *fmt, ...)
{
    char buf[PD_STRING];
    va_list ap;

    va_start (ap, fmt);
    vsnprintf (buf, PD_STRING - 1, fmt, ap);
    va_end (ap);
    strcat (buf, "\n");

    doerror (NULL, buf);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
