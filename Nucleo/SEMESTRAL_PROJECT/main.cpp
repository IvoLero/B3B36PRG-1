
#include "mbed.h"

//////////////////////////////////////////////////////////////////////////////////
//MESSAGES TYPES
//messages = packets they have defined length

#define STARTUP_MSG_LEN 9
#define VERSION_MAJOR 0
#define VERSION_MINOR 9
#define VERSION_PATCH 0


#define MESSAGE_SIZE 255

enum {
    MSG_OK,               // FROM Nucleo                positive answer for MSG_ABORT, MSG_COMPUTE, MSG_SET_COMPUTE
    MSG_ERROR,            // FROM Nucleo                negative answer for MSG_ABORT, MSG_COMPUTEreport error on the previously received command
    MSG_ABORT,            // FROM PC                    abort - from user button or from serial port
    MSG_DONE,             // FROM Nucleo                report the requested work has been done
    MSG_GET_VERSION,      // FROM PC --> msg_version    request version of the firmware
    MSG_VERSION,          // FROM Nucleo                send version of the firmware as major,minor, patch level, e.g., 1.0p1
    MSG_STARTUP,          // FROM Nucleo                init of the message (id, up to 8 bytes long string, cksum
    MSG_SET_COMPUTE,       //FROM PC            changes teh computation
    MSG_COMPUTE,          // FROM PC --> msg_compute    request computation of a batch of tasks (chunk_id, nbr_tasks)
    MSG_COMPUTE_DATA,     // FROM Nucleo --> msg_computed data  computed result (chunk_id, task_id, result)
    MSG_ENHANCE_BAUD,
    MSG_NBR                 //pocet ruznych message typu
} message_type;


typedef struct {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
} msg_version;

msg_version VERSION = { .major = VERSION_MAJOR, .minor = VERSION_MINOR, .patch = VERSION_PATCH };
 
 
typedef struct {
    uint8_t message[STARTUP_MSG_LEN];
} msg_startup;

typedef struct {
   double c_re;  // re (x) part of the c constant in recursive equation
   double c_im;  // im (y) part of the c constant in recursive equation
   double d_re;  // increment in the x-coords
   double d_im;  // increment in the y-coords
   uint8_t n;    // number of iterations per each pixel
} msg_set_compute;


typedef struct {
    uint8_t cid; // chunk id
    double re;    // start of the x-coords (real)
    double im;    // start of the y-coords (imaginary)
    uint8_t n_re; // number of cells in x-coords
    uint8_t n_im; // number of cells in y-coords
} msg_compute;
 
typedef struct {
   uint8_t cid;  // chunk id
   uint8_t i_re; // x-coords 
   uint8_t i_im; // y-coords
   uint8_t iter; // number of iterations
} msg_compute_data;

typedef struct {
   uint8_t type;   // message type
   union {
      msg_version version;
      msg_startup startup;
      msg_set_compute set_compute;
      msg_compute compute;
      msg_compute_data compute_data;
   } data;
   uint8_t cksum; // message command
} message;



typedef struct {
    uint8_t cid;
    uint8_t n;
    double iR, iI;
    double sR, sI;
    double cR, cI;
    uint8_t chunk_size;
    bool computing;
    int cx;
    int cy;

}computation;

computation parameters;

//////////////////////////////////////////////////////////////////////////////////



Serial serial(SERIAL_TX, SERIAL_RX);
DigitalOut myled(LED1);
InterruptIn button_event(USER_BUTTON);
Ticker ticker;



//////////////////////////////////////////////////////////////////////////////////
//BUFFER AND INTERRUPTS
// buffers have maximum size
#define BUF_SIZE 255

char tx_buffer[BUF_SIZE];
char rx_buffer[BUF_SIZE];
volatile int tx_in = 0;     //tail
volatile int tx_out = 0;    //head
volatile int rx_in = 0;
volatile int rx_out = 0;



// TRANSMIT INTERRUPT -send bytes from the tx_buffer and stop if the buffer is empty
void Tx_interrupt(){
    while ((serial.writeable()) && (tx_in != tx_out)) {
        serial.putc(tx_buffer[tx_out]);
        tx_out = (tx_out + 1) % BUF_SIZE;
    }
    return;
}
 // RECEIVE INTERRUPT -receive bytes, save them to buffer and stop if rx_buffer is full
void Rx_interrupt(){
    myled =1;
    while ((serial.readable()) && (((rx_in + 1) % BUF_SIZE) != rx_out)) {
        rx_buffer[rx_in] = serial.getc();  
        rx_in = (rx_in + 1) % BUF_SIZE;  
    }
    return;
}

void tick(){
            myled = !myled;     
}

//////////////////////////////////////////////////////////////////////////////////



//BUTTON INTERRUPT 
volatile bool abort_request = false;
void button(){
   if (parameters.computing)
        abort_request = true;
}


//////////////////////////////////////////////////////////////////////////////////



//FUNCTION HEADERS
bool receive_message(uint8_t *msg_buf, int size, int *len);
bool send_buffer(const uint8_t* msg, unsigned int size);
bool get_message_size(uint8_t msg_type, int *len);
bool parse_message(const uint8_t *buf, int size, message *msg);
bool send_message(const message *msg, uint8_t *buf, int size);


//FUNCTION FOR SENDING BUFFER
// reads data from message buffer and copies them to txbuffer 
// @parm msg        pointer to message buffer
// @parm size       length of message
bool send_buffer(const uint8_t* msg, unsigned int size){
    
    if (!msg && size == 0) {
        return true;    // size must be > 0
    }
    
    bool ret = false;
    int i = 0;
    
    
    // START CRITICAL SECTION  for accessing global data -  disable External Interrupt.
       //The function disables a device-specific interrupt in the NVIC interrupt controller.
    NVIC_DisableIRQ(USART2_IRQn); 
    
    
    bool empty = (tx_in == tx_out);       
    while ( (i == 0) || i < size ) { //must be read at least at once -  end reading when message has been read
        
        //CONTROL buffer is full 
        if ( ((tx_in + 1) % BUF_SIZE) == tx_out) {              // needs buffer space
            
            NVIC_EnableIRQ(USART2_IRQn);                        // enable interrupts for sending buffer
            while (((tx_in + 1) % BUF_SIZE) == tx_out) {}       // let interrupt routine empty the buffer
            NVIC_DisableIRQ(USART2_IRQn);                       // disable interrupts for accessing global buffer
        }
        
        //SAVE the bytes from message to transmitt buffer
        tx_buffer[tx_in] = msg[i];
        i += 1;
        tx_in = (tx_in + 1) % BUF_SIZE;
    }
    ret = true;
    
    //!!!problem with empty and do while
    
    
    
    //WRITING TO SERIAL 
    if (empty && serial.writeable()) {
        uint8_t c = tx_buffer[tx_out];
        tx_out = (tx_out + 1) % BUF_SIZE;
        serial.putc(c); // send first character to start tx interrupts (if stopped)
    }
    NVIC_EnableIRQ(USART2_IRQn); // end critical section
    return ret;
}








//FUNCTION FOR RECEIVING MESSAGE
// reads data from rxbuffer, control them and saves to message buffer
// @parm msg_buf       pointer to message buffer
// @parm size          length of messagebuffer
// @parm len           message size - for defining in get_message_size

bool receive_message(uint8_t *msg_buf, int size, int *len)
{
    bool ret = false;
    int i = 0;
    *len = 0;
    
    NVIC_DisableIRQ(USART2_IRQn); // start critical section for accessing global data
   
    
    while ( (i == 0) || (i != *len) ) {
        
        //CONTROL wait if buffer is empty
        if (rx_in == rx_out) {
            
            NVIC_EnableIRQ(USART2_IRQn);    // enable interrupts for receivng buffer
            while (rx_in == rx_out) {}      // wait for next character - the reading was not complete yet
            NVIC_DisableIRQ(USART2_IRQn);   // disable interrupts for accessing global buffer
        }
        
        uint8_t c = rx_buffer[rx_out];
        
        //GETTING MESSAGE TYPE
        if (i == 0) {   
            if (get_message_size(c, len)) {         // message type recognized -> vime kolik dat vzit z rx bufferu, doufame ze velikost je mensi nez velikost message bufferu = size
                msg_buf[i++] = c;
                ret = *len <= size; // msg_buffer must be large enough
            } else { // unknown message
                ret = false;
                break; 
                }
        } 
        else {
            msg_buf[i++] = c;
        }
        
        rx_out = (rx_out + 1) % BUF_SIZE;
    }
    NVIC_EnableIRQ(USART2_IRQn); // end critical section
    return ret;
}








//FUNCTION FOR GETTING MESSAGE SIZE
// @parm msg_type       
// @parm len      
bool get_message_size(uint8_t msg_type, int *len)
{
   bool ret = true;
   switch(msg_type) {
      case MSG_OK:
      case MSG_ERROR:
      case MSG_ABORT:
      case MSG_DONE:
      case MSG_GET_VERSION:
       MSG_ENHANCE_BAUD:
      case MSG_ENHANCE_BAUD:
         *len = 2; // 2 bytes message - id + cksum
         break;
      case MSG_STARTUP:
         *len = 2 + STARTUP_MSG_LEN;
         break;
      case MSG_VERSION:
         *len = 2 + 3 * sizeof(uint8_t); // 2 + major, minor, patch
         break;
      case MSG_SET_COMPUTE:
         *len = 2 + 4 * sizeof(double) + 1; // 2 + 4 * params + n 
         break;
      case MSG_COMPUTE:
         *len = 2 + 1 + 2 * sizeof(double) + 2; // 2 + cid (8bit) + 2x(double - re, im) + 2 ( n_re, n_im)
         break;
      case MSG_COMPUTE_DATA:
         *len = 2 + 4; // cid, dx, dy, iter
         break;
      default:
         ret = false;
         break;
   }
   return ret;
}



//FUNCTION FOR FILLING MESSAGE STRUCTURE
// @parm buf    messagebuffer - created by receivemessage  
// @parm size   size of the message   
// @parm msg    pointer to message, where the created structure will be saved      

bool parse_message(const uint8_t *buf, int size, message *msg){
    
    //CHECKING THE SUM
    uint8_t cksum = 0;
   for (int i = 0; i < size; ++i) {
      cksum += buf[i];
   }
   bool ret = false;
   int message_size;
   if (
         size > 0 && cksum == 0xff && // sum of all bytes must be 255
         ((msg->type = buf[0]) >= 0) && msg->type < MSG_NBR &&
         get_message_size(msg->type, &message_size) && size == message_size) {
      ret = true;
      switch(msg->type) {
         case MSG_OK:
         case MSG_ERROR:
         case MSG_ABORT:
         case MSG_DONE:
         case MSG_ENHANCE_BAUD:
         case MSG_GET_VERSION:
            break;
         case MSG_STARTUP:
            for (int i = 0; i < STARTUP_MSG_LEN; ++i) {
               msg->data.startup.message[i] = buf[i+1];
            }
            break;
         case MSG_VERSION:
            msg->data.version.major = buf[1];
            msg->data.version.minor = buf[2];
            msg->data.version.patch = buf[3];
            break;
         case MSG_SET_COMPUTE: 
            memcpy(&(msg->data.set_compute.c_re), &(buf[1 + 0 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.set_compute.c_im), &(buf[1 + 1 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.set_compute.d_re), &(buf[1 + 2 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.set_compute.d_im), &(buf[1 + 3 * sizeof(double)]), sizeof(double));
            msg->data.set_compute.n = buf[1 + 4 * sizeof(double)];
            break;
         case MSG_COMPUTE: // type + chunk_id + nbr_tasks
            msg->data.compute.cid = buf[1];
            memcpy(&(msg->data.compute.re), &(buf[2 + 0 * sizeof(double)]), sizeof(double));
            memcpy(&(msg->data.compute.im), &(buf[2 + 1 * sizeof(double)]), sizeof(double));
            msg->data.compute.n_re = buf[2 + 2 * sizeof(double) + 0];
            msg->data.compute.n_im = buf[2 + 2 * sizeof(double) + 1];
            break;
         case MSG_COMPUTE_DATA:  // type + chunk_id + task_id + result
            msg->data.compute_data.cid = buf[1];
            msg->data.compute_data.i_re = buf[2];
            msg->data.compute_data.i_im = buf[3];
            msg->data.compute_data.iter = buf[4];
            break;
         default: // unknown message type
            ret = false;
            break;
      } // end switch
   }
   return ret;
}


//FUNCTION FOR SENDING THE MESSAGE 
// use sendbuffer function
// @parm msg    message 
// @parm buf    buffer for sending  
// @parm size   size of the message      

bool send_message(const message *msg, uint8_t *buf, int size)
{
   if (!msg || size < sizeof(message) || !buf) {
      return false;
   }
   // 1st - serialize the message into a buffer
   bool ret = true;
   int len = 0;
   switch(msg->type) {
      case MSG_OK:
      case MSG_ERROR:
      case MSG_ABORT:
      case MSG_DONE:
      case MSG_ENHANCE_BAUD:
      case MSG_GET_VERSION:
         len = 1;
         break;
      case MSG_STARTUP:
         for (int i = 0; i < STARTUP_MSG_LEN; ++i) {
            buf[i+1] = msg->data.startup.message[i];
         }
         len = 1 + STARTUP_MSG_LEN;
         break;
      case MSG_VERSION:
         buf[1] = msg->data.version.major;
         buf[2] = msg->data.version.minor;
         buf[3] = msg->data.version.patch;
         len = 4;
         break;
      case MSG_SET_COMPUTE:
         memcpy(&(buf[1 + 0 * sizeof(double)]), &(msg->data.set_compute.c_re), sizeof(double));
         memcpy(&(buf[1 + 1 * sizeof(double)]), &(msg->data.set_compute.c_im), sizeof(double));
         memcpy(&(buf[1 + 2 * sizeof(double)]), &(msg->data.set_compute.d_re), sizeof(double));
         memcpy(&(buf[1 + 3 * sizeof(double)]), &(msg->data.set_compute.d_im), sizeof(double));
         buf[1 + 4 * sizeof(double)] = msg->data.set_compute.n;
         len = 1 + 4 * sizeof(double) + 1;
         break;
      case MSG_COMPUTE:
         buf[1] = msg->data.compute.cid; // cid
         memcpy(&(buf[2 + 0 * sizeof(double)]), &(msg->data.compute.re), sizeof(double));
         memcpy(&(buf[2 + 1 * sizeof(double)]), &(msg->data.compute.im), sizeof(double));
         buf[2 + 2 * sizeof(double) + 0] = msg->data.compute.n_re;
         buf[2 + 2 * sizeof(double) + 1] = msg->data.compute.n_im;
         len = 1 + 1 + 2 * sizeof(double) + 2;
         break;
      case MSG_COMPUTE_DATA:
         buf[1] = msg->data.compute_data.cid;
         buf[2] = msg->data.compute_data.i_re;
         buf[3] = msg->data.compute_data.i_im;
         buf[4] = msg->data.compute_data.iter;
         len = 5;
         break;
      default: // unknown message type
         ret = false;
         break;
   }
   // 2nd - send the message buffer
   if (ret) { // message recognized
      buf[0] = msg->type;
      buf[len] = 0; // cksum
      for (int i = 0; i < len; ++i) {
         buf[len] += buf[i];
      }
      buf[len] = 255 - buf[len]; // compute cksum
      len += 1; // add cksum to buffer        ret = send_buffer(buf, len);
      ret=send_buffer(buf,len);
    }
    return ret;
}


uint8_t compute(double cR, double cI, double px, double py, uint8_t n){

    double newR,newI, oldR,oldI;
    uint8_t counter =0; 
    newR= px; newI= py;

    while (newR*newR + newI *newI < 4 && counter < n){
            oldR = newR;
        oldI = newI;
        newR = oldR *oldR - oldI* oldI + cR;
        newI = 2* oldR *oldI + cI;
        counter++;
    }
    //printf("Compute pixel: %f %f %f %f %u %u \n", px,py,cR,cI,counter,n);
    return counter;
}










int main() {
    
    uint8_t msg_buf [BUF_SIZE];
    int msg_len;
    message msg;

    float period = 0.25;

    
    //setting up the communication
    parameters.computing = false;
    serial.baud(115200);
    for (int i = 0; i < 5*2; ++i) { // 5x zablikání LED s periodou 50 ms
        myled = !myled;
        wait(0.05);
    }
    
    
    //adding interrupt handlers
    serial.attach(&Rx_interrupt, Serial::RxIrq); 
    serial.attach(&Tx_interrupt, Serial::TxIrq); 
    button_event.rise(&button);

    
    //send message startup
    msg.type = MSG_STARTUP;
    sprintf((char*)msg.data.startup.message, "PRG-SEM-1");;
    send_message(&msg, msg_buf, MESSAGE_SIZE);
    
    
    

    
    while (1) {
        //HANDLE ABORT
        if (abort_request) {
            if (parameters.computing) {  //abort computing
                msg.type = MSG_ABORT;
                parameters.computing = false;
                abort_request = false;
                send_message(&msg, msg_buf, MESSAGE_SIZE);
                ticker.detach();
                myled = 0;
            }
        }
        
        //RECEIVING SOMETHING
        if (rx_in != rx_out) { 
        
            if (receive_message(msg_buf, MESSAGE_SIZE, &msg_len)) {
                if (parse_message(msg_buf, msg_len, &msg)) {
                    
                    switch(msg.type) {
                        case MSG_GET_VERSION:
                            msg.type = MSG_VERSION;
                            msg.data.version  = VERSION;
                            send_message(&msg, msg_buf, MESSAGE_SIZE);
                            break;
                            
                        case MSG_ENHANCE_BAUD:
                            if (!parameters.computing){
                                   serial.baud(576000);
                         }
                        msg.type = MSG_OK;
                        send_message(&msg, msg_buf, MESSAGE_SIZE);
                         break;

                            
                        case MSG_ABORT:
                            msg.type = MSG_OK;
                            parameters.computing = false;
                            abort_request = false;
                            ticker.detach();
                            myled = 0;
                            send_message(&msg, msg_buf, MESSAGE_SIZE);
                            break;
                        case MSG_SET_COMPUTE:
                            msg.type = MSG_OK;
                            parameters.n =msg.data.set_compute.n;
                            parameters.cR =msg.data.set_compute.c_re;
                            parameters.cI =msg.data.set_compute.c_im;
                            parameters.iR =msg.data.set_compute.d_re;
                            parameters.iI =msg.data.set_compute.d_im;
                            send_message(&msg, msg_buf, MESSAGE_SIZE);
                            break;
                            
                        case MSG_COMPUTE:   
                            
                            if (!parameters.computing) { //assert
                                ticker.attach(tick, period);
                                //handle computation???
                                parameters.cid =msg.data.compute.cid;
                                parameters.sR =msg.data.compute.re;
                                parameters.sI =msg.data.compute.im;
                                parameters.cx = 0; // reset the task counter
                                parameters.cy = 0;
                                parameters.chunk_size =msg.data.compute.n_re;   //my chunk is always square
                                parameters.computing = true;
                                myled=0;
                            }
                            msg.type = MSG_OK;
                            send_message(&msg, msg_buf, MESSAGE_SIZE);
                            break;
                    } 
                } 
        else { // message has not been parsed send error
                    msg.type = MSG_ERROR;
                    send_message(&msg, msg_buf, MESSAGE_SIZE);
                }
            } // end message received
        } 
        else if (parameters.computing) {
            int x = parameters.cx;
            int y = parameters.cy;
            int size = parameters.chunk_size;
            myled=1;
            
            if (x != size || y !=0) {        //last pixel
                msg.type = MSG_COMPUTE_DATA;
                msg.data.compute_data.cid = parameters.cid;
                msg.data.compute_data.iter = compute(parameters.cR, parameters.cI, parameters.sR + x*parameters.iR, parameters.sI + y*parameters.iI,parameters.n);
                msg.data.compute_data.i_re = (uint8_t)x;
                msg.data.compute_data.i_im = (uint8_t)y;
                send_message(&msg, msg_buf, MESSAGE_SIZE);
                
                //set next point
                if ((y+1) % size ==0){
                    parameters.cx+=1;
                    parameters.cy=0;
                }
                else{
                    parameters.cy+=1;
                }

                
            } 
            else { //computation done
                ticker.detach();
                myled = 0;
                msg.type = MSG_DONE;
                parameters.computing = false;
                send_message(&msg, msg_buf, MESSAGE_SIZE);
            }
        } else {
            sleep(); // put the cpu to sleep mode, it will be wakeup on interrupt
        }
    } // end while (1)
}

    