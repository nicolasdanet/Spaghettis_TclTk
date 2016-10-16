
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#ifdef _WIN32

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <winsock.h>
    
#else

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#endif // _WIN32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* 
    This is a standalone program that receives messages from PureData via the
    netsend/netreceive ("FUDI") protocol, and copies them to standard output.
    
    Note that it does NOT support binary mode.
*/

/* < http://en.wikipedia.org/wiki/FUDI > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

typedef struct _poll {
    int     p_fd;
    int     p_messageIsTruncated;
    int     p_messageLength;
    char    *p_buffer;
    } t_poll;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define PDRECEIVE_BUFFER_SIZE   4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_poll   *pdreceive_pollers;

static int      pdreceive_pollersSize;
static int      pdreceive_maximumFileDescriptor;
static int      pdreceive_socketFileDescriptor;
static int      pdreceive_socketProtocol;

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int pdreceive_usage (void)
{
    fprintf (stderr, "usage: pdreceive < portnumber > [ udp | tcp ]\n");
    fprintf (stderr, "(default is tcp)\n");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef _WIN32

void pdreceive_socketError (char *s)
{
    int err = WSAGetLastError();
    
    if (err != 10054) { fprintf (stderr, "%s: error %d / %s\n", s, err, strerror (err)); }
}

void pdreceive_socketClose (int fd)
{
    closesocket (fd);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

void pdreceive_socketError (char *s)
{
    int err = errno; fprintf (stderr, "%s: error %d / %s\n", s, err, strerror (err));
}

void pdreceive_socketClose (int fd)
{
    close (fd);
}

#endif // _WIN32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifdef _WIN32

static int pdreceive_outputRaw (char *t, int size)
{
    int j; for (j = 0; j < size; j++) { putchar (t[j]); } return 0;
}

#else

static int pdreceive_outputRaw (char *t, int size)
{   
    if (write (1, t, size) < size) { pdreceive_socketError ("write"); return 1; }
    else {
        return 0;
    }
}

#endif // _WIN32

static int pdreceive_outputUDP (char *t, int size)
{
    return pdreceive_outputRaw (t, size);
}

static int pdreceive_outputTCP (t_poll *x, char *t, int size)
{
    char *p = x->p_buffer;
    int i, length = x->p_messageLength;
        
    for (i = 0; i < size; i++) {
    //
    char c = t[i];
    
    if (c != '\n') { p[length++] = c; }
    
    if (length >= (PDRECEIVE_BUFFER_SIZE - 1)) {
        fprintf (stderr, "overflow: discard message\n");
        length = 0;
        x->p_messageIsTruncated = 1;
    }  

    if (c == ';') {
        p[length++] = '\n';
        if (!x->p_messageIsTruncated) { pdreceive_outputRaw (p, length); }
        length = 0; x->p_messageIsTruncated = 0;
    }
    //
    }

    x->p_messageLength = length;
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int pdreceive_addPort (int fd)
{
    pdreceive_pollers = (t_poll *)realloc (pdreceive_pollers, (pdreceive_pollersSize + 1) * sizeof (t_poll));
    
    if (pdreceive_pollers) {
    //
    t_poll *p = pdreceive_pollers + pdreceive_pollersSize;
    
    pdreceive_pollersSize++;
    
    p->p_fd                 = fd;
    p->p_messageIsTruncated = 0;
    p->p_messageLength      = 0;
    p->p_buffer             = (char *)malloc (PDRECEIVE_BUFFER_SIZE);
    
    if (fd >= pdreceive_maximumFileDescriptor) { pdreceive_maximumFileDescriptor = fd + 1; }
    
    if (p->p_buffer) { return 0; }
    //
    }
    
    return 1;   /* Just quit if a memory error occurs. */
}

static int pdreceive_removePort (t_poll *x)
{
    int i, k = -1;
    
    for (i = 0; i < pdreceive_pollersSize; i++) {
        if (pdreceive_pollers + i == x) {
            pdreceive_socketClose (pdreceive_pollers[i].p_fd);
            free (pdreceive_pollers[i].p_buffer);
            k = i;
            break;
        }
    }
    
    if (k == -1) { return 1; }
    else {
    //
    for (i = k; i < pdreceive_pollersSize - 1; i++) { pdreceive_pollers[i] = pdreceive_pollers[i + 1]; }
    pdreceive_pollers = (t_poll *)realloc (pdreceive_pollers, (pdreceive_pollersSize - 1) * sizeof (t_poll));
    pdreceive_pollersSize--;
    //
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int pdreceive_readUDP (void)
{
    char t[PDRECEIVE_BUFFER_SIZE] = { 0 };
    ssize_t n = recv (pdreceive_socketFileDescriptor, t, PDRECEIVE_BUFFER_SIZE, 0);
    
    if (n < 0) { pdreceive_socketError ("recv"); return 1; }
    else {
        if (n > 0) { 
            return pdreceive_outputUDP (t, n);
        }
    }
        
    return 0;
}

static int pdreceive_readTCP (t_poll *x)
{
    char t[PDRECEIVE_BUFFER_SIZE] = { 0 };
    ssize_t n = recv (x->p_fd, t, PDRECEIVE_BUFFER_SIZE, 0);
    
    if (n <= 0) { if (n) { pdreceive_socketError ("recv"); } return pdreceive_removePort (x); }
    else { 
        return pdreceive_outputTCP (x, t, n);
    }
}

static int pdreceive_connect (void)
{
    int fd = accept (pdreceive_socketFileDescriptor, NULL, NULL);
    
    if (fd < 0) { pdreceive_socketError ("accept"); }
    else {
        return pdreceive_addPort (fd);
    }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static int pdreceive_poll (void)
{
    int err = 0;
    
    fd_set rSet;
    fd_set wSet;
    fd_set eSet;
    
    FD_ZERO (&wSet);
    FD_ZERO (&rSet);
    FD_ZERO (&eSet);

    FD_SET (pdreceive_socketFileDescriptor, &rSet);
    
    if (pdreceive_socketProtocol == SOCK_STREAM) {
        int i;
        for (i = 0; i < pdreceive_pollersSize; i++) { FD_SET (pdreceive_pollers[i].p_fd, &rSet); }
    }
    
    if (select (pdreceive_maximumFileDescriptor + 1, &rSet, &wSet, &eSet, 0) < 0) {
        err = 1; pdreceive_socketError ("select");
        
    }
    
    if (!err) {
    //
    if (pdreceive_socketProtocol == SOCK_DGRAM) {
        if (FD_ISSET (pdreceive_socketFileDescriptor, &rSet)) {
            err = pdreceive_readUDP();
        }
    }
    
    if (pdreceive_socketProtocol == SOCK_STREAM) {
        int i;
        for (i = 0; i < pdreceive_pollersSize; i++) {
            if (FD_ISSET (pdreceive_pollers[i].p_fd, &rSet)) {
                err |= pdreceive_readTCP (&pdreceive_pollers[i]);
            }
        }
        if (FD_ISSET (pdreceive_socketFileDescriptor, &rSet)) {
            err |= pdreceive_connect(); 
        }
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int main (int argc, char **argv)
{
    int portNumber = 0;
    
    /* Get parameters. */
    
    int err = (argc < 2 || sscanf (argv[1], "%d", &portNumber) < 1 || portNumber <= 0);
    
    pdreceive_socketProtocol = SOCK_STREAM;
    
    if (!err) {
        if (argc >= 3) {
            if (strcmp (argv[2], "udp") == 0) { pdreceive_socketProtocol = SOCK_DGRAM; }
            else { 
                err = (strcmp (argv[3], "tcp") != 0); 
            }
        }
    }
    
    /* Make connection. */
    
    if (err) { pdreceive_usage(); }
    else {
    //
    #ifdef _WIN32
    
    short version = MAKEWORD (2, 0);
    WSADATA nobby;
    
    if (WSAStartup (version, &nobby)) {
        pdreceive_socketError ("WSAstartup");
        return 1; 
    }
        
    #endif
    
    pdreceive_socketFileDescriptor = socket (AF_INET, pdreceive_socketProtocol, 0);
    
    if (pdreceive_socketFileDescriptor < 0) { err = 1; pdreceive_socketError ("socket"); }
    else {
    //
    struct sockaddr_in server;
    
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons ((unsigned short)portNumber);

    pdreceive_maximumFileDescriptor = pdreceive_socketFileDescriptor + 1;
    
    if (bind (pdreceive_socketFileDescriptor, (struct sockaddr *)&server, sizeof (struct sockaddr)) < 0) {
        err = 1; pdreceive_socketError ("bind");
    }
    
    /* Loop selecting on sockets. */
    
    if (!err) {
    
        if (pdreceive_socketProtocol == SOCK_STREAM) {
            if (listen (pdreceive_socketFileDescriptor, 5) < 0) {
                err = 1; pdreceive_socketError ("listen");
            }
        }
        
        while (!err) { err = pdreceive_poll(); }
    }
    
    pdreceive_socketClose (pdreceive_socketFileDescriptor);
    //
    }
    //
    }
    
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

