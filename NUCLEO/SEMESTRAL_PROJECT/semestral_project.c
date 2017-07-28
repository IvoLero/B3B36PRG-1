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
#include "screen.h"

#define SERIAL_READ_TIMOUT_MS 500 //timeout for reading from serial port

//defualt parameters
#define def_bR 0.5
#define def_bI 0.5
#define def_aR -0.5
#define def_aI -0.5
#define def_N 60
#define def_cR -0.4
#define def_cI 0.6
#define def_W 320
#define def_H 240
#define def_chunksize 20


#define maxchunksize 250
#define maxN 100
#define minN 0

// shared data structure
typedef struct {
   bool quit;
   int fd; // serial port file descriptor
} data_t;


typedef enum{
	NO_PAR,
	C_RE,
	C_IM,
	WIDTH,
	HEIGHT,
	N,
	A_RE,
	A_IM,
	B_RE,
	B_IM,
} param;

const char* parNames[] = {"NO","C_REAL", "C_IMAGINARY", "WIDTH", "HEIGHT", "N", "A_REAL", "A_IMAGINARY", "B_REAL", "B_IMAGINARY"};

typedef struct {
	uint8_t n;
      	double cR, cI;
	double aR, aI, bR, bI;
	double inR, inI;
      	bool computing;
	bool computation_set;
	uint8_t current_chunk;
	param parameter_to_be_changed;
}computation;


pthread_mutex_t mtx;
pthread_cond_t cond;

void call_termios(int reset);

void* input_thread(void*);
void* serial_rx_thread(void*); // serial receive buffer

bool send_message(data_t *data, message *msg);


//TOOL functions
image* compute_chunk (image* img, uint8_t cid, computation parameters);
image* computeCPU(image* img, computation parameters);
image* computeCPU2(image* img, computation parameters, bool change);
image* computeCPUanimation(image* img, computation parameters);
double getStartRe(image* img, computation parameters);
double getStartIm(image* img, computation parameters);
double countIncrement(int width, double apoint, double bpoint);
uint8_t computeChunkSize(int width, int height);

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




             
   computation parameters = { .n=def_N, .cR=def_cR, .cI=def_cI, .aR=def_aR, .aI=def_aI, .bR=def_bR, .bI=def_bI, .computing=false, .computation_set=false,.current_chunk=-1, .parameter_to_be_changed=NO_PAR};
  
   //image initialization
   image* img= initImage(def_W, def_H, def_chunksize);
   img =setBlackScreen(img);


   message msg;
   bool quit = false;
   int h, w;


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
	    case EV_CHANGE_PAR:
	    	parameters.parameter_to_be_changed = ev.data.param;
		fprintf(stderr, "INFO: Parameter %s has been selected\r\n",parNames[ev.data.param]);
		break;
	    case EV_SETCHANGE:

		switch(parameters.parameter_to_be_changed){
				case NO_PAR:
					fprintf(stderr, "INFO: No proper parameter has been selected to be changed\r\n");
					break;
				case C_RE:
					parameters.cR += (ev.data.param ==1)? 0.01 : -0.01;
					fprintf(stderr, "INFO: Constant has been set C= %.2f + i %.2f\n\r",parameters.cR, parameters.cI);
					break;
				case C_IM:
					parameters.cI += (ev.data.param ==1)? 0.01 : -0.01;
					fprintf(stderr, "INFO:: Constant has been set C= %.2f + i %.2f\n\r",parameters.cR, parameters.cI);
					break;
				case A_RE:
					parameters.aR += (ev.data.param ==1)? 0.01 : -0.01;
					fprintf(stderr, "INFO: A point has been set A= %.2f + i %.2f\n\r",parameters.aR, parameters.aI);
					break;
				case A_IM:
					parameters.aI += (ev.data.param ==1)? 0.01 : -0.01;
					fprintf(stderr, "INFO: A point has been set A= %.2f + i %.2f\n\r",parameters.aR, parameters.aI);
					break;
				case B_RE:
					parameters.bR += (ev.data.param ==1)? 0.01 : -0.01;
					fprintf(stderr, "INFO: B point has been set B= %.2f + i %.2f\n\r",parameters.bR, parameters.bI);
					break;
				case B_IM:
					parameters.bI += (ev.data.param ==1)? 0.01 : -0.01;
					fprintf(stderr, "INFO: B point has been set B= %.2f + i %.2f\n\r",parameters.bR, parameters.bI);
					break;
				case N:
					parameters.n += (ev.data.param ==1)? 1 : -1;
					parameters.n= (parameters.n > maxN)? maxN : parameters.n;
					parameters.n= (parameters.n < minN)? minN : parameters.n;
					fprintf(stderr, "INFO: N has been set N= %d \n\r",parameters.n );
					break;
				case WIDTH:
					if (!parameters.computing ){
						img->W +=(ev.data.param==1)? 10 : -10;
						img->refresh_require = true; 
					fprintf(stderr, "INFO: WIDTH has been set: %d x %d , plese refresh the image\n\r",img->W,  img->H);
					}
					else{
						fprintf(stderr, "INFO: Could not change image size during computation\r\n");

					}
					break;

				case HEIGHT:
					if (!parameters.computing ){
						img ->H +=(ev.data.param==1)? 10 : -10;
						img->refresh_require = true; 
						fprintf(stderr, "INFO: HEIGHT has been set: %d x %d , plese refresh the image\n\r",img->W,  img->H);
					}
					else{
						fprintf(stderr, "INFO: Could not change image size during computation\r\n");
					}
					break;
					
			}
			parameters.computation_set = false;
			break;
	    
	    case EV_SHOW_PARAMETERS:
	    	
		fprintf(stderr, "INFO: Parameters of the computation will be displayed\n\r");
		fprintf(stderr, "A= %.2f + i %.2f   B= %.2f + i %.2f   C= %.2f + i %.2f   inR= %.6f inIm= %.6f   N= %d\n\r",parameters.aR, parameters.aI, parameters.bR, parameters.bI, parameters.cR, parameters.cI, parameters.inR, parameters.inI,parameters.n);
		fprintf(stderr, "Image: WIDTH= %d HEIGHT= %d", img->W, img->H);
		break;	

	    case EV_SET_COMPUTE:
	    	if (!parameters.computing){
			parameters.computation_set = true;
			parameters.inR = countIncrement(img->W, parameters.aR, parameters.bR); 
			parameters.inI = countIncrement(img->H, parameters.aI, parameters.bI); 
	    		msg.type = MSG_SET_COMPUTE;
			msg.data.set_compute.c_re = parameters.cR;
			msg.data.set_compute.c_im = parameters.cI;
			msg.data.set_compute.d_re = parameters.inR; 			
			msg.data.set_compute.d_im = parameters.inI;
			msg.data.set_compute.n = parameters.n;
			fprintf(stderr,"INFO: Computation parameters has been set_up\n\r");
		}
		else{

			fprintf(stderr,"WARN: New computation parameters requested but it is discarded due to on ongoing computation\n");
		}
		break;
	    //EVENT - compute number of tasks
	    case EV_COMPUTE:
	    	if (img -> refresh_require){
			fprintf(stderr, "WARN: Size of the image has been change, refresh the image\n\r");
		}
		else if (parameters.computing){
			fprintf(stderr, "WARN: New computation requested but it is discarded due on ongoing computation\n\r");
		}
		else if (!parameters.computation_set){
			fprintf(stderr, "Computation was requested, but parameters has not been set up yet\n\r");
		}
		else{
			
			//changing computation
			parameters.current_chunk++;
			parameters.computing = true;
			//creating message
	    		msg.type = MSG_COMPUTE;
			msg.data.compute.cid = parameters.current_chunk;
			msg.data.compute.n_re = img->chunksize;
			msg.data.compute.n_im = img->chunksize;
			msg.data.compute.re = getStartRe(img,parameters);	
			msg.data.compute.im = getStartIm(img,parameters);

			fprintf(stderr,"INFO: New computation chunk id: %d for part %d x %d\n", msg.data.compute.cid, msg.data.compute.n_re, msg.data.compute.n_im);

		}
	    		break;

	    //EVENT - reset chunk, it is not sent
	    case EV_RESET_CHUNK:
		//pthread_mutex_lock(&mtx);
		if (parameters.computing){
			fprintf(stderr, "WARN: Chunk reset request discarded, it is currently computing\n\r");
		}
		else{
			parameters.current_chunk=-1;
			fprintf(stderr, "INFO: Chunk reset request\n\r");
		}
		//pthread_mutex_unlock(&mtx);
	    	break;
		
	    //EVENT -abort - negate computing and send message
	    case EV_ABORT:
	    	//pthread_mutex_lock(&mtx);
		if (!parameters.computing){
			fprintf(stderr, "WARN: Abort requested but it is not computing\n\r");	
		}
		else{
			
			fprintf(stderr, "INFO: Abort from Computer\n\r");
			msg.type=MSG_ABORT;
			parameters.computing=false;
			parameters.current_chunk--;
		}
		//pthread_mutex_unlock(&mtx);
	    	break;
	    case EV_COMPUTE_CPU:
	    	if (img -> refresh_require){
			fprintf(stderr, "WARN: Size of the image has been change, refresh the image\n\r");
		}
		else if (!parameters.computation_set){
			fprintf(stderr, "WARN: Computation was requested, but parameters has not been set up yet\n\r");
		}
	    	else{ 
	    		fprintf(stderr, "INFO: CPU just showed Nucleo who is the biggest guy in the neighbourhood\n\r");
			img=computeCPU2(img, parameters, false);
		}
	    	break;
	    case EV_CLEAR_BUFFER:
	    	
	    	if (img -> refresh_require){
			fprintf(stderr, "WARN: Size of the image has been change, refresh the image\n\r");
		}
	    	else if (parameters.computing)
			fprintf(stderr,"WARN: Clear buffer request discarderd, abort computation at first\n\r");
		
		else{
			img = setBlackScreen(img);
			parameters.current_chunk = -1;
			fprintf(stderr,"INFO: Buffer was cleared, chunkid is restarted\n\r");
		}
		break;

	    case EV_REFRESH:
	    	h = img->H;
		w = img->W;
		closeImage();
		free(img);
		img = initImage(w, h, computeChunkSize(w, h));
		img =setBlackScreen(img);
		parameters.current_chunk = -1;
	    	fprintf(stderr, "INFO: Screen has been refreshed, sizes has been updated\r\n");
	    	repaintScreen(img);
		break;

	    case EV_SHOW_ANIMATION:
	    	if (img -> refresh_require)
			fprintf(stderr, "WARN: Size of the image has been change, refresh the image\n\r");
		
		else if (parameters.computing)
			fprintf(stderr, "WARN: Nucleo is not able to create pallete durint computation\r\n");
		else{
			fprintf(stderr, "INFO: Animation has been created, art has no borders!\r\n");
			//img=computeCPU2(img, parameters, true);
			img = computeCPUanimation(img, parameters);
		}
		break;
	    case EV_SHOW_ARTWORK:

	    	if (img -> refresh_require)
			fprintf(stderr, "WARN: Size of the image has been change, refresh the image\n\r");
		
		else if (parameters.computing)
			fprintf(stderr, "WARN: Nucleo is not able to create pallete durint computation\r\n");
		else{
			fprintf(stderr, "INFO: Piece of art has been created, art has no borders!\r\n");
			//img=computeCPU2(img, parameters, true);
			img = computeCPU2(img, parameters, true);
		}
		break;
	    
	    	 

	    case EV_SAVE_AS_PPM:

	    	if (img -> refresh_require){
			fprintf(stderr, "WARN: Size of the image has been change, refresh the image\n\r");
		}
		else if (parameters.computing)
			fprintf(stderr, "WARN: Nucleo is able to save image only when it is not computing\r\n");
		else{
	    		fprintf(stderr, "INFO: Buffer was saved as PPM Image\n\r");
	    		saveScreenPPM(img);
			saveScreenPNG(img);
		}
		break;
	    case EV_ENHANCE_BAUD:
	    	if (parameters.computing){
			fprintf(stderr, "WARN: Nucleo is able to save image only when it is not computing\r\n");
		}
		else{
			msg.type= MSG_ENHANCE_BAUD;
			fprintf(stderr, "INFO: Baud rate has been enhanced\r\n");
			
		}
		break;



            default:
               break;
         }

         if (msg.type != MSG_NBR) { // messge has been set
            if (!send_message(&data, &msg)) 
               fprintf(stderr, "ERROR: send_message() does not send all bytes of the message!\n");
	       //send message before increasing baud
		if (msg.type ==MSG_ENHANCE_BAUD){	
			printf ("%d\n", data.fd);
			pthread_mutex_lock(&mtx);
			data.fd = serial_open_enhanced("/dev/ttyACM0");
			printf ("%d\n", data.fd);
			pthread_mutex_unlock(&mtx);
		}
            
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
		   parameters.computing = false;
		   parameters.computation_set =false;
		   parameters.current_chunk = -1;

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
			if (!parameters.computing){
				fprintf(stderr, "WARN: Abort from NUCLEO is requested but it is not computing\n\r");	
			}
			else{
				parameters.computing = false;
				parameters.current_chunk--;
			}
			break;

		//MSG with computed data
		case MSG_COMPUTE_DATA:
			if (parameters.computing){
				if (parameters.current_chunk != msg->data.compute_data.cid){
					fprintf(stderr,"WARN: received compute data has cid %d which is different from cid %d - cannot align data to the grid properly\x0a\r\n", msg->data.compute_data.cid, parameters.current_chunk);
				}
				else{
					int x= (parameters.current_chunk%img->chunksinrow) * img->chunksize + msg->data.compute_data.i_re; 
					int y= parameters.current_chunk /img->chunksinrow * img->chunksize + msg->data.compute_data.i_im; 	
					if (x<0 || y<0 || x>= img->W || y>= img->H){
						
						fprintf(stderr,"WARN: received grid results is out of range of the current grid\r\n");}
					else {
						img = setPixelbyIter(img, parameters.current_chunk, x,y,msg->data.compute_data.iter, parameters.n);
						repaintScreen(img);
					}
				}
			}

			
			else{
				fprintf(stderr, "WARN: Nucleo sends new data without computing \r\n");
			}
			break;

		case MSG_DONE:
			//Nucleo sends done even when it was aborted
			if (parameters.computing){
				fprintf(stderr, "INFO: Nucleo sent a chunk %d\n",parameters.current_chunk);
				parameters.computing = false;

                		if (parameters.current_chunk != img-> nbr_chunks){
                      			fprintf(stderr, "INFO: Sending request for another chunk to nucleo\n");
					event ev = { .source = EV_KEYBOARD, .type =EV_COMPUTE };
					queue_push(ev);
				}
				else{
					fprintf(stderr, "UNPREDICTABLE SUCCESS: Nucleo computed the whole image!!!\r\n");
					parameters.computing = false;
				}
			}
			else{
				fprintf(stderr, "INFO: Nucleo has the pleasure to announce you, that it is finally ready after abort\r\n");
			}
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
    freeImage(img);
    closeImage();

   	for (int i = 0; i < NUM_THREADS; ++i) {
      		fprintf(stderr, "INFO: Call join to the thread %s\n", threads_names[i]);
      		int r = pthread_join(threads[i], NULL);
      		fprintf(stderr, "INFO: Joining the thread %s has been %s\n", threads_names[i], (r == 0 ? "OK" : "FAIL"));
   }
   serial_close(data.fd);
   call_termios(1); // restore terminal settings
   return EXIT_SUCCESS;
}


double getStartRe(image* img, computation parameters){
	return parameters.aR + (parameters.current_chunk % img->chunksinrow) * img->chunksize *parameters.inR;
}
double getStartIm (image* img, computation parameters){
	return parameters.aI + (parameters.current_chunk/img->chunksinrow) * img->chunksize *parameters.inI;
}

double countIncrement(int width, double a, double b){
	return (b-a)/width;
}
uint8_t computeChunkSize (int width, int height){
	for (int i = def_chunksize; i < maxchunksize; i+=10){
		if (i * i * 255 >= width * height){
			return (uint8_t)i;
		}
	}
	return 255;
}
	

image* compute_chunk(image* img, uint8_t cid, computation parameters){

	if (cid <0|| cid >= img->nbr_chunks)
		return NULL;
	int xx = (cid%img->chunksinrow) * img->chunksize; 
	int yy = cid /img->chunksinrow * img->chunksize; 	
	printf("Chunk %d computation Requested: xx: %d yy: %d\n", cid, xx,yy);
	image* computed;
	
	for (int y = 0; y< img->chunksize; y++){
		for (int x =0; x < img->chunksize; x++){
			uint8_t iter = compute(parameters.cR, parameters.cI, parameters.aR + (xx+x)* parameters.inR, parameters.aI + (yy+y)*parameters.inI,parameters.n); 
			double t = (double) iter/(double)parameters.n;
			double R = 9 * (1-t) * t*t*t *255;
			double G = 15 * (1-t) * (1-t) * t*t *255;
			double B = 8.5 * (1-t) * (1-t) * (1-t) * t *255;
				
			computed = setPixel(img, cid, xx+x, yy+y, (uint8_t)R, (uint8_t) G, (uint8_t) B);
			if (computed!=NULL){
				img=computed;
			}
			//printf("Pixel x %d y %d SET to %d\n ",x+xx,y+yy,iter);
		}
	}
	return img;
}

image* computeCPU(image* img, computation parameters){
	 for (int i=0; i< img->nbr_chunks;i++){
   		img =compute_chunk(img, i,parameters);
   	 }
	 repaintScreen(img);
	 return img;
}
image* computeCPU2(image* img, computation parameters, bool changeduring){
	for (int x=img->H-1; x >= 0; x--){
		for (int y=0; y < img->W; y++){
			uint8_t iter= compute(parameters.cR, parameters.cI, parameters.aR + y*parameters.inR, parameters.aI + x* parameters.inI, parameters.n);
			double t = (double) iter/(double)parameters.n;
			double R = 9 * (1-t) * t*t*t *255;
			double G = 15 * (1-t) * (1-t) * t*t *255;
			double B = 8.5 * (1-t) * (1-t) * (1-t) * t *255;
			img->data[x* img->W +y] = (pix){.r=(int8_t) R, .g=(uint8_t) G, .b=(uint8_t) B};
			if (changeduring){
				parameters.cR+= 0.000001;
				parameters.cI+=	0.000001;
			}


		}       
	}

	repaintScreen(img);
	return img;
}

			

image* computeCPUanimation(image* img, computation parameters){
	//backup parameters

	for (int i = 0; i < 500; i++){
		computeCPU2(img,parameters,false);
		parameters.cR +=0.001;
		parameters.cI +=0.001;
		sleep(0.5);
	}
	return img;
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
		case 'u':
			fprintf(stderr,"INFO: Inset the parameter that you want to change\r\n");
	     		c = getchar();
	     		ev.type = EV_CHANGE_PAR;
	     		switch (c){
	     		case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
		   		ev.data.param = c -'0';
		   		break;
			default:
		   		ev.data.param = 0;
			}
	     		break;

		case '+':
	    		ev.type = EV_SETCHANGE;
	    		ev.data.param = 1;
	    		break;
	 	case '-':
	    		ev.type = EV_SETCHANGE;
	    		ev.data.param = 0;
	    		break;
	 	case 'c':
	     		ev.type = EV_COMPUTE;
	     		ev.data.param = c - '0';
	    		break;
	 	case 'a':
	     		ev.type = EV_ABORT;
	    		break;
	  	case 'd':
	     		ev.type = EV_RESET_CHUNK;
	     		break;
	  	case 'l':
	     		ev.type= EV_CLEAR_BUFFER;
	     		break;
	   	case 'r':
	     		ev.type= EV_REFRESH;
	     		break;
	   	case 'f':
	     		ev.type= EV_COMPUTE_CPU;
	     		break;
	  	case 's':
	     		ev.type= EV_SET_COMPUTE;
	     		break;
	    	case 'm':
	     		ev.type= EV_SAVE_AS_PPM;
	     		break;
		case 'j':
			ev.type= EV_SHOW_ARTWORK;
			break;
	    	case 'p':
	     		ev.type= EV_SHOW_PARAMETERS;	
	     		break;
		case 't':
			ev.type= EV_SHOW_ANIMATION;
			break;
		case 'b':
			ev.type= EV_ENHANCE_BAUD;
			break;
		case 'h':

			fprintf(stderr, "INFO: Basic manual for people who can not remember key shortcuts\n\r");
			fprintf(stderr, "KEY q -> QUIT\n\r");
			fprintf(stderr, "KEY u -> UPDATE parameters ->  1...8 for selecting parameters -> +/- for upgrading\n\r");
			fprintf(stderr, "KEY g -> GET version \n\r");
			fprintf(stderr, "KEY c -> COMPUTE by Nucleo \r\n");
			fprintf(stderr, "KEY s -> SET computation\r\n");
			fprintf(stderr, "KEY f -> FAST computation by PC \r\n");
			fprintf(stderr, "KEY a -> ABORT computation \r\n");
			fprintf(stderr, "KEY r -> REFRESH image \r\n");
			fprintf(stderr, "KEY p -> show PARAMETERS\r\n");
			fprintf(stderr, "KEY d -> RESET chunk id \r\n");
			fprintf(stderr, "KEY l -> CLEAR buffer \r\n");
			fprintf(stderr, "KEY m -> SAVE as PPM and PNG\r\n");
			fprintf(stderr, "KEY t -> SHOW ANIMATION \r\n");
			fprintf(stderr, "KEY j -> SHOW ARTWORK - change constatnt durin computation\r\n");
			fprintf(stderr, "KEY b -> BAUD rate enhance mode\r\n");
			break;

         	default: // discard all other keys
	    		fprintf(stderr, "WARN: Key was not recognised\r\n");
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
     		if (reading){ fprintf(stderr, "ERROR: UNHANDELED ERROR - sama data has been lost due to restart!!!\r\n");
			reading=false;
			counter=0;
		}
		
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

   if (fill_message_buf(msg, msg_buf,size,&length)){
   	// printf ("%d\n", length);

	for (int i=0; i < length; i++){
		serial_putc(fd, msg_buf[i]);
	}
	return true;
   } 
   return false;
   
  
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
