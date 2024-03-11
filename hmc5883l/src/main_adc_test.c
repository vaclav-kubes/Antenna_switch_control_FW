
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h> // I2C library
#include "timer.h"
#include <uart.h>
#include <stdlib.h> 
#include <gpio.h>
//#include <i2cmaster.h> 
#include <util/delay.h>
//#include <math.h>
#include <ftoa.h>
#include <string.h>
#include <adc.h>
#include "antenna_switch_IO.h"
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif
volatile uint8_t new_sensor_data = 0;
volatile float temp;

int main (void){
  char string[12];
  
  GPIO_mode_output(&DDRC, LED);
  GPIO_mode_output(&DDRD, ANT01);
  GPIO_mode_output(&DDRD, ANT02);

  GPIO_write_low(&PORTD, ANT01);
  GPIO_write_low(&PORTD, ANT02);

  //twi_init();
  uart_init(UART_BAUD_SELECT(9600, F_CPU));
  ADC_init();

  sei();
  TIM1_ovf_262ms();
  TIM1_ovf_enable();

  while(1){
    if (new_sensor_data){
      float u = ADC_get_U();
      //float i = ADC_get_I();

      ftoa(u,string,2);
      uart_puts("Voltage: ");
      uart_puts(string);
      uart_puts("\n");
      /*ftoa(i,string,2);
      uart_puts("Current: ");
      uart_puts(string);
      uart_puts("\n");*/

      

      new_sensor_data = 0;
    }
  }
  return 0;
}


ISR(TIMER1_OVF_vect)
{
    PORTC = PORTC ^ (1<<LED);
    
    //ADC_read_I(I_DIAG1);
    //ADC_read_I(I_DIAG2);
    ADC_read_U();

    new_sensor_data = 1;
      
}
