/**
 * @file datastorage.h
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-12-05
 */

#ifndef DATASTORAGE_H
#define DATASTORAGE_H

struct STORAGE{
	uint8_t temperature : 7;
	uint8_t pump_state : 1;
	uint8_t soil;
	uint8_t light;
	uint8_t hour;
	uint8_t minutes;
	uint8_t seconds;
};

typedef struct STORAGE storage_data;

void storageAppendData( storage_data to_write );
uint8_t storageGetData( uint8_t index, storage_data *data );
void storageClean();

#endif
