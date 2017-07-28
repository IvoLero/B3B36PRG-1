#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "prg_serial.h"

void set_raw(_Bool reset);

int main(int argc, char *argv[])
{
   int ret = 0;
   char c;
   const char *serial = argc > 1 ? argv[1] : "/dev/cuaU0";
   int fd = serial_open(serial);
   if (fd != -1) { // read from serial port
      set_raw(true); // set the raw mode
      _Bool quit = false;
      while (!quit) {
	 if (
	       (c = getchar()) == 's' || c == 'e'
	    || (c >= '1' && c <= '5')
	    ) {
	    if (serial_putc(fd, c) == -1) {
	       fprintf(stderr, "ERROR: Error in received responses\n");
	       quit = true;
	    }
	 }
	 quit = c == 'q';
      } // end while()
      serial_close(fd);
      set_raw(false);
   } else {
      fprintf(stderr, "ERROR: Cannot open device %s\n", serial);
   }
   return ret;
}

void set_raw(_Bool set)
{
   if (set) {
      system("stty raw");  // enable raw
   } else {
      system("stty -raw"); // disable raw
   }
}
