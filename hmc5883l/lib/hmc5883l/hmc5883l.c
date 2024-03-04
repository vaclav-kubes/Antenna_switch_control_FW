

#ifndef HMC5883L_H
# define HMC5883L_H

#include <hmc5883l.h>


//typedef struct data new_data;

uint8_t HMC588L_init(){
    twi_init();
    twi_start();
    if (twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE)==0){
        //uart_puts("setting sensor\n");
        //if(! twi_write(0)) uart_puts("start config\n");
        twi_write(CONFIG_REG_A);
        twi_write(SAMPLES_8 << 5 | DATARATE_15HZ << 2);
        twi_write(GAIN_1_3 << 5);
        twi_write(CONTINUOUS_READING);
        twi_stop();
        return 1;
    }
    twi_stop();
    return 0;
}


struct data HMC5883L_rawData(struct data write_to){
    //struct data new_data = write_to;
    twi_start();
    twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE);
    twi_write(DATA_X);
    twi_start();
    twi_write(HMC588L_I2C_ADR<<1 | TWI_READ);
    write_to.X = (twi_read(TWI_ACK)<<8 | twi_read(TWI_ACK));
    write_to.Z = (twi_read(TWI_ACK)<<8 | twi_read(TWI_ACK));
    write_to.Y = (twi_read(TWI_ACK)<<8 | twi_read(TWI_NACK));
    twi_stop();
    return write_to;
}


float HMC5883L_azimuth(int16_t X, int16_t Y){
    double heading = 180*atan2((double)(Y + 182)/1.090f, (double)(X - 10)/1.090f)/M_PI - 90;

    heading += MAG_POLE_DEC;

    if (heading < 0.0){
        heading += 360.0;
    }
    if (heading > 360.0){
        heading -= 360.0;
    }
    return heading;
}

void HMC5883L_ftoa(float f, char *outp_str, uint8_t after_point, uint8_t size_of_inp_array){
    char int_part[4];
    char frac_part[after_point + 1];
    char str [7 + after_point];
    char add_zeros[after_point + 1];
    uint8_t pos = 0;

    uint16_t n = (uint16_t) f;
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