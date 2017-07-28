#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "prg_serial_nonblock.h"
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#define NUMOFTHREADS 4
#define initialperiod 5


typedef struct {
	int fd;
	int counter;
	bool quit;
	int period;
	char* ledstate;
	char lastsent;
	char lastreceived;
} data;


void* receiver(void*);
void* sender (void*);
void* printingoutput(void*);
void* periodcounter (void*);

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


int main(int argc, char *argv[]){
	int ret = 0;
	const char *serial = argc > 1 ? argv[1] : "/dev/cuaU0"; 
	fprintf(stderr, "INFO: open serial port at %s\n", serial);
	int fd = serial_open(serial);

	if (fd == -1) {
	 	fprintf(stderr, "ERROR: Cannot open device %s\n", serial);
		return -1;
	}


	set_raw(true);
	void* (*functptr[]) (void*) ={receiver,sender,printingoutput, periodcounter};
	pthread_t threads [NUMOFTHREADS];
	char* ls = (char*) malloc(sizeof(char) *3);
	ls[0] = '-'; ls[1] = '-'; ls[2] = '-';
	data state = {.fd = fd, .counter =0, .quit =false, .period = 0.1, .ledstate = ls, .lastsent ='?', .lastreceived = '?'};
	pthread_mutex_init(&mtx, NULL);
	pthread_cond_init(&condvar,NULL);
	
	for (int i =0; i< NUMOFTHREADS; ++i){
   		pthread_create(&threads[i], NULL, functptr[i],&state);
  	}
   	for (int i =0; i< NUMOFTHREADS; ++i){
   		pthread_join (threads[i], NULL);
   	}
	free(ls);
   	set_raw(false);
   	return 0;
}
	

void* sender(void* dat){
		
	data* d = (data*) dat;
	int fd = d->fd;
	 _Bool quit = false;

	 while (!quit) {
	 	
		char c;
		int response = serial_getc_timeout(0, 1000, &c);


		 pthread_mutex_lock (&mtx);
	    if (response != 0){

		 _Bool write = true;
	 	 switch (c) {

	 	 		case 'b':
	 	 			//printf("\n\rProgram was terminated!!!");
	 	 			//quit = true;
					//d->quit = true;
					d->lastsent = 'b';
					d->lastreceived = '?';
	 	 			break;
	 	 		case 's':
	 	 			//fprintf(stderr,"\n\rSend 's' - LED on\n");
					d->lastsent='s';
					d->lastreceived = '?';
	 	 			break;
	 	 		case 'e':
	 	 			//fprintf(stderr,"\n\rSend 'e' - LED off\n");
					d->lastsent='e';
					d->lastreceived = '?';
	 	 			break;
				case 'h':
					//fprintf(stderr,"\n\rSend 'h' - greeting\n");
					d->lastsent= 'h';
					d->lastreceived = '?';
					break;
				case '1' ...'5': 
					//fprintf(stderr,"\n\rChanging the period\n");
					d->lastsent = c;
					d->lastreceived = '?';
					break;
				case 'x':
					exit(0);
	 	 		default: 
					write=false;
	 	 			break;
	 	 } // end switch c

	 	 if (write){
	 
	 		if (serial_putc(fd,c) != -1) 
				//sends message that smth smth should be received
				pthread_cond_signal(&condvar);
			else{

	 	 		fprintf(stderr, "rDEBUG: Error in writing to serial!");
			}
	 	 } 
		 else 
	 	 	fprintf(stderr, "\rERROR: Error in sending characters");
	}

	quit = d->quit;
	pthread_mutex_unlock(&mtx);

	 } // end while()
	serial_close(fd);
	return 0;
}

void* receiver (void* dat){
	//char fname [] = "/dev/ttyACM0";

	data* d = (data*) dat;
	int fd = d->fd;
	 _Bool quit = false;

	 while (!quit) {
	 	char c;
		int response = 1;
		response = serial_getc_timeout(fd, 1000, &c);
		//c= serial_getc(fd);
		//fprintf(stderr,"\n\routput: %c %d\n", c,response);
		
		pthread_mutex_lock(&mtx);
		
		if (response != 0){	
			_Bool output = true;
			
			switch(c){
				case 'a':
					if (d->lastsent == 's'){
						strcpy(d->ledstate, "on");
					}
					else if (d->lastsent =='e'){
						strcpy(d->ledstate, "off");
					}
					else if (d->lastsent >='1' && d->lastsent <='5'){
						d->period = -1; 
					}
					break;
				case 'h':
					break;
				case 'b':
					//usleep(5000000);
					d->quit = true;
					fprintf(stderr,"\n\r==========================\n\rNucleo quit\n\r==========================\n\r");
					//exit(0);
					break;
				case 'i':
					fprintf(stderr,"\n\r==========================\n\rNucleo is initialized\n\r==========================\n\r");
					break;
				case 'x':
					d->counter++;
					break;
				case 'o':
					break;
				default:
					output=false;
					break;
			}
			 //should output???
			if (output){
				d->lastreceived = c;
				pthread_cond_signal(&condvar);
			}
			
			else 
				fprintf(stderr, "\rERROR: Error in received responses");	
			

	 	}
		quit= d->quit;
		pthread_mutex_unlock(&mtx);
	}
}

void* printingoutput (void*dat){
	
	data* d = (data*) dat;
	 _Bool quit = false;

	 while (!quit) {
		pthread_mutex_lock(&mtx);
		pthread_cond_wait(&condvar,&mtx);
		quit= d->quit;
		if (!quit){
			fprintf(stderr,"\rLED %3s send: '%c' received: '%c', T = %4d ms, ticker = %4d	input: ", d->ledstate, d->lastsent, d->lastreceived,d-> period, d->counter);
			fflush(stdout);
		}
		pthread_mutex_unlock(&mtx);
	}
}
void* periodcounter(void*dat){
	data*d = (data*) dat;
	_Bool quit = false;
	
	while (!quit){
		usleep (initialperiod *1000*1000);
		pthread_mutex_lock(&mtx);
		quit = d->quit;	
		if (!quit){
			d->period = (d->counter!=0)? 500 *initialperiod/d->counter:0;
			d -> counter = 0;
			pthread_cond_signal(&condvar);
		}
		pthread_mutex_unlock(&mtx);
	}
}
	

		



		



