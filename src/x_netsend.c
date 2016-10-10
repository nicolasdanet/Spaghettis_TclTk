
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

extern t_class *netreceive_class;

static t_class *netsend_class;

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
            error__post (argc, argv);
        }
        argc--; argv++;
    }
    if (argc)
    {
        post_error ("netsend: extra arguments ignored:");
        error__post (argc, argv);
    }
    x->x_sockfd = -1;
    if (x->x_protocol == SOCK_STREAM)
        x->x_msgout = outlet_new(&x->x_obj, &s_anything);
    return (x);
}

void netsend_readbin(t_netsend *x, int fd)
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
        outlet_list(x->x_msgout, ret, ap);
    }
    else
    {
        for (i = 0; i < ret; i++)
            outlet_float(x->x_msgout, inbuf[i]);
    }
}

void netsend_doit(void *z, t_buffer *b)
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
                    outlet_list(x->x_msgout, emsg-msg, at + msg);
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

int netsend_dosend(t_netsend *x, int sockfd, t_symbol *s, int argc, t_atom *argv)
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
        buffer_appendAtom(b, &at);
        buffer_toStringUnzeroed(b, &buf, &length);
    }
    for (bp = buf, sent = 0; sent < length;)
    {
        static double lastwarntime;
        static double pleasewarn;
        double timebefore = sys_getRealTimeInSeconds();
        int res = send(sockfd, bp, length-sent, 0);
        double timeafter = sys_getRealTimeInSeconds();
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

void netsend_setup(void)
{
    netsend_class = class_new(sym_netsend, (t_newmethod)netsend_new,
        (t_method)netsend_free,
        sizeof(t_netsend), 0, A_GIMME, 0);
    class_addMethod(netsend_class, (t_method)netsend_connect,
        sym_connect, A_SYMBOL, A_FLOAT, 0);
    class_addMethod(netsend_class, (t_method)netsend_disconnect,
        sym_disconnect, 0);
    class_addMethod(netsend_class, (t_method)netsend_send, sym_send,
        A_GIMME, 0);
}

