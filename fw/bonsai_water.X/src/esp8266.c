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
#include "datastorage.h"
#include "user.h"

#define CONNECTION_REQUEST_MSG "+IPD"
#define SEND_DATA_RPLY_ACK "OK"
#define DATA_SENT "SEND OK"
#define AT_COMMAND_ECHO "AT"
#define AT_REPLY_OK "OK"

extern uint16_t esp_n_rx;
uint16_t esp_n_rd = 0;
#define MAX_ESP_N_RANGE (uint16_t)((uint16_t)0 - 1);
#define MAX_ESP_CMD_LEN 32
#define MAX_HTTP_SIZE 2048
#define DEFAULT_HTTP_TIMEOUT 2
#define DEFAULT_TIMEOUT 2

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
void espPrint(char *str);

#define TMR_PR_256 3
#define TMR_TCY_SRC 0
#define TMR_PRESCALER 156200
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
	// Configure timer source 
	PR3 = TMR_PRESCALER >> 16;
	PR2 = TMR_PRESCALER & 0xFFFF;
	T2CONbits.T32 = 1;
	T2CONbits.TCKPS = TMR_PR_256;
	T2CONbits.TCS = TMR_TCY_SRC;
	T2CONbits.TON = 1;
	IFS0bits.T3IF = 0;
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
	uint16_t size;
	char data_register[50];
	uint8_t temperature, soil_wet, light_val;
	uint8_t week_day, hour, minutes, seconds;
	uint8_t measurement_index = 0;
	static storage_data data;
	temperature = readTemperature();
	rtccReadClock( &week_day, &hour, &minutes, &seconds);
	lightSensorTask();
	soil_wet = getSoilWet();
	light_val = getLight();
	/* fill snapshot and histogram */
	sprintf(snapshot_page, SNAPSHOT_MACRO,hour, minutes, seconds, \
		temperature,light_val,soil_wet,PUMP);
	sprintf(data_hist_page, "");
	while(storageGetData( measurement_index, &data )){
		measurement_index++;
		sprintf(data_register, DATA_HIST_MACRO, data.hour, data.minutes, data.seconds,\
			data.temperature,data.light,data.soil,data.pump_state*100);
		strcat(data_hist_page, data_register);
		strcat(data_hist_page, ",");
	}
	data_hist_page[strlen(data_hist_page)-1]=0;

	size = strlen(head_page) + strlen(data_hist_page)+strlen(body_page) +\
					strlen(snapshot_page) + strlen(bot_page);
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
		if ( *(http+i) == 0){
			break;
		}
		esp_putch( *(http+i) );
	}
}

void sendMainPage()
{
	sendHTTP(head_page);
	sendHTTP(data_hist_page);
	sendHTTP(body_page);
	sendHTTP(snapshot_page);
	sendHTTP(bot_page);
	espEndCmd();
}

uint8_t isHttpSend()
{
	uint8_t ready = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, DATA_SENT);
	if (ptr != NULL){
		LOG_DBG("HTTP sent");
		ready = 1;
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


uint8_t getStatus()
{
	uint8_t status_ok = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, AT_COMMAND_ECHO);
	if (ptr != NULL){
		ptr = strstr( esp_rx_buf, AT_REPLY_OK);
		if (ptr != NULL){
			status_ok = 1;
			clearBuffer();
		}
	} else {
		espAttention();
		espEndCmd();
	}
	return status_ok;
}

void ESP8266Task()
{
	static uint16_t sending_http_timeout = DEFAULT_HTTP_TIMEOUT;
	static uint8_t timeout_status = 0;
	static int8_t timeout_count = DEFAULT_TIMEOUT;

	if( IFS0bits.T3IF ){
		timeout_status = 1;
		IFS0bits.T3IF = 0;
	} else {
		timeout_status = 0;
	}

	if( timeout_status  ){
		if ( current_state != ESP_ST_WAITING_CONNECTION ){
			timeout_count--;
			if ( timeout_count < 0){
				timeout_count = DEFAULT_TIMEOUT;
				current_state = ESP_ST_WAITING_CONNECTION;
				closeConnection();
			}
		} else {
			timeout_count = DEFAULT_TIMEOUT;
		}
	}

	switch(current_state){
		case ESP_ST_WAITING_CONNECTION:
			if( isConnectionRequest() ){
				current_state++;
				goToConnecting();
				timeout_count = DEFAULT_TIMEOUT;
			} else {
				if( timeout_status ){
					getStatus();
				}
			}
			break;
		case ESP_ST_CONNECTING:
			if( isConnected() ){
				current_state++;
				goToSending();
				timeout_count = DEFAULT_TIMEOUT;
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_SENDING:
			if( readyToSend() ){
				sendMainPage();
				current_state++;
				timeout_count = DEFAULT_TIMEOUT;
				sending_http_timeout = DEFAULT_HTTP_TIMEOUT;
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_WAITING_SENT:
			if( isHttpSend() ){
				closeConnection();
				current_state = ESP_ST_WAITING_CONNECTION;
			} else {
				if ( timeout_status ){
					if( sending_http_timeout < 0){
						if( getStatus()){
							closeConnection();
							current_state = ESP_ST_WAITING_CONNECTION;
						}
						sending_http_timeout = DEFAULT_HTTP_TIMEOUT;
					} else {
						sending_http_timeout--;
					}
				}
				
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


void espPrint(char *str)
{
	for(int i=0; i < MAX_ESP_CMD_LEN; i++){
		if ( str[i] == 0){
			break;
		}
		esp_putch( str[i] );
	}
	espEndCmd();
}

void clearBuffer()
{
	memset(esp_rx_buf, 0, ESP_MAX_IN_LEN);
	esp_n_rx = 0;
	esp_n_rd = 0;
}
