
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

#include <hmc5883l.h>
#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif
volatile uint8_t new_sensor_data = 0;
volatile struct data raw_meas;

int main (void){
  char string[12];
  
  GPIO_mode_output(&DDRB, PB5);
  //twi_init();
  uart_init(UART_BAUD_SELECT(9600, F_CPU));
  
  HMC588L_init();
  sei();
  TIM1_ovf_262ms();
  TIM1_ovf_enable();

  while(1){
    if (new_sensor_data){
      //uart_puts("new data: ");
      itoa(raw_meas.X, string, 10);
      uart_puts(string);
      uart_puts("\t");

      itoa(raw_meas.Y, string, 10);
      uart_puts(string);
      uart_puts("\t");

      itoa(raw_meas.Z, string, 10);
      uart_puts(string);
      uart_puts("\t→→\t");


      HMC5883L_ftoa(HMC5883L_azimuth(raw_meas.X, raw_meas.Y), string, 2, sizeof(string));
      //ftoa(HMC5883L_azimuth(raw_meas.X, raw_meas.Y), string, 2);
      uart_puts(string);
      uart_puts("\n");

      

      new_sensor_data = 0;
    }
  }
  return 0;
}


ISR(TIMER1_OVF_vect)
{
    PORTB = PORTB ^ (1<<PB5);
    
    raw_meas = HMC5883L_rawData(raw_meas);

    new_sensor_data = 1;
      
}
