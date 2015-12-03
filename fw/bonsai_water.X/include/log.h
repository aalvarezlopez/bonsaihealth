/*
 * =====================================================================================
 *
 *       Filename:  log.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  29/08/15 10:49:06
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Adrian Alvarez (), alvarez.lopez.adri@gmail.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef LOG_FILE_H
#define LOG_FILE_H
/* Hardware definition */
#define RPOR_TX RPOR7 /* RPOR register that will change correct RPXX to TX1 */
#define RPOR_PADDING 0 /*  Could be HB o LB of the register */
#define PIN_AS_TX_1 3 /*  This value is always the same, configure current pin
							as TX1 peripheral pin*/
#define RX_PIN 29 /*  Fill this value with the RPXX use for reception (RX1) */

#define LOG_DBG(x) do{ print("[DEBUG]:");print(x);print("\r\n"); }while(0)
#define LOG_WARN(x) do{ print("[WARNING]:");print(x);print("\r\n"); }while(0)
#define LOG_INFO(x) do{ print("[INFO]:");print(x);print("\r\n"); }while(0)
#define LOG_RF(x) do{ print("[RADIO]:");print(x);print("\r\n"); }while(0)
#define LOG_ECHO(x) do{ print("[MSG RCV]:");print(x);print("\r\n"); }while(0)
/* Log error function. Print message and halt */
#define LOG_ERR(x) do{ print("[ERROR]:");print(x);while(1);}while(0)
void configureLog();
void initLog();
void logGetCh(char recv);
#endif
