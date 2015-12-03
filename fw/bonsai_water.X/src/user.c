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
/* Include functions needed to print out message through FTDI */
#include "i2c.h"
#include "adc.h"

#define _XTAL_FREQ  8000000     // oscillator frequency for _delay()

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/

/* <Initialize variables in user.h and insert code for user algorithms.> */

void InitApp(void)
{
	//configure I/O
	AD1PCFG = 0xFFFFFFFF;

	TRISG = TRISG & ~(1 << 6);
	TRISE = TRISE & ~(1 << 5);
	MUX_EN = 0;

	configureI2C();
	configureADC();
}

