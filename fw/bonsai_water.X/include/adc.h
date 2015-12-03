/**
 * @file adc.h
 * @author Adrian Alvarez
 * @version 1.0
 * @date 2015-09-28
 */

#ifndef ADC_H
#define ADC_H

#define AI_SOIL 6
#define AI_LDR 2

void configureADC();
void triggerScan( uint8_t channel);
uint8_t isConversionFinished();
uint8_t getADCValues(uint8_t n_channels, uint8_t *buffer);
uint8_t getADCMean();

#endif
