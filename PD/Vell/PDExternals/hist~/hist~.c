#include "m_pd.h"
#include "math.h"
#include <stdlib.h>
/*#include "frequencies.h"*/
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#define FS 48000
#define Flow 100
#define Fhigh 1000

static t_class *hist_tilde_class;

typedef struct _hist_tilde {
  t_object x_obj;
  t_float counter;
  t_float bufferLenIn;
  int noBins;
  double minHist;
  double maxHist;
  t_outlet *f_out;
  t_outlet *bufferLenOut;
  t_float bufferLength;
  int bufNo;
  t_float *histogram;
  t_sample f;
} t_hist_tilde;


int calcHistBin(double value, t_hist_tilde *x)
{
  /* check into which bin a value belongs */
	double a;
	double step;
	int i;
	int returnBin;
	
	returnBin = 0;
	i = 0;
	a = x->minHist;
	step = (x->maxHist - x->minHist) / x->noBins;

	while (a<value && i<x->noBins) {
		returnBin = i+1;
		a = a + step;
		i++;
	}
	return returnBin-1;
}

static void resetHistogram(t_hist_tilde *x)
{
  /* re-initialize historam memory space */
	int i = x->noBins*(int)x->bufferLength;
	t_float *histogram = x->histogram;

	while (i--) {
    *histogram++= 0.;
  }
  
	x->counter = 0;
}

void printInfo(t_hist_tilde *x)
{
  post("Histogram bins %d",x->noBins);
  post("Histogram limits %f %f", x->minHist, x->maxHist);
}

t_int *hist_tilde_perform(t_int *w)
{
	/* inputs and outputs  */
  t_hist_tilde *x   = (t_hist_tilde *)(w[1]);
  t_sample  *in1    = (t_sample *)(w[2]);
  t_sample  *in2    = (t_sample *)(w[3]); /* obsolete (adaptive MFCC) */
  t_sample  *out    = (t_sample *)(w[4]);
  int         n     = (int)(w[5]);
  
  int i = x->noBins;
  int j;
  int returnBin;
  
  int bufNo = x->bufNo;
  
  double temp1[n];
  double outputSum[n];

  /* init */

  t_float histSum = 0.00000;
  t_float histMax = 0.00001;
  t_float tempSum = 0.;
  
  t_float *histogram = x->histogram;
  t_int returnBins[x->noBins];
  t_int currentBin;

  for(i=0; i<x->noBins; i++) {
	  returnBins[i] = 0.;
  }

  int lowBin = (int) floor(Flow/(FS/n));
  int highBin = (int) floor(Fhigh/(FS/n));


  for(i=0; i<n; i++) {
    if(i>lowBin && i<highBin) { 
      /* only count selected interval from intensity vector analysis bins */
      /* then increase the correct bin count by one */
      temp1[i] = (double) in1[i];
      currentBin = calcHistBin(temp1[i], x);
      returnBins[currentBin]++;
      }
  }
  
  /* replace current buffer */ 
  for(i=0; i<x->noBins; i++) {
	  histogram[x->noBins*bufNo + i] = returnBins[i];
  }

  /* calculate sum for each bin over frames*/
  for(i=0; i < x->noBins; i++) {
    tempSum = 0.;
    for(j=0; j < x->bufferLength; j++) {

        tempSum = tempSum + histogram[j*x->noBins + i];

      }
	  
	  if(tempSum > histMax) histMax = tempSum;
	  outputSum[i] = tempSum;
	  histSum = histSum + tempSum;
  }
  
  
  /* scale histogram output */ 
  for(i=0; i < n; i++) {
    if(i<x->noBins) {
      out[i] = (t_sample) (10.0 * outputSum[i] / histSum);
	  }
	  else {
        out[i] = (t_sample) (0.0);
	  }
  }
  
  
  /* advance or reset current histogram buffer position */
  if (x->bufNo < x->bufferLength -1) {
    x->bufNo++;
  }
  else {
    x->bufNo = 0;
  }
  
  /* outlet histogram length */
  outlet_float(x->bufferLenOut, x->bufferLength);

  return (w+6);
}


void hist_tilde_dsp(t_hist_tilde *x, t_signal **sp)
{
  dsp_add(hist_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}

static void hist_tilde_free(t_hist_tilde *x)
{
  /* free reserved memory when destrying object */
	freebytes(x->histogram, (int)(x->bufferLength) * x->noBins*sizeof(t_float));
}

void *hist_tilde_new(t_floatarg f)
{
  t_hist_tilde *x = (t_hist_tilde *)pd_new(hist_tilde_class);

  t_float *histogram;
  t_float noBins;
  t_float minHist = 0.;
  t_float maxHist = 1.;
  
  /* optional creation argument defines number of histogram bins */
  noBins = (f)?f:100.;

  x->counter = 0;
  x->bufNo = 0;
  x->minHist = (double) minHist;
  x->maxHist = (double) maxHist;  

  
  x->noBins = (int) noBins;
  x->bufferLength = 200; /* frames */

  post("Histogram bins %d",x->noBins);
  post("Histogram limits %f %f", x->minHist, x->maxHist);

  /* reserve memory for storing the updating histogram */
  x->histogram = (t_float *)getbytes(x->bufferLength * x->noBins * sizeof(t_float));
  histogram = x->histogram;

  resetHistogram(x);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  x->bufferLenOut = outlet_new(&x->x_obj, &s_float);

  floatinlet_new(&x->x_obj, &x->bufferLenIn);


  return (void *)x;
}



void hist_tilde_setup(void) {
  hist_tilde_class = class_new(gensym("hist~"), (t_newmethod) hist_tilde_new, (t_method) hist_tilde_free,
        sizeof(t_hist_tilde),CLASS_DEFAULT,A_DEFFLOAT,0);

  /* reset histogram with bang */
  class_addbang(hist_tilde_class,resetHistogram);
  
  class_addmethod(hist_tilde_class, (t_method)printInfo, gensym("info"), 0);

  class_addmethod(hist_tilde_class,
        (t_method)hist_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(hist_tilde_class, t_hist_tilde, f);
}

