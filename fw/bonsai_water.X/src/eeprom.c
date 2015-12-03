/*
 * =====================================================================================
 *
 *       Filename:  eeprom.c
 *
 *    Description:  Library for write/read in external eeprom (M24C02). This 
 *    library use I2C communication. i2c lib needed.
 *
 *        Version:  1.0
 *        Created:  31/08/15 21:18:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Adrian Alvarez (), alvarez.lopez.adri@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdint.h>
#include "eeprom.h"
#include "log.h"
#include "i2c.h"
#include "user.h"

#define MAX_RETRIES 10
#define PAGE_SIZE 16
#define EEPROM_SIZE 128

#define DEV_ADD 0xA0

/**
* \brief Write in EEPROM number of bytes begining from indicated address
* \param nbytes Number of bytes that are going to be written in the memory
* \param address Value of the address for the first byte. Next bytes are going
* to be save below.
* \param din Pointer to a buffer of the bytes that are going to be written
*/
uint8_t eepromWrite(unsigned int nBytes, unsigned int address, uint8_t *din)
{
	unsigned int bytes_written = 0;
	unsigned n_retries = 0;
	if( nBytes > EEPROM_SIZE ){
		LOG_WARN("Number of bytes to be written is greater than memory size");
		nBytes = EEPROM_SIZE;
	}
	if( nBytes + address > EEPROM_SIZE ){
		LOG_WARN("It is not possible to roll over memory boundaries");
		nBytes = EEPROM_SIZE - address;
	}
	
	while( bytes_written < nBytes ){
		unsigned int next_page_boundaries = ((address / PAGE_SIZE ) + 1) * PAGE_SIZE;
		unsigned int bytes_to_boundaries = next_page_boundaries - address;
		unsigned int bytes_to_write =
			bytes_to_boundaries < (nBytes - bytes_written) ? bytes_to_boundaries :
			(nBytes - bytes_written);
		unsigned int last_bytes_written = 
			writeSeqRegisters(DEV_ADD, address, bytes_to_write, din+bytes_written);
		if ( last_bytes_written == 0 ){
			n_retries++;
		} else {
			n_retries = 0;
		}

		address += last_bytes_written;
		bytes_written += last_bytes_written;

		if( n_retries > MAX_RETRIES ){
			LOG_WARN("Reach max number of retries. EEPROM written is incomplete");
			break;
		}
	}
	return bytes_written;
}


/**
* \brief Read from EEPROM number of bytes beginig from indicated address
* \param nBytes Number of bytes that are going to be written in the memory
* \param address Address of the first byte of the block
* \param dout Pointer to a buffer where data are going to be save
*/
uint8_t eepromRead(unsigned int nBytes, unsigned int address, uint8_t *dout)
{
	unsigned int bytes_read = 0;
	unsigned n_retries = 0;
	if( nBytes > EEPROM_SIZE ){
		LOG_WARN("Number of bytes to be read is greater than memory size");
		nBytes = EEPROM_SIZE;
	}
	if( nBytes + address > EEPROM_SIZE ){
		LOG_WARN("It is not possible to roll over memory boundaries");
		nBytes = EEPROM_SIZE - address;
	}

	while( bytes_read < nBytes ){
		unsigned int last_bytes_read =
			readSeqRegisters(DEV_ADD, address, nBytes, dout);
		bytes_read += last_bytes_read;
		address += last_bytes_read;

		if ( last_bytes_read == 0 ){
			n_retries++;
		} else {
			n_retries = 0;
		}
		if( n_retries > MAX_RETRIES ){
			LOG_WARN("Reach max number of retries. EEPROM read is incomplete");
			break;
		}
	}
	return (uint8_t)bytes_read;
}
