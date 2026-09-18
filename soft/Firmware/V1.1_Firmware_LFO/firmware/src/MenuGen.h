#ifndef MenuGen_h
#define MenuGen_h

// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  03.02.2016
// Fichier MenuGen.h
// Gestion du menu  du générateur
// Traitement cyclique à 1 ms du Pec12


#include <stdbool.h>
#include <stdint.h>
#include "DefMenuGen.h"

//typedef struct {
//    uint16_t LineAddr : 2;
//    uint16_t Ptrstate : 1;      //0-> select mod , 1-> edit mod
//    uint16_t MenuScreen : 2;    /*0-> settings menu , 1-> save menu(appui long),
//                                 *2-> save ANNULEE! , 3-> Save OK  */ 
//    uint16_t MenuPreviousScreen : 2;
//    uint16_t Previousactivity : 1 ;  //pour savoir si on était inactif avant
//    uint16_t UpdatePtrFlag : 1;  //0-> pas d'update , 1-> update l'affichage Ptr
//    uint16_t UpdateScreenWithBackup : 1; //afficher valeur old quand ESC
//    
//} S_MenuState;
enum MenuWindows{Main, PidSettings, DisplayParam, DrawShape};

typedef struct {
    uint16_t LineAddr : 2;
    uint16_t Ptrstate : 1;      //0-> select mod , 1-> edit mod
    uint16_t CurrentWindow : 2;    /*0-> Menu de bienvenu , 1-> Menu réglage param K, Ki, Kd
                                 *2-> menu affichage Param , 3-> Menu séquence  */ 
    uint16_t PreviousWindow : 2;
    uint16_t Previousactivity : 1 ;  //pour savoir si on était inactif avant
    
} S_MenuState;

void MENU_Initialize(S_ParamGen *pParam);


void MENU_Execute(S_ParamGen *pParam);

#endif




  
   







