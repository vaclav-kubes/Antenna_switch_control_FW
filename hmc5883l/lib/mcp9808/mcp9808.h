
#include <twi.h> // I2C library
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <stdlib.h> 
#include <math.h> 
#include <string.h>

#define TEMP_A 1
#define TEMP_B 2
#define MCP9808_I2C_ADR 0b00011000
#define MCP9808_I2C_ADR_B 0b00011001
#define MCP9808_REG_CONFIG 0b00000001
#define MCP9808_REG_RESOL 0b00001000
#define MCP9808_REG_T 0b000000101
#define ERROR_VAL 255
uint8_t MCP9808_init(uint8_t sens);

float MCP9808_read_temp(uint8_t sens);