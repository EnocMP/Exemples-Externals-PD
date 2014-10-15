#include "m_pd.h"

/***************************CLASS DECLARATION***************************/

static t_class *counter_class; //create counter class


/***************************DATA SPACE***************************/
//variables of the class
typedef struct _counter {
	t_object x_obj; //object properties for PD environment
	t_int i_count;	//counter value
	t_float step;	//counter's step
	t_int i_down, i_up;//upper and lower limits
	t_outlet *f_out, *b_out; //outputs
} 
t_counter;


/***************************METHODS***************************/
void counter_bang (t_counter *x) {
	post("Bang received");
	t_float f = x->i_count; //f equals to the i_count field of the x object
	t_int step = x->step;
	x->i_count+=step; //increment the counter
	
	if(x->i_down - x-> i_up) { //if upper and lower limits are not equal
		if((step>0) && (x-> i_count > x-> i_up)){ //check the upper limit
			x->i_count = x->i_down;	//the value of i_count is truncated to i_down
			outlet_bang(x->b_out);	//send a bang to the b_out outlet
		}
		else if (x-> i_count < x->i_down) { //check the lower limit
			x->i_count = x->i_up;
			outlet_bang(x->b_out);
		}
	}
	outlet_float(x->f_out, f); //send the counter level
}

void counter_reset (t_counter *x) {
	post("reset received");
	x->i_count = x->i_down; //reset to the lower limit

}

void counter_set (t_counter *x, t_floatarg f) {
	post("set received");
	x -> i_count = f;
}

void counter_bound(t_counter *x, t_floatarg f1, t_floatarg f2) {
	post("bound received");
	x -> i_down= (f1<f2)?f1:f2;
	x -> i_up = (f1>f2)?f1:f2; /*Ternary operator equivalent to:  
					if(f1>f2) result=f1;
					else result=f2;*/
}


/***************************CONSTRUCTOR***************************/

void *counter_new(	//Constructor function
	t_symbol *s,	//Symbolic name used in the creation
	int argc,	//Number of arguments passed to the object 
	t_atom *argv)  //pointer to a list of argc atoms
			//if three arguments are passed these should be the lower boundary,
			//the upper boundary and the step width
{
	t_counter *x = //pointer *x to a t_counter object
	(t_counter *)pd_new(counter_class); 	// creates (casts) a new pointer to a t_counter

	t_float f1=0, 	//lower boundary				
		f2=0;	//upper boundary
	
	x->step=1; //step increment 

	/* depending on the number of inputs, the initialization of some parameters is executed:
	-if three arguments are passed these should be the lower boundary, the upper boundary and the step
	-if two arguments are passed the step-width defaults to 1
	-if one argument is passed this should be the initial value of the counter */

	switch(argc){
		default:
		case 3:
			x->step=atom_getfloat(argv+2); //define the step
		case 2:
			f2=atom_getfloat(argv+1);	//get one limit
		case 1:
			f1=atom_getfloat(argv);		//get the other limit
			break;
		case 0:
			break;
	}
	if (argc<2) {
		f2=f1; //if only one limit is sent both limits are equal
	}

	x -> i_down= (f1<f2)?f1:f2;
	x -> i_up = (f1>f2)?f1:f2; /*Ternary operator equivalent to:
					if(f1>f2) result=f1;
					else result=f2;*/

	x -> i_count = x-> i_down; //set the counter value to the lower limit

/*------------------------------------WARNING!!---------------------------------------------------
The order of the generation of inlets (inlet_new) and outlets (outlet_new is important, since it
corresponds to the order of the inlets and outlets in the graphical representation of the object!
--------------------------------------------------------------------------------------------------*/
	
	inlet_new(		//creates a new "active inlet"(1)
		&x->x_obj,	//pointer to the internal object (necessary)
		&x->x_obj.ob_pd, //pointer to the graphical representation
		gensym("list"), //Symbolic selector to be substituted (2)
		gensym("bound"));//substitute selector

//when "list" message is received, the "bound" routine is called


	floatinlet_new( // generates a new passive inlet for numerical values (3) 
		&x->x_obj, //pointer to the internal object
		&x->step); //address of the data space-memory where other objects can write too

/*------------------------------------INLET GENERATION--------------------------------------------------
1- active means that a class method is called each time a message is sent to an active inlet. The first
inlet is always active. 
2- the selector defined with the third argument is to be substituted with the fourth argument selector.
Because of this substitution of selectors a message on a certain right inlet (bound in this case) 
can be treated as a message with a certain selector on the leftmost inlet. This means:
	* The substituting selector has to be declared by class_method in the setup-routine
	* It is possible to simulate a certain right inlet, by sending a message with this inlet's selector
	  to the leftmost inlet
	* It is not possible to add methods for more than one selector to a right inlet. Particularly it is
	  not possible to add a universal method for arbitrary selectors to a right inlet.
3 - Passive inlets allow parts of the data space-memory to be written directly from the outside. 
--------------------------------------------------------------------------------------------------*/

	x->f_out = 		//object outlet field f_out
		outlet_new(	//creates a new outlet and returns a pointer to this outlet
		&x-> x_obj,	//pointer to the internal object
		 &s_float);	//defines the output data type
	x -> b_out = outlet_new(&x->x_obj, &s_bang);
	return (void *)x;
}


/***************************SETUP***************************/

void counter_setup(void) {
	counter_class=class_new	//Create the new class
		(gensym("counter"),	//symbolic name of the class
		(t_newmethod)counter_new,//constructor (called when PD instantiates a new object)
		0,			//destructor(never in this case)
		sizeof(t_counter),	//logic space of the object
		CLASS_DEFAULT, 		//graphical representation of the object
		A_GIMME, 		//Variable number of inputs
		0); 			//0 indicate the ending of the list of arguments

	class_addbang(		//new method 
		counter_class, 	//belonging class
		counter_bang); 	//adding the bang method

	class_addmethod(		//new method 
		counter_class, 		//belonging class
		(t_method)counter_reset,//defines the method
		gensym("reset"),	//symbolic name (message that triggers the method)
		0); 			//adding the method reset
	
	class_addmethod(	//new method 
		counter_class, 		//belonging class
		(t_method)counter_set,	//defines the method
		gensym("set"),		//symbolic name
		A_DEFFLOAT,		//defining first argument type
		0); 			//adding the method reset
	
	class_addmethod(		//new method 
		counter_class, 		//belonging class
		(t_method)counter_bound,//defines the method
		gensym("bound"),	//symbolic name
		A_DEFFLOAT,		//defining first argument type
		A_DEFFLOAT,		//defining second argument type
		0); 			//adding the method reset

	//class_sethelpsymbol(counter_class, gensym("help-counter")); //help location for the counter class
}

