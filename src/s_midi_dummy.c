/* Copyright (c) 1997-2003 Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

   this file is a dummy for systems without any MIDI support

*/

void midi_openNative(int nmidiin, int *midiinvec,
    int nmidiout, int *midioutvec)
{
}

void midi_closeNative( void)
{
}

void midi_pushNextMessageNative(int portno, int a, int b, int c)
{
}

void midi_pushNextByteNative(int portno, int byte)
{
}

void sys_poll_midi(void)
{
}

void midi_getdevs(char *indevlist, int *nindevs,
    char *outdevlist, int *noutdevs)
{
}
