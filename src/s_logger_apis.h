
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#ifndef __s_logger_apis_h_
#define __s_logger_apis_h_

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Lock-free logger for low latency debugging. */
/* Handy to post small constant strings while developing DSP code. */
/* By the way probably not a good idea to use it in the release product. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#if ( PD_WITH_DEBUG && PD_WITH_LOGGER ) 
    #define PD_LOG(s)               logger_appendStringNative (s)
    #define PD_LOG_NUMBER(f)        logger_appendFloatNative ((t_float)f)
#else
    #define PD_LOG(s)
    #define PD_LOG_NUMBER(f)
#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error logger_initializeNative     (void);
void    logger_releaseNative        (void);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void    logger_appendStringNative   (const char *s);
void    logger_appendFloatNative    (t_float f);

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#define LOGGER_FLOAT_STRING     32      /* Must be big enough to avoid overflow. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline char *logger_stringWithFloat (char *dest, t_float f)
{
    char digits[] = "0123456789";
    
    int minus        = (f < 0.0);
    t_float absolute = PD_MIN ((t_float)PD_INT_MAX, PD_ABS (f));
    int integer      = (int)absolute;
    int fractional   = (int)((absolute - (t_float)integer) * 1000000.0);
       
    char *s = dest + (LOGGER_FLOAT_STRING - 1);
    
    *s-- = 0;
    
    if (fractional) {
        do {
        //
        *s-- = digits[fractional % 10]; fractional /= 10; 
        //
        } while (fractional != 0);
        
        *s-- = '.';
    }
    
    do {
    //
    *s-- = digits[integer % 10]; integer /= 10;
    //
    } while (integer != 0);
    
    if (minus) { *s = '-'; }
    else {
        *s = '+';
    }
    
    return s;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#endif // __s_logger_apis_h_
