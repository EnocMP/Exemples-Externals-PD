#include<stdio.h>

void main (){
	int a = 3;
	int *b = a;
	int **c = &b;
	int ***d = &c;

	int pb = *b;
	printf("la variable a = %d\n", a);
	printf("la posicio de memòria b val %d, la variable *b val %d\n", b, *b);
	
}
