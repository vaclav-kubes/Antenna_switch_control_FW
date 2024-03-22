#ifndef ADC_H
#define ADC_H

#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "antenna_switch_IO.h"
#include <stdlib.h>
#include <uart.h>

void ADC_init();

void ADC_read_I(uint8_t i_diag_pin);

void ADC_read_U();

float ADC_get_I(uint8_t i_diag_pin);

float ADC_get_U();

float ADC_U();

float ADC_I(uint8_t i_diag_pin);
#endif