/*
Copyright:

This software is copyrighted by Miller Puckette and others.  The following
terms apply to all files associated with the software unless explicitly
disclaimed in individual files.

The authors hereby grant permission to use, copy, modify, distribute,
and license this software and its documentation for any purpose, provided
that existing copyright notices are retained in all copies and that this
notice is included verbatim in any distributions. No written agreement,
license, or royalty fee is required for any of the authorized uses.
Modifications to this software may be copyrighted by their authors
and need not follow the licensing terms described here, provided that
the new terms are clearly indicated on the first page of each file where
they apply.

IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
MODIFICATIONS.

RESTRICTED RIGHTS: Use, duplication or disclosure by the government
is subject to the restrictions as set forth in subparagraph (c) (1) (ii)
of the Rights in Technical Data and Computer Software Clause as DFARS
252.227-7013 and FAR 52.227-19.
*/

/* modified 15.03.2021 */
/* modified 24.03.2021 */
#include "m_pd.h"
#include "math.h"

#define AC_PI 3.1415927
#define AC_2PI 6.2831853

static t_class *buzz_tilde_class;

typedef struct _buzz_tilde
{
  t_object x_obj;
  t_float phase;
  t_float div_2pi_sr;
  t_float h1;
  t_float h2;
  t_float ha;
  t_float hm;
} t_buzz_tilde;

// -------------------------------------------------------------------------- //
static void *buzz_tilde_new(void)
{
  t_buzz_tilde *x = (t_buzz_tilde *)pd_new(buzz_tilde_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, gensym("signal"));
  x->phase = 0;
  return (x);
}

// -------------------------------------------------------------------------- //
static t_int *buzz_tilde_perform(t_int *w)
{
  t_buzz_tilde *x = (t_buzz_tilde *)(w[1]);
  t_float *in_freq = (t_float *)(w[2]);
  t_float *in_a = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  int n = (int)(w[5]);
  t_float phase = x->phase;
  t_float div_2pi_sr = x->div_2pi_sr;
  t_float h1 = x->h1;
  t_float h2 = x->h2;
  t_float ha = x->ha;
  t_float hm = x->hm;
  t_float a;
  t_float a2;
  t_float ap2;
  t_float s1;
  t_float s2;
  t_float cs;
  t_float sn;
  t_float h_ph;
  t_float inc;
  t_float ah;
  t_float ca;
  t_float f; // buf
  
  
  while (n--)
    {
      // increment phase
      inc = *(in_freq++) * div_2pi_sr;
      if      (inc > AC_PI) inc = AC_PI;
      else if (inc < 0.0)   inc = 0.0;

      // correction
      f = inc / AC_PI;
      f = f*4;
      if (f>1) f=1;
      ca = 1.0 - f;

      // phase
      phase += inc;
      if (phase > AC_2PI) phase -= AC_2PI;

      // a 0 ... 1
      a = *(in_a++);

      // clip
      if      (a < 0) a = 0;
      else if (a > 1) a = 1;

      // ca
      a = a * ca;

      // scale
      a = (a*hm)+ha;

      // 
      a2 = a+a;
      ap2 =a*a;

      // calc
      ah = pow(a, h1);
      s1 = (1 - a) / (1 - fabs(ah));
      s2 = (1 - a) / (1/fabs(ah) - fabs(ah)/ah);
	
      // do work with phase:
      cs = cos(phase);
      sn = sin(phase);
      h_ph = h2 * phase;
      *out++ = ((1 - a*cs) * (s1*cs - s2*cos(h_ph)) - (a*sn*(s2*sin(h_ph) - (s1*sn))))
	/ (1 - (a2*cs) + ap2);
    }
  // store
  x->phase = phase;
  return (w+6);
}

// -------------------------------------------------------------------------- //
static void buzz_tilde_dsp(t_buzz_tilde *x, t_signal **sp)
{
  x->div_2pi_sr = AC_2PI / sp[0]->s_sr;
  dsp_add(buzz_tilde_perform,
	  5, 
	  x, 
	  sp[0]->s_vec, 
	  sp[1]->s_vec, 
	  sp[2]->s_vec, 
	  sp[0]->s_n);
}

// -------------------------------------------------------------------------- //
static void buzz_tilde_phase(t_buzz_tilde *x, t_floatarg f)
{
  x->phase = f;
}

// -------------------------------------------------------------------------- //
static void buzz_tilde_h(t_buzz_tilde *x, t_floatarg f)
{
  int i = f;
  if (i<0) i = 0;
  x->h1 = i+1;
  x->h2 = i+2;
  x->ha = ((t_float)i*0.0043)+0.011;
  x->hm = 0.995 - x->ha;
}


// -------------------------------------------------------------------------- //
void buzz_tilde_setup(void)
{
  buzz_tilde_class = class_new(gensym("buzz~"), (t_newmethod)buzz_tilde_new, 0,
			       sizeof(t_buzz_tilde), 0, 0, 0);
  class_addmethod(buzz_tilde_class, nullfn, gensym("signal"), 0);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_phase, gensym("phase"), A_FLOAT, 0);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_h, gensym("h"), A_FLOAT, 0);
}
