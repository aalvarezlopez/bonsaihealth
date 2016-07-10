/**
 * @file rtcc.c
 * @brief Real Time Clock and Calendar component. This library configure 
 * microcontroller peropheral.
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-12-04
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
#include "rtcc.h"
#include "log.h"


enum MASK{
	EVERY_SECOND = 1,
	EVERY_10_SECONDS ,
	EVERY_MINUTE,
	EVERY_10_MIN,
	EVERY_HOUR,
	EVERY_10_HOURS
};

/**
 * @brief Configure RTCC peripheral and enable SOSC
 */
void InitRTCC(uint8_t hour, uint8_t min)
{
	if( RCFGCALbits.RTCEN == 0 ){
		rtccUnlock();
		/*  enable SOSC 32 kHz */
		asm volatile ("mov #OSCCON,W1");
		asm volatile ("mov.b #0x46,W2");
		asm volatile ("mov.b #0x57,W3"); /*  Unlock secuence */
		asm volatile ("mov.b #0x02,W0"); /*  Unlock secuence  */
		asm volatile ("mov.b W2, [W1]"); /*  SOSCEN = 1 */
		asm volatile ("mov.b W3, [W1]");
		asm volatile ("mov.b W0, [W1]");
		RCFGCALbits.RTCEN = 1;
		rtccLock();
		rtccSetClock( DEF_WEEKDAY, hour, min, DEF_SEC);
		rtccSetDate( DEF_YEAR, DEF_MONTH, DEF_DAY);
	}
	configureAlarm();
}

inline void rtccLock()
{
	asm("MOV #NVMKEY, W1 ;move the address of NVMKEY into W1\n"\
		"MOV #0x55, W2\n"\
		"MOV W2, [W1] ;start 55/AA sequence\n"\
		"MOV #0xAA, W3\n"\
		"MOV W3, [W1]\n"\
		"BCLR RCFGCAL, #13 ;clear the RTCWREN bit\n"\
	);
}

inline void rtccUnlock()
{
	asm("MOV #NVMKEY, W1 ;move the address of NVMKEY into W1\n"\
		"MOV #0x55, W2\n"\
		"MOV W2, [W1] ;start 55/AA sequence\n"\
		"MOV #0xAA, W3\n"\
		"MOV W3, [W1]\n"\
		"BSET RCFGCAL, #13 ;set the RTCWREN bit\n"\
	);
}

void rtccReadClock( uint8_t *week_day, uint8_t *hour, uint8_t *minutes, uint8_t *seconds)
{
	uint16_t min_secs_reg;
	uint16_t weekday_hour_reg;

	RCFGCALbits.RTCPTR = DAY_HOURS;
	weekday_hour_reg = RTCVAL;
	*week_day = (weekday_hour_reg >> 8) & 0x3;
	*hour = (weekday_hour_reg & 0xF) +\
			(((weekday_hour_reg >> 4) & 0xF) * 10);
	min_secs_reg = RTCVAL;
	*minutes = ((min_secs_reg >> 8) & 0xF) +\
			(((min_secs_reg >> 12) & 0xF) * 10);
	*seconds = ((min_secs_reg >> 0) & 0xF) +\
			(((min_secs_reg >> 4) & 0xF) * 10);
}


void rtccReadDate(uint8_t *year, uint8_t *month, uint8_t *day)
{
	uint16_t day_month_reg;
	uint16_t year_reg;

	RCFGCALbits.RTCPTR = YEAR;
	year_reg = RTCVAL;
	*year = (year_reg & 0xF) +\
			(((year_reg >> 4) & 0xF) * 10);
	day_month_reg = RTCVAL;
	*month = ((day_month_reg >> 8) & 0xF) +\
			(((day_month_reg >> 12) & 0xF) * 10);
	*day = ((day_month_reg >> 0) & 0xF) +\
			(((day_month_reg >> 4) & 0xF) * 10);
}

void rtccSetClock( uint8_t week_day, uint8_t hour, uint8_t minutes, uint8_t seconds)
{
	hour = hour > MAX_HOUR ? MAX_HOUR : hour;
	minutes = minutes > MAX_MIN ? MAX_MIN : minutes;
	seconds = seconds > MAX_SEC ? MAX_SEC : seconds;
	rtccUnlock();
	RCFGCALbits.RTCPTR = MIN_SEC;
	RTCVAL = (( minutes /10) << 12) |\
			 (( minutes % 10) << 8) |\
			 (( seconds / 10) << 4) |\
			 (( seconds % 10) << 0) ;

	RCFGCALbits.RTCPTR = DAY_HOURS;
	RTCVAL = (( week_day % 7) << 8) |\
			 (( hour / 10) << 4) |\
			 (( hour % 10) << 0) ;
	rtccLock();
}

void rtccSetDate( uint8_t year, uint8_t month, uint8_t day)
{
	day = day > MAX_DAY ? MAX_DAY : day;
	month = month > MAX_MONTH ? MAX_MONTH : month;

	rtccUnlock();
	RCFGCALbits.RTCPTR = MONTH_DAY;
	RTCVAL = (( month /10) << 12) |\
			 (( month % 10) << 8) |\
			 (( day / 10) << 4) |\
			 (( day % 10) << 0) ;

	RCFGCALbits.RTCPTR = YEAR;
	RTCVAL = (( year / 10) << 4) |\
			 (( year % 10) << 0) ;
	rtccLock();
}


void configureAlarm()
{
	ALCFGRPTbits.CHIME = 1;
	ALCFGRPTbits.AMASK = EVERY_10_MIN;
	rtccUnlock();
	ALCFGRPTbits.ALRMPTR = 2;
	ALRMVAL =  0;
	ALRMVAL =  0;
	ALRMVAL =  0;
	rtccLock();
	IFS3bits.RTCIF = 0;
	IEC3bits.RTCIE = 1;
	ALCFGRPTbits.ALRMEN = 1;
}


void RTCC_Alarm_TASK();

void __attribute__((interrupt,auto_psv)) _RTCCInterrupt()
{
	RTCC_Alarm_TASK();
}
