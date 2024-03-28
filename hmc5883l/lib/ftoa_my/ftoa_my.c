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

#include <ftoa_my.h>

/* Functions ---------------------------------------------------------*/

/**********************************************************************
 * Function: ftoa()
 * Purpose:  Convert float type to array of characters. (max number of 
 *          digits before dec. point is 4)
 * Inputs:  (float) f - float to be converted
 *          (char pointer) outp_str - pointer to char array for storing final 
 *                            string
 *          (uint8_t) after_point - number of decimal places
 *          (uint8_t) size_of_inp_array - size of array pointed to by 
 *                                          *outp_str
 * Returns:  none
 **********************************************************************/
void ftoa(float f, char *outp_str, uint8_t after_point, uint8_t size_of_inp_array){
    
    char int_part[5];   //char array for integer part (max. 4 places)
    char frac_part[after_point + 1];    //char array for decimal part
    char str [7 + after_point]; //final char array to put it together
    char add_zeros[after_point + 1];    //char array for adding zeros
    uint8_t pos = 0;    //position counter

    int16_t n = (int16_t) f;    //convert float to uint16_t - this cuts of the decimal part
    uint16_t m = (uint16_t)((f - (float)n) * pow(10, after_point)); //by multiplying decimal part get it to ineger
    itoa(n, int_part, 10);  //convert int to string
    itoa(m, frac_part, 10);
    
    /*add zeros before numbers in decimal part lost by itoa*/
    for(int8_t i = after_point - 1; i >= 1; i--){
        if(m / (uint16_t)(pow(10, i)) == 0){
            add_zeros[pos] = '0';
            pos++;
        }
    }
    add_zeros[pos] = '\0';

    /*add zeros and dec part togehter*/
    char frc [after_point + 1]; 
    strcpy(frc, add_zeros);
    strcat(frc, frac_part);

    /*copy integer part to final string*/
    for(uint8_t i = 0; i < sizeof(int_part) - 1; i++){
        str[i] = int_part[i];
    }

    str[sizeof(int_part) - 1] = '.';    //add decimal point
   
    /*copy decimal part to final dtring*/
    for(uint8_t i = 0; i < sizeof(frc); i++){
        str[sizeof(int_part) + i] = frc[i];
    }

    /*connect integer part, dec. point and decimal part together*/
    //??not sure why I did it again - this part or that upper could by deleted (?)
    char str_out [sizeof(str)];
    char dot [2] = ".";
    strcpy(str_out, int_part);
    strcat(str_out, dot);
    strcat(str_out, frc);

    /*unnecessary clearing of the char array where the result should be placed*/
    for(uint8_t i = 0; i < size_of_inp_array; i++){
        *(outp_str + i) = '\0';
    }

    /*copy the result string where it is expected*/
    for(uint8_t i = 0; i < (sizeof(int_part) + sizeof(frc)); i++){
        *(outp_str + i) = str_out[i];
    }
}