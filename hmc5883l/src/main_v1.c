
/* Includes ----------------------------------------------------------*/

#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <util/delay.h> //for delay usage
#include <stdlib.h> //for itoa usage
#include <stdio.h>  //for printf usage

#include <twi.h> // I2C library for AVR (modified for ATmega328PB)
#include <uart.h>   //UART library AVR  (modified for ATmega328PB)
#include <gpio.h>   //GPIO library ATmega328

#include <adc.h> //Custom ADC library for antenna switch controller outside unit
#include <hmc5883l.h>   //Custom Electronics compass module library
#include <mcp9808.h>    //Custom MCP9808 temperature sensor library
#include <ftoa_my.h>
#include "timer.h"  //Custom Timer library for ATmega328PB
#include "antenna_switch_IO.h"  //Custom library for Antenna switch controller outside unit

#ifndef F_CPU
# define F_CPU 16000000UL  // CPU frequency in Hz required for UART_BAUD_SELECT
#endif

#define START_MSG "START\r\n"
#define  LWR_BYTE 0x00FF
/* Global variables definitions/declarations -------------------------*/

/*Structure for storing all diagnostic data*/
volatile struct d_data {
    float temp_A;   //temp. from A unit
    float temp_B;   //temp. from B unit
    float azimuth;  //azimuth from e-compass
    float U;    //measured phantom voltage
    float I_A;  //measured current to LNAs at A unit
    float I_B;  //measured current to LNAs at B unit
    uint8_t CB; //indication of B unit connection
    uint16_t ANT;   //antennas currently in use
}diag_data;

struct data raw_compass;    //struc. for saving X, Y and Z values of raw mag. strength from e-compass
char tx_msg[70];
uint8_t ant_switch_vect_old = 0;

void ANT_init(){
    GPIO_mode_output(&DDRD, ANT01);
    GPIO_write_high(&PORTD,  ANT01);
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
    if(CB){
        diag_data.temp_B = MCP9808_read_temp(TEMP_B);
        diag_data.I_B = ADC_I(I_DIAG2);
    }
    //raw_compass = HMC5883L_rawData(raw_compass);
    diag_data.azimuth = 100.0;//diag_data.azimuth = HMC5883L_azimuth(raw_compass.X, raw_compass.Y);
    diag_data.U = ADC_U();
    diag_data.I_A = ADC_I(I_DIAG1);
}

void get_U_I(uint8_t CB){
    if(CB){
        diag_data.I_B = ADC_I(I_DIAG2);
    }
    diag_data.I_A = ADC_I(I_DIAG1); 
    diag_data.U = ADC_U();
}

void get_temp(uint8_t CB, uint8_t temp_A_B){
    if(temp_A_B == TEMP_B){
        if(CB){
            diag_data.temp_B = MCP9808_read_temp(TEMP_B);
       }else{
            diag_data.temp_B = 0.0;
       } 
    }else if (temp_A_B == TEMP_A){
        diag_data.temp_A = MCP9808_read_temp(TEMP_A);
    } 
}

void get_compass(){
    raw_compass = HMC5883L_rawData(raw_compass);
    diag_data.azimuth = HMC5883L_azimuth(raw_compass.X, raw_compass.Y);
}

void uart_rx_str(char *var_to_save){
    uint16_t c;
    uint8_t i = 0;
    while(uart_available()){
        c = uart_getc();
        if(((c & 0x00FF) != '\n') && (c < 0x0100)){
            *(var_to_save + i) = (c & 0x00FF);
            i++;
        }else{   
            break;
        }
    }
    *(var_to_save + i)= '\0'; 
}

void switch_ant(uint8_t toggle_mask){
    uint8_t A_ant_pin [] = {ANT05, ANT04, ANT03, ANT02, ANT01};
    uint8_t B_ant_pin [] = {ANT15, ANT14, ANT13, ANT12, ANT11};
    uint8_t *B_ant_port [] = {&PORTB, &PORTB, &PORTD, &PORTC, &PORTC};
    if(toggle_mask & (1 << 5)){
        ALL_ANT_OFF();
    }else{
        for(int8_t i = 4; i >= 0; i--){
            if(toggle_mask & (1 << i)){
                GPIO_toggle(&PORTD, A_ant_pin[i]);
            }
    }
    }
}


void serve_request( char *req_msg, uint8_t req_msg_len){
    uint8_t i = 0;
    char str [req_msg_len + 1];
    char outp_str [10];
    char str_float [7];

    while (*(req_msg + i) != '\0') {
        str[i] = *(req_msg + i);
        i++;
    }
    str[i] = '\0';

    if(strstr(str, "AL") != NULL){
        get_all_data(diag_data.CB);
        send_all_data();
    }else if(strstr(str, "IA") != NULL){
        get_U_I(diag_data.CB);
        ftoa(diag_data.I_A, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "IA");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
    }else if(strstr(str, "IB") != NULL){
        ftoa(diag_data.I_B, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "IB");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "FU") != NULL){
        //get_U_I(diag_data.CB);
        ftoa(diag_data.U, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "FU");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "EC") != NULL){
        get_compass();
        ftoa(diag_data.azimuth, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "EC");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "TA") != NULL){
        get_temp(diag_data.CB, TEMP_A);
        ftoa(diag_data.temp_A, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "TA");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "TB") != NULL){
        get_temp(diag_data.CB, TEMP_B);
        ftoa(diag_data.temp_B, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "TB");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "AN") != NULL){
        uint8_t len = strlen(str);
        if(len > 2){
            uint8_t ant_switch_vect_new = 0;
            uint8_t off_all_flag = 0;
            diag_data.ANT = 0;
            for(uint8_t i = 2; i < len; i++){
                if(off_all_flag){
                    ant_switch_vect_new = (1 << 5);
                    break;
                }
                switch (str[i])
                {
                case '0':
                    off_all_flag = 1;
                    break;
                case '1':
                    ant_switch_vect_new |= (1 << 4);
                    diag_data.ANT = diag_data.ANT * 10 + 1;
                    break;
                case '2':
                    ant_switch_vect_new |= (1 << 3);
                    diag_data.ANT = diag_data.ANT * 10 + 2;
                    break;
                case '3':
                    ant_switch_vect_new |= (1 << 2);
                    diag_data.ANT = diag_data.ANT * 10 + 3;
                    break;
                case '4':
                    ant_switch_vect_new |= (1 << 1);
                    diag_data.ANT = diag_data.ANT * 10 + 4;
                    break;
                case '5':
                    ant_switch_vect_new |= 1;
                    diag_data.ANT = diag_data.ANT * 10 + 5;
                    break;
                default:
                    uart_puts("ER\n");
                    break;
                }
            }
            switch_ant(ant_switch_vect_new ^ ant_switch_vect_old);
            ant_switch_vect_old = ant_switch_vect_new;
        }
        sprintf(str_float, "%u", diag_data.ANT);
        strcpy(outp_str, "AN");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "CB") != NULL){
        itoa(diag_data.CB, str_float, 10);
        strcpy(outp_str, "CB");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "!!") != NULL){
        uart_puts("YE");
}else{
    uart_puts("\r\n→");
    uart_puts(str);
    uart_puts("←\r\n") ;
}
}


int main (void){
    
    ANT_init();
    GPIO_mode_output(&DDRC, LED);
    twi_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    ADC_init();
    sei();
    
    MCP9808_init(TEMP_A);
    diag_data.CB = MCP9808_init(TEMP_B);

    uart_puts(START_MSG);

    GPIO_write_low(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_high(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_low(&PORTC, LED);
  
    TIM3_ovf_50ms();
    TIM3_ovf_enable();
    
    uint8_t conn = 0;
    while(1){
        if(uart_available()){
            char c = uart_getc() & LWR_BYTE;
            if((c == '!') || (c  == '\n')){
                conn++;
            }else{
                conn = 0;
            }
        }
        if(conn == 3){
            break;
        }
    }

    TIM3_ovf_disable();
    TIM3_stop();
    
    uart_puts("YE");

    get_all_data(diag_data.CB);

    //TIM1_ovf_1sec();
    //TIM1_ovf_enable();
    
    char uart_get_msg [6];
    uint8_t n = 0;
    char c = '\0';
    
    while(1){
        if(uart_available()){
            c = uart_getc() & 0x00FF;
            if(c != '\n'){ //po testovani nutno zmenit na \n
                uart_get_msg[n] = c;
                if(n < 6){
                    n++;
                }else{
                    n = 0;
                }          
            }else{
                uart_get_msg[n] = '\0';             
                n = 0;             
                serve_request(uart_get_msg, strlen(uart_get_msg));
            }
        }     
}
}

/*ISR(TIMER1_OVF_vect){
    static uint8_t sec_cnt;
    
    if(sec_cnt < 60){
        sec_cnt++;
    }else{
        sec_cnt = 0;
        new_data++;
    }  
}*/

ISR(TIMER3_COMPA_vect){
    //TCNT3 = 50036;
    GPIO_toggle(&PORTC, LED);
}

