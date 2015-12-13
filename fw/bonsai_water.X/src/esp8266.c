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
#include <stdlib.h>

#include "log.h"
#include "http.h"

#define CONNECTION_REQUEST_MSG "+IPD"
#define SEND_DATA_RPLY_ACK "OK"
#define DATA_SENT "SEND OK"

extern uint16_t esp_n_rx;
uint16_t esp_n_rd = 0;
#define MAX_ESP_N_RANGE (uint16_t)((uint16_t)0 - 1);
#define MAX_ESP_CMD_LEN 32
#define MAX_HTTP_SIZE 2048

enum{
	ESP_ST_WAITING_CONNECTION,
	ESP_ST_CONNECTING,
	ESP_ST_SENDING,
	ESP_ST_WAITING_SENT
};
uint8_t current_state = ESP_ST_WAITING_CONNECTION;

char esp_rx_buf[ESP_MAX_IN_LEN];
uint16_t esp_n_rx = 0;

uint16_t nbytes_to_receive = 0;


void clearBuffer();
void espSendCmd(uint16_t len);

/**
 * @brief ESP8266 initialitation.
 * UART2 configuration
 */
void ESP8266Init()
{
	//configure serial port
	//pins TX 13 RX 12
	RPINR19 = RX2_PIN;
	RPOR_TX2 =  PIN_AS_TX2 << RPORTX2_PADDING;
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

uint8_t isConnectionRequest()
{
	uint8_t request = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, CONNECTION_REQUEST_MSG);
	if (ptr != NULL){
		LOG_DBG("Connection REQUEST");
		// Mark this message as read to avoid read this message again.
		*ptr = '#';
		esp_n_rd = ptr - esp_rx_buf;
		request = 1;
	} else {
	}
	return request;
}

#define MAX_HEADER_LEN 20
#define SPLIT_CHAR ','
#define END_CHAR ':'
#define MSG_LEN_ARG_POS 2
void goToConnecting()
{
	uint8_t n_splitted_args = 0;
	uint16_t msg_len_start_offset = 0;
	uint16_t msg_len_end_offset = 0;
	char len_str[ MAX_HEADER_LEN ];
	for( int i=0; i < MAX_HEADER_LEN; i++ ){
		if( esp_rx_buf[ (esp_n_rd + i) % ESP_MAX_IN_LEN] == SPLIT_CHAR ){
			n_splitted_args++;
		}
		if (n_splitted_args >= MSG_LEN_ARG_POS){
			esp_n_rd = esp_n_rd + i;
			msg_len_start_offset = esp_n_rd + 1;
			break;
		}
	}

	for( int i=0; i < MAX_HEADER_LEN; i++ ){
		if( esp_rx_buf[ (esp_n_rd + i) % ESP_MAX_IN_LEN] == END_CHAR ){
			esp_n_rd = esp_n_rd + i;
			msg_len_end_offset = esp_n_rd;
			break;
		}
	}
	if ( n_splitted_args >= MSG_LEN_ARG_POS &&\
		msg_len_end_offset != msg_len_start_offset ){

		for(int i = msg_len_start_offset; i < msg_len_end_offset; i++){
			len_str[ i - msg_len_start_offset ] = esp_rx_buf[ i % ESP_MAX_IN_LEN];
		}
		len_str[ msg_len_end_offset - msg_len_start_offset ] = 0;
		nbytes_to_receive = atoi(len_str);
	} else {
		nbytes_to_receive = 0;
	}
}

uint8_t isConnected()
{
	uint8_t is_connected = 0;
	uint16_t nbytes_received = esp_n_rx > esp_n_rd ? esp_n_rx - esp_n_rd : \
							   MAX_ESP_N_RANGE - esp_n_rd + esp_n_rx;
	char str[100];
	sprintf(str, "N bytes to receive =%d\r\n N bytes received =%d",\
		nbytes_to_receive, nbytes_received);
	LOG_DBG(str);
	if ( nbytes_received >= nbytes_to_receive ){
		is_connected = 1;
	}
	return is_connected;
}

void goToSending()
{
	char str[20];
	uint16_t size = strlen(main_page);
	sprintf(str, "LENGTH=%d", size);
	LOG_DBG(str);
	clearBuffer();
	LOG_DBG("HTTP sending data");
	espSendCmd(size);
}

uint8_t readyToSend()
{
	uint8_t ready = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, SEND_DATA_RPLY_ACK);
	if (ptr != NULL){
		LOG_DBG("Data ready to be sent");
		ready = 1;
	} else {
	}
	return ready;
}

void sendHTTP(char *http)
{
	for(uint16_t i=0; i < MAX_HTTP_SIZE; i++){
		if ( main_page[i] == 0){
			break;
		}
		esp_putch( main_page[i] );
	}
	espEndCmd();
}

void sendMainPage()
{
	sendHTTP(main_page);
}

uint8_t isHttpSend()
{
	uint8_t ready = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, DATA_SENT);
	if (ptr != NULL){
		LOG_DBG("HTTP sent");
		ready = 1;
	} else {
		LOG_DBG(esp_rx_buf);
	}
	return ready;
}


void closeConnection()
{
	char cmd[MAX_ESP_CMD_LEN];
	sprintf(cmd, "AT+CIPCLOSE=0");
	for(int i=0; i < MAX_ESP_CMD_LEN; i++){
		if ( cmd[i] == 0){
			break;
		}
		esp_putch( cmd[i] );
	}
	espEndCmd();
}

void ESP8266Task()
{
	switch(current_state){
		case ESP_ST_WAITING_CONNECTION:
			if( isConnectionRequest() ){
				current_state++;
				goToConnecting();
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_CONNECTING:
			if( isConnected() ){
				current_state++;
				goToSending();
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_SENDING:
			if( readyToSend() ){
				sendMainPage();
				current_state++;
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_WAITING_SENT:
			if( isHttpSend() ){
				closeConnection();
				current_state = ESP_ST_WAITING_CONNECTION;
			}
			break;
		default:
			current_state = ESP_ST_WAITING_CONNECTION;
	}
}

/**
 * \brief Put char through UART2
 */
void esp_putch(char c)
{
    while(U2STAbits.UTXBF) continue;
    U2TXREG = c;
}

void espSendCmd(uint16_t len)
{
	char cmd[MAX_ESP_CMD_LEN];
	sprintf(cmd, "AT+CIPSEND=0,%d", len);
	for(int i=0; i < MAX_ESP_CMD_LEN; i++){
		if ( cmd[i] == 0){
			break;
		}
		esp_putch( cmd[i] );
	}
	espEndCmd();
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

void espEndCmd()
{
	esp_putch(0x0d);
	esp_putch(0xa);
}

void clearBuffer()
{
	memset(esp_rx_buf, 0, ESP_MAX_IN_LEN);
	esp_n_rx = 0;
	esp_n_rd = 0;
}
