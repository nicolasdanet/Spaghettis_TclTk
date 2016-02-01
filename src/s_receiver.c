
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

extern t_buffer             *interface_inBuffer;
extern t_socketreceiver     *interface_inReceiver;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_socketreceiver *socketreceiver_new(void *owner, t_notifyfn notifier,
    t_receivefn socketreceivefn, int udp)
{
    t_socketreceiver *x = (t_socketreceiver *)PD_MEMORY_GET(sizeof(*x));
    x->sr_inHead = x->sr_inTail = 0;
    x->sr_owner = owner;
    x->sr_fnNotify = notifier;
    x->sr_fnReceive = socketreceivefn;
    x->sr_isUdp = udp;
    if (!(x->sr_inBuffer = malloc(SOCKET_BUFFER_SIZE))) { PD_BUG; }
    return (x);
}

void socketreceiver_free(t_socketreceiver *x)
{
    free(x->sr_inBuffer);
    PD_MEMORY_FREE(x);
}

    /* this is in a separately called subroutine so that the buffer isn't
    sitting on the stack while the messages are getting passed. */
static int socketreceiver_doread(t_socketreceiver *x)
{
    char messbuf[SOCKET_BUFFER_SIZE], *bp = messbuf;
    int indx, first = 1;
    int inhead = x->sr_inHead;
    int intail = x->sr_inTail;
    char *inbuf = x->sr_inBuffer;
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
            x->sr_inHead = inhead;
            x->sr_inTail = intail;
            return (1);
        }
    }
    return (0);
}

static void socketreceiver_getudp(t_socketreceiver *x, int fd)
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
            if (x->sr_fnReceive)
                (*x->sr_fnReceive)(x->sr_owner, interface_inBuffer);
            else { PD_BUG; }
        }
    }
}

void socketreceiver_read(t_socketreceiver *x, int fd)
{
    if (x->sr_isUdp)   /* UDP ("datagram") socket protocol */
        socketreceiver_getudp(x, fd);
    else  /* TCP ("streaming") socket protocol */
    {
        char *semi;
        int readto =
            (x->sr_inHead >= x->sr_inTail ? SOCKET_BUFFER_SIZE : x->sr_inTail-1);
        int ret;

            /* the input buffer might be full.  If so, drop the whole thing */
        if (readto == x->sr_inHead)
        {
            fprintf(stderr, "pd: dropped message from gui\n");
            x->sr_inHead = x->sr_inTail = 0;
            readto = SOCKET_BUFFER_SIZE;
        }
        else
        {
            ret = recv(fd, x->sr_inBuffer + x->sr_inHead,
                readto - x->sr_inHead, 0);
            if (ret < 0)
            {
                PD_BUG;
                if (x == interface_inReceiver) scheduler_needToExitWithError();
                else
                {
                    if (x->sr_fnNotify)
                        (*x->sr_fnNotify)(x->sr_owner, fd);
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
                    if (x->sr_fnNotify) (*x->sr_fnNotify)(x->sr_owner, fd);
                    interface_socketRemovePollCallback(fd);
                    sys_closesocket(fd);
                }
            }
            else
            {
                x->sr_inHead += ret;
                if (x->sr_inHead >= SOCKET_BUFFER_SIZE) x->sr_inHead = 0;
                while (socketreceiver_doread(x))
                {
                    if (x->sr_fnReceive)
                        (*x->sr_fnReceive)(x->sr_owner, interface_inBuffer);
                    else buffer_eval(interface_inBuffer, 0, 0, 0);
                    if (x->sr_inHead == x->sr_inTail)
                        break;
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
