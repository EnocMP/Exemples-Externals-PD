#include<stdio.h>
#include<unistd.h>


void main () {

    int father_pid = getpid();
    fork();
    if (father_pid == getpid()) {
      printf("Soc el pare i tincn PID: %d\n", getpid());
    }
    else {
      printf("Soc el fill i tinc PID %d\n", getpid());
    }
  
}

