/* 
 * File:   i2c.h
 * Author: adrian
 *
 */

#ifndef I2C_H
#define	I2C_H

#define ADD_HIGH_WORD 8u
#define ADD_8_BITS_MASK 0xFF

void configureI2C();
int readSeqRegisters(uint8_t device_add, int address, int dlen, uint8_t *din);
int readRegister(uint8_t device_add, int address, int *value);
int readSeqRegisters16add(uint8_t device_add, uint16_t address, int dlen, uint8_t *dout);
int read(uint8_t device_add, int *value);
int writeSeqRegisters(uint8_t device_add, int address, int dlen, uint8_t *din);
int writeSeqRegisters16add(uint8_t device_add, uint16_t address, int dlen, uint8_t *din);
int writeRegister(uint8_t device_add, int address, int value);
int isDevAvailable(uint8_t device_add);
int writeI2Cdata( uint8_t device_add, uint8_t *data, uint8_t nData);

#endif	/* I2C_H */

