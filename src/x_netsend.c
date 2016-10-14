
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

extern t_class *netreceive_class;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *netsend_class;                      /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static void netsend_socketOption (t_netsend *x, int fd)
{
    if (x->ns_protocol == SOCK_STREAM) {
        int option = 1;
        if (setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (char *)&option, sizeof (option)) < 0) {
            PD_BUG;
        }
        
    } else {
        int option = 1;
        if (setsockopt (fd, SOL_SOCKET, SO_BROADCAST, (const void *)&option, sizeof (option)) < 0) {
            PD_BUG;
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int netsend_dosend(t_netsend *x, int sockfd, t_symbol *s, int argc, t_atom *argv)
{
    char *buf, *bp;
    int length, sent, fail = 0;
    t_buffer *b = 0;
    if (x->ns_isBinary)
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
    if (!x->ns_isBinary)
    {
        PD_MEMORY_FREE(buf);
        buffer_free(b);
    }
    return (fail);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void netsend_connect (t_netsend *x, t_symbol *hostName, t_float f)
{
    int portNumber = f;
    
    if (x->ns_fd >= 0) { error_unexpected (sym_netsend, hostName); }
    else {
    //
    int fd = socket (AF_INET, x->ns_protocol, 0);

    if (fd < 0) { error_canNotOpen (sym_netsend); }
    else {
    //
    struct hostent *h = gethostbyname (hostName->s_name);
    
    netsend_socketOption (x, fd);
    
    if (h == NULL) { error_invalid (sym_netsend, hostName); }
    else {
    //
    struct sockaddr_in server;
    
    post ("netsend: connecting to port %d", portNumber);
    
    server.sin_family = AF_INET;
    server.sin_port = htons ((u_short)portNumber);
    memcpy ((char *)&server.sin_addr, (char *)h->h_addr, h->h_length);

    if (connect (fd, (struct sockaddr *)&server, sizeof (server)) < 0) {
        error_failed (sym_netsend);
        interface_closeSocket (fd);
        return;
        
    } else { x->ns_fd = fd; outlet_float (x->ns_outlet, 1); }
    //
    }
    //
    }
    //
    }
}

static void netsend_disconnect(t_netsend *x)
{
    if (x->ns_fd >= 0)
    {
        interface_monitorRemovePoller(x->ns_fd);
        interface_closeSocket(x->ns_fd);
        x->ns_fd = -1;
        outlet_float(x->ns_outlet, 0);
    }
}

static void netsend_send(t_netsend *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->ns_fd >= 0)
    {
        if (netsend_dosend(x, x->ns_fd, s, argc, argv))
            netsend_disconnect(x);
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void *netsend_new (t_symbol *s, int argc, t_atom *argv)
{
    t_netsend *x = (t_netsend *)pd_new (netsend_class);

    x->ns_fd        = -1;
    x->ns_isBinary  = 0;
    x->ns_protocol  = SOCK_STREAM;
    x->ns_outlet    = outlet_new (cast_object (x), &s_float);
    
    #if PD_WITH_LEGACY 
    
    if (argc && IS_FLOAT (argv)) {
        x->ns_protocol = (GET_FLOAT (argv) != 0.0 ? SOCK_DGRAM : SOCK_STREAM); 
        argc = 0;
    }
    
    #endif
    
    while (argc > 0) {
    //
    t_symbol *t = atom_getSymbolAtIndex (0, argc, argv);

    if (t == sym___dash__b || t == sym___dash__binary)   { argc--; argv++; x->ns_isBinary = 1; }
    else if (t == sym___dash__u || t == sym___dash__udp) { argc--; argv++; x->ns_protocol = SOCK_DGRAM; }
    else {
        break;
    }
    //
    }
    
    error__options (s, argc, argv);
    
    if (argc) { 
        warning_unusedArguments (s, argc, argv); 
    }
    
    return x;
}

static void netsend_free (t_netsend *x)
{
    netsend_disconnect (x);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void netsend_setup (void)
{
    t_class *c = NULL;
    
    c = class_new (sym_netsend,
            (t_newmethod)netsend_new,
            (t_method)netsend_free,
            sizeof (t_netsend),
            CLASS_DEFAULT,
            A_GIMME,
            A_NULL);
            
    class_addMethod (c, (t_method)netsend_connect,      sym_connect,    A_SYMBOL, A_FLOAT, A_NULL);
    class_addMethod (c, (t_method)netsend_disconnect,   sym_disconnect, A_NULL);
    class_addMethod (c, (t_method)netsend_send,         sym_send,       A_GIMME, A_NULL);
        
    netsend_class = c;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

