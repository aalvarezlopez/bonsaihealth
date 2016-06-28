/**
 * @file datastorage.c
 * @brief this library storage the data acquire from the device in the external
 * eeprom (I2C) of the microcontroller. Data coulb  be save and read one by one
 * or in block.
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-12-05
 */
/* Copyright (C) 
 * 2015 - Adrian Alvarez
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */
#include "stdint.h"
#include "eeprom.h"
#include "datastorage.h"
#include "log.h"
/**
 * EXTERNAL EEPROM CONFIGURATION
 */
#define EEPROM_CODE 0xA4
#define CODE_MASK 0xFE
#define NOT_OVR 0
#define IS_OVR 1

#define HEADER_SIZE 3
#define HEADER_ADD 0
enum{
	HWORD_NEXTPOS_ADD,
	LWORD_NEXTPOS_ADD,
	STATUS_ADD
};

#define ORF_MASK 0x01

#define DATA_SIZE sizeof(storage_data)
#define MEM_SIZE 65535


// +------------------------------------+
// | Position of the next element HWORD |
// +------------------------------------+
// | Position of the next element LWORD |
// +------------------------------------+
// |   CODE (0xA4)                | OR  |
// +------------------------------------+
// | position 1 temperature       | PS  |
// +------------------------------------+
// | position 1 soil                    |
// +------------------------------------+
// |position 1 light                    |
// +------------------------------------+
// |position 1 hour                     |
// +------------------------------------+
// |position 1 minutes                  |
// +------------------------------------+
// |position 1 seconds                  |
// +------------------------------------+
// | position 2 temperature       | PS  |
// +------------------------------------|
// |     ....                           |
// +------------------------------------+
// | position 84 temperature       | PS |
// +------------------------------------+
// | position 84 soil                   |
// +------------------------------------+
// |position 84 light                   |
// +------------------------------------+
// |position 84 hour                    |
// +------------------------------------+
// |position 84 minutes                 |
// +------------------------------------+
// |position 84 seconds                 |
// +------------------------------------+
// |      SPARE  2 bytes                |
// +------------------------------------+
//
// PS = PUMP STATE. '1' if the pump was switch on in the last period of time
// '0' in other case
// OR = Owerrun Flag. '1' it he memory has been written at least one time whole.

static uint16_t _next_position;
static uint8_t _overrunflag;
void storageUpdateSt();


/**
 * @brief Initialitation of the static variables of status.
 */
void storageInit()
{
	uint8_t buffer[HEADER_SIZE];
	if (eepromRead( HEADER_SIZE, HEADER_ADD, buffer)){
		_next_position = (buffer[HWORD_NEXTPOS_ADD] << 8u) | \
						 buffer[LWORD_NEXTPOS_ADD];
		_overrunflag = buffer[STATUS_ADD] & ORF_MASK ;
		if ( (buffer[STATUS_ADD] & CODE_MASK) != EEPROM_CODE ){
			LOG_DBG("EEPROM without format");
			_next_position = 0;
			_overrunflag = NOT_OVR;
			storageUpdateSt();
		}
	} else {
		LOG_DBG("Error reading EEPROM");
		_next_position = 0;
		_overrunflag = NOT_OVR;
	}
}

void storageClean()
{
	LOG_DBG("EEPROM will be erased");
	_next_position = 0;
	_overrunflag = NOT_OVR;
	storageUpdateSt();
}

/**
 * @brief Write in eeprom the status variables.
 */
void storageUpdateSt()
{
	uint8_t buffer[HEADER_SIZE] = { (_next_position >> 8u), _next_position, EEPROM_CODE |  _overrunflag };
	eepromWrite( HEADER_SIZE, HEADER_ADD, (uint8_t*)buffer);
}


/**
 * @brief Append new acquisition data to the EEPROM. Also update the status
 * of the storage. Once the memory is full the older acquisiton will be removed.
 *
 * @param to_write Struct with the new data to write.
 */
void storageAppendData( storage_data to_write )
{
	uint16_t address = HEADER_SIZE + ( _next_position * DATA_SIZE);
	char str[50];

	/* End of data address should be lower than size */
	if( ( address + DATA_SIZE ) > MEM_SIZE ){
		LOG_DBG("memory overflow");
		_overrunflag = IS_OVR;
		_next_position = 0;
		address = HEADER_SIZE;
	}
	sprintf(str, "ADD %x OR %x NEXT_POS %x", address, _overrunflag, _next_position);
	LOG_DBG(str);
	eepromWrite( DATA_SIZE, address, (uint8_t*) &to_write);
	storageUpdateSt();
	_next_position++;
}

uint16_t maxStorage()
{
	uint16_t n_measurements = ((uint16_t)MEM_SIZE - HEADER_SIZE) / DATA_SIZE;
	return n_measurements;
}

/**
 * @brief Read data storaged in EEPROM. Index indicate which data should be read
 * '0' is the last data storaged and 'n' is the n last dta storage.
 *
 * @param index
 * @param data
 *
 * @return  If there are data of this index
 */
uint8_t storageGetData( uint16_t index, storage_data *data )
{
	/* Address of the next position */
	uint32_t address = HEADER_SIZE + ( _next_position * DATA_SIZE);
	uint8_t position;

	if ( index >= maxStorage() ){
		LOG_DBG("Index should be lower than storage size");
		return 0;
	}

	address -= ( (index+1) * DATA_SIZE);
	if ( address > MEM_SIZE && _overrunflag == NOT_OVR ){
		return 0;
	} else if ( address > MEM_SIZE ){
		address = address % MEM_SIZE;
		address += HEADER_SIZE;
		position = (address / DATA_SIZE) - 1;
		address = HEADER_SIZE + position * DATA_SIZE;
	} else {
		/*  Nothing to do */
	}
	eepromRead( DATA_SIZE, address, (uint8_t *) data);
	return 1;
}

