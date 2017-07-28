/*
 * File name: hw10-main.c
 * Date:      2017/04/14 18:51
 * Author:    Jan Faigl
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include <assert.h>

#include <termios.h> 
#include <unistd.h>  // for STDIN_FILENO

#include <pthread.h>

#include "prg_serial_nonblock.h"
#include "messages.h"
#include "event_queue.h"

#define SERIAL_READ_TIMOUT_MS 500 //timeout for reading from serial port

// shared data structure
typedef struct {
   bool quit;
   int fd; // serial port file descriptor
} data_t;

pthread_mutex_t mtx;
pthread_cond_t cond;

void call_termios(int reset);

void* input_thread(void*);
void* serial_rx_thread(void*); // serial receive buffer

bool send_message(data_t *data, message *msg);

// - main ---------------------------------------------------------------------
int main(int argc, char *argv[])
{
   data_t data = { .quit = false, .fd = -1};
   const char *serial = argc > 1 ? argv[1] : "/dev/ttyACM0";
   data.fd = serial_open(serial);

   if (data.fd == -1) {
      fprintf(stderr, "ERROR: Cannot open serial port %s\n", serial);
      exit(100);
   }

   call_termios(0);
   pthread_mutex_init(&mtx, NULL); // initialize mutex with default attributes
   pthread_cond_init(&cond, NULL); // initialize mutex with default attributes

   enum { INPUT, SERIAL_RX, NUM_THREADS };
   const char *threads_names[] = { "Input", "Serial In" };
   void* (*thr_functions[])(void*) = { input_thread, serial_rx_thread};
   pthread_t threads[NUM_THREADS];


   for (int i = 0; i < NUM_THREADS; ++i) {
      int r = pthread_create(&threads[i], NULL, thr_functions[i], &data);
      fprintf(stderr, "INFO: Create thread '%s' %s\n", threads_names[i], ( r == 0 ? "OK" : "FAIL") );
   }

    // example of local variables for computation and messaging
   struct {
      uint16_t chunk_id;
      uint16_t nbr_tasks;
      uint16_t task_id;
      bool computing;
   } computation = { 0, 0, 0, false };



   message msg;
   bool quit = false;

   while (!quit) {

     //wait until the next event will be pushed to queue
     event ev = queue_pop();

      //PC REQUEST
      if (ev.source == EV_KEYBOARD) {
	 //saves as MSG_NBR which wont be sent
         msg.type = MSG_NBR;

         switch(ev.type) {
	    //EVENT - get version
            case EV_GET_VERSION:
               msg.type = MSG_GET_VERSION;
               fprintf(stderr, "INFO: Get version requested\n");
               break;
	    
	    //EVENT - compute number of tasks
	    case EV_COMPUTE:
		//pthread_mutex_lock(&mtx);
		if (computation.computing){
			fprintf(stderr, "WARN: New computation requested but it is discarded due on ongoing computation\n\r");
		}
		else{
			int numoftasks = ev.data.param *10;
			//changing computation
			computation.chunk_id++;
			computation.computing = true;
			computation.nbr_tasks = numoftasks;
			//creating message
	    		msg.type = MSG_COMPUTE;
			msg.data.compute.chunk_id = computation.chunk_id;
			msg.data.compute.nbr_tasks = numoftasks;
			//fprintf(stderr, "INFO: New computation chunk id: %d no. of tasks: %d\n\r", msg.data.compute.chunk_id, msg.data.compute.nbr_tasks);
			fprintf(stderr, "INFO: New computation chunk id: %d no. of tasks: %d\n\r", computation.chunk_id, computation.nbr_tasks);
		}
		//pthread_mutex_unlock(&mtx);
	    		break;

	    //EVENT - reset chunk, it is not sent
	    case EV_RESET_CHUNK:
		//pthread_mutex_lock(&mtx);
		if (computation.computing){
			fprintf(stderr, "WARN: Chunk reset request discarded, it is currently computing\n\r");
		}
		else{
			computation.chunk_id=0;
			fprintf(stderr, "INFO: Chunk reset request\n\r");
		}
		//pthread_mutex_unlock(&mtx);
	    	break;
		
	    //EVENT -abort - negate computing and send message
	    case EV_ABORT:
	    	//pthread_mutex_lock(&mtx);
		if (!computation.computing){
			fprintf(stderr, "WARN: Abort requested but it is not computing\n\r");	
		}
		else{
			
			msg.type=MSG_ABORT;
			computation.computing=false;
			computation.chunk_id--;
		}
		//pthread_mutex_unlock(&mtx);
	    	break;

            default:
               break;
         }

         if (msg.type != MSG_NBR) { // messge has been set
            if (!send_message(&data, &msg)) 
               fprintf(stderr, "ERROR: send_message() does not send all bytes of the message!\n");
            
         }
      }


      
      else if (ev.source == EV_NUCLEO) { // handle nucleo events
         
	 //one event type for all events from NUCLEO
	 if (ev.type == EV_SERIAL) {
            message *msg = ev.data.msg;
	    
	    switch (msg->type) {
               
	       //MSG - STARTUP -- jsut read
	       case MSG_STARTUP: 
	          {
                    char str[STARTUP_MSG_LEN+1];
                    for (int i = 0; i < STARTUP_MSG_LEN; ++i) {
                       str[i] = msg->data.startup.message[i];
                    }
                    str[STARTUP_MSG_LEN] = '\0';
                    fprintf(stderr, "INFO: Nucleo restarted - '%s'\n", str);
		    //restart variables
		   computation.computing = false;
		   computation.chunk_id = 0;

                    break;
		   }
               //MSG - VERSION
	       case MSG_VERSION:
                  if (msg->data.version.patch > 0) {
                     fprintf(stderr, "INFO: Nucleo firmware ver. %d.%d-p%d\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
                  } else {
                     fprintf(stderr, "INFO: Nucleo firmware ver. %d.%d\n", msg->data.version.major, msg->data.version.minor);
                  }
                  break;
		case MSG_OK:
			fprintf(stderr, "INFO: Receive ok from Nucleo\r\n");
			break;
		case MSG_ERROR:
			fprintf(stderr, "WARN: Receive error from Nucleo\r\n");
			break;
		case MSG_ABORT:
			fprintf(stderr, "INFO: Abort from Nucleo\r\n");
			if (!computation.computing){
				fprintf(stderr, "WARN: Abort from NUCLEO is requested but it is not computing\n\r");	
			}
			else{
				computation.computing = false;
				computation.chunk_id--;
			}
			break;

		//MSG with computed data
		case MSG_COMPUTE_DATA:
			if (computation.computing){
				fprintf(stderr, "INFO: New data chunk id: %d, task id: %d - results %d\r\n", msg->data.compute_data.chunk_id, msg->data.compute_data.task_id, msg->data.compute_data.result);
				//handle the computing data
				//if (computation.nbr_tasks -1== msg->data.compute_data.task_id){
				//}
			}
			else{
				fprintf(stderr, "WARN: Nucleo sends new data without computing \r\n");
			}
			break;
		case MSG_DONE:
			fprintf(stderr, "INFO: Nucleo reports the computation is done computing: %d\r\n", computation.computing);
			computation.computing = false;
			break;

               	default:
			break;
            }

            if (msg) {
               free(msg);
            }

         } 
	 
	 
	 else if (ev.type == EV_QUIT) {
            quit = true;
         } 
	 else {
            // ignore all other events
         }
      }
      
   }
   
   // AFTER PARSING EVENT QUIT 
    queue_cleanup(); // cleanup all events and free allocated memory for messages.

   	for (int i = 0; i < NUM_THREADS; ++i) {
      		fprintf(stderr, "INFO: Call join to the thread %s\n", threads_names[i]);
      		int r = pthread_join(threads[i], NULL);
      		fprintf(stderr, "INFO: Joining the thread %s has been %s\n", threads_names[i], (r == 0 ? "OK" : "FAIL"));
   }
   serial_close(data.fd);
   call_termios(1); // restore terminal settings
   return EXIT_SUCCESS;
}

// - function -----------------------------------------------------------------
void call_termios(int reset)
{
   static struct termios tio, tioOld;
   tcgetattr(STDIN_FILENO, &tio);
   if (reset) {
      tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
   } else {
      tioOld = tio; //backup 
      cfmakeraw(&tio);
      tio.c_oflag |= OPOST;
      tcsetattr(STDIN_FILENO, TCSANOW, &tio);
   }
}

// - function -----------------------------------------------------------------
void* input_thread(void* d)
{
   data_t *data = (data_t*)d;
   bool end = false;
   int c;
   event ev = { .source = EV_KEYBOARD };
   while ( !end && (c = getchar())) {
   	   //default
           ev.type = EV_TYPE_NUM;
      switch(c) {
         case 'g': // get version
             ev.type = EV_GET_VERSION;
             break;
         case 'q':
	     //Quit event is created after the loop ends
	     //ev.type = EV_QUIT;
             end = true;
            break;
	 case '1':
	 case '2':
	 case '3':
	 case '4':
	 case '5':
	     ev.type = EV_COMPUTE;
	     ev.data.param = c - '0';
	    break;
	 case 'a':
	     ev.type = EV_ABORT;
	    break;
	  case 'r':
	     ev.type = EV_RESET_CHUNK;
	    break;

         default: // discard all other keys
            break;
      }
	  //if the key was allowed push the evet to the queue
          if (ev.type != EV_TYPE_NUM) { // new event 
             queue_push(ev);
          }

      pthread_mutex_lock(&mtx);
      end = end || data->quit; 
      data->quit = end;
      pthread_mutex_unlock(&mtx);
   }
   ev.type = EV_QUIT;
   queue_push(ev);
   fprintf(stderr, "INFO: Exit input thead %p\n", (void*)pthread_self());
   return NULL;
}

// - function -----------------------------------------------------------------
void* serial_rx_thread(void* d) 
{ 
// read bytes from the serial and puts the parsed message to the queue
   data_t *data = (data_t*)d;
   //must malloc memory, if you use local buffer -> it will be overwritten when accepting more messages
   uint8_t msg_buf [sizeof(message)]; // maximal buffer for all possible messages defined in messages.h
   message* msg = NULL;
   event ev = { .source = EV_NUCLEO, .type = EV_SERIAL, .data.msg = NULL };
   bool end = false;
   bool reading = false;
   int size;
   int counter=0;
   unsigned char c;

   while (serial_getc_timeout(data->fd, SERIAL_READ_TIMOUT_MS, &c) > 0) {}; // discard garbage

   while (!end) {
      int r = serial_getc_timeout(data->fd, SERIAL_READ_TIMOUT_MS, &c);
      if (r > 0) { // character has been read
      	 if (!reading){
		//allocate new message buffer
	 	if (!get_message_size((uint8_t)c, &size)){
			fprintf(stderr, "ERROR: Unknown message type has been received 0x%x\n - '%c'\r", c, c);
		}
		reading=true;
		msg_buf[counter++]=(uint8_t)c;
	}
	else if (reading){
		msg_buf[counter++]=(uint8_t)c;
		if (counter==size){
			msg = (message*) malloc(sizeof(message));
			if (parse_message_buf (msg_buf, size, msg)){
				reading=false;
				counter=0;
				ev.data.msg = msg;
				queue_push(ev);

			}
			else
				fprintf(stderr, "ERROR: Cannot parse message type %d\n\r", msg_buf[0]);
		}
    	}

	} 
	else if (r == 0) {
     		if (reading) fprintf(stderr, "ERROR: UNHANDELED ERROR!!!");
		
      	} //read but nothing has been received

      else {
         fprintf(stderr, "ERROR: Cannot receive data from the serial port\n");
         end = true;
      }   

      pthread_mutex_lock (&mtx);
      end = end || data->quit;
      pthread_mutex_unlock(&mtx);
      }
    ev.type = EV_QUIT;
    queue_push(ev);
   fprintf(stderr, "INFO: Exit serial_rx_thread %p\n", (void*)pthread_self());
   return NULL;
}

// - function -----------------------------------------------------------------
bool send_message(data_t *data, message *msg) 
{
  
   pthread_mutex_lock(&mtx);
   int fd = data->fd;
   pthread_mutex_unlock(&mtx);
   
   int size = sizeof(message);
   int length;

   get_message_size(msg->type, &length);
  // printf ("%d\n", length);

   uint8_t msg_buf[size];

   if (!fill_message_buf(msg, msg_buf,size,&length))
   	return false;
   else{
   	
	for (int i=0; i < length; i++){
		serial_putc(fd, msg_buf[i]);
	}
	return true;
   }
}
/*
--fprintf(stderr, "ERROR: Cannot open serial port %s\n", serial);
--fprintf(stderr, "INFO: Create thread '%s' %s\r\n", threads_names[i], ( r == 0 ? "OK" : "FAIL") );
--fprintf(stderr, "INFO: Get version requested\r\n");
--fprintf(stderr, "INFO: New computation chunk id: %d no. of tasks: %d\n\r", msg.data.compute.chunk_id, msg.data.compute.nbr_tasks);

--fprintf(stderr, "WARN: New computation requested but it is discarded due on ongoing computation\n\r");

--fprintf(stderr, "INFO: Chunk reset request\n\r");
--fprintf(stderr, "WARN: Chunk reset request discarded, it is currently computing\n\r");
--fprintf(stderr, "WARN: Abort requested but it is not computing\n\r");
--fprintf(stderr, "ERROR: send_message() does not send all bytes of the message!\n\r");
--fprintf(stderr, "INFO: Nucleo restarted - '%s'\r\n", str);
--fprintf(stderr, "INFO: Receive ok from Nucleo\r\n");
--fprintf(stderr, "INFO: Nucleo firmware ver. %d.%d-p%d\r\n", msg->data.version.major, msg->data.version.minor, msg->data.version.patch);
--fprintf(stderr, "INFO: Nucleo firmware ver. %d.%d\r\n", msg->data.version.major, msg->data.version.minor);
--fprintf(stderr, "WARN: Receive error from Nucleo\r\n");
--fprintf(stderr, "INFO: Abort from Nucleo\r\n");
--fprintf(stderr, "INFO: New data chunk id: %d, task id: %d - results %d\r\n", msg->data.compute_data.chunk_id, msg->data.compute_data.task_id, msg->data.compute_data.result);
--fprintf(stderr, "WARN: Nucleo sends new data without computing \r\n");
--fprintf(stderr, "INFO: Nucleo reports the computation is done computing: %d\r\n", computation.computing);
--fprintf(stderr, "INFO: Call join to the thread %s\r\n", threads_names[i]);
--fprintf(stderr, "INFO: Joining the thread %s has been %s\r\n", threads_names[i], (r == 0 ? "OK" : "FAIL"));
--fprintf(stderr, "INFO: Exit input thead %p\r\n", pthread_self());
fprintf(stderr, "DEBUG: computing: %d\r\n", computation.computing);
--fprintf(stderr, "ERROR: Unknown message type has been received 0x%x\n - '%c'\r", c, c);
--fprintf(stderr, "ERROR: Cannot parse message type %d\n\r", msg_buf[0]);
fprintf(stderr, "WARN: the packet has not been received discard what has been read\n\r");
--fprintf(stderr, "ERROR: Cannot receive data from the serial port\r\n");
--fprintf(stderr, "INFO: Exit serial_rx_thread %p\r\n", pthread_self());
fprintf(stderr, "DEBUG: Write message: ");
*/
