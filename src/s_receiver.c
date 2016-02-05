
/* 
    Copyright (c) 1997-2015 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_receiver   *interface_inGuiReceiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void receiver_closeSocketAndRemoveCallback (t_receiver *x)
{
    if (!x->r_isClosed) {
        interface_closeSocket (x->r_fd);
        interface_monitorRemovePoller (x->r_fd);
        x->r_isClosed = 1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_receiver *receiver_new (void *owner, int fd, t_notifyfn notify, t_receivefn receive, int isUdp)
{
    t_receiver *x = (t_receiver *)PD_MEMORY_GET (sizeof (t_receiver));
    
    x->r_owner      = owner;
    x->r_message    = buffer_new();
    x->r_inRaw      = (char *)PD_MEMORY_GET (SOCKET_BUFFER_SIZE);
    x->r_inHead     = 0;
    x->r_inTail     = 0;
    x->r_fd         = fd;
    x->r_isUdp      = isUdp;
    x->r_isClosed   = 0;
    x->r_fnNotify   = notify;
    x->r_fnReceive  = receive;

    interface_monitorAddPoller (x->r_fd, (t_pollfn)receiver_read, x);
    
    return x;
}

void receiver_free (t_receiver *x)
{
    receiver_closeSocketAndRemoveCallback (x);
    
    buffer_free (x->r_message);
    
    PD_MEMORY_FREE (x->r_inRaw);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int receiver_readHandleSemicolonEscaped (t_receiver *x, int i)
{
    if (i == 0) {
        return (x->r_inRaw[SOCKET_BUFFER_SIZE - 1] == '\\');
    } else { 
        return (x->r_inRaw[i - 1] == '\\');
    }
}

static int receiver_readHandleTCP (t_receiver *x)
{
    char t[SOCKET_BUFFER_SIZE] = { 0 };
    char *p = t;
    int i, first = 1;
    
    PD_ASSERT ((x->r_inHead != x->r_inTail) || (x->r_inHead == 0 && x->r_inTail == 0));
        
    for (i = x->r_inTail; first || (i != x->r_inHead); first = 0, (i = (i + 1) & (SOCKET_BUFFER_SIZE - 1))) {
    //
    char c = *p = x->r_inRaw[i];
    
    p++;

    if (c == ';' && (first || !receiver_readHandleSemicolonEscaped (x, i))) {
        x->r_inTail = (i + 1) & (SOCKET_BUFFER_SIZE - 1);
        buffer_withStringUnzeroed (x->r_message, t, p - t);
        return 1;
    }
    //
    }
    
    return 0;
}

static void receiver_readHandleDisconnect (t_receiver *x, int fd, int withError)
{
    if (x == interface_inGuiReceiver) { 
        if (withError) { scheduler_needToExitWithError(); }
        else {
            scheduler_needToExit(); 
        }
        
    } else {
        if (x->r_fnNotify) { 
            (*x->r_fnNotify) (x->r_owner, fd); 
        }

        receiver_closeSocketAndRemoveCallback (x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_error receiver_readUDP (t_receiver *x, int fd)
{
    char t[SOCKET_BUFFER_SIZE + 1] = { 0 };
    ssize_t length = recv (fd, t, SOCKET_BUFFER_SIZE, 0);
    t_error err = PD_ERROR;
    
    if (length < 0)       { receiver_readHandleDisconnect (x, fd, 1); PD_BUG; }
    else if (length == 0) { receiver_readHandleDisconnect (x, fd, 0); err = PD_ERROR_NONE; }
    else if (length > 0)  {
    //
    t[length] = 0;
    
    if (t[length - 1] != '\n') { PD_BUG; }
    else {
        char *semicolon = strchr (t, ';');
        if (semicolon) { *semicolon = 0; }
        buffer_withStringUnzeroed (x->r_message, t, strlen (t));
        if (x->r_fnReceive) { (*x->r_fnReceive) (x->r_owner, x->r_message); }
        err == PD_ERROR_NONE;
    }
    //
    }
    
    return err;
}

static t_error receiver_readTCP (t_receiver *x, int fd)
{
    int endOfAvailableSpace  = (x->r_inHead >= x->r_inTail ? SOCKET_BUFFER_SIZE : x->r_inTail - 1);
    int sizeOfAvailableSpace = endOfAvailableSpace - x->r_inHead;
    t_error err = PD_ERROR;
    
    if (sizeOfAvailableSpace == 0) { x->r_inHead = x->r_inTail = 0; PD_BUG; }
    else {
    //
    int length = recv (fd, x->r_inRaw + x->r_inHead, sizeOfAvailableSpace, 0);
    
    if (length < 0)       { receiver_readHandleDisconnect (x, fd, 1); PD_BUG; }
    else if (length == 0) { receiver_readHandleDisconnect (x, fd, 0); err = PD_ERROR_NONE; }
    else
    {
        x->r_inHead += length; if (x->r_inHead >= SOCKET_BUFFER_SIZE) { x->r_inHead = 0; }
        
        while (receiver_readHandleTCP (x)) {
            if (x->r_fnReceive) { (*x->r_fnReceive) (x->r_owner, x->r_message); }
            else { 
                buffer_eval (x->r_message, NULL, 0, NULL); 
            }
            if (x->r_inTail == x->r_inHead) { break; }
        }
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void receiver_read (t_receiver *x, int fd)
{
    if (x->r_isUdp) { receiver_readUDP (x, fd); }       /* Without buffer. */
    else {  
        receiver_readTCP (x, fd);                       /* With buffer. */
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
