#ifndef MCP9808_H
# define MCP9808_H

/***********************************************************************
 * 
 * Library for MCP9808 temeperature sensor
 * (Registers of MPC9808 are 16b.)
 * 
 * For ATmega328PB, 16 MHz
 * 
 * Dependent on TWI library by doc. T. Fryza
 *
 *  Václav Kubeš
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/

#include <twi.h> // I2C library
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <stdlib.h> 
#include <math.h> 
#include <string.h>

/* Defines ----------------------------------------------------------*/

#define TEMP_A 1    //to easily pick temp. sens. 1 (on A)
#define TEMP_B 2    //to easily pick temp. sens. 2 (on B)
#define MCP9808_I2C_ADR 0b00011000  //i2c address of MCP9808 1 (on A) 
#define MCP9808_I2C_ADR_B 0b00011001    //i2c address of MCP9808 2 (on B) 
#define MCP9808_REG_CONFIG 0b00000001   //address of config. reg. of MCP9808
#define MCP9808_REG_RESOL 0b00001000    //address of resolution config. reg. of MCP9808
#define MCP9808_REG_T 0b000000101   //address of temperature data reg. of MCP9808
#define ERROR_VAL 255   //if MCP9808 doesn respod when reading temp.


/* Function prototypes -----------------------------------------------*/

/**
 * @brief  Initialize MCP9808 temp. sens. for continous measuring.
 * @param  sens(uint8_t) Indication of which of two MCP9808 should be initialized
 * @return (uint8_t) 1 if temp. sens responded, 0 didnt respond
 */
uint8_t MCP9808_init(uint8_t sens);

/**
 * @brief  Read the temp. data from MCP9808 and return temp. value.
 * @param  sens(uint8_t) Indication of which of two MCP9808 should be read
 * @return (float) measured temperature
 */
float MCP9808_read_temp(uint8_t sens);

#endif