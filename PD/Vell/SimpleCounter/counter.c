#include "m_pd.h"


//CLASS
static t_class *counter_class;

//DATA SPACE
typedef struct _counter {
	t_object x_obj; //object properties for PD environmnet
	t_int i_count;
} t_counter;

//BANG METHOD
void counter_bang(t_counter *x) {
	t_float f = x -> i_count;
	x -> i_count++;
	outlet_float(x -> x_obj.ob_outlet, f);
}

//CONSTRUCTOR
void *counter_new(t_floatarg f) {
	t_counter *x = (t_counter *)pd_new(counter_class);
	x -> i_count=f;
	outlet_new(&x-> x_obj, &s_float);
	return (void *)x;
}

//SETUP
void counter_setup(void) {
	counter_class=class_new(gensym("counter"), 
	(t_newmethod)counter_new,
	0, sizeof(t_counter),
	CLASS_DEFAULT, 
	A_DEFFLOAT, 0);
	class_addbang(counter_class, counter_bang);
}
