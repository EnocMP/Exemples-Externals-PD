#include "lib.h"
#include <string.h>
#include <stdio.h>

void main (){
	printf("Introdueix num 1: \n");
	char a = getchar();
	getchar();
	printf("has introduït el num %d\n", a);

	printf("Introdueix num 2: \n");
	char b = getchar();
	getchar();
	printf("has introduït el num %d\n", b);
	int r= a - b;
	printf("el resultat de la operacio es %d \n", r);
	if (r) {
		printf("he entrat\n");
	}
	else {
		printf("No he entrat\n");
	}
}
