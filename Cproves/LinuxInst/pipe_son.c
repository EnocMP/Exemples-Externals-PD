#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>

//the father process writes random numbers into the pipe
//fd[0] = input side
//fd[1] = output side
  int i=0;  

void main (int argc, char *argv[] ) {
  printf("Son\n");  

  int fd[2];
  fd[0] = atoi (argv[1]);
  fd[1] = atoi (argv[2]);
  printf("argv[0]: %d, argv[1]: %d\n", fd[0], fd[1]);
  close(fd[1]);
  char readbuffer;
  int nbytes;
  int j;

 for (i=0; i< 10; i++) {
    nbytes = read(fd[0], &readbuffer, 4);
    printf("Received %d bytes received string %d, print j %d\n",
	   nbytes, readbuffer, i);
  } 
}