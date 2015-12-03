/**
 * @file adc.c
 * @brief ADC module for controlling internal microcontrol peripheral.
 * @author Adrian Alvarez
 * @version 1.0
 * @date 2015-09-28
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
#include <stdint.h>
#include "adc.h"

#define N_ACQ 8

/**
 * @brief Configure adc driver.
 */
void configureADC()
{
	uint32_t AD1PFCG_MASK = ( 1 << AI_SOIL ) | ( 1 << AI_LDR);
	AD1CON2bits.VCFG = 0; /* VDD and VSS selected as references */
	AD1CON3bits.ADRC = 1; /* Select internal RC as ADC clock */
	AD1CON3bits.SAMC = 28; /* Auto-sample time bits */
	AD1CON3bits.ADCS = 64; /* A/D conversion clock select bits */
	AD1CON2bits.CSCNA = 1; /* Scan inputs */
	AD1CON2bits.SMPI = N_ACQ; /*  Interruption after one conversions */
	AD1CON2bits.BUFM = 0; /* Buffer acces as one 16 bits */
	AD1CON2bits.ALTS = 0; /* Always use MUXA configuration */
	AD1CON1bits.ASAM = 1; /*  Sample auto start */
	AD1PCFG &= ~AD1PFCG_MASK;
	AD1CHS = 0;
	AD1CON1bits.SSRC = 0b111; /*  auto-convert */
	AD1CON1bits.ADON=1;
}


/**
 * @brief Start conversion of the group selected
 */
void triggerScan( uint8_t channel )
{
	AD1CSSL = 1 << channel ;
	AD1CSSH = 0x0;
	IFS0bits.AD1IF = 0;
	AD1CON1bits.ASAM = 1; /*  Sample auto start */
}


/**
 * @brief Check wether las conversion triggered has finished. If the conversion
 * has finished, this function will automatically turn of the conversion.
 *
 * @return 1 if the conversion has finished, 0 in other case
 */
uint8_t isConversionFinished()
{
	uint8_t finished = IFS0bits.AD1IF;
	if( finished ){
		AD1CON1bits.ASAM = 0;
	}
	return finished;
}



/**
 * @brief Read NAQC_s conversion result and return mean of them.
 *
 * @return 
 */
uint8_t getADCMean()
{
	uint16_t temp;
	uint32_t sum = 0;
	int *ADC16ptr = &ADC1BUF0;
	for(int i = 0; i < N_ACQ; i++){
		temp = *ADC16ptr;
		temp >>= 2;
		sum += temp;
		ADC16ptr++;
	}
	return sum / N_ACQ;
}

/**
 * @brief  Check if the conversion has finished. If it's finished
 * write the values of the ADC conversion in the buffer
 *
 * @param n_channels Number of channels that are going to be read
 * @param buffer Pointer to a buffer where the values are going to be
 *			written
 *
 * @return  True if the values are ready
 */
uint8_t getADCValues(uint8_t n_channels, uint8_t *buffer)
{
	uint8_t finished = IFS0bits.AD1IF;
	uint16_t temp;
	int *ADC16ptr = &ADC1BUF0;
	if( finished ){
		for(int i = 0; i < n_channels; i++){
			temp = *ADC16ptr;
			temp >>= 2;
			buffer[i] = (uint8_t) temp;
			ADC16ptr++;
		}
	}
	return finished;
}
