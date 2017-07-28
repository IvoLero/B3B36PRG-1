#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
	 
#include "prg_serial.h"


void set_raw(_Bool set)
{
   if (set) {
      system("stty raw");  // enable raw
   } else {
      system("stty -raw"); // disable raw
   }
}


int main(int argc, char *argv[])
{
	const char *serial = argc > 1 ? argv[1] : "/dev/cuaU0"; // cuaU0 nahraďte příslušným rozhraním sériového portu
	fprintf(stdout, "\nINFO: open serial port at %s\n", serial);
	int fd = serial_open(serial);
	if (fd == -1) { // read from serial port
		fprintf(stderr, "Can not open the serial port!!!\n");
		return 100;
	}

	set_raw(true);
	_Bool quit = false;
	 
	while (!quit) {
	     int c = getchar();
	     _Bool putonserial =0;

		 if (c=='s'){
		 	fprintf(stderr,"\nPC: switch the LED on\n");
			putonserial=1;
		 }
		 else if (c=='e'){
		 	fprintf(stderr,"\nPC: switch the LED off\n");
			putonserial=1;
		 }
		 else if (c >='1' && c<='5'){
		 	fprintf(stderr,"\nPC: changing the period of LED\n");
			putonserial=1;
		}
		else if (c=='h'){
			fprintf(stderr,"\nPC: greeting\n");
			putonserial=1;
		}
		else if (c=='b'){
			fprintf(stderr,"\nPC: closing the program\n");
			quit=true;
			break;
		}
		
		if (putonserial){
			if (serial_putc(fd,c)==-1){
				fprintf(stderr, "\nERROR: Error in received responses\n");
	       			quit = true;
				break;
			}
			//reading response
			int r = serial_getc(fd);
			if (r!= -1){
				fprintf(stderr,"NUCLEO: Received response '%c'\n",r);
			}
			else{
				fprintf(stderr, "ERROR in received response\n");
			}
		}
		else{
			fprintf(stderr, "\nPC: Error in reading the command!\n");
		}
	}
	serial_putc(fd,'b');
	set_raw(false);
	serial_close(fd); 
	return 0;
}
