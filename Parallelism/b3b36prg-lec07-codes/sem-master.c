#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
   struct sembuf sem[2];
   int id = semget(1000, 1, IPC_CREAT | 0666);
   int r;
   if (id != -1) {
      r = semctl(id, 0, SETVAL, 0) == 0;
      sem[0].sem_num = 0;
      sem[0].sem_op = -2;
      sem[1].sem_num = 0;
      sem[1].sem_op = 2;
      sem[0].sem_flg = 0;
      sem[1].sem_flg = 0;
      printf("Semaphore value is %d\n", semctl(id, 0, GETVAL, 0));
      printf("Wait for semvalue >= 2, two processes\n"); 
      r = semop(id, sem, 2);
      printf("allocate Semaphore value is %d\n", semctl(id, 0, GETVAL, 0));
      printf("Press ENTER to set semaphore to 0\n");
      getchar();
      r = semctl(id, 0, SETVAL, 0) == 0;
      r = semctl(id, 0, IPC_RMID, 0) == 0; //remove semaphore from system 
   }
   return 0;
}
