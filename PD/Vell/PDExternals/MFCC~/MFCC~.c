#include <stdio.h>
#include <math.h>
#include "m_pd.h"

static t_class *MFCC_tilde_class;

typedef struct _MFCC_tilde {
  t_object x_obj;
  t_sample f_MFCC;
  t_sample f;
  int numOfBands;
} t_MFCC_tilde;


const double PI = 3.1415926538;

void dft(double *x, double *y, double *r, double *i, int N)
{
   int   n, k;
   double c, s, p, x0, y100;

   for(k = 0; k < N ; k++)
   { 
      x0 = 0;
      y100 = 0;

      for(n = 0; n < N; n++)
      {
         p  = -2*PI*k*n/N;
         c  = cos(p);
         s  = sin(p);
         /*x0 = x0 + c*x[n] - s*y[n];
         y0 = y0 + c*y[n] + s*x[n];*/
         x0 = x0 + c*x[n];
         y100 = y100 + s*x[n];
         
      }
      r[k] = x0;
      i[k] = y100;
   }
}

void MFCC(double *MFCCoeffs, double *xr, double *xi, int N, t_MFCC_tilde *x)
{
   
    int numOfBands = x->numOfBands;
    double fs = 48000;
    double mel_f, linear_f, tri;
    double fftlen = N;
    
   
    
    /* frequency mappings */
    
    int i, k;
    double d1 = 0;
    double d2 = 1127.01048*log(1+(fs/2)/700);
    int fft_ind[numOfBands];
    double P[numOfBands*2], wr, wi, FCr[numOfBands*2],FCi[numOfBands*2];
    
    for(i = 0; i < numOfBands+2; i++)
    {
        mel_f    = d1 + i*(d2-d1)/(floor(numOfBands+2-1)); /* mel */
        linear_f = 700*(exp(mel_f/1127.01048)-1);/* Hz */
        fft_ind[i] = (int)floor(fftlen*linear_f/fs+0.5);

    }
    
    /* Spectral power */
    for(i = 0;i < numOfBands;i++)
    {
        P[i] = 0;
        for(k = fft_ind[i]; k < fft_ind[i+2];k++)
        {
		/* Create the triangluar filter in frequency domain */
            if( k < fft_ind[i+1])
            {
                tri = 1+((double)k-fft_ind[i+1])*1/
		((double)fft_ind[i+1] - (double)fft_ind[i]);
            }
            else
            {
                tri = 1 + ((double)k-fft_ind[i+1])*(-1)/
		((double)fft_ind[i+2] - (double)fft_ind[i+1]);
            }
             
            P[i] = P[i] + (pow(xr[k],2) + pow(xi[k],2))*tri;
        }
        P[i] = log10(P[i] + 0.00001);
        /* for dct */
        P[2*numOfBands-(i+1)] = P[i];
        
        
    }
 
    /* DCT */
    dft(P, P, FCr, FCi, numOfBands*2);
    
    for(i = 0; i < numOfBands; i++)
    {
        /* DCT weights */
        wr = (cos(-(i)*PI/(2*numOfBands))/pow(2*numOfBands,.5));
        wi = (sin(-(i)*PI/(2*numOfBands))/pow(2*numOfBands,.5));
        
        if( i == 0)
        {
            wr = wr / pow(2,.5);
            wi = wi / pow(2,.5);
        }
        /* only the real part is interesting since input is real */
       
        MFCCoeffs[i] = FCr[i] * wr  - wi * FCi[i];
    }
    
}




t_int *MFCC_tilde_perform(t_int *w)
{

  t_MFCC_tilde  *x  = (t_MFCC_tilde *)(w[1]);
  t_sample  *in1 = (t_sample *)(w[2]);
  t_sample  *in2 = (t_sample *)(w[3]);
   
  t_sample  *out = (t_sample *)(w[4]);
  int        n   = (int)(w[5]);
  
  double xr[n], xi[n];
  int numOfBands = x->numOfBands;
  double MFCCoeffs[n];

  int i;
  for (i = 0;i < n;i++)
  { 	
	/* input */
	xr[i] = (double) in1[i];
	xi[i] = (double) in2[i];
	MFCCoeffs[i] = 0.0;
        
  }
  
  /* activity detected */

  
    MFCC(MFCCoeffs, xr, xi, n, x);
    
    for (i = 0;i < numOfBands;i++)
    {  
    out[i] = (t_sample) MFCCoeffs[i];
    }
    
    /* fill the rest of the block beyond 25 MFCC bands with zeros */
    for (i = numOfBands;i < n;i++)
    {  
    out[i] = (t_sample) 0.0;
    }
    
    out[0] = (t_sample) 0.0; /* remove the first coefficient */
    
  /* if not activity detected, fill output 
  else {
    for (i = 0;i < n;i++)
    {  
      out[i] = (t_sample) -1000;
    }
  }
  */

  return (w+6);

}

void MFCC_tilde_dsp(t_MFCC_tilde *x, t_signal **sp)
{
    dsp_add(MFCC_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

void *MFCC_tilde_new(t_floatarg f)
{
  t_MFCC_tilde *x = (t_MFCC_tilde *)pd_new(MFCC_tilde_class);

  /*x->f_MFCC = f;*/
  
  
  x->numOfBands = (f)?f:50; /* defaults to 50 */

  post("Number of MFCC bands: %d", x->numOfBands);
  
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  
  
  outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void MFCC_tilde_setup(void) {
  MFCC_tilde_class = class_new(gensym("MFCC~"),
        (t_newmethod)MFCC_tilde_new,
        0, sizeof(t_MFCC_tilde),
        CLASS_DEFAULT, 
        A_DEFFLOAT, 0);

  class_addmethod(MFCC_tilde_class,
        (t_method)MFCC_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(MFCC_tilde_class, t_MFCC_tilde, f);
}


