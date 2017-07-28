#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define SIZE 512

int main(int argc, char *argv[])
{
   char *buf;
   int id;
   if ((id = shmget(1000, SIZE, IPC_CREAT | 0666)) != -1) {
      if ( (buf = (char*)shmat(id, 0, 0)) ) {
         fgets(buf, SIZE, stdin);
         shmdt(buf);
      }
   }
   return 0;
}
