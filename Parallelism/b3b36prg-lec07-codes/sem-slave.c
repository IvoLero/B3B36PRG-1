#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   struct sembuf sem;
   int id = semget(1000, 1, 0);
   int r;
   if (id != -1) {
      sem.sem_num = 0;
      sem.sem_op = 1;
      sem.sem_flg = 0;
      printf("Increase semafore value (add resource)\n");
      r = semop(id, &sem, 1);
      sem.sem_op = 0;
      printf("Semaphore value is %d\n", semctl(id, 0, GETVAL, 0));
      printf("Wait for semaphore value 0\n");
      r = semop(id, &sem, 1);
      printf("Done\n");
   }
   return 0;
}
