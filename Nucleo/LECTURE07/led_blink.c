#include "stm32f4xx_hal.h"
 
/* method to delay by busy loop */
void ms_delay(int ms)
{
   while (ms-- > 0) {
      volatile int x=5971;
      while (x-- > 0)
         __asm("nop"); //no operation
   }
}
 
/* main function */
int main(void)
{         
  /* Enable GPIOA Clock (to be able to program the GPIO configuration registers) */
  /* RCC - real-time clock control, AHB1 - advanced high-performance bus 1, ENR - enable register */
  RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOAEN);
 
  /* GPIO_INIT pin PA5 - output, fast, pull-up */
  GPIOA->MODER   |= ((uint32_t) 0x00000001 << 10); 	//set port A pin 5 output; MODER - mode setup register
  GPIOA->OSPEEDR |= ((uint32_t) 0x00000002 << 10);	//set port A pin 5 output speed fast; OSPEEDR - output speed setup register
  GPIOA->PUPDR   |= ((uint32_t) 0x00000001 << 10); 	//set port A pin 5 pull-up; PUPDR - pull-up pull-down setup register   
 
  /* Btn to led control */   
  while (1)
  {  
  	/* toggle LED - PA5; ODR - output data register */
	GPIOA->ODR ^= ((uint32_t) 0x00000001 << 5);
    /* wait in busy loop */
    ms_delay(100); 	
  }
}