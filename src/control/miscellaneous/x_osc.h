
/* Copyright (c) 1997-2019 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#ifndef __x_osc_h_
#define __x_osc_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define OSC_GETCHAR(x)          (int)GET_FLOAT (x)
#define OSC_SETCHAR(x, c)       SET_FLOAT (x, (t_float)((unsigned char)(c) & 0xff))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define OSC_4ROUND(x)           (((x) + 3) & (~3))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define OSC_4READ(x)            ((((uint32_t)(GET_FLOAT ((x) + 0))) & 0xff) << 24) | \
                                ((((uint32_t)(GET_FLOAT ((x) + 1))) & 0xff) << 16) | \
                                ((((uint32_t)(GET_FLOAT ((x) + 2))) & 0xff) << 8)  | \
                                ((((uint32_t)(GET_FLOAT ((x) + 3))) & 0xff) << 0)

#define OSC_4WRITE(x, i)        SET_FLOAT ((x) + 0, (((uint32_t)(i) >> 24) & 0xff)); \
                                SET_FLOAT ((x) + 1, (((uint32_t)(i) >> 16) & 0xff)); \
                                SET_FLOAT ((x) + 2, (((uint32_t)(i) >> 8)  & 0xff)); \
                                SET_FLOAT ((x) + 3, (((uint32_t)(i))       & 0xff))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#define OSC_8READ(x)            ((((uint64_t)(GET_FLOAT ((x) + 0))) & 0xff) << 56) | \
                                ((((uint64_t)(GET_FLOAT ((x) + 1))) & 0xff) << 48) | \
                                ((((uint64_t)(GET_FLOAT ((x) + 2))) & 0xff) << 40) | \
                                ((((uint64_t)(GET_FLOAT ((x) + 3))) & 0xff) << 32) | \
                                ((((uint64_t)(GET_FLOAT ((x) + 4))) & 0xff) << 24) | \
                                ((((uint64_t)(GET_FLOAT ((x) + 5))) & 0xff) << 16) | \
                                ((((uint64_t)(GET_FLOAT ((x) + 6))) & 0xff) << 8)  | \
                                ((((uint64_t)(GET_FLOAT ((x) + 7))) & 0xff) << 0)

#define OSC_8WRITE(x, i)        SET_FLOAT ((x) + 0, (((uint64_t)(i) >> 56) & 0xff)); \
                                SET_FLOAT ((x) + 1, (((uint64_t)(i) >> 48) & 0xff)); \
                                SET_FLOAT ((x) + 2, (((uint64_t)(i) >> 40) & 0xff)); \
                                SET_FLOAT ((x) + 3, (((uint64_t)(i) >> 32) & 0xff)); \
                                SET_FLOAT ((x) + 4, (((uint64_t)(i) >> 24) & 0xff)); \
                                SET_FLOAT ((x) + 5, (((uint64_t)(i) >> 16) & 0xff)); \
                                SET_FLOAT ((x) + 6, (((uint64_t)(i) >> 8)  & 0xff)); \
                                SET_FLOAT ((x) + 7, (((uint64_t)(i))       & 0xff))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

static inline int osc_isValidTypetag (char c)
{
    if (c == 'i')      { return 1; }
    else if (c == 'f') { return 1; }
    else if (c == 's') { return 1; }
    else if (c == 'b') { return 1; }
    else if (c == 't') { return 1; }
    else if (c == 'd') { return 1; }
    else if (c == 'T') { return 1; }
    else if (c == 'F') { return 1; }
    else if (c == 'N') { return 1; }
    else if (c == 'I') { return 1; }
    
    return 0;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#endif // __x_osc_h_
