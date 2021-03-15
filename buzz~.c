/* modified 15.03.2021 */
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
  t_float a;
  t_float h;
  t_float s1;
  t_float s2;
  t_float x_f;	    /* scalar frequency */
} t_buzz_tilde;

static void *buzz_tilde_new(t_floatarg f)
{
  t_buzz_tilde *x = (t_buzz_tilde *)pd_new(buzz_tilde_class);
  x->x_f = f;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("f_a"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("f_H"));
  x->phase = 0;
  x->div_2pi_sr = 0;
  x->a = 0;
  x->h = 0;
  x->s1 = 0;
  x->s2 = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static t_int *buzz_tilde_perform(t_int *w)
{
  t_buzz_tilde *x = (t_buzz_tilde *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  t_float *out = (t_float *)(w[3]);
  int n = (int)(w[4]);
  t_float phase = x->phase;
  t_float div_2pi_sr = x->div_2pi_sr;
  t_float a = x->a;
  t_float a2 = 2*a ;
  t_float ap2 = a*a;
  t_float s1 = x->s1;
  t_float s2 = x->s2;
  t_float h2 = x->h + 2;
  t_float cs;
  t_float sn;
  t_float h_ph;
  t_float inc;
  
  
  while (n--)
    {
      // increment phase
      inc = *in++ * div_2pi_sr;
      if      (inc > AC_PI) inc = AC_PI;
      else if (inc < 0.0)   inc = 0.0;
      phase += inc;
      if (phase > AC_2PI) phase -= AC_2PI;
	
      // do work with my_phi:
      cs = cos(phase);
      sn = sin(phase);
      h_ph = h2 * phase;
      *out++ = ((1 - a*cs) * (s1*cs - s2*cos(h_ph)) - (a*sn*(s2*sin(h_ph) - (s1*sn))))
	/ (1 - (a2*cs) + ap2);
    }
  x->phase = phase;
  return (w+5);
}

static void buzz_tilde_dsp(t_buzz_tilde *x, t_signal **sp)
{
  x->div_2pi_sr = AC_2PI / sp[0]->s_sr;
  dsp_add(buzz_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void varsetup (t_buzz_tilde *x)
{
  t_float a = x->a;
  t_float h = x->h;
  t_float ah = pow(a, h+1);
  if (a != 1 )
    {
      x->s1 = (1 - fabs(a)) / (1 - fabs(ah));
      x->s2 = (1 - fabs(a)) / (1/fabs(ah) - fabs(ah)/ah);
    }
}

static void buzz_tilde_ft1(t_buzz_tilde *x, t_float f)
{
  x->phase = f;
}


static void buzz_tilde_a(t_buzz_tilde *x, t_float f)
{
  x->a = f;
  varsetup(x);
}

static void buzz_tilde_H(t_buzz_tilde *x, t_float f)
{
  x->h = f;
  varsetup(x);
}


void buzz_tilde_setup(void)
{
  buzz_tilde_class = class_new(gensym("buzz~"), (t_newmethod)buzz_tilde_new, 0,
			       sizeof(t_buzz_tilde), 0, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(buzz_tilde_class, t_buzz_tilde, x_f);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_a, gensym("f_a"), A_FLOAT, 0);
  class_addmethod(buzz_tilde_class, (t_method)buzz_tilde_H, gensym("f_H"), A_FLOAT, 0);
}
