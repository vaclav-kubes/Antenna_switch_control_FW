
#ifndef HMC5883L_H
# define HMC5883L_H

/***********************************************************************
 * 
 * Library for HMC5883L electronics compass module.
 * 
 * ATmega328PB, 16 MHz
 *
 *Dependent on twi.h library by doc. Fryza
 *
 * Václav Kubeš
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <twi.h>    // I2C library
#include <avr/io.h> // AVR device-specific IO definitions
#include <stdlib.h> //for itoa()
#include <math.h>   //for azimuth calculation
#include <string.h> //for string manipulation

/* Defines ----------------------------------------------------------*/
#define HMC588L_I2C_ADR 0x1E    //i2c HMC5883L address
#define CONFIG_REG_A 0  //registers of HMC5883L
#define CONFIG_REG_B 1
#define MODE_REG_ 2
#define DATA_X 3
#define DATA_Z 5
#define DATA_Y 7

#define SAMPLES_8 0b11  //values to be set to config HMC5883L registers
#define GAIN_1_3 0b001
#define CONTINUOUS_READING 0b00
#define DATARATE_15HZ 0b100

#define MAG_POLE_DEC (5.0 + (23.0 / 60.0))  //calculation of mag. pole declination

/* Global varibales prototypes -----------------------------------------------*/

struct data //struc to save raw data from e-compass
{
    int16_t X;
    int16_t Y;
    int16_t Z;
};

/* Function prototypes -----------------------------------------------*/

/**
 * @brief  Initialize HMC5883L e-compass. (8 samples, gain 1.3, continuos reading, datarate 15 Hz)
 * 
 * @return 1 ACK received, 0 NACK received
 */
uint8_t HMC588L_init();

/**
 * @brief  Get measured raw data from HMC5883L
 * 
 * @param  write_to(struc_prototype)    Struc prototype for saving read raw values
 * 
 * @return structure containing int16_t X, Y and Z raw data
 */
struct data HMC5883L_rawData(struct data write_to);

/**
 * @brief  Calcualte measured azimuth frm raw data
 * 
 * @param   X(int16_t)  X raw value
 * @param   Y(int16_t)  Yraw value
 * 
 * @return calculated azimuth float
 */
float HMC5883L_azimuth(int16_t X, int16_t Y);

/**
 * @brief  Convert float to string
 */
void HMC5883L_ftoa(float f, char *string, uint8_t after_point, uint8_t size_of_inp_array);

#endif