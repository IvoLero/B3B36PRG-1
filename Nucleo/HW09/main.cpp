#include "mbed.h"
 
Serial serial(SERIAL_TX, SERIAL_RX);
DigitalOut myled(LED1);

Ticker flipper;
int ledon =0;
float periods [5] = {0.05, 0.1, 0.2, 0.5, 1.0};
 
 
void timer(){
        if (ledon==1){
            myled = !myled;
            serial.putc(myled ==1 ? 'x':'o');       
        }
}
 
 
int main()
{
    serial.baud(115200);
    for (int i = 0; i < 5*2; ++i) { // 5x zablikání LED s periodou 50 ms
        myled = !myled;
        wait(0.05);
    }
    serial.putc('i');
    myled = 0;
    
    
    
    while (1) {
        while(!serial.readable()) {
        //aktivní čekání na přijetí bytu
        }
        
        int c = serial.getc();
        switch(c) {
            //start
            case 's':
                ledon = 1;
                flipper.detach();
                myled = 1;
                serial.putc('a');
                break;
            //end
            case 'e':
                ledon = 0;
                flipper.detach();
                myled = 0;
                serial.putc('a');
                break;
            //hello
            case 'h':
                serial.putc('h');
                break;

            //set period
            case '1':
                flipper.attach(&timer, periods[0]);
                serial.putc('a');
                break;
            case '2':
                 
                flipper.attach(&timer, periods[1]);
                serial.putc('a');
                break;
            case '3':
                  
                flipper.attach(&timer, periods[2]);
                serial.putc('a');
                break;
            case '4':
                 
                flipper.attach(&timer, periods[3]);
                serial.putc('a');
                break;
            case '5':
                 
                flipper.attach(&timer, periods[4]);
                serial.putc('a');
                break;
            //bye
            case 'b':
                serial.putc('b');
                //serial.printf("\nProgram has been closed\n");
                myled=0;
                flipper.detach();
                exit(0);
            default:
                serial.putc('f');
        }
        while (!serial.writeable()) {}
    
    }
}
