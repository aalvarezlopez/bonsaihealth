/**
 * @file soil.c
 * @brief Module that read soil humedity. Both digital and analog values.
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-11-30
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
#include "soil.h"
#include "adc.h"
#include "log.h"

static uint8_t soil_wet = 0;
void soilInit()
{
	TRIS_DO_SOIL = TRIS_DO_SOIL | ~( 1 << PIN_SOIL_N );
}

void soilTask()
{
	triggerScan ( AI_SOIL );
	while(!isConversionFinished()) { continue; }
	soil_wet = getADCMean();
}

uint8_t getSoilWet()
{
	return soil_wet;
}
