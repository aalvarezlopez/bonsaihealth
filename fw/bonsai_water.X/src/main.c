/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
    #include <xc.h>
#elif defined(__C30__)
    #if defined(__PIC24E__)
    	#include <p24Exxxx.h>
    #elif defined (__PIC24F__)||defined (__PIC24FK__)
	#include <p24Fxxxx.h>
    #elif defined(__PIC24H__)
	#include <p24Hxxxx.h>
    #endif
#endif

#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */
#include "libpic30.h"

#include "eeprom.h"
#include "tc74.h"
#include "log.h"
#include "esp8266.h"
#include "soil.h"
#include "light_sensor.h"

#include "usb.h"
#include "usb_device.h"
#include "usb_device_cdc.h"
/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/

/* i.e. uint16_t <variable_name>; */

/******************************************************************************/
/* Main Program                                                               */
/******************************************************************************/

extern char esp_rx_buf[];
extern unsigned int esp_n_rx;

int16_t main(void)
{
    uint8_t buffer[] = {1,2,4,5,6};
    uint8_t soil_wet, light_val;
    char str[100];
    /* Configure the oscillator for the device */
    ConfigureOscillator();
    InitApp();
    configureLog();
    initLog();
    soilInit();
    /* Initialize IO ports and peripherals */
    USBDeviceInit();
    USBDeviceAttach();
    LOG_DBG("Device attached");
	ESP8266Init();
	#if 0
	while(1){
		espAttention();
		soilTask();
		__delay_ms(500);
		soil_wet = getSoilWet();
		sprintf(str, "Soil wet %x", soil_wet);
		LOG_DBG(str);
		for(int i =0; i < esp_n_rx; i++){
			putch(*(esp_rx_buf+i));
		}
		esp_n_rx = 0;
	}
	#endif

    /* TODO <INSERT USER APPLICATION CODE HERE> */
    eepromWrite(2,0,buffer);
    while(1)
    {
		uint8_t temperature = readTemperature();
		soilTask();
		lightSensorTask();
		espAttention();
		/* esp_n_rx should be reset once read reply */
		esp_n_rx = 0;
		soil_wet = getSoilWet();
		light_val = getLight();
		sprintf( str, "Temperature: %d Soil: %d Light: %d", temperature,\
			soil_wet, light_val);
		LOG_DBG(str);
        //eepromRead(4,0,buffer);
        if( USBGetDeviceState() < CONFIGURED_STATE ){
		} else if ( USBIsDeviceSuspended() == true ) {
		} else {
			APP_DeviceCDCEmulatorTasks();
		}
    }
}

bool USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, uint16_t size)
{
    switch( (int) event )
    {
        case EVENT_TRANSFER:
            break;

        case EVENT_SOF:
            break;

        case EVENT_SUSPEND:
            break;

        case EVENT_RESUME:
            break;

        case EVENT_CONFIGURED:
            /* When the device is configured, we can (re)initialize the 
             * demo code. */
            APP_DeviceCDCEmulatorInitialize();
            break;

        case EVENT_SET_DESCRIPTOR:
            break;

        case EVENT_EP0_REQUEST:
            /* We have received a non-standard USB request.  The HID driver
             * needs to check to see if the request was for it. */
            USBCheckCDCRequest();
            break;

        case EVENT_BUS_ERROR:
            break;

        case EVENT_TRANSFER_TERMINATED:
            break;

        default:
            break;
    }
    return true;
}
