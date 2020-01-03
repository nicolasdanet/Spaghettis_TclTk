
/* Copyright (c) 1997-2020 Miller Puckette and others. */

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#include "../m_spaghettis.h"
#include "../m_core.h"
#include "../s_system.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
// MARK: -

#if defined ( PD_INTEL ) && defined ( __SSE3__ )

#include <pmmintrin.h>
#include <xmmintrin.h>

/* < https://www.carlh.net/plugins/denormals.php > */
/* < https://en.wikipedia.org/wiki/Denormal_number > */

void denormal_setPolicy (void)
{
    _MM_SET_DENORMALS_ZERO_MODE (_MM_DENORMALS_ZERO_ON);
    _MM_SET_FLUSH_ZERO_MODE (_MM_FLUSH_ZERO_ON);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* < https://www.raspberrypi.org/forums/viewtopic.php?t=148600 > */

#elif defined ( PD_ARM ) && defined ( __ARM_NEON )

/* Stolen from JUCE (FloatVectorOperations::enableFlushToZeroMode). */

static intptr_t denormal_getFPSR (void)
{
    intptr_t fpsr = 0; asm volatile("vmrs %0, fpscr" : "=r" (fpsr)); return fpsr;       // --
}

static void denormal_setFPSR (intptr_t fpsr)
{
    asm volatile("vmsr fpscr, %0" : : "ri" (fpsr));                                     // --
}

void denormal_setPolicy (void)
{
    denormal_setFPSR ((denormal_getFPSR() | (intptr_t)(1 << 24)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#else

void denormal_setPolicy (void)
{

}

#endif

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
