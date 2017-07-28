#include "mbed.h"
 
 
DigitalOut myled(LED1);
DigitalIn mybutton(USER_BUTTON); 
Ticker flipper, timer;


int bcounter=0; 
int periodcounter=0;
float periods [8] = {1.0,0.5,0.4,0.3,0.2,0.1,0.05,0};
 

 

void timer50ms(){
     if (mybutton == 0) {
         bcounter++;
     }
     
     else if (mybutton==1 && bcounter!=0){
         if (bcounter< 20){
              periodcounter++;
              if (periodcounter >= 8){
                   periodcounter=7;
              }
              
          }
          else {
            periodcounter=0;         
        }
        bcounter=0;
    }
          

}
 
int main() {
  
  
   timer.attach(&timer50ms,0.05);
   
   while (1) {
      myled = !myled;
      wait(periods[periodcounter]);
   }
}
