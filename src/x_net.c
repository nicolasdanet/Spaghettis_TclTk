/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* network */

/* Memory leak!!!!*/
/* ?????????????????????????????????????? receiver_free never called ??? */

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "s_system.h"

#include <sys/types.h>
#include <string.h>
#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#define SOCKET_ERROR -1
#endif

static t_class *netsend_class;

typedef struct _netsend
{
    t_object x_obj;
    t_outlet *x_msgout;
    t_outlet *x_connectout;
    int x_sockfd;
    int x_protocol;
    int x_bin;
} t_netsend;

static t_class *netreceive_class;

typedef struct _netreceive
{
    t_netsend x_ns;
    int x_nconnections;
    int x_sockfd;
    int *x_connections;
    int x_old;
} t_netreceive;

static void netreceive_notify(t_netreceive *x, int fd);

static void *netsend_new(t_symbol *s, int argc, t_atom *argv)
{
    t_netsend *x = (t_netsend *)pd_new(netsend_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_protocol = SOCK_STREAM;
    x->x_bin = 0;
    if (argc && argv->a_type == A_FLOAT)
    {
        x->x_protocol = (argv->a_w.w_float != 0 ? SOCK_DGRAM : SOCK_STREAM);
        argc = 0;
    }
    else while (argc && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        if (!strcmp(argv->a_w.w_symbol->s_name, "-b"))
            x->x_bin = 1;
        else if (!strcmp(argv->a_w.w_symbol->s_name, "-u"))
            x->x_protocol = SOCK_DGRAM;
        else
        {
            post_error ("netsend: unknown flag ...");
            post_atoms(argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post_error ("netsend: extra arguments ignored:");
        post_atoms(argc, argv);
    }
    x->x_sockfd = -1;
    if (x->x_protocol == SOCK_STREAM)
        x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    return (x);
}

static void netsend_readbin(t_netsend *x, int fd)
{
    unsigned char inbuf[PD_STRING];
    int ret = recv(fd, inbuf, PD_STRING, 0), i;
    if (!x->x_msgout)
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
        if (x->x_obj.te_g.g_pd == netreceive_class)
            netreceive_notify((t_netreceive *)x, fd);
    }
    else if (x->x_protocol == SOCK_DGRAM)
    {
        t_atom *ap = (t_atom *)alloca(ret * sizeof(t_atom));
        for (i = 0; i < ret; i++)
            SET_FLOAT(ap+i, inbuf[i]);
        outlet_list(x->x_msgout, 0, ret, ap);
    }
    else
    {
        for (i = 0; i < ret; i++)
            outlet_float(x->x_msgout, inbuf[i]);
    }
}

static void netsend_doit(void *z, t_buffer *b)
{
    t_atom messbuf[1024];
    t_netsend *x = (t_netsend *)z;
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
                    outlet_list(x->x_msgout, 0, emsg-msg, at + msg);
                else outlet_float(x->x_msgout, at[msg].a_w.w_float);
            }
            else if (at[msg].a_type == A_SYMBOL)
                outlet_anything(x->x_msgout, at[msg].a_w.w_symbol,
                    emsg-msg-1, at + msg + 1);
        }
    nodice:
        msg = emsg + 1;
    }
}


static void netsend_connect(t_netsend *x, t_symbol *hostname,
    t_float fportno)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno = fportno;
    int intarg;
    if (x->x_sockfd >= 0)
    {
        post_error ("netsend_connect: already connected");
        return;
    }

        /* create a socket */
    sockfd = socket(AF_INET, x->x_protocol, 0);
#if 0
    fprintf(stderr, "send socket %d\n", sockfd);
#endif
    if (sockfd < 0)
    {
        PD_BUG;
        return;
    }
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
        post("bad host?\n");
        return;
    }
#if 0
    intarg = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF,
        &intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_RCVBUF) failed\n");
#endif
    intarg = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, 
                  (const void *)&intarg, sizeof(intarg)) < 0)
        post("setting SO_BROADCAST");
        /* for stream (TCP) sockets, specify "nodelay" */
    if (x->x_protocol == SOCK_STREAM)
    {
        intarg = 1;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&intarg, sizeof(intarg)) < 0)
                post("setsockopt (TCP_NODELAY) failed\n");
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((u_short)portno);

    post("connecting to port %d", portno);
        /* try to connect.  LATER make a separate thread to do this
        because it might block */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
        PD_BUG;
        interface_closeSocket(sockfd);
        return;
    }
    x->x_sockfd = sockfd;
    if (x->x_msgout)    /* add polling function for return messages */
    {
        if (x->x_bin)
            interface_monitorAddPoller(sockfd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, sockfd, NULL, netsend_doit, 0);
        }
    }
    outlet_float(x->x_obj.te_outlet, 1);
}

static void netsend_disconnect(t_netsend *x)
{
    if (x->x_sockfd >= 0)
    {
        interface_monitorRemovePoller(x->x_sockfd);
        interface_closeSocket(x->x_sockfd);
        x->x_sockfd = -1;
        outlet_float(x->x_obj.te_outlet, 0);
    }
}

static int netsend_dosend(t_netsend *x, int sockfd,
    t_symbol *s, int argc, t_atom *argv)
{
    char *buf, *bp;
    int length, sent, fail = 0;
    t_buffer *b = 0;
    if (x->x_bin)
    {
        int i;
        buf = alloca(argc);
        for (i = 0; i < argc; i++)
            ((unsigned char *)buf)[i] = atom_getFloatAtIndex(i, argc, argv);
        length = argc;
    }
    else
    {
        t_atom at;
        b = buffer_new();
        buffer_append(b, argc, argv);
        SET_SEMICOLON(&at);
        buffer_append(b, 1, &at);
        buffer_toStringUnzeroed(b, &buf, &length);
    }
    for (bp = buf, sent = 0; sent < length;)
    {
        static double lastwarntime;
        static double pleasewarn;
        double timebefore = sys_getRealTime();
        int res = send(sockfd, bp, length-sent, 0);
        double timeafter = sys_getRealTime();
        int late = (timeafter - timebefore > 0.005);
        if (late || pleasewarn)
        {
            if (timeafter > lastwarntime + 2)
            {
                 post("netsend/netreceive blocked %d msec",
                    (int)(1000 * ((timeafter - timebefore) +
                        pleasewarn)));
                 pleasewarn = 0;
                 lastwarntime = timeafter;
            }
            else if (late) pleasewarn += timeafter - timebefore;
        }
        if (res <= 0)
        {
            PD_BUG;
            fail = 1;
            break;
        }
        else
        {
            sent += res;
            bp += res;
        }
    }
    done:
    if (!x->x_bin)
    {
        PD_MEMORY_FREE(buf);
        buffer_free(b);
    }
    return (fail);
}


static void netsend_send(t_netsend *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_sockfd >= 0)
    {
        if (netsend_dosend(x, x->x_sockfd, s, argc, argv))
            netsend_disconnect(x);
    }
}

static void netsend_free(t_netsend *x)
{
    netsend_disconnect(x);
}

static void netsend_setup(void)
{
    netsend_class = class_new(gensym("netsend"), (t_newmethod)netsend_new,
        (t_method)netsend_free,
        sizeof(t_netsend), 0, A_GIMME, 0);
    class_addMethod(netsend_class, (t_method)netsend_connect,
        gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addMethod(netsend_class, (t_method)netsend_disconnect,
        gensym("disconnect"), 0);
    class_addMethod(netsend_class, (t_method)netsend_send, gensym("send"),
        A_GIMME, 0);
}

/* ----------------------------- netreceive ------------------------- */
static void netreceive_notify(t_netreceive *x, int fd)
{
    int i;
    for (i = 0; i < x->x_nconnections; i++)
    {
        if (x->x_connections[i] == fd)
        {
            memmove(x->x_connections+i, x->x_connections+(i+1),
                sizeof(int) * (x->x_nconnections - (i+1)));
            x->x_connections = (int *)PD_MEMORY_RESIZE(x->x_connections,
                x->x_nconnections * sizeof(int), 
                    (x->x_nconnections-1) * sizeof(int));
            x->x_nconnections--;
        }
    }
    outlet_float(x->x_ns.x_connectout, x->x_nconnections);
}

static void netreceive_connectpoll(t_netreceive *x)
{
    int fd = accept(x->x_ns.x_sockfd, 0, 0);
    if (fd < 0) post("netreceive: accept failed");
    else
    {
        int nconnections = x->x_nconnections+1;
        
        x->x_connections = (int *)PD_MEMORY_RESIZE(x->x_connections,
            x->x_nconnections * sizeof(int), nconnections * sizeof(int));
        x->x_connections[x->x_nconnections] = fd;
        if (x->x_ns.x_bin)
            interface_monitorAddPoller(fd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, fd, 
            (t_notifyfn)netreceive_notify,
                (x->x_ns.x_msgout ? netsend_doit : NULL), 0);
        }
        outlet_float(x->x_ns.x_connectout, (x->x_nconnections = nconnections));
    }
}

static void netreceive_closeall(t_netreceive *x)
{
    int i;
    for (i = 0; i < x->x_nconnections; i++)
    {
        interface_monitorRemovePoller(x->x_connections[i]);
        interface_closeSocket(x->x_connections[i]);
    }
    x->x_connections = (int *)PD_MEMORY_RESIZE(x->x_connections, 
        x->x_nconnections * sizeof(int), 0);
    x->x_nconnections = 0;
    if (x->x_ns.x_sockfd >= 0)
    {
        interface_monitorRemovePoller(x->x_ns.x_sockfd);
        interface_closeSocket(x->x_ns.x_sockfd);
    }
    x->x_ns.x_sockfd = -1;
}

static void netreceive_listen(t_netreceive *x, t_float fportno)
{
    int portno = fportno, intarg;
    struct sockaddr_in server;
    netreceive_closeall(x);
    if (portno <= 0)
        return;
    x->x_ns.x_sockfd = socket(AF_INET, x->x_ns.x_protocol, 0);
    if (x->x_ns.x_sockfd < 0)
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
    if (setsockopt(x->x_ns.x_sockfd, SOL_SOCKET, SO_REUSEADDR,
        (char *)&intarg, sizeof(intarg)) < 0)
            post("netreceive: setsockopt (SO_REUSEADDR) failed\n");
#endif
#if 0
    intarg = 0;
    if (setsockopt(x->x_ns.x_sockfd, SOL_SOCKET, SO_RCVBUF,
        &intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_RCVBUF) failed\n");
#endif
    intarg = 1;
    if (setsockopt(x->x_ns.x_sockfd, SOL_SOCKET, SO_BROADCAST, 
        (const void *)&intarg, sizeof(intarg)) < 0)
            post("netreceive: failed to sett SO_BROADCAST");
        /* Stream (TCP) sockets are set NODELAY */
    if (x->x_ns.x_protocol == SOCK_STREAM)
    {
        intarg = 1;
        if (setsockopt(x->x_ns.x_sockfd, IPPROTO_TCP, TCP_NODELAY,
            (char *)&intarg, sizeof(intarg)) < 0)
                post("setsockopt (TCP_NODELAY) failed\n");
    }
        /* assign server port number etc */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons((u_short)portno);

        /* name the socket */
    if (bind(x->x_ns.x_sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        PD_BUG;
        interface_closeSocket(x->x_ns.x_sockfd);
        x->x_ns.x_sockfd = -1;
        return;
    }

    if (x->x_ns.x_protocol == SOCK_DGRAM)        /* datagram protocol */
    {
        if (x->x_ns.x_bin)
            interface_monitorAddPoller(x->x_ns.x_sockfd, (t_pollfn)netsend_readbin, x);
        else
        {
            t_receiver *y = receiver_new((void *)x, x->x_ns.x_sockfd, 
                (t_notifyfn)netreceive_notify, (x->x_ns.x_msgout ? netsend_doit : NULL), 1);
            x->x_ns.x_connectout = 0;
        }
    }
    else        /* streaming protocol */
    {
        if (listen(x->x_ns.x_sockfd, 5) < 0)
        {
            PD_BUG;
            interface_closeSocket(x->x_ns.x_sockfd);
            x->x_ns.x_sockfd = -1;
        }
        else
        {
            interface_monitorAddPoller(x->x_ns.x_sockfd, (t_pollfn)netreceive_connectpoll, x);
            x->x_ns.x_connectout = outlet_new(&x->x_ns.x_obj, &s_float);
        }
    }
}


static void netreceive_send(t_netreceive *x, 
    t_symbol *s, int argc, t_atom *argv)
{
    int i;
    for (i = 0; i < x->x_nconnections; i++)
    {
        if (netsend_dosend(&x->x_ns, x->x_connections[i], s, argc, argv))
            post_error ("netreceive send message failed");
                /* should we now close the connection? */
    }
}

static void *netreceive_new(t_symbol *s, int argc, t_atom *argv)
{
    t_netreceive *x = (t_netreceive *)pd_new(netreceive_class);
    int portno = 0;
    x->x_ns.x_protocol = SOCK_STREAM;
    x->x_old = 0;
    x->x_ns.x_bin = 0;
    x->x_nconnections = 0;
    x->x_connections = (int *)PD_MEMORY_GET(0);
    x->x_ns.x_sockfd = -1;
    if (argc && argv->a_type == A_FLOAT)
    {
        portno = atom_getFloatAtIndex(0, argc, argv);
        x->x_ns.x_protocol = (atom_getFloatAtIndex(1, argc, argv) != 0 ?
            SOCK_DGRAM : SOCK_STREAM);
        x->x_old = (!strcmp(atom_getSymbolAtIndex(2, argc, argv)->s_name, "old"));
        argc = 0;
    }
    else 
    {
        while (argc && argv->a_type == A_SYMBOL &&
            *argv->a_w.w_symbol->s_name == '-')
        {
            if (!strcmp(argv->a_w.w_symbol->s_name, "-b"))
                x->x_ns.x_bin = 1;
            else if (!strcmp(argv->a_w.w_symbol->s_name, "-u"))
                x->x_ns.x_protocol = SOCK_DGRAM;
            else
            {
                post_error ("netreceive: unknown flag ...");
                post_atoms(argc, argv);
            }
            argc--; argv++;
        }
    }
    if (argc && argv->a_type == A_FLOAT)
        portno = argv->a_w.w_float, argc--, argv++;
    if (argc)
    {
        post_error ("netreceive: extra arguments ignored:");
        post_atoms(argc, argv);
    }
    if (x->x_old)
    {
        /* old style, nonsecure version */
        x->x_ns.x_msgout = 0;
    }
    else x->x_ns.x_msgout = outlet_new(&x->x_ns.x_obj, &s_anything);
        /* create a socket */
    if (portno > 0)
        netreceive_listen(x, portno);

    return (x);
}

static void netreceive_setup(void)
{
    netreceive_class = class_new(gensym("netreceive"),
        (t_newmethod)netreceive_new, (t_method)netreceive_closeall,
        sizeof(t_netreceive), 0, A_GIMME, 0);
    class_addMethod(netreceive_class, (t_method)netreceive_listen,
        gensym("listen"), A_FLOAT, 0);
    class_addMethod(netreceive_class, (t_method)netreceive_send,
        gensym("send"), A_GIMME, 0);
}

void x_net_setup(void)
{
    netsend_setup();
    netreceive_setup();
}

