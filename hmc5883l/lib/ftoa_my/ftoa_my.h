#ifndef FTOA_MY_H
# define FTOA_MY_H

/***********************************************************************
 * 
 * Function to convert float to string.
 * 
 * max num. before dec. point: 4
 *
 * Václav Kubeš
 *
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>

/* Function prototypes -----------------------------------------------*/

/**
 *  @brief  Convert float number to char array 
 *          (max number of integer places is 4)
 * @param   (float)f - float to be converted
 * @param   (char_pointer)outp_str - pointer to char array for storing final 
 *                            string
 * @param   (uint8_t)after_point - number of decimal places
 * @param (uint8_t)size_of_inp_array - size of array pointed to by 
 *                                          *outp_str
 *  @return  none
 */
void ftoa(float f, char *outp_str, uint8_t after_point, uint8_t size_of_inp_array);

#endif