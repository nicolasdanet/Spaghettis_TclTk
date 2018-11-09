
/* Copyright (c) 1997-2018 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __d_sfthread_h_
#define __d_sfthread_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

typedef struct _sfthread {
    t_pd                sft_pd;                /* Must be the first. */
    t_audioproperties   sft_properties;
    int                 sft_type;
    int                 sft_fileDescriptor;
    int                 sft_remainsToRead;
    int                 sft_alreadyWritten;
    int                 sft_maximumToWrite;
    t_error             sft_error;
    pthread_t           sft_thread;
    t_int32Atomic       sft_flag;
    t_int32Atomic       sft_corrupted;
    t_ringbuffer        *sft_buffer;
    } t_sfthread;

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

#define SFTHREAD_QUIT           1
#define SFTHREAD_CORRUPTED      1

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

/* Takes file ownership. */

t_sfthread  *sfthread_new       (int type, int bufferSize, int fd, t_audioproperties *p);

void    sfthread_release        (t_sfthread *x);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline t_ringbuffer *sfthread_getBuffer (t_sfthread *x)
{
    return x->sft_buffer;
}

static inline int sfthread_isEnd (t_sfthread *x)
{
    return (PD_ATOMIC_INT32_READ (&x->sft_flag) != 0);
}

static inline void sfthread_setCorrupted (t_sfthread *x)
{
    if (!sfthread_isEnd (x)) { PD_ATOMIC_INT32_WRITE (SFTHREAD_CORRUPTED, &x->sft_corrupted); }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __d_sfthread_h_
