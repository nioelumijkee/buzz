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
/* modified 2.04.2021 */
#include "m_pd.h"
#include "math.h"

#define AC_PI 3.1415927
#define AC_2PI 6.2831853
#define MAXH 64
#define MAXH_1 63
#define MAXA 1024
#define MAXA_1 1022

static t_class *buzz_class;

typedef struct _buzz
{
  t_object x_obj;
  t_float phase;
  t_float div_1_sr;
  int h;
} t_buzz;

t_float a_cs[MAXA];
t_float a_sn[MAXA];
t_float a_csh[MAXH][MAXA];
t_float a_snh[MAXH][MAXA];
t_float a_s1[MAXH][MAXA];
t_float a_s2[MAXH][MAXA];
t_float a_ha[MAXH];
t_float a_hm[MAXH];

// -------------------------------------------------------------------------- //
static void *buzz_new(void)
{
  t_buzz *x = (t_buzz *)pd_new(buzz_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, gensym("signal"));
  x->phase = 0;
  return (x);
}

// -------------------------------------------------------------------------- //
static t_int *buzz_perform(t_int *w)
{
  t_buzz *x = (t_buzz *)(w[1]);
  t_float *in_freq = (t_float *)(w[2]);
  t_float *in_a = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  int n = (int)(w[5]);
  t_float phase = x->phase;
  t_float div_1_sr = x->div_1_sr;
  t_float a;
  t_float a2;
  t_float ap2;
  t_float s1;
  t_float s2;
  t_float cs;
  t_float sn;
  t_float csh;
  t_float snh;
  t_float inc;
  t_float ca;
  int h = x->h;
  t_float f; // buf
  int i0,i1; // buf
  
  while (n--)
    {
      // increment phase
      inc = *(in_freq++) * div_1_sr;
      if      (inc > 0.5)   inc = 0.5;
      else if (inc < 0.0)   inc = 0.0;

      // correction
      f = inc*4.0;
      if (f>1.0) f=1.0;
      ca = 1.0 - f;

      // phase
      phase += inc;
      if (phase > 1.0) phase -= 1.0;

      // a 0 ... 1
      a = *(in_a++);

      // clip
      if      (a < 0) a = 0;
      else if (a > 1) a = 1;

      // ca
      a = a * ca;

      // scale
      a = (a*a_hm[h])+a_ha[h];

      // 
      a2 = a+a;
      ap2 = a*a;

      // a
      // int-fract
      f = a * MAXA_1;
      i0 = f;
      i1 = i0+1;
      f = f - i0;

      // read
      s1 = (a_s1[h][i1] - a_s1[h][i0]) * f + a_s1[h][i0];
      s2 = (a_s2[h][i1] - a_s2[h][i0]) * f + a_s2[h][i0];

      // phase
      // int-fract
      f = phase * MAXA_1;
      i0 = f;
      i1 = i0+1;
      f = f - i0;

      // read
      cs = (a_cs[i1] - a_cs[i0]) * f + a_cs[i0];
      sn = (a_sn[i1] - a_sn[i0]) * f + a_sn[i0];
      csh = (a_csh[h][i1] - a_csh[h][i0]) * f + a_csh[h][i0];
      snh = (a_snh[h][i1] - a_snh[h][i0]) * f + a_snh[h][i0];
	
      // work
      *out++ = ((1 - a*cs) * (s1*cs - s2*csh) - (a*sn*(s2*snh - (s1*sn))))
	/ (1 - (a2*cs) + ap2);
    }
  // store
  x->phase = phase;
  return (w+6);
}

// -------------------------------------------------------------------------- //
static void buzz_dsp(t_buzz *x, t_signal **sp)
{
  x->div_1_sr = 1.0 / sp[0]->s_sr;
  dsp_add(buzz_perform,
	  5, 
	  x, 
	  sp[0]->s_vec, 
	  sp[1]->s_vec, 
	  sp[2]->s_vec, 
	  sp[0]->s_n);
}

// -------------------------------------------------------------------------- //
static void buzz_phase(t_buzz *x, t_floatarg f)
{
  x->phase = f;
}

// -------------------------------------------------------------------------- //
static void buzz_h(t_buzz *x, t_floatarg f)
{
  int i = f;
  if      (i<0)      i = 0;
  else if (i>MAXH_1) i = MAXH_1;
  x->h = i;
}

// -------------------------------------------------------------------------- //
static void calc_tables(void)
{
  int i,h;
  t_float a;
  t_float ah;
  t_float h1;
  t_float h2;
  t_float ha;
  t_float h_ph;
  t_float hm;
  for(h=0; h<MAXH; h++)
    {
      h1 = h+1;
      h2 = h+2;
      ha = ((t_float)h*0.0043)+0.011;
      hm = 0.995 - ha;
      for (i=0; i<MAXA; i++)
	{
	  a = (t_float)i / (t_float)MAXA; // 0...1

	  // calc
	  ah = pow(a, h1);
	  a_s1[h][i] = (1.0 - a) / (1.0 - fabs(ah));
	  a_s2[h][i] = (1.0 - a) / (1.0/fabs(ah) - fabs(ah)/ah);

	  h_ph = h2 * a * AC_2PI;
	  a_csh[h][i] = cos(h_ph);
	  a_snh[h][i] = sin(h_ph);

	}
      // write
      a_ha[h] = ha;
      a_hm[h] = hm;
    }

  for (i=0; i<MAXA; i++)
    {
      a = (t_float)i / (t_float)MAXA; // 0...1
      a_cs[i]  = cos(a * AC_2PI);
      a_sn[i]  = sin(a * AC_2PI);
    }
}


// -------------------------------------------------------------------------- //
void buzz_tilde_setup(void)
{
  buzz_class=class_new(gensym("buzz~"),(t_newmethod)buzz_new,0,
		       sizeof(t_buzz),0,0,0);
  class_addmethod(buzz_class,nullfn,gensym("signal"),0);
  class_addmethod(buzz_class,(t_method)buzz_dsp,gensym("dsp"),0);
  class_addmethod(buzz_class,(t_method)buzz_phase,gensym("phase"),A_FLOAT,0);
  class_addmethod(buzz_class,(t_method)buzz_h,gensym("h"),A_FLOAT,0);
  calc_tables();
}
