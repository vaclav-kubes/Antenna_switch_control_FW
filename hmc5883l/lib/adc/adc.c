
#include <adc.h>
#include "antenna_switch_IO.h"
#include <util/delay.h>

#define VREF 5
#define VGAP 1.1
#define AMP 50
#define R_SENS 0.5
#define R_RATIO 12

//#define TIM2_10ms_start() TCNT2 = 99; TCCR2B = (1<<CS22) | (1<<CS21) | (1<<CS20); TCCR2A = 0x00; TIMSK2 |= 1<<TOIE2; 
//#define TIM2_stop TIMSK2() &= ~(1<<TOIE2);

volatile static uint16_t adc_val = 0;
//volatile static uint8_t n = 0;
volatile static uint8_t adc_end_flg = 0;

void ADC_init(){
    ADMUX = 0x00;
	ADCSRA = 0x00;
	ADCSRB = 0x00;
    ADCSRA = ADCSRA | 1<<ADEN;//ADC ENABLE
    ADCSRA = ADCSRA | (1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0); //adc prescaler 128
    ADCSRA = ADCSRA | 1<<ADIE;//adc interrupt enble
    DIDR0 = DIDR0 | (1<< I_DIAG1 | 1<< I_DIAG2 | 1<< U_DIAG); //disable digital input on pins used for adc
    ADMUX = ADMUX | 1<<REFS0 | 0<<ADLAR  | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0);//adc internal reference 5V | (1<<REFS1 | 1<<REFS0); 
    ADCSRA = ADCSRA | 1<< ADSC;

  }  

void ADC_read_I(uint8_t i_diag_pin){
    if(ADMUX != 0b01000110 || ADMUX != 0b01000011){
        if (i_diag_pin == I_DIAG1){
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 0<<MUX0); //I_diag1 PE2
        }else{
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 0<<MUX2 | 1<<MUX1 | 1<<MUX0); //I_diag2 PC3
        }
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS0);//adc internal reference 5V
    }
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion
}

void ADC_read_U(){
    //ADMUX &= 0b11110000;
    if(ADMUX != 0b11000111){
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //U_diag PE3
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
    }
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion
}

float ADC_get_I(uint8_t i_diag_pin){
    /*if (i_diag_pin == I_DIAG1){
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 0<<MUX0); //I_diag1 PE2
    }else{
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 0<<MUX2 | 1<<MUX1 | 1<<MUX0); //I_diag2 PC3
    }
    ADMUX = ADMUX | (1<<REFS0);//adc internal reference 5V
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion*/
    
    
    while(!adc_end_flg);
    //float adc_val_avg = adc_val_acc / 10;
    float u = VREF * (adc_val / 1024.0);
    //adc_val = 0;
    u = u / AMP;
    return  (u / R_SENS)*1000;//(u / R_SENS);
}

float ADC_get_U(){
    /*ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //U_diag PE3
    ADMUX = ADMUX | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion*/

    while(!adc_end_flg);
    //float adc_val_avg = adc_val_acc / 10;
    
    float u = VGAP * (adc_val / 1024.0);
    //adc_val = 0;
    return  (u * R_RATIO);
}

float ADC_U(){
    if(ADMUX != 0b11000111){
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //U_diag PE3
        _delay_ms(10);
    }
    //ADCSRA = ADCSRA | 1<< ADSC;
    
    
    for(uint8_t i = 0; i < 2; i++){
        ADCSRA = ADCSRA | 1<< ADSC;//adc start conversion
        while(!(ADCSRA & (1<<ADIF)));
        //adc_val = ADC;
        //uart_putc('a');
    }
    adc_val = ADC;
    /*ADCSRA = ADCSRA | 1<< ADSC;//adc start conversion
    while(!(ADCSRA & (1<<ADIF)));
    //adc_val = ADC;*/
    float u = VGAP * (adc_val / 1024.0);
    //adc_val = 0;
    return  (u * R_RATIO);
}

float ADC_I(uint8_t i_diag_pin){
    //uart_puts("2.2");
    if((ADMUX != 0b01000110) || (ADMUX != 0b01000011)){
        if (i_diag_pin == I_DIAG1){
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 0<<MUX0); //I_diag1 PE2
        }else{
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 0<<MUX2 | 1<<MUX1 | 1<<MUX0); //I_diag2 PC3
        }
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS0);//adc internal reference 5V
        _delay_ms(10);
    }
    //uart_puts("2.2");
    for(uint8_t i = 0; i < 2; i++){
        ADCSRA = ADCSRA | 1<< ADSC;//adc start conversion
        while(!(ADCSRA & (1<<ADIF))); 
        //uart_puts('a');
    }
    //uart_puts("2.2");
    adc_val = ADC;
    float u = VREF * (adc_val / 1024.0);
    //adc_val = 0;
    u = u / AMP;
    return  (u / R_SENS)*1000;
}

/*float ADC_U2(){
    if(ADMUX != 0b11000111){
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //U_diag PE3
        TIM2_10ms_start();
    }
    //ADCSRA = ADCSRA | 1<< ADSC;
    
    
    for(uint8_t i = 0; i < 11; i++){
        ADCSRA = ADCSRA | 1<< ADSC;//adc start conversion
        while(!(ADCSRA & (1<<ADIF)));
        //adc_val = ADC;
        //uart_putc('a');
    }
    adc_val = ADC;
    /*ADCSRA = ADCSRA | 1<< ADSC;//adc start conversion
    while(!(ADCSRA & (1<<ADIF)));
    //adc_val = ADC;
    float u = VGAP * (adc_val / 1024.0);
    //adc_val = 0;
    return  (u * R_RATIO);
}*/

ISR(ADC_vect){
    //ADCSRA |= (1<<ADIF);
    //adc_val = ADC;
    /*if(n < 2){
        adc_end_flg = 0;
        n++;
        //ADCSRA = ADCSRA | 1 <<ADIF;
        ADCSRA = ADCSRA | 1<<ADSC;
    }else{
        n = 0;
        adc_val = ADC;
        //char str[8];
        //itoa(adc_val, str, 10);
        //uart_puts(str);
        adc_end_flg = 1;
    }*/
    /*char str[11];
    itoa(adc_val, str, 2);
    uart_puts(str);*/
}