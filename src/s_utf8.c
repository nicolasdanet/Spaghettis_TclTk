
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
#pragma mark -

#define UTF8_IS_UTF(c) (((c) & 0xC0) != 0x80)       /* Is it the start of an UTF-8 sequence? */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static const uint32_t u8_offsetsFromUTF8[6] = 
    {
        0x00000000UL,
        0x00003080UL,
        0x000E2080UL,
        0x03C82080UL,
        0xFA082080UL,
        0x82082080UL
    };

static const char u8_trailingBytesForUTF8[256] = 
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
#pragma mark -

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
#pragma mark -

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
#pragma mark -

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

#if 0 

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Basic unit test. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include <stdio.h>
#include <string.h>

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char *borisVian = "Le sentier longeait la falaise."                                              // --
    " Il \xc3\xa9tait bord\xc3\xa9 de calamines en fleur et de brouillouses un peu pass\xc3\xa9"        // --
    "es dont les p\xc3\xa9tales noircis jonchaient le sol. Des insectes pointus avaient creus\xc3\xa9"  // --
    " le sol de mille petits trous ; sous les pieds, c\xe2\x80\x99\xc3\xa9tait comme "                  // --
    "de l\xe2\x80\x99\xc3\xa9ponge morte de froid.";                                                    // --

static char *arnoSchmidt = "Mein Leben ? ! : ist kein Kontinuum!"                                       // --
    " (nicht blo\xc3\x9f Tag und Nacht in wei\xc3\x9f und schwarze St\xc3\xbc""cke zerbroche"           // --
    "n ! Denn auch am Tage ist bei mir der ein Anderer, der zur Bahn geht; im Amt sitzt; b\xc3\xbc"     // --
    "chert; durch Haine stelzt; begattet; schwatzt; schreibt;"                                          // --
    " Tausendsdenker; auseinanderfallender F\xc3\xa4"                                                   // --
    "cher; der rennt; raucht; kotet; radioh\xc3\xb6rt; \'Herr Landrat\' sagt: that\'s me!)"             // --
    " ein Tablett voll glitzender snapshots.";                                                          // --

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static char *test = 
    "\xc3\x84" "a" "\xc3\xa4" "b"
    "\xc3\x9c" "c" "\xc3\xbc" "d"
    "\xc3\x9f" "e" "\xd0\xaf" "f"
    "\xd0\x91" "g" "\xd0\x93" "h"
    "\xd0\x94" "i" "\xd0\x96" "j"
    "\xd0\x99" "k" "\xc5\x81" "l"
    "\xc4\x84" "m" "\xc5\xbb" "n"
    "\xc4\x98" "o" "\xc4\x86" "p";

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

int main (int argc, char **argv)
{
    int i, j = 0;
    uint16_t w1[1024] = { 0 };
    uint16_t w2[1024] = { 0 };
    char u1[1024] = { 0 };
    char u2[1024] = { 0 };
    
    int size1 = u8_utf8toucs2 (w1, 1024, borisVian, -1);
    int size2 = u8_utf8toucs2 (w2, 1024, arnoSchmidt, -1);
    
    int err = 0;
    
    err += (size1 != 271);
    err += (size2 != 396);
    
    err += (u8_ucs2toutf8 (u1, 1024, w1, -1) == -1);
    err += (u8_ucs2toutf8 (u2, 1024, w2, -1) == -1);

    err += (strcmp (borisVian, u1) != 0);
    err += (strcmp (arnoSchmidt, u2) != 0);
    
    for (i = 0; i < 32; i++) {
        err += (u8_charnum (test, u8_offset (test, i)) != i);
    }
    
    for (i = 0; i < 16; i++) {
        u8_inc (test, &j); u8_inc (test, &j); err += ((j % 3) != 0);
    }
    
    for (i = 0; i < 16; i++) {
        u8_dec (test, &j); u8_dec (test, &j); err += ((j % 3) != 0);
    }
    
    printf ("%s\n", borisVian);
    printf ("%s\n", u1);
    printf ("%s\n", arnoSchmidt);
    printf ("%s\n", u2);
    printf ("%s\n", test);
    
    if (err) { printf ("FAILED\n"); }
    else {
        printf ("SUCCEEDED\n");
    }
    
    return (err != 0);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
