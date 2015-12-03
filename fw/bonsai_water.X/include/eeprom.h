/*
 * =====================================================================================
 *
 *       Filename:  eeprom.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  31/08/15 21:20:32
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Adrian Alvarez (), alvarez.lopez.adri@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef EEPROM_H
#define EEPROM_H

enum ERRORCODE{
    EEPROM_ACK,
    EEPROM_ERROR_DEVICE
};

uint8_t eepromWrite(unsigned int nBytes, unsigned int address, uint8_t *din);
uint8_t eepromRead(unsigned int nBytes, unsigned int address, uint8_t *dout);

#endif
