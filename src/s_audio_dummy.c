/*
 * Copyright (c) 2010 Peter Brinkmann (peter.brinkmann@gmail.com)
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 */

#ifdef USEAPI_DUMMY

#include <stdio.h>

int dummy_open_audio() {
  return 0;
}

void dummy_close_audio() {

}

int dummy_send_dacs() {
  return 0;
}

void dummy_getdevs(char *indevlist, int *nindevs, char *outdevlist,
    int *noutdevs, int *canmulti, int *canCallback) {
    int maxndev = MAXIMUM_DEVICES;
    int devdescsize = MAXIMUM_DESCRIPTION;
  sprintf(indevlist, "NONE");
  sprintf(outdevlist, "NONE");
    *canmulti = 0;
    *canCallback = 0;
  *nindevs = *noutdevs = 1;
}

void dummy_listdevs() {
  // do nothing
}

#endif

