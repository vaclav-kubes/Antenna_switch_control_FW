#ifndef ANTENNA_SWITCH_IO_H
# define ANTENNA_SWITCH_IO_H

/***********************************************************************
 * 
 * Library for Antena switch controller outside unit.
 * 
 * MCU: ATmega328PB, 16 MHz
 *
 *Václav Kubeš
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/

#include <avr/io.h>

/* Defines ----------------------------------------------------------*/
/*I/O pins and corresponding connected devices*/
#define LED PC0
#define BUTTON PB2

#define ANT01 PD2
#define ANT02 PD3
#define ANT03 PD4
#define ANT04 PD5
#define ANT05 PD6

#define ANT11 PC1
#define ANT12 PC2
#define ANT13 PD7
#define ANT14 PB0
#define ANT15 PB1

#define I_DIAG1 PE2
#define I_DIAG2 PC3

#define U_DIAG PE3

/*macro*/
/** @brief Disable receiving of UART0 and received interrupt too.*/
#define RX_DISABLE() UCSR0B &=  ~((1 << RXEN0) | (1<<RXCIE0)); //disable RX of UART0, necessary due to TJA1020 echoing when UART0 transmitting 

/** @brief Toggle all pins where antennas switch is connected.*/
#define ALL_ANT_TOGGLE() PORTD ^= ((1<< ANT01) | (1<< ANT02) | (1<< ANT03) | (1<< ANT04)| (1<< ANT05) | (1<< ANT13));\
        PORTB ^= ((1<<ANT14) | (1<<ANT15)); PORTC ^= ((1<<ANT11) | (1<<ANT12));

/** @brief Turn off all pins where antennas switch is connected (negative logic).*/
#define ALL_ANT_OFF() PORTD |= ((1<< ANT01) | (1<< ANT02) | (1<< ANT03) | (1<< ANT04)| (1<< ANT05) | (1<< ANT13));\
        PORTB |= ((1<<ANT14) | (1<<ANT15)); PORTC |= ((1<<ANT11) | (1<<ANT12));

#endif