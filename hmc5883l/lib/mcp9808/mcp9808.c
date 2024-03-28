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

#include <mcp9808.h>

/* Function definitions ----------------------------------------------*/

/**********************************************************************
 * Function: MCP9808_init()
 * Purpose:  Configure the MCP9808 for continuous temp. reading (accuarcy +-0.0625°C).
 * Input:    (uint8_t) sens - indication which MCP9808 should be 
 *               initialized (valid values TEMP_A (1), TEMP_B (2) for this library)
 * Returns:  1 if MCP9808 responded, 0 if MCP9808 didnt respond
 **********************************************************************/
uint8_t MCP9808_init(uint8_t sens){
    uint8_t responded = 0; 
    uint8_t adr = MCP9808_I2C_ADR;  //i2c addr. of MCP9808
    /*if sensor 2 should be initialized then change i2c address*/
    if(sens == 2){
        adr = MCP9808_I2C_ADR_B;   
    }
    twi_start();    //start i2c comm.
    responded = twi_write(adr << 1 | TWI_WRITE);    //ack/nack
    twi_write(MCP9808_REG_CONFIG);  //set register of MCP9808 to be written to
    twi_write(0b00000000);  //set registers
    twi_write(0b00000000);
    twi_stop();
    return ! responded; //return negated ACK from i2c
}

/**********************************************************************
 * Function: MCP9808_read_temp()
 * Purpose:  Get measured temperature from MCP9808.
 * Input:    (uint8_t) sens - indication from which MCP9808 should the 
 *              temp. be read (valid values TEMP_A (1), TEMP_B (2) 
 *              for this library)
 * Returns:  (float) measured temperature
 **********************************************************************/
float MCP9808_read_temp(uint8_t sens){
    uint8_t upper_t;
    uint8_t lower_t;
    uint8_t adr = MCP9808_I2C_ADR;
    /*set i2c addr. according to selected sensor*/
    if(sens == 2){
        adr = MCP9808_I2C_ADR_B;
    }

    twi_start();
    if(twi_write(adr << 1 | TWI_WRITE)) return ERROR_VAL;   //if the MCP9808 is unconnected, return ERROR_VAL
    twi_write(MCP9808_REG_T);   //read from register where measured data representing temp. are stored
    twi_start();
    twi_write(adr << 1 | TWI_READ);
    upper_t = twi_read(TWI_ACK);    //read lower and higher reg. of temp. values
    lower_t = twi_read(TWI_NACK);
    twi_stop();

    /*calculation of temperature (as described in MCP9808 datasheet)*/
    upper_t &= 0b00001111;
    if ((upper_t & 0b00001000) == 0b00001000){
        return (float)(256 - (upper_t * 16 + (float)lower_t / 16));
    }else{
        return (float)(upper_t * 16 + (float)lower_t / 16);
    }
}



