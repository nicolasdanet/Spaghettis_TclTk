
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include <signal.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* 
    PD_WATCHDOG
    
    To prevent lockup, a watchdog process is forked with higher real-time priority. 
    The GUI send a stream of ping messages to the watchdog THROUGH the main process. 
    If pinging stops the watchdog start to SIGHUP to make the main process timeshare. 
    
*/

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define WATCHDOG_STRING     128

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int main (int argc, char **argv)
{
    int happy = 1;
    
    while (1) {
    //
    struct timeval timeOut;
    
    fd_set rSet;
    fd_set eSet;
    
    timeOut.tv_sec  = (happy ? 5 : 2);
    timeOut.tv_usec = 0;

    FD_ZERO (&rSet);
    FD_ZERO (&eSet);
    
    FD_SET (0, &rSet);
    FD_SET (0, &eSet);
    
    select (1, &rSet, NULL, &eSet, &timeOut);
    
    if (FD_ISSET (0, &eSet)) { return 0; }
    if (FD_ISSET (0, &rSet)) {
        char t[WATCHDOG_STRING];
        happy = 1;
        if (read (0, &t, WATCHDOG_STRING) <= 0) { return 0; }
        else { 
            continue;
        }
    }
    
    happy = 0; kill (getppid(), SIGHUP); fprintf (stderr, "SIGHUP\n"); 
    //
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
