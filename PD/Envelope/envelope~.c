/**************************envelope EXTERNAL*********************************
This external has two signal inputs and a single float input. The signal
inputs are mixed according to the third inlet (mixing factor).
***********************************************************************/

#include "m_pd.h"

/***************************CLASS DECLARATION***************************/

static t_class *envelope_tilde_class; //create envelope_tilde_class

/***************************DATA SPACE***************************/
//variables of the class
typedef struct _envelope_tilde {
	t_object x_obj; //object properties for PD environment

			//t_sample type is an audio signal value (floating point)
	t_sample f;

	t_float attack; //attack time
	t_float release; // release time

	t_float envelope_value; //envelope current value

	t_int rise; //rising flag
	t_int fall; //falling flag

	t_inlet *x_in2; //pointer to an inlet

	t_outlet *x_out; //pointer to an output

} 
t_envelope_tilde;

/***************************METHODS***************************/

void envelope_tilde_bang (t_envelope_tilde *x) {
	post("Bang received");
	x->rise = 1;
}

/*The perform routine contains the functionality of the external*/
t_int *envelope_tilde_perform(t_int *w) {
	t_envelope_tilde *x = (t_envelope_tilde *)(w[1]); //the first 
	t_sample *in1 = (t_sample *)(w[2]); 	//cast to the first signal inlet
	t_sample *out = (t_sample *)(w[3]); 	//cast to the first output
	int n= (int)(w[4]); 	    		//cast to the length of the signal vector

	//t_sample f_envelope = (x->f_envelope<0)?0.0:(x->f_envelope>1)?1.0:x->f_envelope; //keep f_envelope between 0.0 <-> 1.0
	t_sample envelope = x-> envelope_value;
	
	
	while (n--) {//executes n times

		if (x->rise) {
			if (envelope==0) post("start rising");

			envelope =envelope + x->attack;
			
			if (envelope >= 1) {
				x->rise = 0;
				x->fall =1;
				envelope = 1;
				post("start falling");		
			}
	
		}

		else if (x->fall){
			envelope += -x->release;
			if (envelope<=0) {
			envelope=0;
			post("steady state");
			x->fall=0;
			}
		}
		*out++ = (*in1++)*envelope;
		/*post("valor de la sortida %f", out);
		post ("memoria de la sortida %f", *out);			*/
	}


	x->envelope_value = envelope; //actualize the envelope value field in x
	return (w+5); //return the next memory position
}

/*The envelope_tilde_dsp method adds a perform-routine to the DSP-tree.*/
void envelope_tilde_dsp(
	t_envelope_tilde *x, //pointer to the class data space
	t_signal **sp) { //pointer to the sp array (1)

	dsp_add(
		envelope_tilde_perform,//performing routine
		4,		//number of the following pointers to variables passed
				//to the envelope_tilde_perform routine
		x,		//pointer to the data space 
		sp[0]->s_vec, 	//first in-signal (2)
		sp[1]->s_vec, 	//out-signal
		sp[0]->s_n);	//pointer to the length of the signal (3)

}

/*------------------------SP, S_VEC AND S_N----------------------------
1- SP:The vector sp is an array of signals containing pointers to each 
signal in object. The signals are read in the graphical representation 
clockwisely. Each signal sp[n] has a pointer ().s_vec (array of t_samples)
and the length of this signal-vector ().s_n

2-S_VEC: pointer to the signal vector (array of sample of type t_sample)

3-S_N: length of the signal vector
---------------------------------------------------------------*/

/***************************CONSTRUCTOR***************************/
void envelope_tilde_free(t_envelope_tilde *x) {
	inlet_free(x->x_in2); //free the inlet x_in2
	outlet_free(x->x_out); //free the output x_out*/
}

/*As the first inlet is generated automatically it is also freed automatically, so
no inlet_free is needed*/

/***************************CONSTRUCTOR***************************/

void *envelope_tilde_new(t_floatarg rise_time, t_float fall_time)  
{
//Initialization of all variables


			//t_sample type is an audio signal value (floating point)

	t_envelope_tilde *x = //pointer *x to a t_envelope_tilde object
		(t_envelope_tilde *) pd_new(envelope_tilde_class); 	// creates (casts) a new pointer to a t_envelope_tilde


	x-> attack = 1000/(rise_time*44100); //attack increment (assuming fs 44100 Hz)
	post (" rising value %f", x-> attack);
	x->  release = 1000/(rise_time*44100); // release time
	post (" falling value %f", x->  release);
	x->  envelope_value=0; //envelope current value

	t_int rise=0; //rising flag

	t_int fall=0; //falling flag


	t_inlet *x_in2; //pointer to an inlet
	t_outlet *x_out; //pointer to an output
	
	
	//also symbolic selector "signal"

	x-> x_in2 = inlet_new(	// new inlet
		&x->x_obj,	//pointer to the internal object (necessary)
		&x->x_obj.ob_pd,//pointer to the graphical representation
		&s_bang, 	//reference to the symbolic selector "bang"
		&s_bang); 	//also symbolic selector "bang"
	//writes  the input data from inlet 3

	x-> x_out = outlet_new(
		&x->x_obj, //pointer to the internal object
		&s_signal);//selector "signal"
	return (void *)x;
}

/***************************SETUP***************************/
void envelope_tilde_setup(void) {
	envelope_tilde_class=class_new	//Create the new class
		(gensym("envelope~"),		//symbolic name of the class
		(t_newmethod)envelope_tilde_new,//constructor (called when PD instantiates a new object)
		(t_method)envelope_tilde_free, //destructor (needed to free the resources)
		sizeof(t_envelope_tilde),	//logic space of the object
		CLASS_DEFAULT, 		//graphical representation of the object
		A_DEFFLOAT, 		//float input 1
		A_DEFFLOAT, 		//float input 2
		0); 			//0 indicate the ending of the list of arguments
	class_addbang(		//new method 
		envelope_tilde_class, 	//belonging class
		envelope_tilde_bang); 	//adding the bang method
	
	class_addmethod(		//new method 
		envelope_tilde_class, 	//belonging class
		(t_method)envelope_tilde_dsp,//defines the method
		gensym("dsp"),		//defining the method for dsp (1)
		0); 			//adding the method reset

	CLASS_MAINSIGNALIN(	//(2)
		envelope_tilde_class,//pointer to the signal class
		t_envelope_tilde, 	//pointer to the class's data space
		f);		//(3)this is a dummy-variable out of the data space
				
//class_sethelpsymbol(envelope_tilde_class, gensym("help-envelope_tilde")); //help location for the envelope_tilde class
}
/*----------------------DSP INITIALIZATION------------------------------------------------
 1- When PD audio generator is started a message with the selector "dsp" is sent to 
    each object. Each class that has a method for the dsp message is recognized as a
    signal class. 
 2- signal classes that want to provide signal-inlets have to declare this via the 
    CLASS_MAINSIGNALIN macro. This enables signals at the first (default) inlet. If 
    more than one signal-inlet is needed, they have to be created explicitly in the
    constructor method. Inlets that are declared as signal-inlets cannot provide methods
    for float messages any longer.
 3- The last argument of CLASS_MAINSIGNALIN is a dummy variable out of the data space that
    is needed to replace non-existing signal at the signal-inlet(s) with t_float messages
-----------------------------------------------------------------------------------------*/
