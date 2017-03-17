
/*
    Basic UTF-8 manipulation routines,
    by Jeff Bezanson,
    placed in the public domain Fall 2005.

    This code is designed to provide the utilities you need to manipulate
    UTF-8 as an internal string encoding. These functions do not perform the
    error checking normally needed when handling UTF-8 data, so if you happen
    to be from the Unicode Consortium you will want to flay me alive.
    I do this because error checking can be performed at the boundaries (I/O),
    with these routines reserved for higher performance on data known to be
    valid.

*/

/* < https://github.com/JeffBezanson/cutef8 > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_utf8_h_
#define __s_utf8_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define UCS4_CODE_POINT         uint32_t
#define UTF8_MAXIMUM_BYTES      4

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

int     u8_utf8toucs2           (uint16_t *dest, int sz, char *src, int srcsz);
int     u8_ucs2toutf8           (char *dest, int sz, uint16_t *src, int srcsz);

int     u8_wc_nbytes            (uint32_t ch);
int     u8_wc_toutf8            (char *dest, uint32_t ch);

int     u8_offset               (char *s, int charnum);
int     u8_charnum              (char *s, int offset);
void    u8_inc                  (char *s, int *i);
void    u8_dec                  (char *s, int *i);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_utf8_h_
