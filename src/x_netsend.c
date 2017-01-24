
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_macros.h"
#include "m_core.h"
#include "s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_class *netsend_class;              /* Shared. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _netsend {
    t_object    ns_obj;                     /* Must be the first. */
    int         ns_fd;
    int         ns_protocol;
    int         ns_isBinary;
    t_outlet    *ns_outlet;
    } t_netsend;
    
// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static void netsend_socketOptions (t_netsend *x, int fd)
{
    int v = 1;
    
    if (x->ns_protocol == SOCK_STREAM) {
        if (setsockopt (fd, IPPROTO_TCP, TCP_NODELAY, (const void *)&v, sizeof (v)) < 0) { PD_BUG; }
    } else {
        if (setsockopt (fd, SOL_SOCKET, SO_BROADCAST, (const void *)&v, sizeof (v)) < 0) { PD_BUG; }
    }
}

static void netsend_socketClose (t_netsend *x)
{
    if (x->ns_fd >= 0) { interface_closeSocket (x->ns_fd); x->ns_fd = -1; }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error netsend_sendPerformRaw (t_netsend *x, int fd, char *t, int length)
{
    t_error err = PD_ERROR_NONE;
    
    ssize_t alreadySent = 0;
    char *p = t;

    while (alreadySent < length) {
        ssize_t n = send (fd, p, length - alreadySent, 0);
        if (n <= 0) {
            error_failsToWrite (sym_netsend);
            err = PD_ERROR;
            break; 
        } else {
            alreadySent += n; p += n;
        }
    }
    
    return err;
}

t_error netsend_sendPerformText (t_netsend *x, int fd, t_symbol *s, int argc, t_atom *argv)
{   
    t_error err = PD_ERROR_NONE;
    
    t_buffer *b = buffer_new();
    char *t = NULL; int length;
    
    buffer_append (b, argc, argv);
    buffer_appendSemicolon (b);
    buffer_toStringUnzeroed (b, &t, &length);
    
    err = netsend_sendPerformRaw (x, fd, t, length);

    PD_MEMORY_FREE (t);
    buffer_free (b);

    return err;
}

t_error netsend_sendPerformBinary (t_netsend *x, int fd, t_symbol *s, int argc, t_atom *argv)
{
    t_error err = PD_ERROR_NONE;
    
    unsigned char *t = (unsigned char *)PD_MEMORY_GET (argc * sizeof (unsigned char));
    int i;
        
    for (i = 0; i < argc; i++) {
        int byte = atom_getFloatAtIndex (i, argc, argv);
        byte = PD_CLAMP (byte, 0, 0xff);
        *(t + i) = (unsigned char)byte; 
    }
    
    err = netsend_sendPerformRaw (x, fd, (char *)t, argc);

    PD_MEMORY_FREE (t);

    return err;
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
    
    netsend_socketOptions (x, fd);
    
    if (h == NULL) { error_invalid (sym_netsend, hostName); }
    else {
    //
    struct sockaddr_in server;
    
    post ("netsend: connecting to port %d", portNumber);    // --
    
    server.sin_family = AF_INET;
    server.sin_port = htons ((u_short)portNumber);
    memcpy ((char *)&server.sin_addr, (char *)h->h_addr, h->h_length);

    if (connect (fd, (struct sockaddr *)&server, sizeof (server)) < 0) {
        error_failed (sym_netsend);
        netsend_socketClose (x);
        outlet_float (x->ns_outlet, 0);
        return;
        
    } else { x->ns_fd = fd; outlet_float (x->ns_outlet, 1); }
    //
    }
    //
    }
    //
    }
}

static void netsend_disconnect (t_netsend *x)
{
    netsend_socketClose (x); outlet_float (x->ns_outlet, 0);
}

static void netsend_send (t_netsend *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->ns_fd >= 0) {
    //
    t_error err = PD_ERROR_NONE;
    
    if (x->ns_isBinary) { err = netsend_sendPerformBinary (x, x->ns_fd, s, argc, argv); }
    else {
        err = netsend_sendPerformText (x, x->ns_fd, s, argc, argv);
    }
    
    if (err) { netsend_disconnect (x); }
    //
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
    netsend_socketClose (x);
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

