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

 #include <stdint.h>          /* For uint32_t definition */
 #include <stdbool.h>         /* For true/false definition */
 #include <stdio.h>

 #include "user.h"            /* variables/params used by user.c */
 #include "log.h"
 #include "datastorage.h"
 #include "system.h"
 #include "libpic30.h"

 #define _XTAL_FREQ  8000000     // oscillator frequency for _delay()

extern uint8_t pumpPeriodEnable;
/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
    uint8_t lastTimeHour, lastTimeMin = 0u;
    //configure I/O
    AD1PCFG = 0xFFFFFFFF;

    TRISG = TRISG & ~(1 << 6); /* PUMP CONTROL */
    RPOR10 = 18 << 8;          /* RP21 as output control 1*/
    TRISE = TRISE & ~(1 << 5);
    MUX_EN = 0;

    OC1CON1bits.OCTSEL = 0b111;
    OC1CON1bits.ENFLT0 = 0;
    OC1CON1bits.OCM = 7;
    OC1CON2bits.SYNCSEL = 1;

    OC1R = 0x0;
    OC1RS = 0x0;


    configureLog();
    initLog();
    configureI2C();
    configureADC();
    soilInit();
    storageInit(&lastTimeHour, &lastTimeMin);
    InitRTCC( lastTimeHour, lastTimeMin);
}


void RTCC_Alarm_TASK()
{
    char str[100];
    uint8_t temperature = readTemperature();
    uint8_t week_day, hour, minutes, seconds;
    uint8_t year, month, day;
    uint8_t soil_wet, light_val;
    lightSensorTask();
    LOG_DBG("ALARMA");
    soil_wet = getSoilWet();
    light_val = getLight();
    rtccReadClock( &week_day, &hour, &minutes, &seconds);
    rtccReadDate(&year, &month, &day);
    sprintf( str, "Temperature: %d Soil: %d Light: %d %d:%d:%d %d/%d/%d", temperature,\
        soil_wet, light_val, hour, minutes, seconds, day, month, year);
    LOG_DBG(str);
    storage_data data;
    data.temperature = temperature;
    data.pump_state = pumpPeriodEnable;
    data.soil = soil_wet;
    data.light = light_val;
    data.hour = hour;
    data.minutes = minutes;
    data.seconds = seconds;
    storageAppendData( data );
    //dumpMem();
    pumpPeriodEnable = 0;
    IFS3bits.RTCIF = 0;
}

void RTCCTriggerAcq()
{
    RTCC_Alarm_TASK();
}
