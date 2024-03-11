
#include <adc.h>
#include "antenna_switch_IO.h"
#define VREF 5
#define VGAP 1.1
#define AMP 50
#define R_SENS 0.5
#define R_RATIO 12

volatile static uint16_t adc_val_acc;
volatile static uint8_t n = 0;
volatile static uint8_t adc_end_flg = 0;

void ADC_init(){
    DIDR0 = DIDR0 | (1<< I_DIAG1 | 1<< I_DIAG2 | 1<< U_DIAG); //disable digital input on pins used for adc
    ADMUX = ADMUX | 1<<REFS0;//adc internal reference 5V | (1<<REFS1 | 1<<REFS0); 
    ADCSRA = ADCSRA | (1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0); //adc prescaler 128
    ADCSRA = ADCSRA | 1<<ADIE;//adc interrupt enble
    ADCSRA = ADCSRA | 1<<ADEN;//ADC ENABLE
}

void ADC_read_I(uint8_t i_diag_pin){
    if (i_diag_pin == I_DIAG1){
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 0<<MUX0); //I_diag1 PE2
    }else{
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 0<<MUX2 | 1<<MUX1 | 1<<MUX0); //I_diag2 PC3
    }
    ADMUX = ADMUX | (1<<REFS0);//adc internal reference 5V
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion
}

void ADC_read_U(){
    //ADMUX &= 0b11110000;
    ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //U_diag PE3
    ADMUX = ADMUX | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion
}

float ADC_get_I(){
    while(!adc_end_flg);
    float adc_val_avg = adc_val_acc / 10;
    float u = VREF * adc_val_avg / 1024;
    u = u / AMP;
    return  adc_val_avg;//(u / R_SENS);
}

float ADC_get_U(){
    while(!adc_end_flg);
    float adc_val_avg = adc_val_acc / 10;
    adc_val_acc = 0;
    float u = VGAP * adc_val_avg / 1024;
    return  adc_val_avg;//(u * R_RATIO);
}

ISR(ADC_vect){
    if(n > 0 && n <= 10){
        adc_end_flg = 0;
        adc_val_acc += ADC;
        n++;
        /*if(n == 11){
            n = 0;
            adc_end_flg = 1;
        }else{
        ADCSRA = ADCSRA | 1<<ADSC;}*/
        ADCSRA = ADCSRA | 1<<ADSC;
    }else if (n == 0){
        n++;
        ADCSRA = ADCSRA | 1<<ADSC;
    }else{
        n = 0;
        //adc_val_acc = 0;
        adc_end_flg = 1;
    }  
    //adc_val_acc = ADC;
}