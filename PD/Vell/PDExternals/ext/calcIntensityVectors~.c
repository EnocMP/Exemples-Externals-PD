#include "m_pd.h"
#include "math.h"
#include <stdlib.h>



#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#define Flow 200
#define Fhigh 4000
#define FS 48000


static t_class *calcIntensityVectors_tilde_class;

typedef struct _calcIntensityVectors_tilde {
  t_object x_obj;
  double f_calcIntensityVectors;
  double f;
  double dist;
  int FootballFlag;
  t_outlet *f_az_out;
  t_outlet *f_el_out;
} t_calcIntensityVectors_tilde;

const double PI = 3.1415926538;

void calcIntensityVector(t_calcIntensityVectors_tilde *x, double *xr, double *xi, double *I, int N)
{
    double rho0 = 1.2;
    double dist = x->dist; /* 0.01; */
    int i,j,ii,c,m1,m2;
    double omega;
    double fs = 48000.0; /* 4*48000 */
    double nfft = N;
    
    /* particle velocity, real, imag */
    double uxr[N],uyr[N],uzr[N], uyi[N], uxi[N], uzi[N] ;
    /* pressure average, real, imag */
    double pr[N],pi[N];
    
    int micorder[6] = {0,1,2,3,4,5};
    
    if(x->FootballFlag > 0) 
    {
       micorder[0] = 0;
       micorder[1] = 2;
       micorder[2] = 1;
       micorder[3] = 3;
       micorder[4] = 4;
       micorder[5] = 5;
       
    }

    for(i = 0;i < N/2; i++ )
    {
        omega = (2 * PI * fs / nfft) * (i);
        /* multiplying with sqrt(-1) changes the order */
        m1 = micorder[0]; m2 = micorder[1];
        /* X-coordinate particle velocity */
        uxi[i] = -(xr[m1*N+i] - xr[m2*N+i])/(omega*rho0*dist);
        uxr[i] = (xi[m1*N+i] - xi[m2*N+i])/(omega*rho0*dist);
        m1 = micorder[2]; m2 = micorder[3];
        /* Y-coordinate particle velocity */
        uyi[i] = -(xr[m1*N+i] - xr[m2*N+i])/(omega*rho0*dist);
        uyr[i] = (xi[m1*N+i] - xi[m2*N+i])/(omega*rho0*dist);
        
        m1 = micorder[4]; m2 = micorder[5];
        /* z-coordinate particle velocity */
        uzi[i] = -(xr[m1*N+i] - xr[m2*N+i])/(omega*rho0*dist);
        uzr[i] = (xi[m1*N+i] - xi[m2*N+i])/(omega*rho0*dist);        
        
        /* average pressure */
        pr[i] = (xr[0*N+i] + xr[1*N+i] + xr[2*N+i] + xr[3*N+i] + xr[4*N+i] + xr[5*N+i])/6;
        pi[i] = (xi[0*N+i] + xi[1*N+i] + xi[2*N+i] + xi[3*N+i] + xi[4*N+i] + xi[5*N+i])/6;
        
        c = 0;
        I[c*N + i] = pr[i]*uxr[i] - (-1*pi[i]*uxi[i]);
        c = 1;
        I[c*N + i] = pr[i]*uyr[i] - (-1*pi[i]*uyi[i]);
        c = 2;
        I[c*N + i] = pr[i]*uzr[i] - (-1*pi[i]*uzi[i]);        
    }
}

void cart2pol(double *decisionVector, double *az, double *el, double *x, double *y, double *z, int N)
{
    int i;
    double AZr = 0;
    double AZi = 0;
    double AZ;
    
    double ELr = 0;
    double ELi = 0;
    double EL;
    
    double sumN = 0;
    
    int lowBin = (int) floor(Flow/(FS/N));
    int highBin = (int) floor(Fhigh/(FS/N));
    
    
    for(i = 1; i < N; i++)
    {
      az[i] = atan2(y[i],x[i]);
      el[i] = acos(z[i] / sqrt(pow(x[i],2)+pow(y[i],2)+pow(z[i],2)));
     
      if(decisionVector[i] == 1 && i>lowBin && i<highBin) {
        
        AZr = AZr + cos(az[i]);
        AZi = AZi + sin(az[i]);
        
        ELr = ELr + cos(el[i]);
        ELi = ELi + sin(el[i]);  
        
        sumN = sumN + 1;
      }      
    }
	
    AZ = atan2(AZi/(sumN/2),AZr/(sumN/2));
    EL = atan2(ELi/(sumN/2),ELr/(sumN/2));
    az[0] = AZ;
    el[0] = EL;
    /* post("Circular mean is %f", AZ*180.0/PI); */ 
}

t_int *calcIntensityVectors_tilde_perform(t_int *w)
{

  t_calcIntensityVectors_tilde *x  = (t_calcIntensityVectors_tilde *)(w[1]);
  t_sample  *r1 = (t_sample *)(w[2]);
  t_sample  *i1 = (t_sample *)(w[3]);
  t_sample  *r2 = (t_sample *)(w[4]);
  t_sample  *i2 = (t_sample *)(w[5]);
  t_sample  *r3 = (t_sample *)(w[6]);
  t_sample  *i3 = (t_sample *)(w[7]);  
  t_sample  *r4 = (t_sample *)(w[8]);
  t_sample  *i4 = (t_sample *)(w[9]);
  t_sample  *r5 = (t_sample *)(w[10]);
  t_sample  *i5 = (t_sample *)(w[11]);
  t_sample  *r6 = (t_sample *)(w[12]);
  t_sample  *i6 = (t_sample *)(w[13]);
  
  /* decisionVector input */
  t_sample  *decisionIn = (t_sample *)(w[14]);
    
    
  t_sample  *outaz = (t_sample *)(w[15]);
  t_sample  *outel = (t_sample *)(w[16]);
  int        n   = (int)(w[17]);
  
  double input1[n];
  double xr[n*6];
  double xi[n*6];
  double I[n*3];
  double az[n], el[n];

  double outAZ, outEL;

  double xc[n], yc[n], zc[n];
  double decisionVector[n];

  int i;

  for (i = 0;i < n;i++)
  { 
	/* input */
	xr[0*n + i] = (double) r1[i];
	xr[1*n + i] = (double) r2[i];
  xr[2*n + i] = (double) r3[i];
	xr[3*n + i] = (double) r4[i];
	xr[4*n + i] = (double) r5[i];
	xr[5*n + i] = (double) r6[i];
	xi[0*n + i] = (double) i1[i];
	xi[1*n + i] = (double) i2[i];
	xi[2*n + i] = (double) i3[i];
	xi[3*n + i] = (double) i4[i];
	xi[4*n + i] = (double) i5[i];
	xi[5*n + i] = (double) i6[i];
	
	decisionVector[i] = (double) decisionIn[i];
		
	/* output */
	I[0*n + i] = 0;
	I[1*n + i] = 0;
	I[2*n + i] = 0;
	
	az[i] = 0;
	el[i] = 0;
  }

  calcIntensityVector(x, xr, xi, I, n);
  

  
  for(i = 0; i < n/2; i++)
  {
	
      xc[i] = I[0*n + i];
      yc[i] = I[1*n + i];
      zc[i] = I[2*n + i];
  }

  cart2pol(decisionVector, az, el, xc, yc, zc, n);

  for (i = 0;i < n;i++)
  { 
    outaz[i] = (t_sample) az[i];
    outel[i] = (t_sample) el[i];
  }

  /* out = az;
  out[0] = az[0];
  */
  outAZ = (double) az[0];
  outEL = (double) el[0];

  outlet_float(x->f_az_out, outAZ*180.0/PI);
  outlet_float(x->f_el_out, outEL*180.0/PI);

  return (w+18);

}

void calcIntensityVectors_tilde_dsp(t_calcIntensityVectors_tilde *x, t_signal **sp)
{
  dsp_add(calcIntensityVectors_tilde_perform, 17, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[4]->s_vec, sp[5]->s_vec, 
          sp[6]->s_vec, sp[7]->s_vec, sp[8]->s_vec, sp[9]->s_vec, sp[10]->s_vec, sp[11]->s_vec, 
          sp[12]->s_vec, sp[13]->s_vec, sp[14]->s_vec, sp[0]->s_n);

}

void *calcIntensityVectors_tilde_new(t_symbol *s, int argc, t_atom *argv) /*t_floatarg f)*/
{
  t_calcIntensityVectors_tilde *x = (t_calcIntensityVectors_tilde *)pd_new(calcIntensityVectors_tilde_class);

  x->FootballFlag = 1;
  x->dist = 0.01;
  if(argc>1) {
    x->FootballFlag = atom_getint(argv+1);
    x->dist = atom_getfloat(argv);
  }
  
  if(argc==1) {  
    x->dist = atom_getfloat(argv);
  }  

  if(x->FootballFlag > 0) {
     post("Using microphone order for football [+x -y -x +y]");
   }
  else {
    post("Using microphone order for GRAS [+x -x +y -y]");
  }
  
  /* x->f_calcIntensityVectors = f; */
  
  post("distance from argument %f", x->dist);
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);    

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal); /* decisionIn vector*/

  /* floatinlet_new(&x->x_obj, &x->f_calcIntensityVectors); */

  outlet_new(&x->x_obj, &s_signal); /* original outlet */
  outlet_new(&x->x_obj, &s_signal);
  
  x->f_az_out = outlet_new(&x->x_obj, &s_float); /* outlets the float azimuth*/
  x->f_el_out = outlet_new(&x->x_obj, &s_float); /* float elevation */
  
  return (void *)x;
}

void calcIntensityVectors_tilde_setup(void) {
  calcIntensityVectors_tilde_class = class_new(gensym("calcIntensityVectors~"),
        (t_newmethod)calcIntensityVectors_tilde_new,
        0, sizeof(t_calcIntensityVectors_tilde),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addmethod(calcIntensityVectors_tilde_class,
        (t_method)calcIntensityVectors_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(calcIntensityVectors_tilde_class, t_calcIntensityVectors_tilde, f);
}

