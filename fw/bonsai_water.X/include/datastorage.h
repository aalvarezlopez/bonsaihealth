/**
 * @file datastorage.h
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-12-05
 */

#ifndef DATASTORAGE_H
#define DATASTORAGE_H

struct STORAGE{
	uint8_t temperature;
	uint8_t soil;
	uint8_t light;
};

typedef struct STORAGE storage_data;

void storageAppendData( storage_data to_write );
uint8_t storageGetData( uint8_t index, storage_data *data );

#endif
