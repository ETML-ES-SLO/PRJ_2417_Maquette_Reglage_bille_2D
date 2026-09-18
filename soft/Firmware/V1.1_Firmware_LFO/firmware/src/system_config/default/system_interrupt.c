/*******************************************************************************
 System Interrupts File

  File Name:
    system_interrupt.c

  Summary:
    Raw ISR definitions.

  Description:
    This file contains a definitions of the raw ISRs required to support the
    interrupt sub-system.

  Summary:
    This file contains source code for the interrupt vector functions in the
    system.

  Description:
    This file contains source code for the interrupt vector functions in the
    system.  It implements the system and part specific vector "stub" functions
    from which the individual "Tasks" functions are called for any modules
    executing interrupt-driven in the MPLAB Harmony system.

  Remarks:
    This file requires access to the systemObjects global data structure that
    contains the object handles to all MPLAB Harmony module objects executing
    interrupt-driven in the system.  These handles are passed into the individual
    module "Tasks" functions to identify the instance of the module to maintain.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2011-2014 released Microchip Technology Inc.  All rights reserved.

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
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "system/common/sys_common.h"
#include "app.h"
#include "system_definitions.h"
#include "Mc32Debounce.h"

extern volatile uint8_t  txBuf[];
extern volatile uint16_t txHead;
extern volatile uint16_t txTail;

extern volatile uint8_t  rxBuf[];
extern volatile uint16_t rxHead;
extern volatile uint16_t rxTail;

extern S_SwitchDescriptor DescrPenirq;

// *****************************************************************************
// *****************************************************************************
// Section: System Interrupt Vector Functions
// *****************************************************************************
// *****************************************************************************
//void __ISR(_UART_1_VECTOR, ipl1AUTO) _IntHandlerDrvUsartInstance0(void)
//{
//    DRV_USART_TasksTransmit(sysObj.drvUsart0);
//    DRV_USART_TasksError(sysObj.drvUsart0);
//    DRV_USART_TasksReceive(sysObj.drvUsart0);
//}

void __ISR(_TIMER_1_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance0(void)
{
    //interruption à 100[kHz] soit toutes les 10[us]
    
    /****************LFO 11.03.26**********************/
    //Mis en commentaire car pas propre de faire comme ceci
    //donc je vais reprendre ce fonctionnement et 
    //juste le mettre dns un callback
    /**************************************************/
    /*
    RC7Toggle();
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_1);
    static uint32_t CntInit = 0;
    static uint8_t initMenu = 0;
    // compteur qui gere le temps d'init
    CntInit++;
    if(CntInit == 300000 && initMenu == 0)
    {
        lcd_clear();
        menuInit();
        initMenu = 1;
        
    }
    if(CntInit >= 300000)
    {
        CntInit = 300000;
        APP_UpdateState (APP_STATE_SERVICE_TASKS); 
    }   
    */
    /**************************************************/
    IntCallBackDrvTmrInstance0();
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_1);
}
void __ISR(_TIMER_2_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance1(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_2);
}
void __ISR(_TIMER_3_VECTOR, ipl1AUTO) IntHandlerDrvTmrInstance2(void)
{
    PLIB_INT_SourceFlagClear(INT_ID_0,INT_SOURCE_TIMER_3);
}

void __ISR(_UART_1_VECTOR, ipl1AUTO) _IntHandlerDrvUsartInstance0(void)
{
    /* ---------- ERROR : clear OERR, FERR, etc. ---------------------- */
    if (U1STAbits.OERR) U1STAbits.OERR = 0;    /* débordement FIFO hard */

    /* ---------- RX  : copie FIFO hard ? buffer logiciel ------------- */
    while (U1STAbits.URXDA)                    /* tant qu'un octet dispo */
    {
        uint16_t next = (rxHead + 1u) & (RX_SIZE - 1);
        char c = U1RXREG;                      /* lit l'octet            */

        if (next != rxTail)                    /* buffer non plein ?     */
        {                                      /* (sinon on jette)       */
            rxBuf[rxHead] = c;
            rxHead = next;
        }
    }
    IFS1CLR = _IFS1_U1RXIF_MASK | _IFS1_U1EIF_MASK;  /* clear flags RX+ERR */

    /* ---------- TX  : pompe la file logicielle ---------------------- */
    while (!U1STAbits.UTXBF && (txTail != txHead))   /* << garde txHead */
    {
        U1TXREG = txBuf[txTail];
        txTail  = (txTail + 1u) & (TX_SIZE - 1);
    }
    if (txTail == txHead)                    /* plus rien à envoyer    */
        IEC1CLR = _IEC1_U1TXIE_MASK;           /* coupe **seulement** TXIE */

    IFS1CLR = _IFS1_U1TXIF_MASK;               /* clear flag TX          */
}
 
/*******************************************************************************
 End of File
*/
