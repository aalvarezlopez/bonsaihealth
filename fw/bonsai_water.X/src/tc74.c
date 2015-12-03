/**
 * @file tc74.c
 * @brief Library to control TC74a2 component (temperature sensor by I2C)
 * @author Adrian Alvarez
 * @version 1.0
 * @date 2015-11-19
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

#include <stdint.h>
#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */
#include "i2c.h"

#define TC74_ADD 0x94
#define TEMP_REG 0x00
#define DEFAULT_VALUE 0x78

uint8_t readTemperature()
{
	int value;
	int succesful = 0;
	if ( readRegister( TC74_ADD, TEMP_REG , &value)){
		succesful = 1;
	}
	return succesful ? (uint8_t) value : DEFAULT_VALUE; 
}
