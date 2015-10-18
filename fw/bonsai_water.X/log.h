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


/* Define log functions*/
#define print(x) do{}while(0)
#define LOG_DBG(x) do{ print("[DEBUG]:");print(x);print("\r\n"); }while(0)
#define LOG_WARN(x) do{ print("[WARNING]:");print(x);print("\r\n"); }while(0)
#define LOG_INFO(x) do{ print("[INFO]:");print(x);print("\r\n"); }while(0)
/* Log error function. Print message and halt */
#define LOG_ERR(x) do{ print("[ERROR]:");print(x);while(1);}while(0)

#endif
