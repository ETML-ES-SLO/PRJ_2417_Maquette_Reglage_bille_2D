//---------------------------------------------------------------------------
// Fichier      : GesBtn.c
// Description  : Implémentation du pilote de gestion des boutons.
//---------------------------------------------------------------------------

#include "GesBtn.h"
#include "Mc32Debounce.h"

// Descripteurs des signaux pour le traitement anti-rebond
S_SwitchDescriptor DescrSW0;
S_SwitchDescriptor DescrSW1;
S_SwitchDescriptor DescrSW2;
S_SwitchDescriptor DescrSW3;

// Structures pour le traitement des états des boutons (mémoire allouée)
S_Btn_Descriptor SW0 = {0};   // Btn_UP
S_Btn_Descriptor SW1 = {0};   // Btn_DOWN
S_Btn_Descriptor SW2 = {0};   // Btn_LEFT
S_Btn_Descriptor SW3 = {0};   // Btn_RIGHT

//----------------------------------------------------------------------------------//
//-- nom fct : ScanBtn  
//-- paramètre entrée : bool - ValSW0, bool - ValSW1, bool - ValSW2, bool - ValSW3 
//-- paramètre sortie : void - aucun 
//-- paramètre référence (IN-OUT) :   aucun 
//-- description : Fonction principale à appeler cycliquement. Elle gère l'anti-rebond 
//                 des 4 boutons puis met à jour leurs états internes via GestBtn. 
//-- démonstration : N/A 
//-- aide - référence - lien : Nécessite l'appel de DoDebounce (Mc32Debounce.h) 
//----------------------------------------------------------------------------------//
void ScanBtn (bool ValSW0, bool ValSW1, bool ValSW2, bool ValSW3) {
   // Traitement antirebond sur SW0, SW1, SW2, SW3
   DoDebounce (&DescrSW0, ValSW0);
   DoDebounce (&DescrSW1, ValSW1);
   DoDebounce (&DescrSW2, ValSW2);
   DoDebounce (&DescrSW3, ValSW3);
   
   // Gestion de l'appui sur bouton SW0, SW1, SW2, SW3
   GestBtn(&SW0, &DescrSW0);
   GestBtn(&SW1, &DescrSW1);
   GestBtn(&SW2, &DescrSW2);
   GestBtn(&SW3, &DescrSW3);
}

//----------------------------------------------------------------------------------//
//-- nom fct : GestBtn  
//-- paramètre entrée : aucun 
//-- paramètre sortie : void - aucun 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - ptrSW, S_SwitchDescriptor* - ptrDescrSW 
//-- description : Évalue l'état d'un bouton spécifique. Incrémente les compteurs de 
//                 pression, détermine si l'appui est simple (SEL) ou long (HOLD) au 
//                 relâchement, et gère le délai de déclenchement de l'inactivité.
//-- démonstration : N/A
//-- aide - référence - lien : Utilise les macros PRESS_DURATION et MAX_INACTIVITY_DELAY 
//----------------------------------------------------------------------------------//
void GestBtn (S_Btn_Descriptor *ptrSW, S_SwitchDescriptor *ptrDescrSW) {        
    // --- Phase 1 : Le bouton est actuellement maintenu pressé ---
    if(DebounceIsPressed(ptrDescrSW)) {
        ptrSW->PressDuration += 1;
        
        // Réinitialisation de l'inactivité dès qu'une pression est détectée
        if(BtnNoActivity(ptrSW)) {
            BtnClearInactivity(ptrSW);
        } else {
            ptrSW->InactivityDuration = 0;
        }
    } 
    
    // --- Phase 2 : Le bouton vient d'être relâché ---
    if(DebounceIsReleased(ptrDescrSW)) {
        DebounceClearPressed(ptrDescrSW);
        DebounceClearReleased(ptrDescrSW);
        
        // Détermination du type d'appui en fonction de la durée
        if(ptrSW->PressDuration < PRESS_DURATION) {
            ptrSW->SEL = 1;                              // Appui simple
        } else {
            ptrSW->HOLD = 1;                             // Appui prolongé
        }
        
        ptrSW->PressDuration = 0; // Remise à zéro pour le prochain cycle
        
        // Réinitialisation de l'inactivité au relâchement
        if(BtnNoActivity(ptrSW)) {
            BtnClearInactivity(ptrSW);
        } else {
            ptrSW->InactivityDuration = 0;
        }
    }  
    
    // --- Phase 3 : Gestion de l'inactivité globale du bouton ---
    if(ptrSW->InactivityDuration < MAX_INACTIVITY_DELAY) {
        ptrSW->InactivityDuration += 1;
    } else {
        ptrSW->NoActivity = 1; // Le délai maximum est atteint, flag levé
    } 
}

//----------------------------------------------------------------------------------//
//-- nom fct : BtnInit  
//-- paramètre entrée : aucun 
//-- paramètre sortie : void - aucun 
//-- paramètre référence (IN-OUT) :   aucun 
//-- description : Initialise à zéro les structures matérielles (anti-rebond) et 
//                 logicielles (SEL, HOLD, compteurs) des 4 boutons.
//-- démonstration : N/A 
//-- aide - référence - lien : Doit être appelée à l'initialisation du système. 
//----------------------------------------------------------------------------------//
void BtnInit (void) {
   // Initialisation des descripteurs de touches matériels
    DebounceInit(&DescrSW0);
    DebounceInit(&DescrSW1);
    DebounceInit(&DescrSW2);
    DebounceInit(&DescrSW3);
   
   // Init de la structure SW0 (Bouton UP)
    SW0.SEL = 0;              // événement action SEL
    SW0.HOLD = 0;             // événement action HOLD
    SW0.NoActivity = 0;       // Indication d'activité
    SW0.PressDuration = 0;    // Pour durée pression du P.B.
    SW0.InactivityDuration = 0; // Durée inactivité
    

   // Init de la structure SW1 (Bouton DOWN)
    SW1.SEL = 0;              
    SW1.HOLD = 0;             
    SW1.NoActivity = 0;      
    SW1.PressDuration = 0;   
    SW1.InactivityDuration = 0; 
    
   // Init de la structure SW2 (Bouton LEFT)
    SW2.SEL = 0;              
    SW2.HOLD = 0;             
    SW2.NoActivity = 0;      
    SW2.PressDuration = 0;   
    SW2.InactivityDuration = 0; 
    
   // Init de la structure SW3 (Bouton RIGHT)
    SW3.SEL = 0;              
    SW3.HOLD = 0;             
    SW3.NoActivity = 0;      
    SW3.PressDuration = 0;   
    SW3.InactivityDuration = 0; 
 }

//----------------------------------------------------------------------------------//
//-- nom fct : BtnIsSEL  
//-- paramètre entrée : aucun 
//-- paramètre sortie : bool - État du flag SEL 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - pBtn 
//-- description : Retourne true si une action d'appui simple (SEL) a été détectée 
//                 sur le bouton passé en paramètre.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
bool BtnIsSEL (S_Btn_Descriptor *pBtn) {
   return (pBtn->SEL);
}

//----------------------------------------------------------------------------------//
//-- nom fct : BtnIsHOLD  
//-- paramètre entrée : aucun 
//-- paramètre sortie : bool - État du flag HOLD 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - pBtn 
//-- description : Retourne true si une action d'appui long (HOLD) a été détectée 
//                 sur le bouton passé en paramètre.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
bool BtnIsHOLD (S_Btn_Descriptor *pBtn) {
   return (pBtn->HOLD);
}

//----------------------------------------------------------------------------------//
//-- nom fct : BtnNoActivity  
//-- paramètre entrée : aucun 
//-- paramètre sortie : bool - État du flag NoActivity 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - pBtn 
//-- description : Retourne true si aucune activité n'a été détectée sur le bouton 
//                 depuis MAX_INACTIVITY_DELAY.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
bool BtnNoActivity (S_Btn_Descriptor *pBtn) {
   return (pBtn->NoActivity);
}

//----------------------------------------------------------------------------------//
//-- nom fct : BtnClearSEL  
//-- paramètre entrée : aucun 
//-- paramètre sortie : void - aucun 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - pBtn 
//-- description : Acquitte (remet à 0) le flag indiquant un appui simple (SEL) une 
//                 fois que l'application a traité l'événement.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
void BtnClearSEL (S_Btn_Descriptor *pBtn) {
   pBtn->SEL = 0;
}

//----------------------------------------------------------------------------------//
//-- nom fct : BtnClearHOLD  
//-- paramètre entrée : aucun 
//-- paramètre sortie : void - aucun 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - pBtn 
//-- description : Acquitte (remet à 0) le flag indiquant un appui long (HOLD) une 
//                 fois que l'application a traité l'événement.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
void BtnClearHOLD (S_Btn_Descriptor *pBtn) {
   pBtn->HOLD = 0;
}

//----------------------------------------------------------------------------------//
//-- nom fct : BtnClearInactivity  
//-- paramètre entrée : aucun 
//-- paramètre sortie : void - aucun 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* - pBtn 
//-- description : Acquitte le flag d'inactivité et réinitialise le compteur associé 
//                 pour relancer le chronométrage.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
void BtnClearInactivity (S_Btn_Descriptor *pBtn) {
  pBtn->NoActivity = 0;
  pBtn->InactivityDuration = 0;
}

/* *****************************************************************************
 End of File
 */