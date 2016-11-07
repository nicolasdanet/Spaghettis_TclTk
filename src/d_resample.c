
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

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------

#define RESAMPLE_PAD        0
#define RESAMPLE_HOLD       1
#define RESAMPLE_LINEAR     2
#define RESAMPLE_DEFAULT    3

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

static t_int *downsampling_perform_0(t_int *w)
{
  t_sample *in  = (t_sample *)(w[1]); /* original signal     */
  t_sample *out = (t_sample *)(w[2]); /* downsampled signal  */
  int down     = (int)(w[3]);       /* downsampling factor */
  int parent   = (int)(w[4]);       /* original vectorsize */

  int n=parent/down;

  while(n--){
    *out++=*in;
    in+=down;
  }

  return (w+5);
}

static t_int *upsampling_perform_0(t_int *w)
{
  t_sample *in  = (t_sample *)(w[1]); /* original signal     */
  t_sample *out = (t_sample *)(w[2]); /* upsampled signal    */
  int up       = (int)(w[3]);       /* upsampling factor   */
  int parent   = (int)(w[4]);       /* original vectorsize */

  int n=parent*up;
  t_sample *dummy = out;
  
  while(n--)*out++=0;

  n = parent;
  out = dummy;
  while(n--){
    *out=*in++;
    out+=up;
  }

  return (w+5);
}

static t_int *upsampling_perform_hold(t_int *w)
{
  t_sample *in  = (t_sample *)(w[1]); /* original signal     */
  t_sample *out = (t_sample *)(w[2]); /* upsampled signal    */
  int up       = (int)(w[3]);       /* upsampling factor   */
  int parent   = (int)(w[4]);       /* original vectorsize */
  int i=up;

  int n=parent;
  t_sample *dum_out = out;
  t_sample *dum_in  = in;
  
  while (i--) {
    n = parent;
    out = dum_out+i;
    in  = dum_in;
    while(n--){
      *out=*in++;
      out+=up;
    }
  }
  return (w+5);
}

static t_int *upsampling_perform_linear(t_int *w)
{
  t_resample *x= (t_resample *)(w[1]);
  t_sample *in  = (t_sample *)(w[2]); /* original signal     */
  t_sample *out = (t_sample *)(w[3]); /* upsampled signal    */
  int up       = (int)(w[4]);       /* upsampling factor   */
  int parent   = (int)(w[5]);       /* original vectorsize */
  int length   = parent*up;
  int n;
  t_sample *fp;
  t_sample a=*x->r_buffer, b=*in;

  
  for (n=0; n<length; n++) {
    t_sample findex = (t_sample)(n+1)/up;
    int     index  = findex;
    t_sample frac=findex - index;
    if (frac==0.)frac=1.;
    *out++ = frac * b + (1.-frac) * a;
    fp = in+index;
    b=*fp;
    a=(index)?*(fp-1):a;
  }

  *x->r_buffer = a;
  return (w+6);
}

static void resample_dsp(t_resample *x,
                  t_sample* in,  int insize,
                  t_sample* out, int outsize,
                  int method)
{
  if (insize == outsize){
    PD_BUG;
    return;
  }

  if (insize > outsize) { /* downsampling */
    if (insize % outsize) {
      post_error ("bad downsampling factor");
      return;
    }
    switch (method) {
    default:
      dsp_add(downsampling_perform_0, 4, in, out, insize/outsize, insize);
    }


  } else { /* upsampling */
    if (outsize % insize) {
      post_error ("bad upsampling factor");
      return;
    }
    switch (method) {
    case 1:
      dsp_add(upsampling_perform_hold, 4, in, out, outsize/insize, insize);
      break;
    case 2:
      if (x->r_bufferSize != 1) {
        PD_MEMORY_FREE(x->r_buffer);
        x->r_bufferSize = 1;
        x->r_buffer = PD_MEMORY_GET(x->r_bufferSize*sizeof(*x->r_buffer));
      }
      dsp_add(upsampling_perform_linear, 5, x, in, out, outsize/insize, insize);
      break;
    default:
      dsp_add(upsampling_perform_0, 4, in, out, outsize/insize, insize);
    }
  }
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void resample_init (t_resample *x, t_symbol *type)
{
    x->r_type = RESAMPLE_DEFAULT;
    
    if (type == sym_hold)   { x->r_type = RESAMPLE_HOLD;    }
    if (type == sym_linear) { x->r_type = RESAMPLE_LINEAR;  }
    if (type == sym_pad)    { x->r_type = RESAMPLE_PAD;     }
    
    #if PD_WITH_LEGACY
    
    if (type == sym_lin)    { x->r_type = RESAMPLE_LINEAR;  }
    
    #endif

    x->r_downSample = 1;
    x->r_upSample   = 1;
    x->r_vectorSize = 0;
    x->r_bufferSize = 0;
    x->r_vector     = NULL;
    x->r_buffer     = NULL;
}

void resample_free (t_resample *x)
{
    if (x->r_vectorSize) { PD_ASSERT (x->r_vector); PD_MEMORY_FREE (x->r_vector); }
    if (x->r_bufferSize) { PD_ASSERT (x->r_buffer); PD_MEMORY_FREE (x->r_buffer); }

    resample_init (x, &s_);
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
#pragma mark -

void resample_toDsp (t_resample *x, t_sample *out, int size, int sizeResampled)
{
    int method = (x->r_type == RESAMPLE_DEFAULT) ? RESAMPLE_HOLD : x->r_type;
    
  if (sizeResampled==size) {
    if (x->r_vectorSize)PD_MEMORY_FREE(x->r_vector);
    x->r_vectorSize = 0;
    x->r_vector = out;
    return;
  }

  if (x->r_vectorSize != sizeResampled) {
    t_sample *buf=x->r_vector;
    PD_MEMORY_FREE(buf);
    buf = (t_sample *)PD_MEMORY_GET(sizeResampled * sizeof(*buf));
    x->r_vector = buf;
    x->r_vectorSize   = sizeResampled;
  }

  resample_dsp(x, x->r_vector, x->r_vectorSize, out, size, method);

  return;
}

void resample_fromDsp (t_resample *x, t_sample *s, int size, int sizeResampled)
{
    int method = (x->r_type == RESAMPLE_DEFAULT) ? RESAMPLE_PAD : x->r_type;
    
  if (size==sizeResampled) {
   PD_MEMORY_FREE(x->r_vector);
    x->r_vectorSize = 0;
    x->r_vector = s;
    return;
  }

  if (x->r_vectorSize != sizeResampled) {
    t_sample *buf=x->r_vector;
    PD_MEMORY_FREE(buf);
    buf = (t_sample *)PD_MEMORY_GET(sizeResampled * sizeof(*buf));
    x->r_vector = buf;
    x->r_vectorSize   = sizeResampled;
  }

  resample_dsp(x, s, size, x->r_vector, x->r_vectorSize, method);
  return;
}

// -----------------------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------------------
