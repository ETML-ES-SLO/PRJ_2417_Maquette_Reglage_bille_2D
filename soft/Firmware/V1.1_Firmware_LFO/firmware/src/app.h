/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_Initialize" and "APP_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_STATES" definition).  Both
    are defined here for convenience.
 
  Modificationss:
    18.03.26 : LFO : ajout du define MOT_CENTERED_PWM
*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
//DOM-IGNORE-END

#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "system_config.h"
#include "system_definitions.h"
#include "GestMenu.h"
#include "driver/usart/drv_usart_static.h" 


// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END 

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************
#define XMIN 121
#define XMAX 3920
#define YMIN 197
#define YMAX 3997 
#define ZEROX 2000
#define ZEROY 2000

#define SERVO_PWM_MAX_US   2400   /* +90°  */
#define SERVO_PWM_MIN_US    600   /* -90°  */
#define SERVO_PWM_MILLEU_US ((SERVO_PWM_MAX_US + SERVO_PWM_MIN_US) / 2) //1500 us 0°
#define SERVO_PWM_RANGE_US (SERVO_PWM_MAX_US - SERVO_PWM_MIN_US)
    
#define SERVO_PWM_MIN_TICK      450     /* -90°  (0,6 ms) */
#define SERVO_PWM_MAX_TICK     1800     /* +90°  (2,4 ms) */
#define SERVO_PWM_NEUTRAL_TICK ((SERVO_PWM_MIN_TICK + SERVO_PWM_MAX_TICK) / 2)   /* 1125 = 0° */    
      
    
#define DIR_X            (-1.0f)    /* -1 ? inverse la réaction sur X            */
#define DIR_Y            (-1.0f)    /* -1 ? inverse la réaction sur Y            */

#define NOMBRE_ECHANTILLONS   10
    
#define IIR_BETA   0.15f
    
#define CTS_IS_READY()   (PORTCbits.RC8 == 0)
#define CTS_TIMEOUT  500  
    
#define TX_SIZE 128 
#define RX_SIZE   128   
    
/* RTS# (PIC ? FT230X) sur RC9 : 
   - 0 = go (FIFO RX prêt) 
   - 1 = stop (FIFO RX plein) */
#define RTS_INIT()    do { TRISCbits.TRISC9 = 0; LATCbits.LATC9 = 0; } while(0)
#define RTS_ASSERT()  (LATCbits.LATC9 = 1)
#define RTS_DEASSERT() (LATCbits.LATC9 = 0)
    
/************************************************************************************************************************************/
#define MOT_CENTERED_PWM 1040 //valeur trouvée après plusieurs tests à faire tenir la bille au centre sans PID
//#define MOT_CENTERED_PWM 1125

/************************************************************************************************************************************/
    
// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
	/* Application's state machine's initial state. */
	APP_STATE_INIT=0,
	APP_STATE_SERVICE_TASKS,
    APP_STATE_WAIT,
	/* TODO: Define states used by the application state machine. */

} APP_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_STATES state;
    uint16_t rawX;
    uint16_t rawY;
    uint16_t posPixelX;
    uint16_t posPixelY;
    /* TODO: Define any additional data used by the application. */
} APP_DATA;

extern APP_DATA appData;

typedef struct
{
    //facteur de réglage
    float Kp;
    float Ki;
    float Kd;
    // Constante de temps du systeme
    float Ts;
    // Type de bloc
    float proportionnel;
    float integrateur;
    float derivateur;
    // Erreur
    float erreurPrecedente;
    // valeur maximum et minimum de tick
    float outMax;
    float outMin;
} PID_DATA;

//typedef struct {
//    union{
//        struct {
//          uint8_t PENIRQ_State_Old:1;
//          uint8_t PENIRQ_State_Previous_Old:1;
//          uint8_t PENIRQ_State_Now:1;
//        };
//        uint8_t PENIRQ_State;
//    };
////  struct {
////    uint32_t w:32;
////  };
//    uint8_t tempo;
//} test;

//typedef struct {
//    float Kp, Ki, Kd;
//    float Ts;                 /* période en seconde */
//    float integ;              /* somme intégrale    */
//    float errPrev;            /* erreur précédente  */
//    float outMin, outMax;     /* limites ?raw       */
//} PID_RAW;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/
	
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the 
    application in its initial state and prepares it to run so that its 
    APP_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_Tasks( void );
void APP_UpdateState(APP_STATES newState);
void PIDInit(PID_DATA *pid, float Kp, float Ki, float Kd, float Ts, int16_t outMin, int16_t outMax);
float PIDProportionnel(PID_DATA *pid, float erreur);
float PIDIntegrateur (PID_DATA *pid, float erreur);
float PIDDerivateur(PID_DATA *pid, float erreur);
void SendAsciiFrame(void);
void ReceiveGainFrame(void);
void PollRequestSend(void);
void moyenneGlissante(uint16_t rawX, uint16_t rawY, uint16_t *outX, uint16_t *outY);

void sendPosition(void);
void sendPIDXY(void);
void PumpUSART_RX(void);
void TX_Enqueue(const char *data, uint8_t len);
void ReceiveGainFSM(char c); 
void PumpRxFifo(void);

void IntCallBackDrvTmrInstance0(void);


//void  PIDraw_Init  (PID_RAW *pid, float Kp, float Ki, float Kd, float Ts, float outMin, float outMax);
//float PIDraw_Update(PID_RAW *pid, float setPoint, float measure);
#endif /* _APP_H */

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

/*******************************************************************************
 End of File
 */

