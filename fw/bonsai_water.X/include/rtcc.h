/**
 * @file rtcc.h
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-12-04
 */

#ifndef RTCC_H
#define RTCC_H

enum DAY{Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};
enum RTCC_PTR{ MIN_SEC, DAY_HOURS, MONTH_DAY, YEAR};

#define MAX_HOUR 23
#define MAX_MIN 59
#define MAX_SEC 59
#define MAX_DAY 31
#define MAX_MONTH 12

#ifndef EXT_DEF_TIME
#define DEF_HOUR 12
#define DEF_MIN 30
#define DEF_SEC 45
#define DEF_WEEKDAY 1
#define DEF_DAY 27
#define DEF_MONTH 3
#define DEF_YEAR 15
#endif

void InitRTCC(uint8_t hour, uint8_t min);
void rtccLock();
void rtccUnlock();
void rtccReadClock( uint8_t *week_day, uint8_t *hour, uint8_t *minutes, uint8_t *seconds);
void rtccReadDate(uint8_t *year, uint8_t *month, uint8_t *day);
void rtccSetClock( uint8_t week_day, uint8_t hour, uint8_t minutes, uint8_t seconds);
void rtccSetDate( uint8_t year, uint8_t month, uint8_t day);
#endif
