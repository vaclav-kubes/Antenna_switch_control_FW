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

#include <hmc5883l.h>

/* Variable definitions ----------------------------------------------*/
static uint16_t g_list [8] = {1370, 1090, 820, 660, 440, 390, 330, 230};
static uint16_t g;
/* Function definitions ----------------------------------------------*/

/**********************************************************************
 * Function: HMC588L_init()
 * Purpose:  Initialize the HMC5883L e-Compass.
 * Returns:  1 ACK received, 0 NACK received 
 **********************************************************************/
uint8_t HMC588L_init(){
    twi_start();
    if (twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE)==0){
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

/**********************************************************************
 * Function: HMC5883L_rawData()
 * Purpose:  Get raw data measured by HMC5883L (X, Y, Z axis mag. field
 *              trength)
 * Input: struct data - structure data globaly defined in this library  
 * Returns:  structure containing X, Y and Z raw measured values of 
 *              mag. field strength
 **********************************************************************/
struct data HMC5883L_rawData(struct data write_to){
    twi_start();
    twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE);
    twi_write(DATA_X);
    twi_start();
    twi_write(HMC588L_I2C_ADR<<1 | TWI_READ);
    write_to.X = (twi_read(TWI_ACK)<<8 | twi_read(TWI_ACK));
    write_to.Z = (twi_read(TWI_ACK)<<8 | twi_read(TWI_ACK));
    write_to.Y = (twi_read(TWI_ACK)<<8 | twi_read(TWI_NACK));
    twi_stop();

    uint8_t gain [8]= {GAIN_0_88, GAIN_1_3, GAIN_1_9, GAIN_2_5, GAIN_4_0, GAIN_4_7, GAIN_5_6, GAIN_8_1};
    uint8_t i = 0;
    while((write_to.X == -4096) || (write_to.Y == -4096) || (write_to.Z == -4096)){
        twi_start();
        twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE);
        twi_write(CONFIG_REG_B);
        twi_write(gain[i]<< 5);
        twi_stop();
        g = g_list[i];
        twi_start();
        twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE);
        twi_write(DATA_X);
        twi_start();
        twi_write(HMC588L_I2C_ADR<<1 | TWI_READ);
        write_to.X = (twi_read(TWI_ACK)<<8 | twi_read(TWI_ACK));
        write_to.Z = (twi_read(TWI_ACK)<<8 | twi_read(TWI_ACK));
        write_to.Y = (twi_read(TWI_ACK)<<8 | twi_read(TWI_NACK));
        twi_stop();
        
        i++;
        if(i > 7){
            i = 0;
            twi_start();
            twi_write(HMC588L_I2C_ADR<<1 | TWI_WRITE);
            twi_write(CONFIG_REG_B);
            twi_write(GAIN_1_3<< 5);
            twi_stop();
            g = g_list[1];
            break;
        }
        
    }
    return write_to;
}

/**********************************************************************
 * Function: HMC5883L_azimuth
 * Purpose:  Calculate the azimuth from raw e-compass data
 * Inputs: (int16_t)X - X raw value  
 *         (int16_t)Y - Y raw value  
 * Returns:  (float) calculated azimuth 
 **********************************************************************/
float HMC5883L_azimuth(int16_t X, int16_t Y){
    double heading = 180*atan2((double)(Y + 182)/(g/100.0), (double)(X - 10)/(g/100.0))/M_PI - 90;

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
    
    for(uint8_t i = 0; i < sizeof(int_part) - 1; i++){
        str[i] = int_part[i];
    }
    str[sizeof(int_part) - 1] = '.';
    
    for(uint8_t i = 0; i < sizeof(frc); i++){
        str[sizeof(int_part) + i] = frc[i];
    }

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

}

