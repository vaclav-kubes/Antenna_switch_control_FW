/***********************************************************************
 * 
 * ADC library for ATMEGA328PB in antenna switch controller.
 * 
 * ATmega328PB, 16 MHz
 *
 * Václav Kubeš
 *
 **********************************************************************/


/* Includes ----------------------------------------------------------*/

#include <adc.h>


/* Defines ----------------------------------------------------------*/

#define VREF 5  //5 V refernce voltage of ATmega328PBs ADC (the VCC voltage)
#define VGAP 1.1    //1.1 V internal reference voltage of ADC of ATmega328PB
#define AMP 20  //amplification of INA180A1 diff. op. amplifier IO
#define R_SENS 0.5  //Resistance of sensing resistor
#define R_RATIO 12  //Ratio of resistive divider

/* Library Global variables ---------------------------------------------------------*/

uint16_t adc_val;

/* Functions ---------------------------------------------------------*/

/**********************************************************************
 * Function: ADC_init()
 * Purpose:  Initialize ADC unit, enable ADC, set prescaler to 128, 
 *           ADC ref. voltage 5 V and start first conversion.
 * Returns:  none
 **********************************************************************/
void ADC_init(){
    PRR0 &= ~(1<<PRADC);    //turn off powersaving 
    ADMUX = 0x00;   //reset ADC controll registers
	ADCSRA = 0x00;
	ADCSRB = 0x00;
    ADCSRA = ADCSRA | 1<<ADEN;  //ADC ENABLE
    ADCSRA = ADCSRA | (1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0); //adc prescaler 128
    DIDR0 = DIDR0 | (1<< I_DIAG1 | 1<< I_DIAG2 | 1<< U_DIAG);   //disable digital input on pins used for adc
    ADMUX = ADMUX | 1<<REFS0 | 0<<ADLAR  | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //adc internal reference 5V
    ADCSRA = ADCSRA | 1<< ADSC; //start first conversion to start the analog circuits in ADC
  }  


/**********************************************************************
 * Function: ADC_read_I()
 * Purpose:  Set ADC peripheral for measuring current converted to  
 *           voltage (ADC ref. voltage 5 V) and start conversion.
 * Input: Pin on which conversion will be done
 * Returns:  none
 **********************************************************************/
void ADC_read_I(uint8_t i_diag_pin){
    if(ADMUX != 0b01000110 || ADMUX != 0b01000011){ //if the pin (input) of ADC doesnt match the selected pin then set it
        if (i_diag_pin == I_DIAG1){
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 0<<MUX0); //I_diag1 PE2
        }else{
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 0<<MUX2 | 1<<MUX1 | 1<<MUX0); //I_diag2 PC3
        }
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS0);//adc internal reference 5V
    }
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion
}


/**********************************************************************
 * Function: ADC_read_U()
 * Purpose:  Set ADC peripheral for measuring voltage 
 *              (ADC ref. voltage 1.1 V) and start conversion.
 * Returns:  none
 **********************************************************************/
void ADC_read_U(){
    if(ADMUX != 0b11000111){
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //U_diag PE3
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
    }
    ADCSRA = ADCSRA | 1<<ADSC;//adc start conversion
}


/**********************************************************************
 * Function: ADC_U()
 * Purpose:  Set ADC peripheral for measuring voltage 
 *          (ADC ref. voltage 1.1 V) and start conversion. Return 
 *          corresponding voltage (at the res. divider) in V.
 * Returns:  (float) measured voltage [V]
 **********************************************************************/
float ADC_U(){
    if(ADMUX != 0b11000111){    //if the current setting of adc input doesnt match the pin where measured voltage is connected 
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS1 | 1<<REFS0);//adc internal reference 1.1V
        ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 1<<MUX0); //set adc input to pin (U_diag) PE3
        _delay_ms(10);  //necessary for adc to settle (find out empirically)
    }
    /*take to conversions, first is thrown out (to rid of inccorect value from firs conversion after switching the ref. voltage)*/
    for(uint8_t i = 0; i < 2; i++){

        ADCSRA = ADCSRA | 1<< ADSC; //adc start conversion

        while((ADCSRA & (1<<ADSC))); //wait untill the conversion has ended
    }
    adc_val = ADC;  //read the ADC conversion result
    float u = VGAP * (adc_val / 1024.0);    //calculation of real voltage at the voltage divider
    return  (u * R_RATIO);
}


/**********************************************************************
 * Function: ADC_I()
 * Purpose:  Set ADC peripheral for measuring current converted to 
 *          voltage (ADC ref. voltage 5 V) and start conversion. Return 
 *          corresponding current in mA.
 * Input: Pin where the adc conv. will be done
 * Returns:  (float) measured current [mA]
 **********************************************************************/
float ADC_I(uint8_t i_diag_pin){
    /*according to selected I diag. pin set the input of adc*/
    if((ADMUX != 0b01000110) || (ADMUX != 0b01000011)){
        if (i_diag_pin == I_DIAG1){
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 1<<MUX2 | 1<<MUX1 | 0<<MUX0); //I_diag1 PE2
        }else{
            ADMUX = (ADMUX & 0b11110000) | (0<<MUX3 | 0<<MUX2 | 1<<MUX1 | 1<<MUX0); //I_diag2 PC3
        }
        ADMUX = (ADMUX & 0b00111111) | (1<<REFS0);  //adc internal reference 5V
        _delay_ms(10);
    }
    
    /*take to conversions, first is thrown out (to rid of inccorect value from firs conversion after switching the ref. voltage)*/
    for(uint8_t i = 0; i < 2; i++){
        
        ADCSRA = ADCSRA | 1<< ADSC ;    //adc start conversion| 1<< ADIE
        
        while((ADCSRA & (1<<ADSC)));    //necessary for adc to settle (find out empirically)
    }
    adc_val = ADC;  //read the ADC conversion result
    
    /*calculation of current*/
    float u = VREF * (adc_val / 1024.0);
    u = u / AMP;
    return  (u / R_SENS)*1000;
}
