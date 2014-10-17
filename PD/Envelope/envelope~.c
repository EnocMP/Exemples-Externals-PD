/**************************envelope EXTERNAL*********************************
This external implements a simple linear ADSR envelope. 

INPUTS:
	1 - Signal input to be multiplied by the envelope
	2 - Attack time in ms
	3 - Decay time in ms
	4 - Sustain (from 0 to 1)
	5 - Release time in ms
	6 - Key trigger (1 to start the attack, 0 to start the release)


***********************************************************************/

#include "m_pd.h"

/***************************CLASS DECLARATION***************************/

static t_class *envelope_tilde_class; //create envelope_tilde_class

/***************************DATA SPACE***************************/
//variables of the class
typedef struct _envelope_tilde {
	t_object x_obj; //object properties for PD environment
			
	t_sample f; //t_sample type is an audio signal value (floating point)

	t_float attack_time; //attack time (ms)
	t_float decay_time; // release time (ms)
	t_float sustain; // sustain value (0 to 1)
	t_float release_time; // release time (ms)

	t_float attack, release,decay; //increment or decrement values to be used

	t_float envelope_value; //envelope current value

	t_int state; 	//This variable contains the state of the envelope
		     	// 0 - non-active (waiting for a key)
			// 1 - attack
			// 2 - decay
			// 3 - sustain
			// 4 - release
			
	t_inlet *in2; //pointer to an inlet
	t_inlet *in3; //pointer to an inlet
	t_inlet *in4; //pointer to an inlet
	t_inlet *in5; //pointer to an inlet
	t_inlet *in6; //pointer to an inlet

	t_outlet *out; //pointer to an output

} 
t_envelope_tilde;

/***************************METHODS***************************/

//This method starts the attack or the release phases of the envelope	
void envelope_tilde_key (t_envelope_tilde *x, t_floatarg f) {

	if (f) {
		post("key pressed");
		x -> state = 1;//start the envelope
	
		x-> attack = 1000/(x->attack_time*44100); //attack increment (assuming fs 44100 Hz)
		post (" attack value %f", x-> attack);
		x-> decay = x->sustain*1000/(x->decay_time*44100); //attack increment (assuming fs 44100 Hz)
		post (" decay value %f", x-> attack);
	}

	else {
		post("key released");
		x -> state = 4;//start the envelope
	
		x->  release = 1000/(x->release_time*44100); // release time
		post (" release value %f", x->  release);
	}
	

}


t_int *envelope_tilde_perform(t_int *w) {
/*The perform routine contains the functionality of the external*/

	t_envelope_tilde *x = (t_envelope_tilde *)(w[1]); //the first argument is the object
	t_sample *in1 = (t_sample *)(w[2]); 	//cast to the first signal inlet
	t_sample *out = (t_sample *)(w[3]); 	//cast to the first output
	int n= (int)(w[4]); 	    		//cast to the length of the signal vector


	t_sample envelope = x-> envelope_value;//get the current value of the envelope
	
	
	while (n--) {//executes n times where n is the signal vector size

		switch(x->state) //depending on the stage of the envelope
		{
			case 1: //attack
				envelope += + x->attack;
				if (envelope >=1.0) {
					envelope = 1.0;
					x->state++; //switch to the decay mode;
				}
				break;

			case 2: //decay
				envelope -= + x->decay;
				if (envelope <= x->sustain) {
					envelope = x->sustain;
					x->state++; //switch to the sustain mode;
				}
				break;

			case 3: //sustain
				envelope=x->sustain;
				break;

			case 4: //release
				envelope -= x->release;
				if (envelope <=0.0) {
					envelope = 0.0;
					x->state=0; //switch to the sustain mode;
				}
				break;

			default: //non-active state
				envelope=0.0;
				break;
		}
		*out++ = (*in1++)*envelope; //outputs the current value
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

/***************************DESTRUCTOR***************************/
void envelope_tilde_free(t_envelope_tilde *x) {
	inlet_free(x->in2); //free the inlet x_in2
	inlet_free(x->in3); //free the inlet x_in2
	inlet_free(x->in4); //free the inlet x_in2
	inlet_free(x->in5); //free the inlet x_in2
	outlet_free(x->out); //free the output x_out*/
}

/*As the first inlet is generated automatically it is also freed automatically, so
no inlet_free is needed*/

/***************************CONSTRUCTOR***************************/

void *envelope_tilde_new()  
{
//Initialization of all variables
	t_envelope_tilde *x = //pointer *x to a t_envelope_tilde object
		(t_envelope_tilde *) pd_new(envelope_tilde_class); 	// creates (casts) a new pointer to a t_envelope_tilde


	x-> state = 0; // non-active stat
	x->  envelope_value=0; //envelope current value

	t_inlet *x_in2; //pointer to an inlet
	t_outlet *x_out; //pointer to an output
	
	x->in2 = floatinlet_new( // generates a new passive inlet for numerical values (3) 
		&x->x_obj, //pointer to the internal object
		&x->attack_time); //address of the data space-memory where other objects can write too
	x->in3 =floatinlet_new(&x->x_obj, &x->decay_time);
	x->in4 =floatinlet_new(&x->x_obj, &x->sustain); 
	x->in5 =floatinlet_new(&x->x_obj, &x->release_time); 

	x->in6 =inlet_new(		//creates a new "active inlet"(1)
		&x->x_obj,	//pointer to the internal object (necessary)
		&x->x_obj.ob_pd, //pointer to the graphical representation
		gensym("float"), //Symbolic selector to be substituted (2)
		gensym("key"));//substitute selector

	//writes  the input data from inlet 3

	x->out = outlet_new(
		&x->x_obj, //pointer to the internal object
		&s_signal);//selector "signal"
	return (void *)x;
}

/*------------------------------------INLET GENERATION--------------------------------------------------
1- active means that a class method is called each time a message is sent to an active inlet. The first
inlet is always active. 
2- the selector defined with the third argument is to be substituted with the fourth argument selector.
Because of this substitution of selectors a message on a certain right inlet (a float variable in this case) 
can be treated as a message with a certain selector on the leftmost inlet. This means:
	* The substituting selector has to be declared by class_method in the setup-routine
	* It is possible to simulate a certain right inlet, by sending a message with this inlet's selector
	  to the leftmost inlet
	* It is not possible to add methods for more than one selector to a right inlet. Particularly it is
	  not possible to add a universal method for arbitrary selectors to a right inlet.
3 - Passive inlets allow parts of the data space-memory to be written directly from the outside. 
--------------------------------------------------------------------------------------------------*/

/***************************SETUP***************************/
void envelope_tilde_setup(void) {
	envelope_tilde_class=class_new	//Create the new class
		(gensym("envelope~"),		//symbolic name of the class
		(t_newmethod)envelope_tilde_new,//constructor (called when PD instantiates a new object)
		(t_method)envelope_tilde_free, //destructor (needed to free the resources)
		sizeof(t_envelope_tilde),	//logic space of the object
		CLASS_DEFAULT, 		//graphical representation of the object		
		0); 			//0 indicate the ending of the list of arguments
	
	class_addmethod(		//new method 
		envelope_tilde_class, 	//belonging class
		(t_method)envelope_tilde_dsp,//defines the method
		gensym("dsp"),		//defining the method for dsp (1)
		0); 	

	class_addmethod(		//new method 
		envelope_tilde_class, 		//belonging class
		(t_method)envelope_tilde_key,//defines the method
		gensym("key"),	//symbolic name (message that triggers the method)
		A_DEFFLOAT,
		0); 



		//adding the method reset

//add class for key_pressed and key release

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
