
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

extern t_buffer     *interface_inBuffer;
extern t_receiver   *interface_inReceiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_receiver *receiver_new (void *owner, t_notifyfn notify, t_receivefn receive, int udp)
{
    t_receiver *x = (t_receiver *)PD_MEMORY_GET (sizeof (t_receiver));
    
    x->r_owner      = owner;
    x->r_inBuffer   = (char *)PD_MEMORY_GET (SOCKET_BUFFER_SIZE);
    x->r_inHead     = 0;
    x->r_inTail     = 0;
    x->r_isUdp      = udp;
    x->r_fnNotify   = notify;
    x->r_fnReceive  = receive;

    return x;
}

void receiver_free (t_receiver *x)
{
    PD_MEMORY_FREE (x->r_inBuffer);
    PD_MEMORY_FREE (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

    /* this is in a separately called subroutine so that the buffer isn't
    sitting on the stack while the messages are getting passed. */
static int socketreceiver_doread(t_receiver *x)
{
    char messbuf[SOCKET_BUFFER_SIZE], *bp = messbuf;
    int indx, first = 1;
    int inhead = x->r_inHead;
    int intail = x->r_inTail;
    char *inbuf = x->r_inBuffer;
    for (indx = intail; first || (indx != inhead);
        first = 0, (indx = (indx+1)&(SOCKET_BUFFER_SIZE-1)))
    {
            /* if we hit a semi that isn't preceeded by a \, it's a message
            boundary.  LATER we should deal with the possibility that the
            preceeding \ might itself be escaped! */
        char c = *bp++ = inbuf[indx];
        if (c == ';' && (!indx || inbuf[indx-1] != '\\'))
        {
            intail = (indx+1)&(SOCKET_BUFFER_SIZE-1);
            buffer_withStringUnzeroed(interface_inBuffer, messbuf, bp - messbuf);
            //if (0 /*sys_debuglevel*/ & DEBUG_MESSDOWN)
            //{
            //    write(2,  messbuf, bp - messbuf);
            //    write(2, "\n", 1);
            //}
            x->r_inHead = inhead;
            x->r_inTail = intail;
            return (1);
        }
    }
    return (0);
}

static void socketreceiver_getudp(t_receiver *x, int fd)
{
    char buf[SOCKET_BUFFER_SIZE+1];
    int ret = recv(fd, buf, SOCKET_BUFFER_SIZE, 0);
    if (ret < 0)
    {
        PD_BUG;
        interface_socketRemovePollCallback(fd);
        sys_closesocket(fd);
    }
    else if (ret > 0)
    {
        buf[ret] = 0;
#if 0
        post("%s", buf);
#endif
        if (buf[ret-1] != '\n')
        {
#if 0
            buf[ret] = 0;
            post_error ("dropped bad buffer %s\n", buf);
#endif
        }
        else
        {
            char *semi = strchr(buf, ';');
            if (semi) 
                *semi = 0;
            buffer_withStringUnzeroed(interface_inBuffer, buf, strlen(buf));
            if (x->r_fnReceive)
                (*x->r_fnReceive)(x->r_owner, interface_inBuffer);
            else { PD_BUG; }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void receiver_read (t_receiver *x, int fd)
{
    if (x->r_isUdp)   /* UDP ("datagram") socket protocol */
        socketreceiver_getudp(x, fd);
    else  /* TCP ("streaming") socket protocol */
    {
        char *semi;
        int readto =
            (x->r_inHead >= x->r_inTail ? SOCKET_BUFFER_SIZE : x->r_inTail-1);
        int ret;

            /* the input buffer might be full.  If so, drop the whole thing */
        if (readto == x->r_inHead)
        {
            fprintf(stderr, "pd: dropped message from gui\n");
            x->r_inHead = x->r_inTail = 0;
            readto = SOCKET_BUFFER_SIZE;
        }
        else
        {
            ret = recv(fd, x->r_inBuffer + x->r_inHead,
                readto - x->r_inHead, 0);
            if (ret < 0)
            {
                PD_BUG;
                if (x == interface_inReceiver) scheduler_needToExitWithError();
                else
                {
                    if (x->r_fnNotify)
                        (*x->r_fnNotify)(x->r_owner, fd);
                    interface_socketRemovePollCallback(fd);
                    sys_closesocket(fd);
                }
            }
            else if (ret == 0)
            {
                if (x == interface_inReceiver)
                {
                    fprintf(stderr, "pd: exiting\n");
                    scheduler_needToExit();
                    return;
                }
                else
                {
                    post("EOF on socket %d\n", fd);
                    if (x->r_fnNotify) (*x->r_fnNotify)(x->r_owner, fd);
                    interface_socketRemovePollCallback(fd);
                    sys_closesocket(fd);
                }
            }
            else
            {
                x->r_inHead += ret;
                if (x->r_inHead >= SOCKET_BUFFER_SIZE) x->r_inHead = 0;
                while (socketreceiver_doread(x))
                {
                    if (x->r_fnReceive)
                        (*x->r_fnReceive)(x->r_owner, interface_inBuffer);
                    else buffer_eval(interface_inBuffer, 0, 0, 0);
                    if (x->r_inHead == x->r_inTail)
                        break;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
