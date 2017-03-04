
/* 
    Copyright (c) 1997-2016 Miller Puckette and others.
*/

/* < https://opensource.org/licenses/BSD-3-Clause > */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

#include "m_pd.h"
#include "m_core.h"
#include "d_dsp.h"
#include "d_soundfile.h"

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

/* Note that 2-3 bytes per sample is assumed LPCM. */
/* Likewise, 4 bytes per sample is assumed 32-bit IEEE float. */

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define SOUNDFILE_SCALE     (1.0 / (1024.0 * 1024.0 * 1024.0 * 2.0))

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static inline int soundfile_encodeLinear16Value (t_sample f, t_sample k)
{
    int v = (int)(32768.0 + (f * k));
    v -= 32768;
    return PD_CLAMP (v, -32767, 32767);
}

static inline void soundfile_encodeLinear16BigEndian (t_sample f, t_sample k, unsigned char *p)
{
    int v = soundfile_encodeLinear16Value (f, k);
    
    p[0] = 0xff & (v >> 8);
    p[1] = 0xff & (v);
}

static inline void soundfile_encodeLinear16LittleEndian (t_sample f, t_sample k, unsigned char *p)
{
    int v = soundfile_encodeLinear16Value (f, k);
    
    p[0] = 0xff & (v);
    p[1] = 0xff & (v >> 8);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline int soundfile_encodeLinear24Value (t_sample f, t_sample k)
{
    int v = (int)(8388608.0 + (f * k));
    v -= 8388608;
    return PD_CLAMP (v, -8388607, 8388607);
}

static inline void soundfile_encodeLinear24BigEndian (t_sample f, t_sample k, unsigned char *p)
{
    int v = soundfile_encodeLinear24Value (f, k);
    
    p[0] = 0xff & (v >> 16);
    p[1] = 0xff & (v >> 8);
    p[2] = 0xff & (v);
}

static inline void soundfile_encodeLinear24LittleEndian (t_sample f, t_sample k, unsigned char *p)
{
    int v = soundfile_encodeLinear24Value (f, k);
    
    p[0] = 0xff & (v);
    p[1] = 0xff & (v >> 8);
    p[2] = 0xff & (v >> 16);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline void soundfile_encodeFloatBigEndian (t_sample f, t_sample k, unsigned char *p)
{
    t_rawcast32 z;
    
    z.z_f = f * k;
    
    p[0] = 0xff & (z.z_i >> 24);
    p[1] = 0xff & (z.z_i >> 16);
    p[2] = 0xff & (z.z_i >> 8);
    p[3] = 0xff & (z.z_i);
}

static inline void soundfile_encodeFloatLittleEndian (t_sample f, t_sample k, unsigned char *p)
{
    t_rawcast32 z;
    
    z.z_f = f * k;
    
    p[0] = 0xff & (z.z_i);
    p[1] = 0xff & (z.z_i >> 8);
    p[2] = 0xff & (z.z_i >> 16);
    p[3] = 0xff & (z.z_i >> 24);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfile_encodeLinear16 (int numberOfChannels,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    t_sample normalFactor,
    int spread)
{
    int i, j;
    unsigned char *p1 = t;
    unsigned char *p2 = NULL;
    t_sample *s = NULL;
    
    int bytesPerFrame = bytesPerSample * numberOfChannels;
    
    t_sample k = (t_sample)(normalFactor * 32768.0);
    int offset = spread * onset;
    
    if (isBigEndian) {
        for (i = 0; i < numberOfChannels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            soundfile_encodeLinear16BigEndian (*s, k, p2);
        }
        
        p1 += bytesPerSample;
        }
        
    } else {
        for (i = 0; i < numberOfChannels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            soundfile_encodeLinear16LittleEndian (*s, k, p2);
        }
        
        p1 += bytesPerSample;
        }
    }
}

void soundfile_encodeLinear24 (int numberOfChannels,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    t_sample normalFactor,
    int spread)
{
    int i, j;
    unsigned char *p1 = t;
    unsigned char *p2 = NULL;
    t_sample *s = NULL;
    
    int bytesPerFrame = bytesPerSample * numberOfChannels;
    
    t_sample k = (t_sample)(normalFactor * 8388608.0);
    int offset = spread * onset;
    
    if (isBigEndian) {
        for (i = 0; i < numberOfChannels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            soundfile_encodeLinear24BigEndian (*s, k, p2);
        }
        
        p1 += bytesPerSample;
        }
        
    } else {
        for (i = 0; i < numberOfChannels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            soundfile_encodeLinear24LittleEndian (*s, k, p2);
        }
        
        p1 += bytesPerSample;
        }
    }
}

void soundfile_encodeFloat (int numberOfChannels,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    t_sample normalFactor,
    int spread)
{
    int i, j;
    unsigned char *p1 = t;
    unsigned char *p2 = NULL;
    t_sample *s = NULL;
    
    int bytesPerFrame = bytesPerSample * numberOfChannels;
    int offset = spread * onset;
    
    if (isBigEndian) {
        for (i = 0; i < numberOfChannels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            soundfile_encodeFloatBigEndian (*s, normalFactor, p2);
        }
        
        p1 += bytesPerSample;
        }
        
    } else {
        for (i = 0; i < numberOfChannels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            soundfile_encodeFloatLittleEndian (*s, normalFactor, p2);
        }
        
        p1 += bytesPerSample;
        }
    }
}

void soundfile_encode (int numberOfChannels,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    int spread, 
    t_sample normalFactor)
{
    if (bytesPerSample == 2) {
    
        soundfile_encodeLinear16 (numberOfChannels,
            v, 
            t, 
            numberOfFrames,
            onset,
            bytesPerSample,
            isBigEndian,
            normalFactor,
            spread); 
    
    } else if (bytesPerSample == 3) {

        soundfile_encodeLinear24 (numberOfChannels,
            v, 
            t, 
            numberOfFrames,
            onset,
            bytesPerSample,
            isBigEndian,
            normalFactor,
            spread);
            
    } else if (bytesPerSample == 4) {

        soundfile_encodeFloat (numberOfChannels,
            v, 
            t, 
            numberOfFrames,
            onset,
            bytesPerSample,
            isBigEndian,
            normalFactor,
            spread);
            
    } else {
    
        PD_BUG;
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

/* Left operand of the shift operator is promote to the int type (assumed 32-bit). */

static inline t_sample soundfile_decodeLinear16BigEndian (unsigned char *p)
{
    return (t_sample)(SOUNDFILE_SCALE * ((p[0] << 24) | (p[1] << 16)));
}

static inline t_sample soundfile_decodeLinear16LittleEndian (unsigned char *p)
{
    return (t_sample)(SOUNDFILE_SCALE * ((p[1] << 24) | (p[0] << 16)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline t_sample soundfile_decodeLinear24BigEndian (unsigned char *p)
{
    return (t_sample)(SOUNDFILE_SCALE * ((p[0] << 24) | (p[1] << 16) | (p[2] << 8)));
}

static inline t_sample soundfile_decodeLinear24LittleEndian (unsigned char *p)
{
    return (t_sample)(SOUNDFILE_SCALE * ((p[2] << 24) | (p[1] << 16) | (p[0] << 8)));
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

static inline t_sample soundfile_decodeFloatBigEndian (unsigned char *p)
{
    t_rawcast32 z;
    
    z.z_i = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | (p[3]);
    
    return z.z_f;
}

static inline t_sample soundfile_decodeFloatLittleEndian (unsigned char *p)
{
    t_rawcast32 z;
    
    z.z_i = (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | (p[0]);
    
    return z.z_f;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void soundfile_decodeLinear16 (int numberOfChannels,
    int n,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    int spread)
{
    int i, j;
    unsigned char *p1 = t;
    unsigned char *p2 = NULL;
    t_sample *s = NULL;
    
    int channels = PD_MIN (numberOfChannels, n);
    int bytesPerFrame = bytesPerSample * numberOfChannels;
    int offset = spread * onset;
    
    if (isBigEndian) {
        for (i = 0; i < channels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            *s = soundfile_decodeLinear16BigEndian (p2);
        }
        
        p1 += bytesPerSample;
        }
        
    } else {
        for (i = 0; i < channels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            *s = soundfile_decodeLinear16LittleEndian (p2);
        }
        
        p1 += bytesPerSample;
        }
    }
}

void soundfile_decodeLinear24 (int numberOfChannels,
    int n,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    int spread)
{
    int i, j;
    unsigned char *p1 = t;
    unsigned char *p2 = NULL;
    t_sample *s = NULL;
    
    int channels = PD_MIN (numberOfChannels, n);
    int bytesPerFrame = bytesPerSample * numberOfChannels;
    int offset = spread * onset;
    
    if (isBigEndian) {
        for (i = 0; i < channels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            *s = soundfile_decodeLinear24BigEndian (p2);
        }
        
        p1 += bytesPerSample;
        }
        
    } else {
        for (i = 0; i < channels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            *s = soundfile_decodeLinear24LittleEndian (p2);
        }
        
        p1 += bytesPerSample;
        }
    }
}

void soundfile_decodeFloat (int numberOfChannels,
    int n,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    int spread)
{
    int i, j;
    unsigned char *p1 = t;
    unsigned char *p2 = NULL;
    t_sample *s = NULL;
    
    int channels = PD_MIN (numberOfChannels, n);
    int bytesPerFrame = bytesPerSample * numberOfChannels;
    int offset = spread * onset;
    
    if (isBigEndian) {
        for (i = 0; i < channels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            *s = soundfile_decodeFloatBigEndian (p2);
        }
        
        p1 += bytesPerSample;
        }
        
    } else {
        for (i = 0; i < channels; i++) {
        for (j = 0, p2 = p1, s = v[i] + offset; j < numberOfFrames; j++, p2 += bytesPerFrame, s += spread) {
            *s = soundfile_decodeFloatLittleEndian (p2);
        }
        
        p1 += bytesPerSample;
        }
    }
}

void soundfile_decode (int numberOfChannels,
    t_sample **v,
    unsigned char *t,
    int numberOfFrames,
    int onset,
    int bytesPerSample,
    int isBigEndian,
    int spread, 
    int n)
{
    if (bytesPerSample == 2) {
        
        soundfile_decodeLinear16 (numberOfChannels,
            n,
            v,
            t,
            numberOfFrames,
            onset,
            bytesPerSample,
            isBigEndian,
            spread);
    
    } else if (bytesPerSample == 3) {
        
        soundfile_decodeLinear24 (numberOfChannels,
            n,
            v,
            t,
            numberOfFrames,
            onset,
            bytesPerSample,
            isBigEndian,
            spread);
            
    } else if (bytesPerSample == 4) {
        
        soundfile_decodeFloat (numberOfChannels,
            n,
            v,
            t,
            numberOfFrames,
            onset,
            bytesPerSample,
            isBigEndian,
            spread);
            
    } else {
        
        PD_BUG;
    }
    
    /* Set to zero the supernumerary channels. */
    
    {
        int i, j;
        t_sample *s = NULL;
        
        for (i = numberOfChannels; i < n; i++) {
            for (j = 0, s = v[i] + (spread * onset); j < numberOfFrames; j++, s += spread) { 
                *s = (t_sample)0.0; 
            }
        }
    }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
