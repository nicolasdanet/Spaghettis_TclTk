
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

/* Note that the original file have been largely modified. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include <stdlib.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "s_utf8.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define UTF8_IS_UTF(c) (((c) & 0xC0) != 0x80)       /* Is it the start of an UTF-8 sequence? */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static const uint32_t u8_offsetsFromUTF8[6] =               /* Static. */
    {
        0x00000000UL,
        0x00003080UL,
        0x000E2080UL,
        0x03C82080UL,
        0xFA082080UL,
        0x82082080UL
    };

static const char u8_trailingBytesForUTF8[256] =            /* Static. */
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
    };

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Conversions without error checking. */
/* Only works for valid UTF-8, i.e. no 5-byte or 6-byte sequences. */
/* The srcsz value is the source size in bytes, or -1 if 0-terminated. */
/* The sz value is the destination size in number of wide characters. */
/* Returns the number of characters converted. */
/* Destination always be 0-terminated. */

int u8_utf8toucs2 (uint16_t *dest, int sz, char *src, int srcsz)
{
    char *src_end = (srcsz == -1 ? NULL : src + srcsz);
    int i = 0;

    while (i < sz - 1) {
    //
    uint16_t ch = 0;
    
    int nb = u8_trailingBytesForUTF8[(unsigned char)*src];
    
    if (!src_end) { if (*src == 0) { break; } }
    else {
        if (src + nb >= src_end) { break; }
    }
    
    /* These fall through (missing break) deliberately. */
    
    switch (nb) {
        case 3  : ch += (unsigned char)*src++; ch <<= 6;
        case 2  : ch += (unsigned char)*src++; ch <<= 6;
        case 1  : ch += (unsigned char)*src++; ch <<= 6;
        case 0  : ch += (unsigned char)*src++;
    }
    
    dest[i++] = ch - u8_offsetsFromUTF8[nb];
    //
    }
    
    dest[i] = 0;
    
    return i;
}

/* The srcsz value is the number of source characters, or -1 if 0-terminated. */
/* The sz value is the size of the destination in bytes. */
/* Returns the number of characters converted (-1 in case of error). */
/* Destination only be 0-terminated if there is enough space. */

int u8_ucs2toutf8 (char *dest, int sz, uint16_t *src, int srcsz)
{
    char *dest_end = dest + sz;
    int i = 0;

    while (srcsz < 0 ? src[i] != 0 : i < srcsz) {
    //
    uint16_t ch = src[i];

    if (ch < 0x80) {
        if (dest >= dest_end)     { return -1; }
        else {
            *dest++ = (char)ch;
        }

    } else if (ch < 0x800) {
        if (dest >= dest_end - 1) { return -1; }
        else { 
            *dest++ = (ch >> 6) | 0xC0; *dest++ = (ch & 0x3F) | 0x80;
        }

    } else {
        if (dest >= dest_end - 2) { return -1; }
        else { 
            *dest++ = (ch >> 12) | 0xE0; *dest++ = ((ch >> 6) & 0x3F) | 0x80; *dest++ = (ch & 0x3F) | 0x80;
        }
    }
    i++;
    //
    }

    if (dest < dest_end) { *dest = '\0'; return i; }
    else {
        return -1;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int u8_wc_nbytes (uint32_t ch)
{
    if (ch < 0x80)      { return 1; }
    if (ch < 0x800)     { return 2; }
    if (ch < 0x10000)   { return 3; }
    if (ch < 0x110000)  { return 4; }

    return 0;
}

int u8_wc_toutf8 (char *dest, uint32_t ch)
{
    if (ch < 0x80) {
        dest[0] = (char)ch;
        return 1;
    }
    if (ch < 0x800) {
        dest[0] = (ch >> 6)             | 0xC0;
        dest[1] = (ch           & 0x3F) | 0x80;
        return 2;
    }
    if (ch < 0x10000) {
        dest[0] = (ch >> 12)            | 0xE0;
        dest[1] = ((ch >> 6)    & 0x3F) | 0x80;
        dest[2] = (ch           & 0x3F) | 0x80;
        return 3;
    }
    if (ch < 0x110000) {
        dest[0] = (ch >> 18)            | 0xF0;
        dest[1] = ((ch >> 12)   & 0x3F) | 0x80;
        dest[2] = ((ch >> 6)    & 0x3F) | 0x80;
        dest[3] = (ch           & 0x3F) | 0x80;
        return 4;
    }

    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

int u8_offset (char *s, int charnum)
{
    int i = 0;

    while (charnum > 0) {
        if (s[i++] & 0x80) { (void)(UTF8_IS_UTF (s[++i]) || UTF8_IS_UTF (s[++i]) || ++i); }
        charnum--;
    }
    
    return i;
}

int u8_charnum (char *s, int offset)
{
    int charnum = 0;
    int i = 0;

    while (i < offset) {
        if (s[i++] & 0x80) { (void)(UTF8_IS_UTF (s[++i]) || UTF8_IS_UTF (s[++i]) || ++i); }
        charnum++;
    }
    
    return charnum;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

void u8_inc (char *s, int *i)
{
    (void)(UTF8_IS_UTF (s[++(*i)]) || UTF8_IS_UTF (s[++(*i)]) || UTF8_IS_UTF (s[++(*i)]) || ++(*i));
}

void u8_dec (char *s, int *i)
{
    (void)(UTF8_IS_UTF (s[--(*i)]) || UTF8_IS_UTF (s[--(*i)]) || UTF8_IS_UTF (s[--(*i)]) || --(*i));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
