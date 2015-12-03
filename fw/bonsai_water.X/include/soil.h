#ifndef SOIL_H
#define SOIL_H

#define TRIS_DO_SOIL TRISB
#define PIN_SOIL PORTBbits.RB7
#define PIN_SOIL_N 7

void soilInit();
void soilTask();
uint8_t getSoilWet();

#endif
