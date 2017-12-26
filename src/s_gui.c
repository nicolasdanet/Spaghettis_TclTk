
/* Copyright (c) 1997-2017 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_spaghettis.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  interface_guiSocket;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char *gui_buffer;        /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  gui_bufferSize;     /* Static. */
static int  gui_bufferHead;     /* Static. */
static int  gui_bufferTail;     /* Static. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define GUI_BUFFER_SIZE     (1024 * 128)
#define GUI_BUFFER_ABORT    (1024 * 128 * 1024)     /* Arbitrary. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gui_jobClear (void);
int  gui_jobFlush (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static void gui_bufferEnlarge()
{
    int oldSize = gui_bufferSize;
    int newSize = oldSize * 2;
    
    PD_ASSERT (newSize <= GUI_BUFFER_ABORT); 
    PD_ABORT (newSize > GUI_BUFFER_ABORT);          /* GUI buffer no more consumed? */
    
    gui_buffer = (char *)PD_MEMORY_RESIZE (gui_buffer, oldSize, newSize);
    gui_bufferSize = newSize;
}

static int gui_bufferFlush (void)
{
    int need = gui_bufferHead - gui_bufferTail;
    
    if (need > 0) {
    //
    char *p = gui_buffer + gui_bufferTail;
    int done = (int)send (interface_guiSocket, (void *)p, need, 0);

    if (done < 0) { PD_BUG; scheduler_needToExitWithError(); }
    else {
        if (done == 0) { return 0; }    
        else if (done == need) { gui_bufferHead = gui_bufferTail = 0; }
        else {
            PD_ASSERT (done < need); gui_bufferTail += done;
        }
        
        return 1;
    }
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gui_vAdd (const char *format, ...)
{
    int bufferWasTooSmall = 1;
    
    do {
    //
    int t;
    size_t size;
    char *dest = NULL;
    va_list ap;
    
    va_start (ap, format);
    dest = gui_buffer + gui_bufferHead;
    size = gui_bufferSize - gui_bufferHead;
    t = vsnprintf (dest, size, format, ap);
    va_end (ap);
    
    if (t < 0) { PD_BUG; return; }
    
    if ((size_t)t >= size) { gui_bufferEnlarge(); }
    else {
        bufferWasTooSmall = 0;
        gui_bufferHead += t;
    }
    //
    } while (bufferWasTooSmall);
}

void gui_add (const char *s)
{
    gui_vAdd ("%s", s);
}

int gui_flush (void)
{
    int didSomething = 0;
    
    didSomething |= gui_jobFlush();
    didSomething |= gui_bufferFlush();

    return didSomething;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void gui_initialize (void)
{
    gui_buffer = (char *)PD_MEMORY_GET (GUI_BUFFER_SIZE);
    gui_bufferSize = GUI_BUFFER_SIZE;
}

void gui_release (void)
{
    gui_jobClear(); PD_MEMORY_FREE (gui_buffer);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
