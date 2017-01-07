
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "m_macros.h"
#include "d_dsp.h"
#include "d_soundfile.h"
#include "d_chunk.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

t_error soundfile_readFileHeaderWAVE (int f, t_headerhelper *t, t_audioproperties *args)
{
    return PD_ERROR;
}

t_error soundfile_readFileHeaderAIFF (int f, t_headerhelper *t, t_audioproperties *args)
{
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
