#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <pthread.h>

#define NUMOFTHREADS 3
#define initialperiod 100 

typedef struct {
	int counter;
	bool quit;
	int period;
} data;



void* myalarm(void*);
void* output(void*);
void* input(void*);

pthread_mutex_t mtx;
pthread_cond_t condvar;

void set_raw(_Bool set)
{
   if (set) {
      system("stty raw");  // enable raw
   } else {
      system("stty -raw"); // disable raw
   }
}

int main(int argc, char**argv)
{
   set_raw(true);
   data mydata = {.counter = 0, .quit = false, .period = (argc>1)? strtol(argv[1], NULL, 10): initialperiod};
   pthread_mutex_init(&mtx, NULL);
   pthread_cond_init(&condvar, NULL);

   pthread_t threads [NUMOFTHREADS];
   void* (*functpointers[]) (void*)= {myalarm,output,input};
   //navratova hodnota funkce je void - pole funkci na void
   // druha hvezdicka pointer na funkci
   


   for (int i =0; i< NUMOFTHREADS; ++i){
   	pthread_create(&threads[i], NULL, functpointers[i],&mydata);
   }


   for (int i =0; i< NUMOFTHREADS; ++i){
   	pthread_join (threads[i], NULL);
   }

   set_raw(false);
   return 0;
}

void* myalarm(void *v)

{
   bool q = false; 
   int p;
   data* mydata = (data*) v;

   while (!q) {
      pthread_mutex_lock (&mtx);
      p = mydata->period;
      pthread_mutex_unlock(&mtx);

      usleep(p * 1000);

      pthread_mutex_lock(&mtx);
      mydata -> counter += 1;
      pthread_cond_signal(&condvar);
      //printf("Second thread works\n");
      q = mydata->quit;
      pthread_mutex_unlock(&mtx);
   }
   return 0;
}

void* input(void *v) 
{
   
   data* mydata = (data*) v;
   bool q = false;

   while (!q){
    	char c = getchar();

	pthread_mutex_lock (&mtx);
	switch (c){
		case 'q':
			q = true;
			mydata->quit = true;
			break;
		case 'r':
			mydata->period = (mydata-> period -10 >= 10)? mydata->period-10:10;
			mydata->counter=0;
			break;
		case 'p':
			mydata->period = (mydata-> period +10 <= 2000) ? mydata->period+10:2000;
			mydata->counter=0;
			break;
		default:
			printf("NOT supported");
		}
	pthread_mutex_unlock(&mtx);
	}	
			
     return 0;
}

void* output(void *v){
	
   bool q = false; 
   data* mydata = (data*) v;

   while (!q) {

      pthread_mutex_lock (&mtx);
      pthread_cond_wait(&condvar, &mtx); 
      
            printf("\rCounter %10d Period %10d ms INPUT:  ", mydata->counter, mydata->period);
	    fflush(stdout);

      q = mydata-> quit;
      pthread_mutex_unlock(&mtx);

   }
    return 0;
}
