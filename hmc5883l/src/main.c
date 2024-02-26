
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h> // I2C library
#include "timer.h"
#include <uart.h>
#include <stdlib.h> 
#include <gpio.h>
//#include <i2cmaster.h> 
#include <util/delay.h>

#ifndef F_CPU
# define F_CPU 16000000  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif

#define HMC588L_I2C_ADR 0x1E
#define CONFIG_REG_A 0


volatile int16_t X = 0;
volatile int16_t Y = 0;
volatile int16_t Z = 0;
volatile uint8_t new_sensor_data = 0;

int main (void){
  char string[2];
  GPIO_mode_output(&DDRB, PB5);
  twi_init();
  uart_init(UART_BAUD_SELECT(9600, F_CPU));
  sei();  // Needed for UART

    if (twi_test_address(HMC588L_I2C_ADR) == 0)
        uart_puts("I2C sensor detected\r\n");
    else {
        uart_puts("[ERROR] I2C device not detected\r\n");
    }

  twi_start();
   if (twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE)==0){
        uart_puts("setting sensor\n");
        if(! twi_write(0)) uart_puts("start config\n");
        twi_write(0b01110000);
        twi_write(0b00100000);
        if(! twi_write(0b00000000)) uart_puts("setting succesfull\n");
        //twi_stop();
        
        twi_start();
        twi_write(0b00111100);
        twi_write(0);
        //twi_stop();

        twi_start();
        if(! twi_write(0b00111101)) uart_puts("reading\n");
        //twi_write(0);

        itoa(twi_read(TWI_ACK), string, 2);
        uart_puts(string);
        uart_puts(" ");

        itoa(twi_read(TWI_ACK), string, 2);
        uart_puts(string);
        uart_puts(" ");

        itoa(twi_read(TWI_NACK), string, 2);
        uart_puts(string);
        uart_puts("\n");
        twi_stop();
        //twi_start();
        //twi_write(0b00111100);
        //twi_write(0x03);
   }
  TIM1_ovf_1sec();
  TIM1_ovf_enable();
  
  while(1){
    if (new_sensor_data){
      uart_puts("new data: ");
      itoa(X, string, 10);
      uart_puts(string);
      uart_puts(" ");

      itoa(Y, string, 10);
      uart_puts(string);
      uart_puts(" ");

      itoa(Z, string, 10);
      uart_puts(string);
      uart_puts("\n");

      new_sensor_data = 0;
      //uart_puts("1\n");
    }
  }
  return 0;
}


ISR(TIMER1_OVF_vect)
{
    //PORTB = PORTB ^ (1<<PB5);
    /*if (twi_test_address(HMC588L_I2C_ADR) == 0)
        uart_puts("I2C sensor detected\r\n");
    else {
        uart_puts("[ERROR] I2C device not detected\r\n");
        //while (1);
    }*/
    twi_start();
    if(twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE) == 0) PORTB = PORTB ^ (1<<PB5);
    twi_write(3);
    //twi_stop();

    //twi_stop();
    twi_start();
    //if(! twi_write(0b00111101)) PORTB = PORTB ^ (1<<PB5);
    twi_write(0b00111101);
    X = (int)(int16_t)(twi_read(TWI_ACK) | twi_read(TWI_ACK) << 8);
    Y = (int)(int16_t)(twi_read(TWI_ACK) | twi_read(TWI_ACK) << 8);
    Z = (int)(int16_t)(twi_read(TWI_ACK) | twi_read(TWI_NACK) << 8);

    twi_stop();

    new_sensor_data = 1;
      
}
