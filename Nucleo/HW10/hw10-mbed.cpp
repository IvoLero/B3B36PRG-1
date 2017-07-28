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
    MSG_OK,               // FROM Nucleo                positive answer for MSG_ABORT, MSG_COMPUTE 
    MSG_ERROR,            // FROM Nucleo                negative answer for MSG_ABORT, MSG_COMPUTEreport error on the previously received command
    MSG_ABORT,            // FROM PC                    abort - from user button or from serial port
    MSG_DONE,             // FROM Nucleo                report the requested work has been done
    MSG_GET_VERSION,      // FROM PC --> msg_version    request version of the firmware
    MSG_VERSION,          // FROM Nucleo                send version of the firmware as major,minor, patch level, e.g., 1.0p1
    MSG_STARTUP,          // FROM Nucleo                init of the message (id, up to 8 bytes long string, cksum
    MSG_COMPUTE,          // FROM PC --> msg_compute    request computation of a batch of tasks (chunk_id, nbr_tasks)
    MSG_COMPUTE_DATA,     // FROM Nucleo --> msg_computed data  computed result (chunk_id, task_id, result)
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
    uint16_t chunk_id;
    uint16_t nbr_tasks;
} msg_compute;
 
typedef struct {
    uint16_t chunk_id;
    uint16_t task_id;
    uint8_t result;
} msg_compute_data;


// MESSAGE STRUCTURE
typedef struct {
    uint8_t type;   // message type -defined in the first byte
    union {
        msg_version version;
        msg_startup startup;
        msg_compute compute;
        msg_compute_data compute_data;
    } data;
    uint8_t cksum; // message command -define the control sum
} message;




typedef struct {
    uint16_t chunk_id;
    uint16_t nbr_tasks;
    uint16_t task_id;
}computation;
bool computing;

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
   if (computing)
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
    //rika ARM procesoru, aby nevyvolaval interruptovy udalosti
    //procesoroa uroven - preruseni se vyvola zpetne
    //The function disables a device-specific interrupt in the NVIC interrupt controller.
    NVIC_DisableIRQ(USART2_IRQn); 
    
    
    bool empty = (tx_in == tx_out);    //prazdny cely posilaci buffer -> nic se neposila???
    
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
        //vyvola interrupt handler, jenom pokud nema nic k precteni...
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
bool get_message_size(uint8_t msg_type, int *len){
    
    bool ret = true;
    switch(msg_type) {
        case MSG_OK:
        case MSG_ERROR:
        case MSG_ABORT:
        case MSG_DONE:
        case MSG_GET_VERSION:
            *len = 2;           // 2 bytes message - id + cksum
            break;     
        case MSG_STARTUP:       //STARTUP MASSAGE
            *len = 2 + STARTUP_MSG_LEN;
            break;
        case MSG_VERSION:       //VERSION OF MASSAGE
            *len = 2 + 3 * sizeof(uint8_t); // 2 + major, minor, patch
            break;
        case MSG_COMPUTE:        //COMPUTE MASSAGE
            *len = 2 + 2 * sizeof(uint16_t); // 2 + chunk_id (16bit) + nbr_tasks (16bit)
            break;
        case MSG_COMPUTE_DATA:
            *len = 2 + 2 * sizeof(uint16_t) + 1; // 2 + chunk_id (16bit) + task_id (16bit) results (8bit)
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
    
    //CONTROL   1.not empty   2.sum of all bytes must be 255  
    //          3.message type is within allowed messages
    //          4.message size is recognized and is size
    //saving message
    if (
        size > 0 && cksum == 0xff && 
        ((msg->type = buf[0]) > 0) && msg->type < MSG_NBR &&
        get_message_size(msg->type, &message_size) && size == message_size) {
        ret = true;
        
        
        switch(msg->type) {
            case MSG_OK:
            case MSG_ERROR:
            case MSG_ABORT:
            case MSG_DONE:
            case MSG_GET_VERSION:
            //do not need to handle 
                break;
            //access to union
            case MSG_STARTUP:
                for (int i = 0; i < STARTUP_MSG_LEN; ++i) {
                    msg->data.startup.message[i] = buf[i+1];
                }
                break;
            case MSG_VERSION:
                msg->data.version.major = buf[1];
                msg->data.version.minor = buf[2];
                msg->data.version.patch = buf[3];
             //16 bit variables  - are sent in 4 bytes - most significant byte is stored first
            case MSG_COMPUTE: // type + chunk_id + nbr_tasks
                msg->data.compute.chunk_id = (buf[1] << 8) | buf[2];
                msg->data.compute.nbr_tasks = (buf[3] << 8) | buf[4];
                break;
            case MSG_COMPUTE_DATA:  // type + chunk_id + task_id + results
                msg->data.compute_data.chunk_id = (buf[1] << 8) | buf[2];
                msg->data.compute_data.task_id = (buf[3] << 8) | buf[4];
                msg->data.compute_data.result = buf[5];
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
        case MSG_GET_VERSION:
            len = 1;
            //do not need to handle them
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
        case MSG_COMPUTE:
            buf[1] = (uint8_t)(msg->data.compute.chunk_id >> 8); // hi - chunk_id
            buf[2] = (uint8_t)msg->data.compute.chunk_id;        // lo - chunk_id
            buf[3] = (uint8_t)(msg->data.compute.nbr_tasks >> 8);// hi - nbr_tasks
            buf[4] = (uint8_t)msg->data.compute.nbr_tasks;       // lo - nbr_tasks
            len = 5;
            break;
        case MSG_COMPUTE_DATA:
            buf[1] = (uint8_t)(msg->data.compute_data.chunk_id >> 8);// hi - chunk_id
            buf[2] = (uint8_t)msg->data.compute_data.chunk_id;       // lo - chunk_id
            buf[3] = (uint8_t)(msg->data.compute_data.task_id >> 8); // hi - task_id
            buf[4] = (uint8_t)msg->data.compute_data.task_id;        // lo - task_id
            buf[5] = msg->data.compute_data.result; // results
            len = 6;
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
        len += 1; // add cksum to buffer
        ret = send_buffer(buf, len);
    }
    return ret;
}









int main() {
    
    uint8_t msg_buf [BUF_SIZE];
    int msg_len;
    message msg;
    computation compute_data;
    float period = 0.25;
    float compute_time = 0.2;
    
    //setting up the communication
    computing = false;
    serial.baud(115200);
    for (int i = 0; i < 5*2; ++i) { // 5x zablikání LED s periodou 50 ms
        myled = !myled;
        wait(0.05);
    }
    //serial.putc('i');
    
    
    //adding interrupt handlers
    serial.attach(&Rx_interrupt, Serial::RxIrq); 
    serial.attach(&Tx_interrupt, Serial::TxIrq); 
    button_event.rise(&button);

    //myled = 0;
    
    //send message startup
    msg.type = MSG_STARTUP;
    char startup [STARTUP_MSG_LEN+1] = "PRG-HW 10";
    for (int i =0; i< STARTUP_MSG_LEN; i++){
        msg.data.startup.message[i]= startup[i];
    }
    send_message(&msg, msg_buf, MESSAGE_SIZE);
    
    
    

    
    while (1) {
        //HANDLE ABORT
        if (abort_request) {
            if (computing) {  //abort computing
                msg.type = MSG_ABORT;
                send_message(&msg, msg_buf, MESSAGE_SIZE);
                computing = false;
                abort_request = false;
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
                            
                        case MSG_ABORT:
                            msg.type = MSG_OK;
                            send_message(&msg, msg_buf, MESSAGE_SIZE);
                            computing = false;
                            abort_request = false;
                            ticker.detach();
                            myled = 0;
                            break;
                            
                            
                        case MSG_COMPUTE:   
                            
                            if (msg.data.compute.nbr_tasks > 0) {
                                ticker.attach(tick, period);
                                //handle computation???
                                compute_data.chunk_id = msg.data.compute.chunk_id;
                                compute_data.nbr_tasks = msg.data.compute.nbr_tasks;
                                compute_data.task_id = 0; // reset the task counter
                                computing = true;
                            }
                            msg.type = MSG_OK;
                            send_message(&msg, msg_buf, MESSAGE_SIZE);
                            break;
                    } // end switch
                } else { // message has not been parsed send error
                    msg.type = MSG_ERROR;
                    send_message(&msg, msg_buf, MESSAGE_SIZE);
                }
            } // end message received
        } 
        else if (computing) {
            
            if (compute_data.task_id < compute_data.nbr_tasks) {
                wait(compute_time); // do some computation
                msg.type = MSG_COMPUTE_DATA;
                msg.data.compute_data.chunk_id = compute_data.chunk_id;
                msg.data.compute_data.task_id = compute_data.task_id;
                msg.data.compute_data.result = compute_data.task_id+1;
                compute_data.task_id += 1;
                send_message(&msg, msg_buf, MESSAGE_SIZE);
            } 
            else { //computation done
                ticker.detach();
                myled = 0;
                msg.type = MSG_DONE;
                send_message(&msg, msg_buf, MESSAGE_SIZE);
                computing = false;
            }
        } else {
            sleep(); // put the cpu to sleep mode, it will be wakeup on interrupt
        }
    } // end while (1)
}

    
