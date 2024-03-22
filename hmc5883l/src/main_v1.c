#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h> // I2C library
//#include "timer.h"
#include "timer_my.h"
#include <uart.h>
#include <stdlib.h> 
#include <util/delay.h>
#include <gpio.h>
//#include <math.h>
#include <string.h>
#include <adc.h>
#include <hmc5883l.h>
#include <mcp9808.h>
#include <ftoa_my.h>
#include "antenna_switch_IO.h"

#ifndef F_CPU
# define F_CPU 16000000UL  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif
//#define RX_STOP UCSR0B &=  ~((1 << RXEN0) | (1<<RXCIE0));
//#define RX_START  while((UCSR0A & (1<<TXC0)) != (1<<TXC0)); UCSR0B |= (1 << RXEN0) |(1<<RXCIE0); //; 

/*volatile float temp_A;
volatile float temp_B;
volatile float azimuth;
volatile float U;
volatile float I_A;
volatile float I_B;

uint8_t BC;
uint8_t ANT;*/
                             //0  1  2  3   4  5  6 7/
//volatile float diag_data[8]; //IA,IB,TA,TB,FU,EC,AN,CB
struct d_data {
    float temp_A;
    float temp_B;
    float azimuth;
    float U;
    float I_A;
    float I_B;
    uint8_t CB;
    uint8_t ANT;
}diag_data;

struct data raw_compass;

char rx_msg[UART_RX_BUFFER_SIZE];
char tx_msg[70];
volatile uint8_t new_data = 0;

void ANT_init(){
    GPIO_mode_output(&DDRD, ANT01);
    GPIO_write_low(&PORTD,  ANT01);
    //GPIO_write_high(&PORTD,  ANT01);
    GPIO_mode_output(&DDRD, ANT02);
    GPIO_write_high(&PORTD, ANT02);
    GPIO_mode_output(&DDRD, ANT03);
    GPIO_write_high(&PORTD, ANT03);
    GPIO_mode_output(&DDRD, ANT04);
    GPIO_write_high(&PORTD, ANT04);
    GPIO_mode_output(&DDRD, ANT05);
    GPIO_write_high(&PORTD, ANT05);

    GPIO_mode_output(&DDRC, ANT11);
    GPIO_write_high(&PORTC, ANT11);
    GPIO_mode_output(&DDRC, ANT12);
    GPIO_write_high(&PORTC, ANT12);
    GPIO_mode_output(&DDRD, ANT13);
    GPIO_write_high(&PORTD, ANT13);
    GPIO_mode_output(&DDRB, ANT14);
    GPIO_write_high(&PORTB, ANT14);
    GPIO_mode_output(&DDRB, ANT15);
    GPIO_write_high(&PORTB, ANT15);
}

void send_all_data(){
    char str [9];
    ftoa(diag_data.I_A, str, 2, sizeof(str));
    strcpy(tx_msg, str);
    strcat(tx_msg, ",");
    ftoa(diag_data.I_B, str, 2, sizeof(str));
    strcat(tx_msg, str);
    strcat(tx_msg, ","); 
    ftoa(diag_data.temp_A, str, 2, sizeof(str));
    strcat(tx_msg, str);
    strcat(tx_msg, ","); 
    ftoa(diag_data.temp_B, str, 2, sizeof(str));
    strcat(tx_msg, str);
    strcat(tx_msg, ","); 
    ftoa(diag_data.U, str, 2, sizeof(str));
    strcat(tx_msg, str);
    strcat(tx_msg, ","); 
    ftoa(diag_data.azimuth, str, 2, sizeof(str));
    strcat(tx_msg, str);
    strcat(tx_msg, ",");
    ftoa(diag_data.ANT, str, 2, sizeof(str));
    strcat(tx_msg, str);
    strcat(tx_msg, ","); 
    itoa(diag_data.CB, str, 10);
    strcat(tx_msg, str);
    strcat(tx_msg, "\n");  

    uart_puts(tx_msg);
}

void get_all_data(uint8_t CB){
    diag_data.temp_A = MCP9808_read_temp(TEMP_A);
    if(CB == 1){
        diag_data.temp_B = MCP9808_read_temp(TEMP_B);
    }
    //raw_compass = HMC5883L_rawData(raw_compass);
    diag_data.azimuth = 100.0;//diag_data.azimuth = HMC5883L_azimuth(raw_compass.X, raw_compass.Y);
    uart_puts("1\n");
    diag_data.U = ADC_U();
    uart_puts("2\n");
    diag_data.I_A = ADC_I(I_DIAG1);
    //uart_puts("3\n");
    diag_data.I_B = ADC_I(I_DIAG2);
}

void uart_rx_str(char *var_to_save){
    uint16_t c;
    /*for(uint8_t i = 0; i <= UART_RX_BUFFER_SIZE; i++){
        *(var_to_save + i) = '\0';
    }*/
    //uart_puts("sub1\r\n");
    for(uint8_t i = 0; i <= UART_RX_BUFFER_SIZE; i++){
        c =  uart_getc();
        //char str[10];
        //itoa(i, str, 10);
        //uart_puts(str);
        //uart_puts("\t");
        //uart_puts(c & 0x00FF);
        //uart_puts("\r\n");
        if ((c & 0x00FF) != '\n' || c != UART_NO_DATA){
            *(var_to_save + i) = (c & 0x00FF);
        }else{
            *(var_to_save + i)= '\0';
            break;
        }
    } 
}


int main (void){
    char str [12];
    //TCCR3A = 0;
    //TCCR4A = 0;
    ANT_init();
    GPIO_mode_output(&DDRC, LED);
    twi_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    ADC_init();
    sei();
    
    //RX_STOP;
    uart_puts("START\r\n");
    //_delay_ms(20);
    //RX_START;
    //HMC588L_init();
    GPIO_write_low(&PORTC, LED);
    //GPIO_write_low(&PORTD, ANT01);
    MCP9808_init(TEMP_A);
    diag_data.CB = MCP9808_init(TEMP_B);

    
    //GPIO_write_high(&PORTC, LED);
    //uart_puts("1\r\n");
    /*temp_A = MCP9808_read_temp(TEMP_A);
    temp_B = MCP9808_read_temp(TEMP_B);
    raw_compass = HMC5883L_rawData(raw_compass);
    azimuth = HMC5883L_azimuth(raw_compass.X, raw_compass.Y);
    U = ADC_get_U();
    I_A = ADC_get_I(I_DIAG1);
    I_B = ADC_get_I(I_DIAG2);*/
    //get_all_data(diag_data.CB);
    //GPIO_write_low(&PORTC, LED);
    //uart_puts("2\r\n");
    _delay_ms(250);
    //uart_rx_str(rx_msg);
    
    //uart_puts("3\r\n");
    GPIO_write_high(&PORTC, LED);
    _delay_ms(250);
    /*RX_STOP;
    uart_puts("\r\n");
    uart_puts(rx_msg);
    RX_START;*/
    //GPIO_write_low(&PORTD, ANT01);
    //uart_puts("4\r\n");
    GPIO_write_low(&PORTC, LED);
  
    TIM3_ovf_50ms();
    TIM3_ovf_enable();
    while((uart_getc() & 0x00ff) != '!');
    TIM3_ovf_disable();
    TIM3_stop();
    //RX_STOP;
    uart_puts("YE");
    //get_all_data(diag_data.CB);
    //uart_puts("YE");
    //diag_data.U = ADC_U();
    send_all_data();
    //RX_START;
    /*while(strchr(rx_msg, '!') == NULL){
        uart_rx_str(rx_msg);
    }*/

    /*GPIO_write_high(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_low(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_high(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_low(&PORTC, LED);
    _delay_ms(300);
    GPIO_write_high(&PORTC, LED);*/
    /*RX_STOP;
    send_all_data();
    _delay_ms(20);
    RX_START;*/
    //uart_rx_str(rx_msg);
    /*RX_STOP;
    uart_puts("→\r\n");
    uart_puts(rx_msg);
    RX_START;*/

    /*
    if(strstr(rx_msg, "AL") == NULL){
        TIM1_ovf_4sec();
        TIM1_ovf_enable();
    }else{ 
        send_all_data();
        
    }*/

    TIM1_ovf_1sec();
    TIM1_ovf_enable();

    while(1){
        if(new_data == 1){
            //GPIO_toggle(&PORTC, LED);
            diag_data.U = ADC_U();
            ftoa(diag_data.U, str, 2, sizeof(str));
            //RX_STOP;
            
            uart_puts(str);
            uart_puts("\r\n");
            /*itoa(ADCH, str, 2);
            uart_puts(str);
            itoa(ADCL, str, 2);
            uart_puts(str);
            uart_puts("\r\n");*/
            /*itoa(ADMUX, str, 2);
            uart_puts(str);
            uart_puts("\r\n");*/
            ftoa(diag_data.I_A, str, 2, sizeof(str));
            uart_puts(str);
            uart_puts("\r\n");
           
            //RX_START;

            new_data = 0;
        }
    }
}



ISR(TIMER1_OVF_vect){
    GPIO_toggle(&PORTC, LED);
    //diag_data.U = ADC_U();
    //diag_data.I_A = ADC_I(I_DIAG1);
    //ADC_read_U();
    //diag_data.U = ADC_get_U();
    //ADC_read_I(I_DIAG1);
    //diag_data.I_A = ADC_get_I(I_DIAG1);
    new_data = 1;
}

ISR(TIMER3_OVF_vect){
    TCNT3 = 50036;
    GPIO_toggle(&PORTC, LED);
}