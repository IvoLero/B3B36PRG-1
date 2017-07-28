#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/wait.h>

#define NUMPROCS 4

void compute(int myid, int n);

int main(int argc, char *argv[])
{
   int n = argc > 1 ? atoi(argv[1]) : 10000;
   for (int i = 0; i < NUMPROCS; ++i) {
      pid_t pid = fork();
      if (pid == 0) {
         compute(i, n);
         exit(0);
      } else {
         printf("Main -- new child: %d has been created\n", pid);
      }
   }
   printf("All processes created\n");
   for (int i = 0; i < NUMPROCS; ++i) {
      int r;
      pid_t pid = wait(&r);
      printf("Wait for pid %d return: %d\n", pid, r);
   }
   return 0;
}

void compute(int myid, int n) 
{
   int a = 0;
   printf("Process myid %d start computing\n", myid);
   for (int i = 0; i < n; ++i) {
      for (int j = 0; j < n; ++j) {
         a += 1;
      }
   }
   printf("Process myid %d finished\n", myid);
}
