#ifndef ANTENNA_SWITCH_IO_H
# define ANTENNA_SWITCH_IO_H

#include <avr/io.h>

//#define RX_STOP UCSR0B &=  ~((1 << RXEN0) | (1<<RXCIE0));
//#define RX_START  while((UCSR0A & (1<<TXC0)) != (1<<TXC0)); UCSR0B |= (1 << RXEN0) |(1<<RXCIE0);

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


#endif