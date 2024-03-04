

#include <twi.h> // I2C library
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <stdlib.h> 
#include <math.h> 
#include <string.h>

#define HMC588L_I2C_ADR 0x1E
#define CONFIG_REG_A 0
#define CONFIG_REG_B 1
#define MODE_REG_ 2
#define DATA_X 3
#define DATA_Z 5
#define DATA_Y 7

#define SAMPLES_8 0b11
#define GAIN_1_3 0b001
#define CONTINUOUS_READING 0b00
#define DATARATE_15HZ 0b100

#define MAG_POLE_DEC (5.0 + (23.0 / 60.0))

struct data
{
    int16_t X;
    int16_t Y;
    int16_t Z;
};


uint8_t HMC588L_init();

struct data HMC5883L_rawData();

float HMC5883L_azimuth(int16_t X, int16_t Y);

void HMC5883L_ftoa(float f, char *string, uint8_t after_point, uint8_t size_of_inp_array);