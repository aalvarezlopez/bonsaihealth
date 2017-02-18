/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

/* TODO Application specific user parameters used in user.c may go here */

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/
#define PUMP OC1R
#define MUX_EN PORTEbits.RE5

void RTCCTriggerAcq();

void InitApp(void);         /* I/O and Peripheral Initialization */
