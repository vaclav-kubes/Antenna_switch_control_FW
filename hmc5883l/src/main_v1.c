#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <twi.h> // I2C library
//#include "timer.h"
#include "timer_my.h"
#include <uart.h>
#include <stdlib.h> 
#include <stdio.h>
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
//#define RX_DISABLE() UCSR0B &=  ~((1 << RXEN0) | (1<<RXCIE0));
//#define RX_START()  while(!(UCSR0A & (1<<TXC0))); UCSR0B &=  ~(1 << RXEN0); UCSR0B |= (1 << RXEN0) |(1<<RXCIE0); //; 
//#define RX_START()  while(!(UCSR0A & (1<<UDRE0))); UCSR0B &=  ~(1 << RXEN0); UCSR0B |= (1 << RXEN0) |(1<<RXCIE0); //; 

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
volatile struct d_data {
    float temp_A;
    float temp_B;
    float azimuth;
    float U;
    float I_A;
    float I_B;
    uint8_t CB;
    uint16_t ANT;
}diag_data;

struct data raw_compass;

char rx_msg[UART_RX_BUFFER_SIZE];
char tx_msg[70];
volatile uint8_t new_data = 0;
uint8_t ant_switch_vect_old = 0;

void ANT_init(){
    GPIO_mode_output(&DDRD, ANT01);
    GPIO_write_high(&PORTD,  ANT01);
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
    //RX_STOP();
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
    //uart_puts("1\n");
    diag_data.U = ADC_U();
    //uart_puts("2\n");
    diag_data.I_A = ADC_I(I_DIAG1);
    //uart_puts("3\n");
}

void get_U_I(uint8_t CB){
    //uart_puts("2\n");
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
    /*for(uint8_t i = 0; i <= UART_RX_BUFFER_SIZE; i++){
        *(var_to_save + i) = '\0';
    }*/
    //uart_puts("sub1\r\n");
    /*for(uint8_t i = 0; i < UART_RX_BUFFER_SIZE; i++){
        c =  uart_getc();
        //char str[10];
        //itoa(i, str, 10);
        //uart_puts(str);
        //uart_puts("\t");
        //uart_putc(c & 0x00FF);
        //uart_puts("\r\n");
        if ((c & 0x00FF) != '\n' || c != UART_NO_DATA){
            *(var_to_save + i) = (c & 0x00FF);
        }else if(c >= 0x0100){
            GPIO_toggle(&PORTD, ANT04);
            *(var_to_save + i)= '\0';
            break;
        }
        else{
            *(var_to_save + i)= '\0';
            break;
        }
    } */
    uint8_t i = 0;
    while(uart_available()){
        c = uart_getc();
        /*if((c&0xff00) == UART_BUFFER_OVERFLOW){
            GPIO_write_low(&PORTD, ANT05);
        }else if((c&0xff00) == UART_FRAME_ERROR){
           GPIO_write_low(&PORTD, ANT04); 
        }else if((c&0xff00) == UART_PARITY_ERROR){
           GPIO_write_low(&PORTD, ANT03); 
        }else if((c&0xff00) == UART_OVERRUN_ERROR){
           GPIO_write_low(&PORTD, ANT02); 
        }*/
        /*char str[9];
        itoa((c >> 8), str, 2);
        uart_puts(str);
        uart_puts("\r\n");*/
        if(((c & 0x00FF) != '\n') && (c < 0x0100)){
            *(var_to_save + i) = (c & 0x00FF);
            i++;
            /*char ch [2] = {c & 0x00FF, '\0'};
            uart_puts(ch);*/
        }else{
            //uart_puts("\r\n");
            
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
                //GPIO_toggle(B_ant_port[i], B_ant_pin[i]);
            }
    }
    }
}


void serve_request( char *req_msg, uint8_t req_msg_len){//, uint8_t req_msg_len
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
        //GPIO_toggle(&PORTD, ANT05);
    }else if(strstr(str, "IB") != NULL){
        //get_U_I(diag_data.CB);
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
        //get_U_I(diag_data.CB);
        uint8_t len = strlen(str);
        //itoa(len, str_float, 10);
        //uart_puts(str);
        //uart_puts("\n");
        if(len > 2){
            uint8_t ant_switch_vect_new = 0;
            uint8_t off_all_flag = 0;
            diag_data.ANT = 0;
            for(uint8_t i = 2; i < len; i++){
                if(off_all_flag){
                    ant_switch_vect_new = (1 << 5);
                    break;
                }
                //uart_putc(str[i]);
                switch (str[i])
                {
                case '0':
                    off_all_flag = 1;
                    //diag_data.ANT = 0;
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
        sprintf(str_float, "%u", diag_data.ANT);   //diag_data.ANT, str_float, 10
        //uart_puts("→\t");
        strcpy(outp_str, "AN");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "CB") != NULL){
        //get_U_I(diag_data.CB);
        itoa(diag_data.CB, str_float, 10);
        strcpy(outp_str, "CB");
        strcat(outp_str, str_float);
        strcat(outp_str, "\n");
        uart_puts(outp_str);
}else if(strstr(str, "!!") != NULL){
        uart_puts("YE");
}else{
    //GPIO_toggle(&PORTD, ANT01);
    uart_puts("\r\n→");
    uart_puts(str);
    uart_puts("←\r\n") ;
    //uart_puts(str);
}
}


int main (void){
    char str [12];
    //uint8_t old_state = 0;
    //TCCR3A = 0;
    //TCCR4A = 0;
    ANT_init();
    GPIO_mode_output(&DDRC, LED);
    twi_init();
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    ADC_init();
    sei();
    
    //RX_STOP();
    uart_puts("START\r\n");
    //_delay_ms(20);
    //RX_START();
    uart_rx_str(rx_msg);
    //RX_STOP();
    uart_puts("→\r\n");
    //RX_STOP();
    uart_puts(rx_msg);
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
    //while((uart_getc() & 0x00ff) != '!');
    while(1){
        while(uart_available() < 3);
        uart_rx_str(rx_msg);
        if(strstr(rx_msg, "!!") != NULL){
            break;
        }
    }
    TIM3_ovf_disable();
    TIM3_stop();
    
    //RX_STOP();
    uart_puts("YE");
    //get_all_data(diag_data.CB);
    //uart_puts("YE");
    //diag_data.U = ADC_U();
    //send_all_data();
    //_delay_ms(50);
    //RX_START();
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
    get_all_data(diag_data.CB);

    /*
    if(strstr(rx_msg, "AL") == NULL){
        TIM1_ovf_4sec();
        TIM1_ovf_enable();
    }else{ 
        send_all_data();
        
    }*/
    /*while(uart_available() < 2);
    uart_rx_str(rx_msg);
    if(strstr(rx_msg, "AL") != NULL){
        get_all_data(diag_data.CB);
        send_all_data();
    }*/
    TIM1_ovf_1sec();
    TIM1_ovf_enable();
    
    uint8_t a_old = 0;
    char uart_get_msg [6];
    uint8_t n = 0;
    char c = '\0';
    while(1){
        if(uart_available()){
            c = uart_getc() & 0x00FF;
            //RX_DISABLE();
            //uart_putc(c);
            //GPIO_toggle(&PORTD, ANT05);
            if(c != '\n'){ //po testovani nutno zmenit na \n
                uart_get_msg[n] = c;
                if(n < 6){
                    n++;
                }else{
                    n = 0;
                }
                
                
            }else{
                /*if(n == 0){
                    uart_get_msg[n] = '\0';
                }else{
                    uart_get_msg[n] = '\0';
                }*/
                uart_get_msg[n] = '\0';
                //uart_get_msg[n+2] = 'X';
                n = 0;
                //uart_puts(uart_get_msg);
                /*uart_puts("\r\nrx: ");
                uart_puts(uart_get_msg);
                uart_puts("\r\n");*/
                serve_request(uart_get_msg, strlen(uart_get_msg));
            }
            //n++;
            
        }

        //uint8_t a = uart_available();
        //if(a > 2){
            //sprintf(str, "%d", a);
            //uart_puts(str);
            //uart_puts("\r\n");
            /*uart_puts("\r\n");
            sprintf(str, "%d", a);
            uart_puts(str);*/
            //uart_puts("\r\n");

            //uart_rx_str(rx_msg);
            //uart_puts(rx_msg);
            //uart_puts("←");
            //serve_request(rx_msg, sizeof(rx_msg));
            
            
            /*sprintf(str, "%d", uart_available());
            uart_puts(str);*/
            //uart_puts("\r\n");

        //}/*else{
            //GPIO_write_low(&PORTD, ANT03);
            //if(a != a_old){
                /*itoa(UART_LastRxError, str, 2);
                uart_puts(str);
                uart_puts("\r\n");*/
                //sprintf(str, "%d", a);
                //uart_puts("AVAILABLE: ");
                //uart_puts(str);
                //uart_puts("\r\n");
                //a_old = a;
            //}
            //}
            /*else if((a <= 2) && (a > 0)){
            sprintf(str, "%d", a);
            uart_puts(str);
            uart_puts("\r\n");
        }*/
            /*uart_rx_str(rx_msg);
                RX_STOP();
                //strcat(rx_msg, "↑(Tx)");
                uart_puts(rx_msg);*/
                //_delay_ms(50);
                //RX_START();
            /*if(!x){
                //RX_STOP();
                itoa(x, str, 10);
                uart_puts(str);
                //RX_STOP();
                uart_puts("RX\r\n");
                //_delay_ms(50);
               
                x++;
            }else{
                uart_rx_str(rx_msg);
                strcat(rx_msg, "↑(Tx)→");
                itoa(x, str, 10);
                
                //RX_STOP();
                
                uart_puts(rx_msg);
                //RX_STOP();
                uart_puts(str);
                //RX_STOP();
                uart_puts("\r\n");
                //_delay_ms(50);
                
                x = 0;
            }*/
            /*uart_rx_str(rx_msg);
            uart_puts(rx_msg);
            uart_puts("\t→\t");
            
            /*itoa(strnlen(UART_RxBuf,UART_TX_BUFFER_SIZE), str, 10);
            uart_puts(str);
            uart_puts("→");*/
            /*uint16_t t = (uint16_t)UART_RxTail;
            itoa(t, str, 10);
            uart_puts(str);*/
            /*char c = uart_getc();
            uart_puts(c);*/
            
            //uart_puts("\r\n");
            //uint16_t c = uart_getc();
            /*if((c&0x00ff) > 0){
                //uart_putc(0x00ff& c);
                //GPIO_toggle(&PORTD, ANT05);
            }*/
        /*if(strlen(UART_RxBuf)>2){
            itoa(strlen(UART_RxBuf), str, 10);
            uart_puts(str);
            uart_puts("→");
            itoa(strlen(UART_RxTail), str, 10);
            uart_puts(str);
            uart_puts("\r\n");
        uart_rx_str(rx_msg);
        if(strstr(rx_msg, "AL") != NULL){
            get_all_data(diag_data.CB);
            send_all_data();
        }
        }*/
       //uint16_t t = (uint16_t)UART_RxTail;
        //uart_puts(str);
        //uart_rx_str(rx_msg);
        //t = (uint16_t)UART_RxTail;
        //itoa(t, str, 10);
        //uart_puts(str);
        /*if(uart_available() > 2){
            uart_rx_str(rx_msg);  
            if(strstr(rx_msg, "AL") != NULL){
                get_all_data(diag_data.CB);
                send_all_data();
                GPIO_toggle(&PORTD, ANT05);
            }
        }*/
        
        /*if(new_data == 1){
            //GPIO_toggle(&PORTC, LED);
            diag_data.U = ADC_U();
            diag_data.I_A = ADC_I(I_DIAG1);
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
            uart_puts("\r\n");
            ftoa(diag_data.I_A, str, 2, sizeof(str));
            uart_puts(str);
            uart_puts("\r\n");
           
            //RX_START;

            new_data = 0;
        }*/
    
}

}

ISR(TIMER1_OVF_vect){
    static uint8_t sec_cnt;
    //GPIO_toggle(&PORTC, LED);
    
    if(sec_cnt < 60){
        sec_cnt++;
    }else{
        sec_cnt = 0;
        new_data++;
    }
    //diag_data.U = ADC_U();
    //diag_data.I_A = ADC_I(I_DIAG1);
    //ADC_read_U();
    //diag_data.U = ADC_get_U();
    //ADC_read_I(I_DIAG1);
    //diag_data.I_A = ADC_get_I(I_DIAG1);
    
}

ISR(TIMER3_OVF_vect){
    TCNT3 = 50036;
    GPIO_toggle(&PORTC, LED);
}

