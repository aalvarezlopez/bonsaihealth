/**
 * @file light_sensor.c
 * @brief Read luminiscency throgh LDR an ADC converter.
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-12-03
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

#include <xc.h>
#include "stdint.h"
#include "light_sensor.h"
#include "adc.h"
#include "log.h"

static uint8_t light_level;

/**
 * @brief Initialitation for Light sensor component
 */
void lightSensorInit()
{
}


/**
 * @brief Task to be done for light sensor component
 */
void lightSensorTask()
{
	triggerScan ( AI_LDR );
	while(!isConversionFinished()) { continue; }
	light_level = getADCMean();
}


/**
 * @brief Get last value of the LDR
 *
 * @return 
 */
uint8_t getLight()
{
	return light_level;
}
