
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_sfthread_h_
#define __d_sfthread_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

enum {
    SFTHREAD_READER     = 0,
    SFTHREAD_WRITER     = 1
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Takes file ownership. */

t_sfthread      *sfthread_new                   (int type, int bufferSize, int fd, t_audioproperties *p);
t_ringbuffer    *sfthread_getBuffer             (t_sfthread *x);

void            sfthread_release                (t_sfthread *x);

int             sfthread_getNumberOfChannels    (t_sfthread *x);
int             sfthread_getBytesPerSample      (t_sfthread *x);
int             sfthread_isBigEndian            (t_sfthread *x);
int             sfthread_isEnd                  (t_sfthread *x);
void            sfthread_setCorrupted           (t_sfthread *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_sfthread_h_
