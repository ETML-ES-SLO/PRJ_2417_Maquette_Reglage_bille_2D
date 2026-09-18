#ifndef _GESBTN_H    
#define _GESBTN_H

//---------------------------------------------------------------------------
// Fichier      : GesBtn.h
// Description  : Pilote (driver) pour la gestion et l'anti-rebond de boutons
//                poussoirs sur PIC32MX. Gère les appuis simples (SEL), longs (HOLD)
//                ainsi que l'inactivité.
// Principe     : Appeler cycliquement la fonction ScanBtn (ex: toutes les 1 ms).
//---------------------------------------------------------------------------

#include <stdbool.h>
#include <stdint.h>
#include "Mc32Debounce.h"

/***************** DEFINITIONS ******************/
// Durée pour considérer un appui comme "long" (HOLD) - Unité dépendante du cycle d'appel
#define PRESS_DURATION 500

// Délai avant de lever le flag d'inactivité (NoActivity)
#define MAX_INACTIVITY_DELAY 5000

/******************** MACROS ********************/
#define BTN_UP_RAW()     (!SW0StateGet())  
#define BTN_DOWN_RAW()   (!SW2StateGet())  
#define BTN_LEFT_RAW()   (!SW1StateGet()) 
#define BTN_RIGHT_RAW()  (!SW3StateGet())  

#define BTN_UP     (&SW0)  
#define BTN_DOWN   (&SW1)  
#define BTN_LEFT   (&SW2) 
#define BTN_RIGHT  (&SW3) 

/******************* TYPEDEF *******************/
// Structure du descripteur d'un Bouton
typedef struct {
            uint8_t SEL  : 1;              // Événement action Select (appui simple)
            uint8_t HOLD : 1;              // Événement action Hold (appui long)
            uint8_t NoActivity : 1;        // Indication de non-activité
            uint16_t PressDuration;        // Compteur pour la durée de pression du bouton
            uint16_t InactivityDuration;   // Compteur pour la durée d'inactivité
} S_Btn_Descriptor;

/******************* VARIABLES ******************/
// Déclaration externe (promesse au compilateur) des structures allouées dans le .c
extern S_Btn_Descriptor SW0;   // Btn_UP
extern S_Btn_Descriptor SW1;   // Btn_DOWN
extern S_Btn_Descriptor SW2;   // Btn_LEFT
extern S_Btn_Descriptor SW3;   // Btn_RIGHT

/************* PROTOTYPES DE FONCTIONS **********/
void ScanBtn (bool ValSW0, bool ValSW1, bool ValSW2, bool ValSW3);
void BtnInit (void);
void GestBtn (S_Btn_Descriptor *ptrSW , S_SwitchDescriptor *ptrDescrSW);

bool BtnIsSEL (S_Btn_Descriptor *Btn);
bool BtnIsHOLD (S_Btn_Descriptor *Btn);
bool BtnNoActivity (S_Btn_Descriptor *Btn);

void BtnClearSEL (S_Btn_Descriptor *Btn);
void BtnClearHOLD (S_Btn_Descriptor *Btn);
void BtnClearInactivity (S_Btn_Descriptor *Btn);

#endif /* _GESBTN_H */

/* *****************************************************************************
 End of File
 */