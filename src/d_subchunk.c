
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static t_error subchunk_traverseStart (t_headerhelper *t, t_audioproperties *args)
{
    if (t->h_bytesSet >= 12 + 8) {
    //
    int swap = args->ap_needToSwap;
    
    t->h_onset     = 12;
    t->h_ID[0]     = *(t->h_c + t->h_onset + 0);
    t->h_ID[1]     = *(t->h_c + t->h_onset + 1);
    t->h_ID[2]     = *(t->h_c + t->h_onset + 2);
    t->h_ID[3]     = *(t->h_c + t->h_onset + 3);
    t->h_ID[4]     = 0;
    t->h_chunkSize = (int)soundfile_swap4Bytes (*((uint32_t *)(t->h_c + t->h_onset + 4)), swap);
    
    return PD_ERROR_NONE;
    //
    }
    
    return PD_ERROR;
}

static t_error subchunk_traverseNext (int f, t_headerhelper *t, t_audioproperties *args)
{
    t_error err = PD_ERROR;
    
    return err;
}

static char *subchunk_getID (t_headerhelper *t)
{
    return t->h_ID;
}

static int subchunk_getSize (t_headerhelper *t)
{
    return t->h_chunkSize;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

t_error soundfile_readFileHeaderWAVE (int f, t_headerhelper *t, t_audioproperties *args)
{
    return PD_ERROR;
}

t_error soundfile_readFileHeaderAIFF (int f, t_headerhelper *t, t_audioproperties *args)
{
    t_error err = subchunk_traverseStart (t, args);
    
    if (!err) {
        post ("%s", subchunk_getID (t));
        post ("%d", subchunk_getSize (t));
    }
    
    return PD_ERROR;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
