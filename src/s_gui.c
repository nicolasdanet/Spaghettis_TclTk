
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  interface_guiSocket;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char *gui_buffer;        /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static int  gui_bufferSize;     /* Shared. */
static int  gui_bufferHead;     /* Shared. */
static int  gui_bufferTail;     /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define GUI_BUFFER_SIZE     (1024 * 128)
#define GUI_BUFFER_ABORT    (1024 * 128 * 1024)

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ! ( PD_WITH_NOGUI )

static void gui_enlargeBuffer()
{
    int oldSize = gui_bufferSize;
    int newSize = oldSize * 2;
    
    PD_ASSERT (newSize <= GUI_BUFFER_ABORT); 
    PD_ABORT (newSize > GUI_BUFFER_ABORT);          /* GUI buffer no more consumed? */
    
    gui_buffer = PD_MEMORY_RESIZE (gui_buffer, oldSize, newSize);
    gui_bufferSize = newSize;
}

static int gui_flushBuffer (void)
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

static int gui_flushBufferAndQueue (void)
{
    int didSomething = 0;
    
    didSomething |= interface_flushQueue();
    didSomething |= gui_flushBuffer();

    return didSomething;
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void gui_initialize (void)
{
    #if ! ( PD_WITH_NOGUI )
    
    gui_buffer = (char *)PD_MEMORY_GET (GUI_BUFFER_SIZE);
    gui_bufferSize = GUI_BUFFER_SIZE;
    
    #endif
}

void gui_release (void)
{
    #if ! ( PD_WITH_NOGUI )
    
    PD_MEMORY_FREE (gui_buffer);
    
    #endif
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ! ( PD_WITH_NOGUI )

void sys_vGui (char *format, ...)
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
    
    if ((size_t)t >= size) { gui_enlargeBuffer(); }
    else {
        bufferWasTooSmall = 0;
        gui_bufferHead += t;
    }
    //
    } while (bufferWasTooSmall);
}

void sys_gui (char *s)
{
    sys_vGui ("%s", s);
}

int sys_guiPollOrFlush (void)
{
    return (interface_monitorNonBlocking() || gui_flushBufferAndQueue());
}

void sys_guiFlush (void)
{
    gui_flushBufferAndQueue();
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if PD_WITH_NOGUI

void sys_vGui (char *fmt, ...)
{
}

void sys_gui (char *s)
{
}

int sys_guiPollOrFlush (void)
{
    return interface_monitorNonBlocking();
}

void sys_guiFlush (void)
{
}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
