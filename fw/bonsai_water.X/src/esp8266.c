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

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "libpic30.h"
#define CONNECTION_REQUEST_MSG "+IPD"
#define SEND_DATA_RPLY_ACK "OK"
#define DATA_SENT "SEND OK"
#define AT_COMMAND_ECHO "AT"
#define AT_REPLY_OK "OK"

#define OFFSET_VAR "offset"
#define STEP_VAR "step"

extern uint16_t esp_n_rx;
uint16_t esp_n_rd = 0;
#define MAX_ESP_N_RANGE (uint16_t)((uint16_t)0 - 1);
#define MAX_ESP_CMD_LEN 64
#define MAX_HTTP_SIZE 2048
#define DEFAULT_HTTP_TIMEOUT 2
#define DEFAULT_TIMEOUT 2
#define MAX_POINTS_IN_CHART 14

enum{
	ESP_ST_WAITING_CONNECTION,
	ESP_ST_CONNECTING,
	ESP_ST_SEND_CMD,
	ESP_ST_SEND_MSG
};


enum ESP_SEND_HTTP_STEP{
	ESP_SEND_HEADER,
	ESP_SEND_DATA,
	ESP_SEND_BODY,
	ESP_SEND_SNAPSHOT,
	ESP_SEND_FOOT,
	ESP_SEND_END
};

const char esp_commands[][20] = {"CWMODE", "CWJAP", "CIFSR", "CIPMUX", "CIPSERVER"};

uint8_t current_state = ESP_ST_WAITING_CONNECTION;
enum ESP_SEND_HTTP_STEP current_http_step = ESP_SEND_HEADER;

char esp_rx_buf[ESP_MAX_IN_LEN];
uint16_t esp_n_rx = 0;

uint16_t nbytes_to_receive = 0;

uint16_t chart_step = 0;
uint16_t chart_offset = 0;

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
	char temp[512];
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
	// Configure ESP8266

	espSendCommands(ESP_IP, NULL, temp);
	LOG_DBG("**** IP ****");
	LOG_DBG(temp);
	espSendCommands(ESP_MUX, "1", temp);
	LOG_DBG("**** MUX****");
	LOG_DBG(temp);
	espSendCommands(ESP_SERVER, "1,80", temp);
	LOG_DBG("**** SERVER ****");
	LOG_DBG(temp);
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

#define OFFSET_VAR "offset"
#define STEP_VAR "step"

void getChartOffset()
{
	uint16_t offset = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, OFFSET_VAR);
	if (ptr != NULL){
		offset = atoi(ptr+strlen(OFFSET_VAR) + 1u);
	}
	chart_offset = offset;
}

void getChartSteps()
{
	uint16_t step = 0;
	char *ptr;
	ptr = strstr( esp_rx_buf, STEP_VAR);
	if (ptr != NULL){
		step = atoi(ptr + strlen(STEP_VAR) + 1u);
	}
	chart_step = step;
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
	getChartOffset();
	getChartSteps();
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
	sprintf(str, "N bytes to receive =%d\r\n N bytes received =%d"\
		" OFFSET %d STEP = %d",
		nbytes_to_receive, nbytes_received, chart_offset, chart_step);
	LOG_DBG(str);
	if ( nbytes_received >= nbytes_to_receive ){
		is_connected = 1;
	}
	return is_connected;
}

uint16_t fillChartData( uint16_t offset, uint16_t step)
{
	char str[256];
	uint16_t number_of_points = 0;
	char data_register[256];
	uint16_t measurement_index = offset;
	static storage_data data;
	sprintf(data_hist_page, "");
	while(storageGetData( measurement_index, &data ) &&\
		number_of_points < MAX_POINTS_IN_CHART){
		sprintf(str, "****\r\nNew data read. %d\r\n", number_of_points);
		LOG_DBG(str);
		measurement_index+= step;
		sprintf(data_register, DATA_HIST_MACRO, data.hour, data.minutes, data.seconds,\
			data.temperature,data.light,data.soil,data.pump_state*100);
		strcat(data_hist_page, data_register);
		strcat(data_hist_page, ",");
		number_of_points++;
	}
	return number_of_points;
}

void fillSnapShotData()
{
	uint8_t temperature, soil_wet, light_val;
	uint8_t week_day, hour, minutes, seconds;
	temperature = readTemperature();
	rtccReadClock( &week_day, &hour, &minutes, &seconds);
	lightSensorTask();
	soil_wet = getSoilWet();
	light_val = getLight();
	/* fill snapshot and histogram */
	sprintf(snapshot_page, SNAPSHOT_MACRO,hour, minutes, seconds, \
		temperature,light_val,soil_wet,PUMP);
}

void goToSending()
{
	uint16_t size;
	char str[256];

	switch( current_http_step ){
		case ESP_SEND_HEADER:
			LOG_DBG("Sending header size");
			size = strlen(head_page);
			break;
		case ESP_SEND_DATA:
			LOG_DBG("Sending data size");
			size = strlen(data_hist_page);
			break;
		case ESP_SEND_BODY:
			LOG_DBG("Sending body size");
			size = strlen(body_page);
			break;
		case ESP_SEND_SNAPSHOT:
			LOG_DBG("Sending snapshot size");
			size = strlen(snapshot_page);
			break;
			break;
		case ESP_SEND_FOOT:
			LOG_DBG("Sending foot size");
			size = strlen(bot_page);
			break;
		default:
			size = 0u;
	}
	sprintf(str, "LENGTH=%d", size);
	LOG_DBG(str);
	clearBuffer();
	LOG_DBG("HTTP sending data");
	espSendCmd(size % MAX_HTTP_SIZE);
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

void sendHttpBlock()
{
	switch(current_http_step){
		case ESP_SEND_HEADER:
			LOG_DBG("Sending header");
			sendHTTP(head_page);
			break;
		case ESP_SEND_DATA:
			LOG_DBG("Sending data");
			sendHTTP(data_hist_page);
			break;
		case ESP_SEND_BODY:
			LOG_DBG("Sending body");
			sendHTTP(body_page);
			break;
		case ESP_SEND_SNAPSHOT:
			LOG_DBG("Sending snapshot");
			sendHTTP(snapshot_page);
			break;
		case ESP_SEND_FOOT:
			LOG_DBG("Sending foot");
			sendHTTP(bot_page);
			break;
		default:
			break;
	}
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
	static uint16_t total_number_of_points = 0u;
	static uint8_t number_of_points_send = 0;

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
				current_http_step = ESP_SEND_HEADER;
				goToSending();
				fillSnapShotData();
				total_number_of_points = 0u;
				number_of_points_send = 0;
				timeout_count = DEFAULT_TIMEOUT;
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_SEND_CMD:
			if( readyToSend() ){
				current_state++;
				if ( current_http_step == ESP_SEND_DATA &&
					total_number_of_points < 128 &&
						number_of_points_send != 0u){
					/* do nothing */
					data_hist_page[strlen(data_hist_page)-1]=',';
					sendHttpBlock();
				} else {
					data_hist_page[strlen(data_hist_page)-1]=' ';
					sendHttpBlock();
					current_http_step++;
				}
				timeout_count = DEFAULT_TIMEOUT;
				sending_http_timeout = DEFAULT_HTTP_TIMEOUT;
			} else {
				/*  nothing to do */
			}
			break;
		case ESP_ST_SEND_MSG:
			if( isHttpSend() ){
				if( current_http_step >= ESP_SEND_END){
					closeConnection();
					current_state = ESP_ST_WAITING_CONNECTION;
				} else if( current_http_step == ESP_SEND_DATA){
					number_of_points_send =
						fillChartData(total_number_of_points*1u + chart_offset , chart_step);
					total_number_of_points += number_of_points_send;
					current_state--;
					goToSending();
					timeout_count = DEFAULT_TIMEOUT;
				}else {
					current_state--;
					goToSending();
					timeout_count = DEFAULT_TIMEOUT;
				}
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


#define ESP_TIMEOUT_REPLY 50
uint8_t espSendCommands(uint8_t id, char *args, char *out)
{
	uint8_t timeout = ESP_TIMEOUT_REPLY;
	char cmd[MAX_ESP_CMD_LEN];
	char *ptr;
	uint8_t successful = false;
	clearBuffer();
	strcpy(cmd, AT_COMMAND_ECHO);
	strcat(cmd, "+");
	strcat(cmd, esp_commands[id]);
	if( args != NULL ){
		strcat(cmd, "=");
		strcat(cmd, args);
	}
	for(int i=0; i < MAX_ESP_CMD_LEN; i++){
		if ( cmd[i] == 0){
			break;
		}
		esp_putch( cmd[i] );
	}
	espEndCmd();
	/* Wait reply*/
	while( timeout > 0 ){
		timeout--;
		ptr = strstr( esp_rx_buf, SEND_DATA_RPLY_ACK);
		if (ptr != NULL){
			timeout = 0;
			successful = true;
			strcpy(out, esp_rx_buf);
		}
		__delay_ms(5);
	}
	clearBuffer();
	return successful;
}
