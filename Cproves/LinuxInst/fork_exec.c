#include<stdio.h>
#include<unistd.h>


void main () {

    int father_pid = getpid();
    fork();
    if (father_pid == getpid()) {
      printf("Soc el pare i tincn PID: %d\n", getpid());
    }
    else {
      execl("/home/enoc/projecte/Cproves/LinuxInst/exec","/home/enoc/projecte/Cproves/LinuxInst/exec", (char *) NULL);
      printf("cagada pastoret");
    }
  
}