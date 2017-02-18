/**
 * @file soil.c
 * @brief Module that read soil humedity. Both digital and analog values.
 * @author Adrian Alvarez
 * @version 1.0.0
 * @date 2015-11-30
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
 #include "soil.h"
 #include "adc.h"
 #include "log.h"
 #include "user.h"

 #define TMR1_PRESCALER 25000u // 100 ms => T1_PRS* 2 / Tcy
 #define TMR1_PR_64 2
 #define TMR1_TCY_SRC 0
 #define N_SOIL_ACQ 10

 #define DEFAULT_RAW_LOW_LEVEL 50u
 #define DEFAULT_RAW_HIGH_LEVEL 80
 #define DEFAULT_RAW_MAX_LEVEL 100

 #define  DEFAULT_TON_CFG 40
 #define  DEFAULT_TOFF_CFG 80
 #define  DEFAULT_NCYCLES_CFG 15

static uint8_t soil_wet = 0;
static uint8_t pump_hal = 0;
static uint8_t pump_sal = 0;
uint8_t pumpPeriodEnable = 0;
static uint8_t soil_wet_buffer[N_SOIL_ACQ];
static uint8_t *ptr_write = soil_wet_buffer;
static uint8_t raw_low_level = DEFAULT_RAW_LOW_LEVEL;
static uint8_t raw_high_level = DEFAULT_RAW_HIGH_LEVEL;
static uint8_t raw_max_level = DEFAULT_RAW_MAX_LEVEL;

static T_PUMP pump;

uint8_t  dgn_pump_state = 0;
uint8_t dgn_pump_ctrl = 0;

void addNewDataToBuffer(uint8_t value);
uint8_t drySoil();
uint8_t wetSoil();
uint8_t soilExecuteCycle(void);

void soilInit()
{
    pump.status = PUMP_OFF;
    pump.cfg.nTon = DEFAULT_TON_CFG;
    pump.cfg.nToff = DEFAULT_TOFF_CFG;
    pump.cfg.numberOfCycles = DEFAULT_NCYCLES_CFG;
    pump.nTon = 0;
    pump.nToff = 0;
    pump.numberOfCycles = 0;

    TRIS_DO_SOIL = TRIS_DO_SOIL | ~( 1 << PIN_SOIL_N );
    // Configure timer source 
    PR1 = TMR1_PRESCALER;
    T1CONbits.TCKPS = TMR1_PR_64;
    T1CONbits.TCS = TMR1_TCY_SRC;
    T1CONbits.TON = 1;
    IFS0bits.T1IF = 0;
}


/**
 * @brief Task executed every 100 ms
 */
void soilTask()
{
    triggerScan ( AI_SOIL );
    while(!isConversionFinished()) { continue; }
    soil_wet = getADCMean();
    addNewDataToBuffer(soil_wet);
    if( drySoil() ){
        pump_sal = 1;
    }
    pump_hal = pump_sal || ( dgn_pump_state && dgn_pump_ctrl);
    if( (pump_hal == 0) && (pump.status != PUMP_ON) ){
        pump.status = PUMP_OFF;
        PUMP = 0x0;
        OC1RS = 0x0;
    } else if( (pump_hal == 1) && (pump.status == PUMP_OFF) ){
        pump.status = PUMP_ON;
        pump.cfg.nTon = DEFAULT_TON_CFG;
        pump.cfg.nToff = DEFAULT_TOFF_CFG;
        pump.cfg.numberOfCycles = DEFAULT_NCYCLES_CFG;
        pump.nTon = 0;
        pump.nToff = 0;
        pump.numberOfCycles = 0;
        pump.cycleOn = CYCLEON_ENABLE;
    } else {
        soilExecuteCycle(); 
    }
    if( pumpPeriodEnable == 0 && pump_hal == 1){
        pumpPeriodEnable = 1;
    }
    pump_sal = 0;
}

uint8_t soilExecuteCycle(void)
{
    if( pump.status == PUMP_END){
        PUMP = 0x00;
        OC1RS = 0x00;
    }else{
        PUMP = pump.cycleOn == CYCLEON_ENABLE ? DUTY_ON : 0x0;
        OC1RS = (pump.cycleOn == CYCLEON_ENABLE) ? 0x7FFF : 0x0;
        if( pump.cycleOn == CYCLEON_ENABLE) {
            if( pump.nTon > pump.cfg.nTon ){
                pump.nTon = 0;
                pump.cycleOn = CYCLEON_DISABLE;
            } else {
                pump.nTon++;
            }
        } else {

            if( pump.nToff > pump.cfg.nToff ){
                pump.nToff = 0;
                pump.cycleOn = CYCLEON_ENABLE;
                pump.numberOfCycles++;
            } else {
                pump.nToff++;
            }
        }
    }

    if( pump.numberOfCycles > pump.cfg.numberOfCycles ){
        pump.status = PUMP_END;
    }
}

uint8_t getSoilWet()
{
    return soil_wet;
}

uint8_t drySoil()
{
    uint8_t soil_is_dry = 1;
    for( int i = 0; i < N_SOIL_ACQ; i++){
        if( (soil_wet_buffer[i] < raw_high_level) ||
            (soil_wet_buffer[i] > raw_max_level)){
            soil_is_dry = 0;
        }
    }
    return soil_is_dry;
}

uint8_t wetSoil()
{
    uint8_t soil_is_wet = 1;
    for( int i = 0; i < N_SOIL_ACQ; i++){
        if( (soil_wet_buffer[i] > raw_low_level) &&
            ( soil_wet_buffer[i] < raw_max_level)){
            soil_is_wet = 0;
        }
    }
    return soil_is_wet;
}

void addNewDataToBuffer(uint8_t value)
{
    *ptr_write = value;
    ptr_write++;
    if(ptr_write-soil_wet_buffer >= N_SOIL_ACQ){
        ptr_write = soil_wet_buffer;
    }
}
