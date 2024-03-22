#ifndef FTOA_MY_H
# define FTOA_MY_H

#include <ftoa_my.h>
#include <math.h>

void ftoa(float f, char *outp_str, uint8_t after_point, uint8_t size_of_inp_array){
    char int_part[5];
    char frac_part[after_point + 1];
    char str [7 + after_point];
    char add_zeros[after_point + 1];
    uint8_t pos = 0;

    int16_t n = (int16_t) f;
    uint16_t m = (uint16_t)((f - (float)n) * pow(10, after_point));
    itoa(n, int_part, 10);
    itoa(m, frac_part, 10);
 
    for(uint8_t i = after_point - 1; i >= 1; i--){
        if(m / (uint16_t)(pow(10, i)) == 0){
            add_zeros[pos] = '0';
            pos++;
        }
    }
    add_zeros[pos] = '\0';

    char frc [after_point + 1];
    strcpy(frc, add_zeros);
    strcat(frc, frac_part);
    //uart_puts("\n");
    /*uart_puts(int_part);
    uart_puts(",");
    uart_puts(frc);
    uart_puts("\t→→\t");*/
    
    //uint8_t shift = 0;

    for(uint8_t i = 0; i < sizeof(int_part) - 1; i++){
        /*if(int_part[i] == '\0'){
            shift++;
            uart_putc('!');
            continue;
        }else{
           str[i - shift] = int_part[i]; 
        }*/
        str[i] = int_part[i];
        
    }
    str[sizeof(int_part) - 1] = '.';
    for(uint8_t i = 0; i < sizeof(frc); i++){
        str[sizeof(int_part) + i] = frc[i];
    }
    /*strcpy(str, int_part);
    strcat(str, ".");
    strcat(str, frac_part);
    uart_puts("\n");
    uart_puts(int_part);
    uart_puts(",");
    uart_puts(frac_part);
    uart_puts(",");
    uart_puts(str);*/
    char str_out [sizeof(str)];
    char dot [2] = ".";
    strcpy(str_out, int_part);
    strcat(str_out, dot);
    strcat(str_out, frc);
    for(uint8_t i = 0; i < size_of_inp_array; i++){
        *(outp_str + i) = '\0';
    }
    for(uint8_t i = 0; i < (sizeof(int_part) + sizeof(frc)); i++){
        *(outp_str + i) = str_out[i];
    }
    /*uart_puts(str);
    uart_puts("\t→→\t");*/

}

#endif