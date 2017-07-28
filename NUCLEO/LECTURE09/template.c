#include <stdio.h>
#include <stdbool.h>

#include <unistd.h>
#include <pthread.h>

void* thread1(void*);
void* thread2(void*);

int counter;
bool quit;
pthread_mutex_t mtx;
pthread_cond_t condvar;

int main(void)
{
   counter = 0;
   pthread_mutex_init(&mtx, NULL);
   pthread_cond_init(&condvar, NULL);
   quit= false;
   pthread_t threads [2];
   pthread_create(&threads[0], NULL, thread1,NULL);
   pthread_create(&threads[1], NULL, thread2,NULL);


   getchar();
   pthread_mutex_lock(&mtx);
   quit = true;
   pthread_mutex_unlock(&mtx);
   for (int i = 0; i < 2; ++i) {
   	pthread_join (threads[i], NULL);
   }
   return 0;
}

void* thread1(void *v)
{
   bool q = false;
   while (!q) {
      usleep(100 * 1000);

      pthread_mutex_lock(&mtx);
      counter += 1;
      pthread_cond_signal(&condvar);
      //printf("Second thread works\n");
      q = quit;
      pthread_mutex_unlock(&mtx);
   }
   return 0;
}

void* thread2(void *v) 
{
      printf("\rCounter %10i", counter);
   pthread_mutex_unlock(&mtx);
   
   
   bool q = false;
   while (!q) {

      //pthread_mutex_lock(&mtx);
      pthread_mutex_lock (&mtx);
      pthread_cond_wait(&condvar, &mtx);
      printf("\rCounter %10i", counter);
      fflush(stdout);
      q = quit;
      pthread_mutex_unlock(&mtx);
   }
   return 0;
}
