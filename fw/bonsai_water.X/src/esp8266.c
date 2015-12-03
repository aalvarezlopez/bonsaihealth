/**
 * @file esp8266.c
 * @brief This library will be use to communicate with the ESP8266 mopdule.
 * A UART peripheral will be configure and use for communication
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-11-26
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

#include <xc.h>            /* HiTech General Includes */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include "esp8266.h"

char esp_rx_buf[100];

/**
 * @brief ESP8266 initialitation.
 * UART2 configuration
 */
void ESP8266Init()
{
	//configure serial port
	//pins TX 13 RX 12
	RPINR19 = RX_PIN;
	RPOR_TX =  PIN_AS_TX << RPOR_PADDING;
	//8 bit data and no parity
	U2MODE = 0;
	//baudrate 1200 => =FCY/( 4 * BDRT ) -1 = 32000000/(4 * 1200) - 1
	U2BRG = 34;
	U2MODEbits.BRGH = 1;

	U2MODEbits.RTSMD = 1;
	U2MODEbits.UARTEN = 1;
	U2STAbits.UTXEN = 1;
	/* Enable interrupts*/
	IFS1bits.U2RXIF = 0;
	IEC1bits.U2RXIE = 1;
}

/**
 * \brief Put char through UART2
 */
void esp_putch(char c)
{
    while(U2STAbits.UTXBF) continue;
    U2TXREG = c;
}


/**
 * @brief Send 'AT' command to check if ESP8266 is alive.
 */
void espAttention()
{

	esp_putch('A');
	esp_putch('T');
	esp_putch(0x0d);
	esp_putch(0xa);
}
