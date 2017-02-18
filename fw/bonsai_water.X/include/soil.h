#ifndef SOIL_H
#define SOIL_H

#define TRIS_DO_SOIL TRISB
#define PIN_SOIL PORTBbits.RB7
#define PIN_SOIL_N 7
#define DUTY_ON 0x5FFF

typedef enum PUMP_ST{
	PUMP_OFF,
	PUMP_ON,
	PUMP_END
}T_PUMP_ST;

enum{
	CYCLEON_ENABLE,
	CYCLEON_DISABLE
};

typedef struct{
	T_PUMP_ST status;
	struct{
		uint8_t numberOfCycles;
		uint8_t nTon;
		uint8_t nToff;
	}cfg;
	uint8_t numberOfCycles;
	uint8_t nTon;
	uint8_t nToff;
	uint8_t cycleOn;
} T_PUMP;

void soilInit();
void soilTask();
uint8_t getSoilWet();

#endif
