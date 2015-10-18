/**
 * @file mcp4017.c
 * @brief Library for MCP4017 device (I2C adjustable resistor)
 * @author Adrian Alvarez
 * @version 1.0
 * @date 2015-09-08
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

#include "stdio.h"
#include "stdint.h"
#include "mcp4017.h"
#include "i2c.h"
#include "log.h"

#define MCP4017_ADD 0x5E
#define NBYTESTR	1
#define MAX_VALUE 127

static uint8_t _current_value = 0;
/**
 * @brief Adjust resistor value through I2C
 *
 * @param value
 */
uint8_t setResistorValue( uint8_t value )
{
	uint8_t data[NBYTESTR] = {value};
	uint8_t result;
	char msg[50];
	if ( value > MAX_VALUE ) {
		LOG_WARN("Resistor value is above limits");
		value = MAX_VALUE;
	}
	sprintf(msg,"RESISTOR =%d", value);
	LOG_DBG(msg);
	if( writeI2Cdata(MCP4017_ADD, data, NBYTESTR) == NBYTESTR ){
		result = 1;
		_current_value = value;
	} else {
		result = 0;
	}
	return result;
}


/**
 * @brief  Return last value set to potentiometer
 */
uint8_t getResistorValue()
{
	return _current_value;
}


/**
 * @brief Get max valid value for the resistor
 *
 * @return Max value
 */
uint8_t getMaxValue()
{
	return MAX_VALUE;
}
