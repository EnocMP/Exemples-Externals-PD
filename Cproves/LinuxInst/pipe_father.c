#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>

//the father process writes random numbers into the pipe
//fd[0] = input side
//fd[1] = output side

const char *path =  "/home/enoc/projecte/Cproves/LinuxInst/son";

void main () {
    int fd[2], i;
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
      close(fd[1]);
      wait(child_PID, NULL);
    }
    
    else { //son
    char fd0[10];
    char fd1[10];
    sprintf(fd0, "%d", fd[0]);
    sprintf(fd1, "%d", fd[1]);
    
      execl("/home/enoc/projecte/Cproves/LinuxInst/son",
	    "/home/enoc/projecte/Cproves/LinuxInst/son",
	    fd0, fd1, (char *) NULL);
      printf("cagada\n");
  }
}
