#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>

//the father process writes random numbers into the pipe
//fd[0] = input side
//fd[1] = output side




void main () {
    int fd[2], i; //
    int father_pid = getpid();
    pipe(fd);
    int child_PID=fork();
    
    if (father_pid == getpid()) {
      printf("Soc el pare i tincn PID: %d\n", getpid());
      close(fd[0]);
      for (i=0; i< 10; i++) {	
	int ret=write(fd[1], &i, sizeof(int));
	printf("retorna %d\n", ret);
      }
      wait(child_PID);
    }
    else { //son
      printf("soc el fill\n");
      close(fd[1]); //closes the input end
      char readbuffer;
      int nbytes;
      int j;
      for ( j = 0; j< 10; j++) {
	nbytes=read(fd[0], &readbuffer, sizeof(int));
	printf("Received %d bytes received string %d, print i %d\n", nbytes, readbuffer, j);
      }    
      close(fd[0]);
  }
}
