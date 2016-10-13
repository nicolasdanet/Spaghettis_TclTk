
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Memory leak!!!!*/
/* receiver_free never called */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"
#include "x_control.h"

t_class *netreceive_class;

/* ----------------------------- netreceive ------------------------- */
void netreceive_notify(t_netreceive *x, int fd)
{
    int i;
    for (i = 0; i < x->x_pollersSize; i++)
    {
        if (x->x_pollers[i] == fd)
        {
            memmove(x->x_pollers+i, x->x_pollers+(i+1),
                sizeof(int) * (x->x_pollersSize - (i+1)));
            x->x_pollers = (int *)PD_MEMORY_RESIZE(x->x_pollers,
                x->x_pollersSize * sizeof(int), 
                    (x->x_pollersSize-1) * sizeof(int));
            x->x_pollersSize--;
        }
    }
    outlet_float(x->x_ns.x_outletLeft, x->x_pollersSize);
}

static void netreceive_connectpoll(t_netreceive *x)
{
    int fd = accept(x->x_ns.x_fd, 0, 0);
    if (fd < 0) post("netreceive: accept failed");
    else
    {
        int nconnections = x->x_pollersSize+1;
        
        x->x_pollers = (int *)PD_MEMORY_RESIZE(x->x_pollers,
            x->x_pollersSize * sizeof(int), nconnections * sizeof(int));
        x->x_pollers[x->x_pollersSize] = fd;
        if (x->x_ns.x_isBinary)
            interface_monitorAddPoller(fd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, fd, 
            (t_notifyfn)netreceive_notify,
                (x->x_ns.x_outletRight ? netsend_doit : NULL), 0);
        }
        outlet_float(x->x_ns.x_outletLeft, (x->x_pollersSize = nconnections));
    }
}

static void netreceive_closeall(t_netreceive *x)
{
    int i;
    for (i = 0; i < x->x_pollersSize; i++)
    {
        interface_monitorRemovePoller(x->x_pollers[i]);
        interface_closeSocket(x->x_pollers[i]);
    }
    x->x_pollers = (int *)PD_MEMORY_RESIZE(x->x_pollers, 
        x->x_pollersSize * sizeof(int), 0);
    x->x_pollersSize = 0;
    if (x->x_ns.x_fd >= 0)
    {
        interface_monitorRemovePoller(x->x_ns.x_fd);
        interface_closeSocket(x->x_ns.x_fd);
    }
    x->x_ns.x_fd = -1;
}

static void netreceive_listen(t_netreceive *x, t_float fportno)
{
    int portno = fportno, intarg;
    struct sockaddr_in server;
    netreceive_closeall(x);
    if (portno <= 0)
        return;
    x->x_ns.x_fd = socket(AF_INET, x->x_ns.x_protocol, 0);
    if (x->x_ns.x_fd < 0)
    {
        PD_BUG;
        return;
    }
#if 0
    fprintf(stderr, "receive socket %d\n", x->x_ sockfd);
#endif

#if 1
        /* ask OS to allow another Pd to repoen this port after we close it. */
    intarg = 1;
    if (setsockopt(x->x_ns.x_fd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&intarg, sizeof(intarg)) < 0)
            post("netreceive: setsockopt (SO_REUSEADDR) failed\n");
#endif
#if 0
    intarg = 0;
    if (setsockopt(x->x_ns.x_fd, SOL_SOCKET, SO_RCVBUF,
        &intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_RCVBUF) failed\n");
#endif
    intarg = 1;
    if (setsockopt(x->x_ns.x_fd, SOL_SOCKET, SO_BROADCAST, 
        (const void *)&intarg, sizeof(intarg)) < 0)
            post("netreceive: failed to sett SO_BROADCAST");
        /* Stream (TCP) sockets are set NODELAY */
    if (x->x_ns.x_protocol == SOCK_STREAM)
    {
        intarg = 1;
        if (setsockopt(x->x_ns.x_fd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&intarg, sizeof(intarg)) < 0)
                post("setsockopt (TCP_NODELAY) failed\n");
    }
        /* assign server port number etc */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((u_short)portno);

        /* name the socket */
    if (bind(x->x_ns.x_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        PD_BUG;
        interface_closeSocket(x->x_ns.x_fd);
        x->x_ns.x_fd = -1;
        return;
    }

    if (x->x_ns.x_protocol == SOCK_DGRAM)        /* datagram protocol */
    {
        if (x->x_ns.x_isBinary)
            interface_monitorAddPoller(x->x_ns.x_fd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, x->x_ns.x_fd, 
                (t_notifyfn)netreceive_notify, (x->x_ns.x_outletRight ? netsend_doit : NULL), 1);
            x->x_ns.x_outletLeft = 0;
        }
    }
    else        /* streaming protocol */
    {
        if (listen(x->x_ns.x_fd, 5) < 0)
        {
            PD_BUG;
            interface_closeSocket(x->x_ns.x_fd);
            x->x_ns.x_fd = -1;
        }
        else
        {
            interface_monitorAddPoller(x->x_ns.x_fd, (t_pollfn)netreceive_connectpoll, x);
            x->x_ns.x_outletLeft = outlet_new(&x->x_ns.x_obj, &s_float);
        }
    }
}


static void netreceive_send(t_netreceive *x, 
    t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < x->x_pollersSize; i++)
    {
        if (netsend_dosend(&x->x_ns, x->x_pollers[i], s, argc, argv))
            post_error ("netreceive send message failed");
                /* should we now close the connection? */
    }
}

static void *netreceive_new(t_symbol *s, int argc, t_atom *argv)
{
    t_netreceive *x = (t_netreceive *)pd_new(netreceive_class);
    int portno = 0;
    x->x_ns.x_protocol = SOCK_STREAM;
    x->x_ns.x_isBinary = 0;
    x->x_pollersSize = 0;
    x->x_pollers = (int *)PD_MEMORY_GET(0);
    x->x_ns.x_fd = -1;
    if (argc && argv->a_type == A_FLOAT)
    {
        portno = atom_getFloatAtIndex(0, argc, argv);
        x->x_ns.x_protocol = (atom_getFloatAtIndex(1, argc, argv) != 0 ?
            SOCK_DGRAM : SOCK_STREAM);
        argc = 0;
    }
    else 
    {
        while (argc && argv->a_type == A_SYMBOL &&
            *argv->a_w.w_symbol->s_name == '-')
        {
            if (!strcmp(argv->a_w.w_symbol->s_name, "-b"))
                x->x_ns.x_isBinary = 1;
            else if (!strcmp(argv->a_w.w_symbol->s_name, "-u"))
                x->x_ns.x_protocol = SOCK_DGRAM;
            else
            {
                post_error ("netreceive: unknown flag ...");
                error__post (argc, argv);
            }
            argc--; argv++;
        }
    }
    if (argc && argv->a_type == A_FLOAT)
        portno = argv->a_w.w_float, argc--, argv++;
    if (argc)
    {
        post_error ("netreceive: extra arguments ignored:");
        error__post (argc, argv);
    }
    if (0)
    {
        /* old style, nonsecure version */
        x->x_ns.x_outletRight = 0;
    }
    else x->x_ns.x_outletRight = outlet_new(&x->x_ns.x_obj, &s_anything);
        /* create a socket */
    if (portno > 0)
        netreceive_listen(x, portno);

    return (x);
}

void netreceive_setup(void)
{
    netreceive_class = class_new(sym_netreceive,
        (t_newmethod)netreceive_new, (t_method)netreceive_closeall,
        sizeof(t_netreceive), 0, A_GIMME, 0);
    class_addMethod(netreceive_class, (t_method)netreceive_listen,
        sym_listen, A_FLOAT, 0);
    class_addMethod(netreceive_class, (t_method)netreceive_send,
        sym_send, A_GIMME, 0);
}

