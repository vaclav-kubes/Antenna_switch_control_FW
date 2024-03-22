
#ifndef MCP9808_H
# define MCP9808_H

#include <mcp9808.h>

uint8_t MCP9808_init(uint8_t sens){
    uint8_t responded = 0;
    uint8_t adr = MCP9808_I2C_ADR;
    if(sens == 2){
        adr = MCP9808_I2C_ADR_B;
    }
    twi_start();
    responded = twi_write(adr << 1 | TWI_WRITE);
    twi_write(MCP9808_REG_CONFIG);
    twi_write(0b00000000);
    twi_write(0b00000000);
    twi_stop();
    return ! responded;
}

float MCP9808_read_temp(uint8_t sens){
    uint8_t upper_t;
    uint8_t lower_t;
    uint8_t adr = MCP9808_I2C_ADR;
    if(sens == 2){
        adr = MCP9808_I2C_ADR_B;
    }
    twi_start();
    if(twi_write(adr << 1 | TWI_WRITE)) return ERROR_VAL;
    twi_write(MCP9808_REG_T);
    twi_start();
    twi_write(adr << 1 | TWI_READ);
    upper_t = twi_read(TWI_ACK);
    lower_t = twi_read(TWI_NACK);
    twi_stop();

    upper_t &= 0b00001111;
    if ((upper_t & 0b00001000) == 0b00001000){
        return (float)(256 - (upper_t * 16 + (float)lower_t / 16));
    }else{
        return (float)(upper_t * 16 + (float)lower_t / 16);
    }
}



#endif