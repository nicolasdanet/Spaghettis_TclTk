
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_class *netreceive_class;                  /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _netreceive {
    t_object            ns_obj;             /* Must be the first. */
    int                 ns_fd;
    int                 ns_protocol;
    int                 ns_isBinary;
    int                 nr_pollersSize;
    int                 *nr_pollers;
    t_outlet            *ns_outletLeft;
    t_outlet            *ns_outletRight;
    } t_netreceive;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

void netsend_readbin (t_netreceive *x, int fd)
{
    unsigned char inbuf[PD_STRING];
    int ret = recv(fd, inbuf, PD_STRING, 0), i;
    if (!x->ns_outletRight)
    {
        PD_BUG;
        return;
    }
    if (ret <= 0)
    {
        if (ret < 0)
            PD_BUG;
        interface_monitorRemovePoller(fd);
        interface_closeSocket(fd);
        if (pd_class (x) == netreceive_class)
            netreceive_notify((t_netreceive *)x, fd);
    }
    else if (x->ns_protocol == SOCK_DGRAM)
    {
        t_atom *ap = (t_atom *)alloca(ret * sizeof(t_atom));
        for (i = 0; i < ret; i++)
            SET_FLOAT(ap+i, inbuf[i]);
        outlet_list(x->ns_outletRight, ret, ap);
    }
    else
    {
        for (i = 0; i < ret; i++)
            outlet_float(x->ns_outletRight, inbuf[i]);
    }
}


void netsend_doit(void *z, t_buffer *b)
{
    t_atom messbuf[1024];
    t_netreceive *x = (t_netreceive *)z;
    int msg, natom = buffer_size(b);
    t_atom *at = buffer_atoms(b);
    for (msg = 0; msg < natom;)
    {
        int emsg;
        for (emsg = msg; emsg < natom && at[emsg].a_type != A_COMMA
            && at[emsg].a_type != A_SEMICOLON; emsg++)
                ;
        if (emsg > msg)
        {
            int i;
            for (i = msg; i < emsg; i++)
                if (at[i].a_type == A_DOLLAR || at[i].a_type == A_DOLLARSYMBOL)
            {
                post_error ("netreceive: got dollar sign in message");
                goto nodice;
            }
            if (at[msg].a_type == A_FLOAT)
            {
                if (emsg > msg + 1)
                    outlet_list(x->ns_outletRight, emsg-msg, at + msg);
                else outlet_float(x->ns_outletRight, at[msg].a_w.w_float);
            }
            else if (at[msg].a_type == A_SYMBOL)
                outlet_anything(x->ns_outletRight, at[msg].a_w.w_symbol,
                    emsg-msg-1, at + msg + 1);
        }
    nodice:
        msg = emsg + 1;
    }
}

void netreceive_notify(t_netreceive *x, int fd)
{
    int i;
    for (i = 0; i < x->nr_pollersSize; i++)
    {
        if (x->nr_pollers[i] == fd)
        {
            memmove(x->nr_pollers+i, x->nr_pollers+(i+1),
                sizeof(int) * (x->nr_pollersSize - (i+1)));
            x->nr_pollers = (int *)PD_MEMORY_RESIZE(x->nr_pollers,
                x->nr_pollersSize * sizeof(int), 
                    (x->nr_pollersSize-1) * sizeof(int));
            x->nr_pollersSize--;
        }
    }
    outlet_float(x->ns_outletLeft, x->nr_pollersSize);
}

static void netreceive_connectpoll(t_netreceive *x)
{
    int fd = accept(x->ns_fd, 0, 0);
    if (fd < 0) post("netreceive: accept failed");
    else
    {
        int nconnections = x->nr_pollersSize+1;
        
        x->nr_pollers = (int *)PD_MEMORY_RESIZE(x->nr_pollers,
            x->nr_pollersSize * sizeof(int), nconnections * sizeof(int));
        x->nr_pollers[x->nr_pollersSize] = fd;
        if (x->ns_isBinary)
            interface_monitorAddPoller(fd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, fd, 
            (t_notifyfn)netreceive_notify,
                (x->ns_outletRight ? netsend_doit : NULL), 0);
        }
        outlet_float(x->ns_outletLeft, (x->nr_pollersSize = nconnections));
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void netreceive_closeall(t_netreceive *x)
{
    int i;
    for (i = 0; i < x->nr_pollersSize; i++)
    {
        interface_monitorRemovePoller(x->nr_pollers[i]);
        interface_closeSocket(x->nr_pollers[i]);
    }
    x->nr_pollers = (int *)PD_MEMORY_RESIZE(x->nr_pollers, 
        x->nr_pollersSize * sizeof(int), 0);
    x->nr_pollersSize = 0;
    if (x->ns_fd >= 0)
    {
        interface_monitorRemovePoller(x->ns_fd);
        interface_closeSocket(x->ns_fd);
    }
    x->ns_fd = -1;
}

static void netreceive_listen(t_netreceive *x, t_float fportno)
{
    int portno = fportno, intarg;
    struct sockaddr_in server;
    netreceive_closeall(x);
    if (portno <= 0)
        return;
    x->ns_fd = socket(AF_INET, x->ns_protocol, 0);
    if (x->ns_fd < 0)
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
    if (setsockopt(x->ns_fd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&intarg, sizeof(intarg)) < 0)
            post("netreceive: setsockopt (SO_REUSEADDR) failed\n");
#endif
#if 0
    intarg = 0;
    if (setsockopt(x->ns_fd, SOL_SOCKET, SO_RCVBUF,
        &intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_RCVBUF) failed\n");
#endif
    intarg = 1;
    if (setsockopt(x->ns_fd, SOL_SOCKET, SO_BROADCAST, 
        (const void *)&intarg, sizeof(intarg)) < 0)
            post("netreceive: failed to sett SO_BROADCAST");
        /* Stream (TCP) sockets are set NODELAY */
    if (x->ns_protocol == SOCK_STREAM)
    {
        intarg = 1;
        if (setsockopt(x->ns_fd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&intarg, sizeof(intarg)) < 0)
                post("setsockopt (TCP_NODELAY) failed\n");
    }
        /* assign server port number etc */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((u_short)portno);

        /* name the socket */
    if (bind(x->ns_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        PD_BUG;
        interface_closeSocket(x->ns_fd);
        x->ns_fd = -1;
        return;
    }

    if (x->ns_protocol == SOCK_DGRAM)        /* datagram protocol */
    {
        if (x->ns_isBinary)
            interface_monitorAddPoller(x->ns_fd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, x->ns_fd, 
                (t_notifyfn)netreceive_notify, (x->ns_outletRight ? netsend_doit : NULL), 1);
            x->ns_outletLeft = 0;
        }
    }
    else        /* streaming protocol */
    {
        if (listen(x->ns_fd, 5) < 0)
        {
            PD_BUG;
            interface_closeSocket(x->ns_fd);
            x->ns_fd = -1;
        }
        else
        {
            interface_monitorAddPoller(x->ns_fd, (t_pollfn)netreceive_connectpoll, x);
            x->ns_outletLeft = outlet_new (cast_object (x), &s_float);
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *netreceive_new(t_symbol *s, int argc, t_atom *argv)
{
    t_netreceive *x = (t_netreceive *)pd_new(netreceive_class);
    int portno = 0;
    x->ns_protocol = SOCK_STREAM;
    x->ns_isBinary = 0;
    x->nr_pollersSize = 0;
    x->nr_pollers = (int *)PD_MEMORY_GET(0);
    x->ns_fd = -1;
    if (argc && argv->a_type == A_FLOAT)
    {
        portno = atom_getFloatAtIndex(0, argc, argv);
        x->ns_protocol = (atom_getFloatAtIndex(1, argc, argv) != 0 ?
            SOCK_DGRAM : SOCK_STREAM);
        argc = 0;
    }
    else 
    {
        while (argc && argv->a_type == A_SYMBOL &&
            *argv->a_w.w_symbol->s_name == '-')
        {
            if (!strcmp(argv->a_w.w_symbol->s_name, "-b"))
                x->ns_isBinary = 1;
            else if (!strcmp(argv->a_w.w_symbol->s_name, "-u"))
                x->ns_protocol = SOCK_DGRAM;
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
        x->ns_outletRight = 0;
    }
    else x->ns_outletRight = outlet_new(cast_object (x), &s_anything);
        /* create a socket */
    if (portno > 0)
        netreceive_listen(x, portno);

    return (x);
}

static void netreceive_free (t_netreceive *x)
{
    netreceive_closeall (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void netreceive_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_netreceive,
            (t_newmethod)netreceive_new,
            (t_method)netreceive_free,
            sizeof (t_netreceive),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addMethod (c, (t_method)netreceive_listen, sym_listen, A_FLOAT, A_NULL);
        
    netreceive_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
