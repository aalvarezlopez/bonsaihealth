/*
 * =====================================================================================
 *
 *       Filename:  log.c
 *
 *    Description:  Library for logging through serial port. In this project UART is
 *    connected to  FTDI (uart to usb converter)
 *
 *        Version:  1.0
 *        Created:  29/08/15 10:47:20
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Adrian Alvarez (), alvarez.lopez.adri@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */

#include <xc.h>            /* HiTech General Includes */
#include <stdint.h>         /* For uint8_t definition */
#include <stdbool.h>        /* For true/false definition */
#include "log.h"

void print(char *str);
void printHex(unsigned int value);
void printDec(int value);

#define MAX_MSG_LEN 30
/**
 * \brief Configure UART transmision
 */
void configureLog()
{
	//configure serial port
	//pins TX 13 RX 12
	RPINR18 = RX_PIN;
	RPOR_TX =  PIN_AS_TX_1 << RPOR_PADDING;
	//8 bit data and no parity
	U1MODE = 0;
	//baudrate 1200 => =FCY/( 4 * BDRT ) -1 = 32000000/(4 * 1200) - 1
	U1BRG = 415;
	U1MODEbits.BRGH = 1;

	U1MODEbits.RTSMD = 1;
	U1MODEbits.UARTEN = 1;
	U1STAbits.UTXEN = 1;
	/* Enable interrupts*/
	IFS0bits.U1RXIF = 0;
	IEC0bits.U1RXIE = 1;
}

/**
 * \brief Send usefull init information.
 */
void initLog()
{
	print("\r\n******\r\nInit Application\r\n");
}


/**
 * \brief This function should be call from U1TX interruption. Gets characters
 * from FTDI device.
 */
void logGetCh(char recv)
{
	static char msg[MAX_MSG_LEN];
	static unsigned int currentMsgLen = 0;
	if( recv == ';' ){
		msg[currentMsgLen] = 0;
		LOG_ECHO(msg);
		currentMsgLen = 0;
	} else {
		msg[currentMsgLen] = recv;
		currentMsgLen++;
		if( currentMsgLen == MAX_MSG_LEN ){
			LOG_WARN("Max message length has been reached. This message"\
				" will be rejected");
			currentMsgLen = 0;
		}
	}
}

/**
 * \brief Put char through UART1
 */
void putch(char c)
{
    while(U1STAbits.UTXBF) continue;
    U1TXREG = c;
}


/**
 * \brief Print message using putch function
 */
void print(char *str)
{
    char *pchar;
    pchar = str;
    while(*pchar != 0){
        putch(*pchar);
        pchar++;
    }
}


/**
 * \brief Convert unsigned int value in his hexadecimal string and print it
 * \param value Value to be convert to string
 */
void printHex(unsigned int value)
{
    char str[10],str_cpy[10];
    int digit, count;

    print("0x");
    if(value == 0){
        print("00\r\n");
        return;
    }
    count = 0;
    if(value<0){
    	LOG_WARN("Conversion could not be done");
    }
    while(value>0){
        digit = value % 16;
        if(digit>9){
            str[count] = 'a'+ digit - 10;
        } else {
            str[count] = '0' + digit;
        }
        value /= 16;
        count++;
    }
    for(int i = 0; i < count; i++){
        str_cpy[i] = str[count - i -1];
    }
    str_cpy[count] = 0;
    print(str_cpy);
    print("\r\n");
}

/**
 * \brief Convert int value in his decimal string and print it
 * \param value Value to be convert to string
 */
void printDec(int value)
{
    char str[10],str_cpy[10];
    int digit, count;

    if(value == 0){
        print("0\r\n");
        return;
    }
    count = 0;
    while(value>0){
        digit = value % 10;
        str[count] = '0' + digit;
        value /= 10;
        count++;
    }
    for(int i = 0; i < count; i++){
        str_cpy[i] = str[count - i -1];
    }
    str_cpy[count] = 0;
    print(str_cpy);
}
