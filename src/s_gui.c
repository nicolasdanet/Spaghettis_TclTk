
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

extern char *interface_outGuiBuffer;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern int  interface_outGuiBufferSize;
extern int  interface_outGuiBufferHead;
extern int  interface_outGuiBufferTail;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void sys_guiEnlarge()
{
    const int overflow = (1024 * 128 * 1024);
    
    int oldSize = interface_outGuiBufferSize;
    int newSize = oldSize * 2;
    
    PD_ASSERT (newSize <= overflow); 
    PD_ABORT (newSize > overflow);      /* GUI buffer no more consumed? */
    
    interface_outGuiBuffer = PD_MEMORY_RESIZE (interface_outGuiBuffer, oldSize, newSize);
    interface_outGuiBufferSize = newSize;
}

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

void sys_guiFlush (void)
{

}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#else

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
    dest = interface_outGuiBuffer + interface_outGuiBufferHead;
    size = interface_outGuiBufferSize - interface_outGuiBufferHead;
    t = vsnprintf (dest, size, format, ap);
    va_end (ap);
    
    if (t < 0) { PD_BUG; return; }
    
    if ((size_t)t >= size) { sys_guiEnlarge(); }
    else {
        bufferWasTooSmall = 0;
        interface_outGuiBufferHead += t;
    }
    //
    } while (bufferWasTooSmall);
}

void sys_gui (char *s)
{
    sys_vGui ("%s", s);
}

void sys_guiFlush (void)
{
    interface_flushBufferAndQueue();
}

#endif // PD_WITH_NOGUI

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
