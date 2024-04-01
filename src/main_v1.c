/***********************************************************************
 * 
 * Main function of firmware for antenna switch controller.
 * 
 * Version: 1 
 * 
 * MCU: ATmega328PB, 16 MHz
 *
 *Václav Kubeš
 *
 **********************************************************************/


/* Includes ----------------------------------------------------------*/

#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <util/delay.h> //for delay usage
#include <stdlib.h> //for itoa usage
#include <stdio.h>  //for printf usage
#include <math.h> //for rounding

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
char tx_msg[70];    //char arry for UART tx message 
uint8_t ant_switch_vect_old = 0;    //auxiliary variable for storing info about which ant. is on 


/**
 * @brief   Set the MCU IO pins to output mode and set them high to switch the antennas off 
*/
void ANT_init(){
    GPIO_mode_output(&DDRD, ANT01); //set the GPIO to the output
    GPIO_write_high(&PORTD,  ANT01);    //set GPIO high - (switch off the antenna)   
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

/**
 * @brief   Read current value from A unit and save it to diag_data struc 
*/
void get_I_A(){
    diag_data.I_A = ADC_I(I_DIAG1); 
}

/**
 * @brief   Read current value from B unit and save it to diag_data struc (if B unit not connected then 0.0)
 * 
 * @param   CB(uint8_t) indication of unit B presence (1 - Yes, 0 - No)
*/
void get_I_B(uint8_t CB){
    if(CB){
        diag_data.I_B = ADC_I(I_DIAG2);
    }else{
        diag_data.I_B = 0.0;
    }  
}

/**
 * @brief   Read current value of voltage and save it to diag_data struc 
*/
void get_U(){
    diag_data.U = ADC_U(); 
}

/**
 * @brief   Read current temperature from temp. sens. and save it to diag_data struc (if B unit not connected then 0.0)
 * 
 * @param   CB(uint8_t) indication of unit B presence (1 - Yes, 0 - No)
 * @param   which_temp(uint8_t) From which temp. sens. value should be read (TEMP_A, TEMP_B)
*/
void get_temp(uint8_t CB, uint8_t which_temp){
    if((CB == 1) && (which_temp == TEMP_B)){
        diag_data.temp_B = MCP9808_read_temp(TEMP_B);
    }else if((CB != 0) && (which_temp == TEMP_B)){
        diag_data.temp_B = 0.0;
    }else{
        diag_data.temp_A = MCP9808_read_temp(TEMP_A);
    }
}

/**
 * @brief   Read current azimuth and save it to diag_data struc 
*/
void get_compass(){
    raw_compass = HMC5883L_rawData(raw_compass);
    diag_data.azimuth = HMC5883L_azimuth(raw_compass.X, raw_compass.Y);
}

/**
 * @brief   Test if unit B is connected 
*/
void get_B_conn(){
    diag_data.CB = MCP9808_init(TEMP_B);
}

/**
 * @brief   Get all diag. data and save them to diag_data struct
 * 
 * @param CB(uint8_t)   indiacation of B unit connection (1 - Yes, 0 - No)   
*/
void get_all_data(uint8_t CB){
    get_temp(CB, TEMP_A);
    get_temp(CB, TEMP_B);
    
    get_compass();
    //diag_data.azimuth = 100.0;//diag_data.azimuth = HMC5883L_azimuth(raw_compass.X, raw_compass.Y);
    get_U();
    get_I_A();
    get_I_B(CB);  
}



/**
 * @brief   Convert all diag. data to string and send it throught UART 
*/
void send_all_data(){
    char str [9];
    ftoa(diag_data.I_A, str, 2, sizeof(str));   //convert float number to string
    strcpy(tx_msg, str);    //add it to resultant string
    strcat(tx_msg, ",");    //separate values by comma
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
    itoa(diag_data.ANT, str, 10);
    strcat(tx_msg, str);
    strcat(tx_msg, ","); 
    itoa(diag_data.CB, str, 10);
    strcat(tx_msg, str);
    strcat(tx_msg, "\n");  

    uart_puts(tx_msg);
}

/**
 * @brief   Switch off/on antennas according to toggle mask which indicates whether the satate should be changed
 *          or it remains the same
 * 
 * @param   toggle_mask(uint8_t)    Representing which ant. state should be toggled:
 *                                  In binary: 0b   x | x | x | all_off | ant5 | ant4 | ant3 | ant2 | ant1
 *                                  1 represents toggling of state of antenna corresponding to position of 1, 0 represents no change
*/
void switch_ant(uint8_t toggle_mask){
    uint8_t A_ant_pin [] = {ANT05, ANT04, ANT03, ANT02, ANT01};
    uint8_t B_ant_pin [] = {ANT15, ANT14, ANT13, ANT12, ANT11};
    uint8_t *B_ant_port [] = {&PORTB, &PORTB, &PORTD, &PORTC, &PORTC};
    if(toggle_mask & (1 << 5)){ //if All of is set then switch off all antennas
        ALL_ANT_OFF();
    }else{
        for(int8_t i = 4; i >= 0; i--){ //if antenna state should be changed then change the state
            if(toggle_mask & (1 << i)){
                GPIO_toggle(&PORTD, A_ant_pin[i]);
                //GPIO_toggle(B_ant_port[i], B_ant_pin[i]);
            }
        }
    }
}

/**
 * @brief   Function to respond to command received by UART
 * 
 * @param   req_msg(char)   Command received to serve
 * @param   req_msg_len(uint8_t)    Length of the command 
*/
void serve_request(char *req_msg, uint8_t req_msg_len){
    
    char str [req_msg_len + 1];
    char outp_str [12];
    char str_float [7];

    /*Copy command to sting to make easy the command recognition*/
    uint8_t i = 0;
    while (*(req_msg + i) != '\0') {
        str[i] = *(req_msg + i);
        i++;
    }
    str[i] = '\0';

    /*Recognize the command and respond to it adequate*/
    if(strstr(str, "AL") != NULL){
        get_all_data(diag_data.CB);
        send_all_data();

    }else if(strstr(str, "IA") != NULL){
        get_I_A();
        ftoa(diag_data.I_A, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "IA");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);

    }else if(strstr(str, "IB") != NULL){
        get_I_B(diag_data.CB);
        ftoa(diag_data.I_B, str_float, 1, sizeof(str_float));
        strcpy(outp_str, "IB");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);

}else if(strstr(str, "FU") != NULL){
        get_U();
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
        if(req_msg_len > 2){    //if the AN command is longer than 2
            uint8_t ant_switch_vect_new = 0;
            uint8_t off_all_flag = 0;
            uint16_t ant = 0;

            for(uint8_t i = 2; i < req_msg_len; i++){   //for each char after AN find which ant should be turn on
                if(off_all_flag == 1){
                    ant_switch_vect_new = (1 << 5);
                    diag_data.ANT = 0;
                    break;
                }

                switch (str[i]){
                    case '0':
                        off_all_flag = 1;   //if there is 0 switch off all ant.
                        break;
                    case '1':
                        ant_switch_vect_new |= (1 << 4);    //inidace which ant should be on
                        ant = (ant * 10) + 1;   //save it in decimal form
                        break;
                    case '2':
                        ant_switch_vect_new |= (1 << 3);
                        ant = (ant * 10) + 2;
                        break;
                    case '3':
                        ant_switch_vect_new |= (1 << 2);
                        ant = (ant * 10) + 3;
                        break;
                    case '4':
                        ant_switch_vect_new |= (1 << 1);
                        ant = (ant * 10) + 4;
                        break;
                    case '5':
                        ant_switch_vect_new |= 1; 
                        ant = (ant * 10) + 5;
                        break;
                    default:
                        uart_puts("ER\n");
                        break;
                }
            }
            switch_ant(ant_switch_vect_new ^ ant_switch_vect_old);  //switch antennas according to toggle mask calculated by XOR of new one and old one
            ant_switch_vect_old = ant_switch_vect_new;
            diag_data.ANT = ant;
        }
        itoa(diag_data.ANT, str_float, 10);  //send switched on antennas back via UART as confirmation
        strcpy(outp_str, "AN");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);

}else if(strstr(str, "CB") != NULL){
        get_B_conn();
        itoa(diag_data.CB, str_float, 10);
        strcpy(outp_str, "CB");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);

}else if(strstr(str, "!!") != NULL){
        uart_puts("YE");
}else{  //if unknown command then return it to UART
    uart_puts("\r\n→");
    uart_puts(str);
    uart_puts("←\r\n") ;
}
}


/**
 * @brief   Main function. Initialize all used sensors and MCU peripherals and
 *          listen to UART until command is received, then it is served.
*/
int main (void){
    /*Initialization of sensors and MCU peripherals*/
    ANT_init();
    GPIO_mode_output(&DDRC, LED);
    twi_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));   //Speed of UART is 9600 bd
    HMC588L_init();
    ADC_init();
    MCP9808_init(TEMP_A); 
    diag_data.CB = MCP9808_init(TEMP_B);    //according to unit B temp. sensor presence on TWI presence of unit B connection is determined
    sei();  //enable interrupts
    
    

    uart_puts(START_MSG);   //just info message about start

    /*Start of program indication*/
    GPIO_write_low(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_high(&PORTC, LED);
    _delay_ms(250);
    GPIO_write_low(&PORTC, LED);
  
    /*Indicate waiting for UART connect estabilishment by fast blinking*/
    TIM3_ovf_50ms();
    TIM3_ovf_enable();
    /*Wait untill '!!' is received by UART*/
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

    TIM3_ovf_disable(); //stop fast blinking
    TIM3_stop();
    
    uart_puts("YE");    //respond to "!!" to let HMI know the connection is working

    get_all_data(diag_data.CB); //measure all diag. data

    TIM1_ovf_1sec();
    TIM1_ovf_enable();
    
    char uart_get_msg [6];
    uint8_t n = 0;
    char c = '\0';  //char variable for reading new char from UART RC buffer
    
    /*infinite loop*/
    while(1){
        if(uart_available()){ //if UART received character
            c = uart_getc() & LWR_BYTE; //extracting the only character 
            if(c != '\n'){  //if the char. is not new line
                uart_get_msg[n] = c;    //add it to received command
                if(n < 6){
                    n++;
                }else{
                    n = 0;
                }          
            }else{  //if new line received it indicates that all comand has been sent and can be processed
                uart_get_msg[n] = '\0';             
                n = 0;             
                serve_request(uart_get_msg, strlen(uart_get_msg));  //act according to command 
            }
        }     
}
}

/* Interrupt service routines ----------------------------------------*/

/**********************************************************************
 * Function: Timer1 overflow interrupt
 * Purpose:  Blink with LED to proper functioning.
 **********************************************************************/
ISR(TIMER1_OVF_vect){
    GPIO_toggle(&PORTC, LED);
}

/**********************************************************************
 * Function: Timer3 compare match A interrupt
 * Purpose:  Blink with LED to proper functioning.
 **********************************************************************/
ISR(TIMER3_COMPA_vect){
    GPIO_toggle(&PORTC, LED);
}

