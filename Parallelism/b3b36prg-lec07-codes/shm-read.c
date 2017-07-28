#include <sys/types.h>
#include <sys/shm.h>
#include <stdio.h>

#define SIZE 512

int main(int argc, char *argv[])
{
   int id;
   char *buf;
   if ((id = shmget(1000, 512, 0)) != -1) {
      if ((buf = (char*)shmat(id, 0, 0)) ) {
         printf("mem:%s\n", buf);
      }
      shmdt(buf);
   } else {
      fprintf(stderr, "Cannot access to shared memory!\n");
   }
   return 0;
}
