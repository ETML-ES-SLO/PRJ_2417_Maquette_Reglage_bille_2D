// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms



/***ATTENTION***/
/*
 * en cours de dev
 * ce n'est pas le menu finale
 * c'est normale qu'il y'ait des erreurs
 * le vrais menu fonctionnel est l'ancienne version soit le GestMenu.c
 * développée le nouveau menu (et en plus propre) ici
 * je laisse volontairement ce fichier ainsi que les erreurs.
 * c'est pour que le prochain ait une bonne base 
 * ( ou du moins de quoi s'inspirer si besoin)
 * ne pas hésiter à tout mettre en commentaire pour build sans erreur
 * et tester d'autres codes ou autre
 */











#include <stdint.h>                   
#include <stdbool.h>
#include "MenuGen.h"
#include "Mc32DriverLcd.h"
#include "Mc32Debounce.h"
#include "GesBtn.h"
#include "DefMenuGen.h"

const char MenuFormes[4][21] = { "Sinus     ", "Triangle  ", "DentDeScie", "Carre     " };
const char MenuPtr[2] = { '*', '?'};

S_MenuState MenuState = {0};
uint8_t saveScreenDisplayDelay;

extern S_SwitchDescriptor DescrPenirq;

// Initialisation du menu et des paramètres
void MENU_Initialize(S_ParamGen *pParam)
{
    
}

// Execution du menu, appel cyclique depuis l'application
void MENU_Execute(S_ParamGen *pParam)
{
    
    /*******GESTION INACTIVITé AVEC BACKLIGHT********/
    if((BtnNoActivity(BTN_UP) &&  BtnNoActivity(BTN_DOWN) &&  BtnNoActivity(BTN_LEFT) &&  BtnNoActivity(BTN_RIGHT) && DebounceIsPressed(&DescrPenirq)) > MenuState.Previousactivity){   //passer à l'état inactif
        lcd_bl_off();
        MenuState.Previousactivity = 1;
    } else if ((BtnNoActivity(BTN_UP) &&  BtnNoActivity(BTN_DOWN) &&  BtnNoActivity(BTN_LEFT) &&  BtnNoActivity(BTN_RIGHT) && DebounceIsPressed(&DescrPenirq)) < MenuState.Previousactivity){ //passer à l'état actif
        lcd_bl_on();
        MenuState.Previousactivity = 0;
    }
//    
//    /*******QUAND ON PASSE DU MENU DE BASE AU MENU DE SAUVEGARDE********/
//    if(MenuState.MenuScreen != MenuState.MenuPreviousScreen){
//        if(saveScreenDisplayDelay <= 200)
//            saveScreenDisplayDelay ++;
//                
//        switch(MenuState.MenuScreen){
//            case 0 :    //settings menu
//                if(saveScreenDisplayDelay > 200){
//                    lcd_gotoxy(1,1);
//                    printf_lcd (" Forme = %s",MenuFormes[pParam->Forme]);
//                    lcd_gotoxy(1,2);
//                    printf_lcd (" Freq [Hz] =  %-6d ",pParam->Frequence);
//                    lcd_gotoxy(1,3);
//                    printf_lcd (" Ampl [mV] = %-5d ",pParam->Amplitude);
//                    lcd_gotoxy(1,4);
//                    printf_lcd (" Offset [mV] = %-5d ",pParam->Offset);   
//                    MenuState.MenuPreviousScreen = MenuState.MenuScreen;
//                    MenuState.UpdatePtrFlag = 1;
////                    pParamNew = *pParam;
//                }
//                break;
//                
//            case 1 :    //save menu(appui long)
//                    lcd_ClearLine(1);
//                    lcd_gotoxy(1,2);
//                    printf_lcd ("   Sauvegarde ?     ");
//                    lcd_gotoxy(1,3);
//                    printf_lcd ("   (appui long)     ");
//                    lcd_ClearLine(4);
//                    MenuState.MenuPreviousScreen = MenuState.MenuScreen;
//                break;
//                
//            case 2 :    //save ANNULEE!
//                    lcd_gotoxy(1,2);
//                    printf_lcd (" Sauvegarde ANNULEE!" );
//                    lcd_ClearLine(3);
//                    saveScreenDisplayDelay = 0;
//                    MenuState.MenuPreviousScreen = MenuState.MenuScreen;
//                    MenuState.MenuScreen = 0;
//                break;
//                
//            case 3 :    //Save OK
//                    lcd_gotoxy(1,2);
//                    printf_lcd ("  Sauvegarde OK !   ");
//                    lcd_ClearLine(3);
//                    saveScreenDisplayDelay = 0;
//                    MenuState.MenuPreviousScreen = MenuState.MenuScreen;
//                    MenuState.MenuScreen = 0;
//                break;
//                
//            default :
//                
//                break;
//        }
//    }
//    
//    /********GESTION DE L APPUIE SIMPLE SUR LE PEC12*********/
////    if(Pec12IsOK()){
////        Pec12ClearOK();
////        if(MenuState.MenuScreen == 1){
////            MenuState.MenuScreen ++;
////        } else if (MenuState.MenuPreviousScreen == 0) {
////            MenuState.Ptrstate ++ ;
////            MenuState.UpdatePtrFlag = 1;
////            
////            if(pParamNew.Frequence != pParam->Frequence){
////                GENSIG_UpdatePeriode(&pParamNew);
////            } else {
////                GENSIG_UpdateSignal(&pParamNew);
////            }
////            
////            *pParam = pParamNew;
////        }
////    }
//    
//    /*********GESTION DU ESCAPE AVEC LE PEC12*********/
////    if(Pec12IsESC()){
////        Pec12ClearESC();
////        if(MenuState.MenuScreen == 1){
////            MenuState.MenuScreen ++;
////        } else if (MenuState.MenuPreviousScreen == 0) {
////            pParamNew = *pParam;
////            if(MenuState.Ptrstate){
////                MenuState.UpdatePtrFlag = 1;
////                MenuState.UpdateScreenWithBackup = 1;
////            }
////            Pec12ClearPlus();
////            Pec12ClearMinus();
////        }
////    }
//    
//    /********GESTION DE L'AFFICHAGE DES VALEURS ********/
//    if(MenuState.Ptrstate){ //edit mod
//        switch (MenuState.LineAddr){
//            case 0:                                 //ligne forme
////                if(Pec12IsPlus()){
////                    Pec12ClearPlus();
////                    if(pParamNew.Forme < SignalCarre)
////                      pParamNew.Forme ++;  
////                } else if (Pec12IsMinus()){
////                    Pec12ClearMinus();
////                    if(pParamNew.Forme > SignalSinus)
////                        pParamNew.Forme--;
////                } 
//                lcd_gotoxy(10,(MenuState.LineAddr + 1));
////                printf_lcd ("%s",MenuFormes[pParamNew.Forme]);
//                break;
//            
//            case 1:                                 //ligne Frequence
////                if(Pec12IsPlus()){
////                    Pec12ClearPlus();
////                    pParamNew.Frequence += 20;
////                    if(pParamNew.Frequence > 2000)
////                        pParamNew.Frequence = 20;
////                } else if (Pec12IsMinus()){
////                    Pec12ClearMinus();
////                    pParamNew.Frequence -= 20;
////                    if(pParamNew.Frequence < 20)
////                        pParamNew.Frequence = 2000;   
////                } 
//                lcd_gotoxy(15,(MenuState.LineAddr + 1));
////                printf_lcd ("%-4d",pParamNew.Frequence);
//                break;
//
//            case 2:                                 //ligne Amplitude
////                if(Pec12IsPlus()){
////                    Pec12ClearPlus();
////                    pParamNew.Amplitude += 100;
////                    if(pParamNew.Amplitude > 10000)
////                        pParamNew.Amplitude = 0;
////                } else if (Pec12IsMinus()){
////                    Pec12ClearMinus();
////                    pParamNew.Amplitude -= 100;
////                    if(pParamNew.Amplitude < 0)
////                        pParamNew.Amplitude = 10000; 
////                } 
//                lcd_gotoxy(14,(MenuState.LineAddr + 1));
////                printf_lcd ("%-5d",pParamNew.Amplitude);
//                break;
//                
//            case 3:                                 //ligne Offset
////                if(Pec12IsPlus()){
////                    Pec12ClearPlus();
////                    if(pParamNew.Offset < 5000)
////                        pParamNew.Offset += 100;
////                } else if (Pec12IsMinus()){
////                    Pec12ClearMinus();
////                    if(pParamNew.Offset > -5000)
////                        pParamNew.Offset -= 100;
////                } 
//                lcd_gotoxy(16,(MenuState.LineAddr + 1));
////                printf_lcd ("%-5d",pParamNew.Offset);
//                break;
//                
//            default:
//                
//                break;
//        }
//        
//        if(MenuState.UpdateScreenWithBackup){
//            MenuState.Ptrstate = 0;
//            MenuState.UpdateScreenWithBackup = 0;
//        }
//        
//    } else {     
//        /*********LECTURE DU BTN S9 POUR GERER LA SAUVEGARDE********/
////        if(BtnIsOK()){
////            BtnClearOK();
////            if(MenuState.MenuScreen < 2)
////                MenuState.MenuScreen ++;
////        } else if(BtnIsESC()){
////            BtnClearESC();
////            if(MenuState.MenuScreen == 1){
//////                NVM_WriteBlock((uint32_t*)pParam,sizeof(*pParam)); //on sauvegarde
////                MenuState.MenuScreen = 3;
////            } 
////        }
//        
//        /******GESTION DU POINTEUR POUR NAVIGUER DANS LE MENU***********/
////        if(Pec12IsPlus()){
////            Pec12ClearPlus();
////            if(MenuState.MenuScreen == 1){
////                MenuState.MenuScreen ++;
////            } else if (MenuState.MenuPreviousScreen == 0) {
////                lcd_gotoxy(1,(MenuState.LineAddr + 1));
////                printf_lcd (" ");
////                MenuState.LineAddr ++;
////                MenuState.UpdatePtrFlag = 1;
////            }
////        } else if (Pec12IsMinus()){
////            Pec12ClearMinus();
////            if(MenuState.MenuScreen == 1){
////                MenuState.MenuScreen ++;
////            } else if (MenuState.MenuPreviousScreen == 0) {
////                lcd_gotoxy(1,(MenuState.LineAddr + 1));
////                printf_lcd (" ");
////                MenuState.LineAddr --;
////                MenuState.UpdatePtrFlag = 1;
////            }
////        }
//    }
//    
//    /*****MISE à JOUR DE L'APPARENCE DU POINTEUR ***********/
//    if(MenuState.UpdatePtrFlag){
//        lcd_gotoxy(1,(MenuState.LineAddr + 1));
//        lcd_putc((uint8_t)MenuPtr[MenuState.Ptrstate]); 
//        MenuState.UpdatePtrFlag = 0;
//    }
}