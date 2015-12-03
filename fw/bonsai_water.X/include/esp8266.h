/* 
 * File:   i2c.h
 * Author: adrian
 *
 */
#ifndef ESP8266_H
#define ESP8266_H

/*  Hardware definition */
#define RPOR_TX RPOR9 /* RPOR register that will change correct RPXX to TX1 */
#define RPOR_PADDING 8 /*  Could be HB o LB of the register */
#define PIN_AS_TX 5 /*  This value is always the same, configure current pin
							as TX1 peripheral pin*/
#define RX_PIN 26 /*  Fill this value with the RPXX use for reception (RX1) */


void ESP8266Init();
#endif
