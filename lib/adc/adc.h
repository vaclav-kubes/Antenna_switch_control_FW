#ifndef ADC_H
#define ADC_H

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
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "antenna_switch_IO.h"  //definitions of IO of antenna switch controller
#include <stdlib.h>
#include <util/delay.h>

/* Function prototypes -----------------------------------------------*/

/**
 *  @brief  Initialize ADC unit, enable ADC, set prescaler to 128,
 *          ADC ref. voltage 5 V and start first conversion.
 *  @return:  none
 */
void ADC_init();

/**
 *  @brief  Set ADC peripheral for measuring current converted to  
 *           voltage (ADC ref. voltage 5 V) and start conversion.
 *  @param Pin on which conversion will be done
 *  @return  none
*/
void ADC_read_I(uint8_t i_diag_pin);

/**
 *  @brief  Set ADC peripheral for measuring voltage 
 *              (ADC ref. voltage 1.1 V) and start conversion.
 *  @return  none
*/
void ADC_read_U();

/**
 *  @brief  Set ADC peripheral for measuring voltage 
 *          (ADC ref. voltage 1.1 V) and start conversion. Return 
 *          corresponding voltage (at the res. divider) in V.
 *  @return  (float) measured voltage [V]
 */
float ADC_U();

/**
 *  @brief  Set ADC peripheral for measuring current converted to 
 *          voltage (ADC ref. voltage 5 V) and start conversion. Return 
 *          corresponding current in mA.
 *  @param Pin where the adc conv. will be done (I_DIAG1/ I_DIAG2)
 *  @return  (float) measured current [mA]
 */
float ADC_I(uint8_t i_diag_pin);
#endif