
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <stdlib.h> 
#include <string.h>

void ftoa(float f, char *outp_str, uint8_t after_point, uint8_t size_of_inp_array);