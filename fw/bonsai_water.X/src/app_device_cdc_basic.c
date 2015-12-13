/********************************************************************
  Software License Agreement:

  The software supplied herewith by Microchip Technology Incorporated
  (the "Company") for its PIC(R) Microcontroller is intended and
  supplied to you, the Company's customer, for use solely and
  exclusively on Microchip PIC Microcontroller products. The
  software is owned by the Company and/or its supplier, and is
  protected under applicable copyright laws. All rights are reserved.
  Any use in violation of the foregoing restrictions may subject the
  user to criminal sanctions under applicable laws, as well as to
  civil liability for the breach of the terms and conditions of this
  license.

  THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
  WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
  TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
  IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
  CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/

/** INCLUDES *******************************************************/
#include "system.h"

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "usb.h"

#include "app_device_cdc_basic.h"
#include "usb_config.h"

#include "log.h"
#include "esp8266.h"

#define MAX_MSG_LEN 100
/** VARIABLES ******************************************************/

static uint8_t usb_out_data[CDC_DATA_OUT_EP_SIZE];
static uint8_t usb_in_data[CDC_DATA_IN_EP_SIZE];
static uint8_t msg_in[MAX_MSG_LEN];
static uint8_t msg_out[MAX_MSG_LEN];
static uint8_t msg_from_esp[MAX_MSG_LEN];
static uint8_t msg_len = 0;

extern uint8_t  dgn_pump_state;
extern uint8_t dgn_pump_ctrl;

enum MENU_SELECTION{
	MAIN_MENU,
	TIME,
	WIFI,
	SNAPSHOT,
	PUMP_CONTROL
};

#define MAIN_MENU_KEY 'm'
enum{
	MAIN_TO_TIME = 't',
	MAIN_TO_WIFI = 'w',
	MAIN_TO_SNAPSHOT = 's',
	NEW_SNAPSHOT = 'n',
	MAIN_TO_PUMP_CTRL = 'p',
	TURN_ON_PUMP_KEY = '1',
	TURN_OFF_PUMP_KEY = '0'
};

static uint8_t current_menu_selected = MAIN_MENU;

unsigned char  NextUSBOut;
unsigned char    NextUSBOut;
USB_HANDLE  lastTransmission;

uint8_t _temperature = 0;
uint8_t _hour, _minutes, _seconds;
uint8_t _soil_wet, _light_val;

#define PRINT_MAIN_MENU() do{\
	sprintf(msg_out, "MAIN MENU\n\n\r[s] get snapshot\r\n"\
		"[t] time menu\r\n"\
		"[p] Pump manual control\r\n"\
		"[w] wifi menu\r\n******\r\n");\
}while(0)

#define PRINT_NEW_SNAPSHOT() do{\
	sprintf( msg_out, "TIME:%02d:%02d:%02d Temperature: %d"\
		" Soil: %d Light: %d\r\n", _hour, _minutes, _seconds,\
		_temperature, _soil_wet, _light_val);\
}while(0)

#define PRINT_SNAPSHOT_MENU() do{\
	sprintf(msg_out, "SNAPSHOT MENU\n\n\r[n] new snapshot\r\n"\
		"[m] main menu\r\n******\r\n");\
}while(0)


#define PRINT_WIFI_MENU() do{\
	sprintf(msg_out, "WIFI MENU:\n\r"\
		"Print ESP commands,"\
		" \"quit\" to come back to main menu\n\r");\
}while(0)

#define PRINT_PUMP_CTRL() do{\
	sprintf(msg_out, "PUMP MANUAL CONTROL:\n\r"\
		"[1] to turn on\r\n"\
		"[0] to turn off\n\r");\
}while(0)

extern char esp_rx_buf[];
extern uint16_t esp_n_rx;
uint16_t esp_n_tx = 0;


void printMenu();

/*********************************************************************
 * Function: void APP_DeviceCDCEmulatorInitialize(void);
 *
 * Overview: Initializes the demo code
 *
 * PreCondition: None
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
void APP_DeviceCDCEmulatorInitialize()
{
	CDCInitEP();
}

void mainMenuSt(char ch)
{
	switch(ch){
		case MAIN_TO_TIME:
			current_menu_selected = TIME;
			break;
		case MAIN_TO_WIFI:
			PRINT_WIFI_MENU();
			current_menu_selected = WIFI;
			break;
		case MAIN_TO_SNAPSHOT:
			PRINT_NEW_SNAPSHOT();
			current_menu_selected = SNAPSHOT;
			break;
		case MAIN_TO_PUMP_CTRL:
			PRINT_PUMP_CTRL();
			dgn_pump_ctrl = 1;
			current_menu_selected = PUMP_CONTROL;
			break;
		case MAIN_MENU_KEY:
		default:
			PRINT_MAIN_MENU();
			current_menu_selected = MAIN_MENU;
	}
}

void timeSt()
{

}

void wifiSt(char *str, uint8_t length)
{
	if( strncmp(str+length-4, "quit", 4) == 0 ){
		PRINT_MAIN_MENU();
		current_menu_selected = MAIN_MENU;
	} else {
		strncpy(msg_out, str, length);
		msg_out[ length ] = 0;
		strcat( msg_out,"\r\n");
		for( int i=0; i < length; i++){
			esp_putch(str[i]);
		}
		espEndCmd();
	}
}

void snapshotSt(char ch)
{
	uint8_t week_day;
	switch(ch){
		case MAIN_MENU_KEY:
			PRINT_MAIN_MENU();
			current_menu_selected = MAIN_MENU;
			break;
		case NEW_SNAPSHOT:
			_temperature = readTemperature();
			rtccReadClock( &week_day, &_hour, &_minutes, &_seconds);
			lightSensorTask();
			_soil_wet = getSoilWet();
			_light_val = getLight();
			PRINT_NEW_SNAPSHOT();
			break;
		default:
			PRINT_SNAPSHOT_MENU();
	}
}

void pumpCtrlSt( char ch )
{
	switch(ch){
		case MAIN_MENU_KEY:
			PRINT_MAIN_MENU();
			dgn_pump_ctrl = 0;
			current_menu_selected = MAIN_MENU;
			break;
		case TURN_ON_PUMP_KEY:
			memset(msg_out, 0, MAX_MSG_LEN);
			dgn_pump_state = 1;
			break;
		case TURN_OFF_PUMP_KEY:
			memset(msg_out, 0, MAX_MSG_LEN);
			dgn_pump_state = 0;
			break;
		default:
			PRINT_PUMP_CTRL();
	}
}

void parseMsg()
{
	for( int i = 0 ; i < msg_len % MAX_MSG_LEN; i++){
		if ( msg_in[i] == '\n' || msg_in[i] == '\r'  ){
			switch(current_menu_selected){
				case MAIN_MENU:
					mainMenuSt(msg_in[i-1]);
					break;
				case TIME:
					timeSt();
					break;
				case WIFI:
					wifiSt(msg_in, i);
					break;
				case SNAPSHOT:
					snapshotSt(msg_in[i-1]);
					break;
				case PUMP_CONTROL:
					pumpCtrlSt(msg_in[i-1]);
					break;
				default:
					current_menu_selected = MAIN_MENU;
			}
			printMenu();
			memset(msg_in, 0, MAX_MSG_LEN);
			msg_len = 0;
			continue;
		}
	}
}

void printMenu()
{
	if(USBUSARTIsTxTrfReady())
	{
		putUSBUSART(msg_out, strlen(msg_out));
	}
}

uint8_t printESPreply()
{
	uint8_t printed = 0;
	if(USBUSARTIsTxTrfReady())
	{
		putUSBUSART(msg_from_esp, strlen(msg_from_esp));
		printed = 1;
	}
	return printed;
}

/*********************************************************************
 * Function: void APP_DeviceCDCEmulatorTasks(void);
 *
 * Overview: Keeps the demo running.
 *
 * PreCondition: The demo should have been initialized and started via
 *   the APP_DeviceCDCEmulatorInitialize() and APP_DeviceCDCEmulatorStart() demos
 *   respectively.
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
void APP_DeviceCDCEmulatorTasks()
{
	uint8_t usb_len;
	static uint8_t esp_n_bytes = 0;
	if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
	usb_len = getsUSBUSART(usb_in_data,64); //until the buffer is free.
	for ( int i=0; i < usb_len; i++){
		msg_in[msg_len % MAX_MSG_LEN] = usb_in_data[i];
		msg_len++;
	}

	parseMsg();
	/* If wifi is selected connect ESP transmission to USB */
	if ( current_menu_selected == WIFI && strlen(msg_out) == 0){

		while( esp_n_rx != esp_n_tx && esp_n_bytes < CDC_DATA_OUT_EP_SIZE ){
			msg_from_esp[esp_n_bytes] = esp_rx_buf[esp_n_tx % ESP_MAX_IN_LEN];
			esp_n_tx++;
			esp_n_bytes++;
		}

		if( esp_n_bytes > 0){
			msg_from_esp[esp_n_bytes] = 0;
			if( printESPreply() ){
				esp_n_bytes = 0;
			}
		}
	}

	CDCTxService();
}
