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

#define MAX_MSG_LEN 100
/** VARIABLES ******************************************************/

static uint8_t usb_out_data[CDC_DATA_OUT_EP_SIZE];
static uint8_t usb_in_data[CDC_DATA_IN_EP_SIZE];
static uint8_t msg_in[MAX_MSG_LEN];
static uint8_t msg_out[MAX_MSG_LEN];
static uint8_t msg_len = 0;
enum MENU_SELECTION{
	MAIN_MENU,
	TIME,
	WIFI,
	SNAPSHOT
};

#define MAIN_MENU_KEY 'm'
enum{
	MAIN_TO_TIME = 't',
	MAIN_TO_WIFI = 'w',
	MAIN_TO_SNAPSHOT = 's',
	NEW_SNAPSHOT = 'n'
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
	sprintf(msg_out, "WIFI MENU\n\n\r"\
		"Print commands to send to ESP,"\
		" \"quit\" to come back to main menu\r\n");\
}while(0)

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
			soilTask();
			lightSensorTask();
			_soil_wet = getSoilWet();
			_light_val = getLight();
			PRINT_NEW_SNAPSHOT();
			break;
		default:
			PRINT_SNAPSHOT_MENU();
	}
}

void parseMsg()
{
	memset(msg_out, 0, MAX_MSG_LEN);
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
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;
	usb_len = getsUSBUSART(usb_in_data,64); //until the buffer is free.
	for ( int i=0; i < usb_len; i++){
		msg_in[msg_len % MAX_MSG_LEN] = usb_in_data[i];
		msg_len++;
	}
	parseMsg();

	/* If wifi is selected connect ESP transmission to USB */
	if ( current_menu_selected == WIFI ){

	}

    CDCTxService();
}
