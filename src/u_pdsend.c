
/* Copyright (c) 1997-2017 Miller Puckette and others. */

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
// MARK: -

/*
    This is a standalone program that forwards messages from its standard input
    to Spaghettis via the netsend/netreceive ("FUDI") protocol.
*/

/* < http://en.wikipedia.org/wiki/FUDI > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define PDSEND_BUFFER_SIZE      4096

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void pdsend_usage (void)
{
    fprintf (stderr, "usage: pdsend < portnumber > [ host ] [ udp | tcp ]\n");  // --
    fprintf (stderr, "(default is localhost and tcp)\n");
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifdef _WIN32

void pdsend_socketError (char *s)
{
    int err = WSAGetLastError();
    
    if (err != 10054) { fprintf (stderr, "%s: error %d / %s\n", s, err, strerror (err)); }  // --
}

void pdsend_socketClose (int fd)
{
    closesocket (fd);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

void pdsend_socketError (char *s)
{
    int err = errno; fprintf (stderr, "%s: error %d / %s\n", s, err, strerror (err));   // --
}

void pdsend_socketClose (int fd)
{
    close (fd);
}

#endif // _WIN32

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int main (int argc, char **argv)
{
    int portNumber = 0;
    int protocol   = SOCK_STREAM;
    char *hostName = "127.0.0.1";
    
    /* Get parameters. */
    
    int err = (argc < 2 || sscanf (argv[1], "%d", &portNumber) < 1 || portNumber <= 0);
    
    if (!err) {
        if (argc >= 3) { hostName = argv[2]; }
        if (argc >= 4) {
            if (strcmp (argv[3], "udp") == 0) { protocol = SOCK_DGRAM; }
            else { 
                err = (strcmp (argv[3], "tcp") != 0); 
            }
        }
    }
    
    /* Make connection. */
    
    if (err) { pdsend_usage(); }
    else {
    //
    #ifdef _WIN32
    
    short version = MAKEWORD (2, 0);
    WSADATA nobby;
    
    if (WSAStartup (version, &nobby)) {
        pdsend_socketError ("WSAstartup");
        return 1; 
    }
        
    #endif
    
    int fd = socket (AF_INET, protocol, 0);
    
    if (fd < 0) { err = 1; pdsend_socketError ("socket"); }
    else {
    //
    struct hostent *host = gethostbyname (hostName);
    
    if (host == NULL) { err = 1; fprintf (stderr, "%s: unknown host\n", hostName); }    // --
    else {
    //
    struct sockaddr_in server;
    
    server.sin_family = AF_INET;
    server.sin_port = htons ((unsigned short)portNumber);
    memcpy ((char *)&server.sin_addr, (char *)host->h_addr, host->h_length);
    err = (connect (fd, (struct sockaddr *) &server, sizeof (struct sockaddr_in)) != 0);
    
    if (err) { pdsend_socketError ("connect"); }

    /* Start the REPL loop. */
    
    while (!err) {
    
        char t[PDSEND_BUFFER_SIZE] = { 0 };

        if (!fgets (t, PDSEND_BUFFER_SIZE, stdin)) { break; }
        else {
        //
        int numberOfCharactersToSend = (int)strlen (t);
        ssize_t alreadySent = 0; char *p = t;
                    
        while (alreadySent < numberOfCharactersToSend) {
            ssize_t n = send (fd, p, numberOfCharactersToSend - alreadySent, 0);
            if (n < 0) { err = 1; pdsend_socketError ("send"); break; }
            else {
                alreadySent += n;
                p += n;
            }
        }
        //
        }
    }
    //
    }
    
    pdsend_socketClose (fd);
    //
    }
    //
    }
        
    return err;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
