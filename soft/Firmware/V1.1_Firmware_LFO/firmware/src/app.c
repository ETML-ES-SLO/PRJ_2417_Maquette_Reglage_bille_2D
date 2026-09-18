// ============================================================================
//  En-tête MPLAB Harmony : informations générales sur le fichier            //
// ============================================================================

/*******************************************************************************
  MPLAB Harmony Application Source File
  ?
 *******************************************************************************/

// DOM-IGNORE-BEGIN
//  Bloc de licence Microchip : laissé inchangé.
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc. ?
 *******************************************************************************/
// DOM-IGNORE-END


// ============================================================================
//  Section : Inclusions                                                       //
// ============================================================================

//  Fichiers en-tête du projet et des pilotes utilisés.
#include "app.h"
#include "system_config.h"
#include "system_definitions.h"
#include "Mc32gestSpiTSC2046.h"
#include "peripheral/oc/plib_oc.h"
#include "Mc32DriverLcd.h"
#include "GestMenu.h" 
#include "driver/usart/drv_usart_static.h"
#include "Mc32Debounce.h"
#include "GesBtn.h"

// ============================================================================
//  Section : Données globales                                                 //
// ============================================================================

//  Structure principale des données de l?application.
APP_DATA appData;

//  Instances PID pour chaque axe du plateau.
PID_DATA pidX;      /* axe X (gauche-droite) */
PID_DATA pidY;      /* axe Y (haut-bas)      */
//static PID_RAW pidX;      /* exemple de buffer brut pour X  */
//static PID_RAW pidY;      /* exemple de buffer brut pour Y  */

//  Gains PID (variables partagées avec le menu).
extern volatile float KpXY; /* déclarations externes (menu.c) */
extern volatile float KiXY;
extern volatile float KdXY;

//  Tampons circulaires pour l?USART (TX et RX).
volatile uint8_t  txBuf[TX_SIZE];
volatile uint16_t txHead = 0;
volatile uint16_t txTail = 0;

//  RX_SIZE doit être une puissance de 2.
volatile uint8_t  rxBuf[RX_SIZE];
volatile uint16_t rxHead = 0;
volatile uint16_t rxTail = 0;

//  Indique qu?une trame de réglage PID complète est disponible.
static volatile bool  frameReady = false;
//  Stockage temporaire des nouveaux gains reçus.
static volatile float kpNew = 0.0f, kiNew = 0.0f, kdNew = 0.0f;


//LFO-01.04.26 structure réprésentant la PIN PENIRQ (détection de bille);
S_SwitchDescriptor DescrPenirq;

//LFO-01.01.26 Déplacement de cette constante de l'app task ici 
const float Ts = 0.002f;     // Période d?échantillonnage (2 ms

// ============================================================================
//  Section : Fonctions de rappel (callbacks)                                  //
// ============================================================================

/* TODO : ajouter ici les callbacks éventuelles. */

// ============================================================================
//  Section : Fonctions locales                                                //
// ============================================================================

/* TODO : ajouter ici les fonctions locales utiles. */

// ============================================================================
//  Section : Initialisation et machine d?états                                //
// ============================================================================

/*******************************************************************************
  Function:
    void APP_Initialize ( void )
  Description:
    Initialise la machine d?état de l?application.
 */
void APP_Initialize ( void )
{
    /* Place la machine d?état dans son état initial. */
    appData.state = APP_STATE_INIT;
    
    /* TODO : initialiser d?autres variables si besoin. */
}

/*******************************************************************************
  Function:
    void APP_Tasks ( void )
  Description:
    Boucle principale gérée par Harmony.
 Modificationss:
    18.03.26 : LFO : ajout de la détection de bille
 */
void APP_Tasks ( void )
{ 
    /* Sélectionne le comportement selon l?état courant. */
    switch ( appData.state )
    {
        // --------------------------------------------------------------------
        //  État d?initialisation                                              //
        // --------------------------------------------------------------------
        case APP_STATE_INIT:
        {
            // Initialisation du LCD et message de Bienvenu
            lcd_init();
            lcd_bl_on();                           // Rétro-éclairage ON
            printf_lcd("2417_B_MaquetteBille2D\n");
            printf_lcd("Created by : MBR");
            lcd_gotoxy(1,3);
            printf_lcd("last edit by : LFO");
            lcd_gotoxy(7,4);
            printf_lcd("Bienvenue");

            // Configuration de la ligne RTS
            RTS_INIT();                 
            
            // Activation de l'interuption RX avec PLIB
            PLIB_INT_SourceEnable(INT_ID_0, INT_SOURCE_USART_1_RECEIVE); 
            
            // Démarrage des périphériques : SPI, Timers, PWM
            SPI_InitTSC2046();  
            DRV_TMR0_Start();
            DRV_TMR1_Start();
            DRV_TMR2_Start();
            DRV_OC0_Start();
            DRV_OC1_Start();
            
            // Centre les servos (plateau à plat).
            PLIB_OC_PulseWidth16BitSet(OC_ID_1, MOT_CENTERED_PWM);
            PLIB_OC_PulseWidth16BitSet(OC_ID_2, MOT_CENTERED_PWM);
            
            // initialisation de l'anti-rebond pour la détection de bille
            DebounceInit(&DescrPenirq);
            BtnInit();
            
            // Passe en attente
            APP_UpdateState (APP_STATE_WAIT);
            break;
        }

        // --------------------------------------------------------------------
        //  État principal : tâches de service                                 //
        // --------------------------------------------------------------------
        case APP_STATE_SERVICE_TASKS:
        {
            //static uint16_t avgX, avgY; // Variables de moyenne glissante
            static uint16_t pwmX = 0;
            static uint16_t pwmY = 0;
            
            static float xfilt = 0.0f;   /* filtrage axe X  */
            static float yfilt = 0.0f;   /* filtrage axe Y  */
            
            static uint8_t menuWasActive = 0; // Souvenir de l?état menu
            
            // 1) Récupère les octets RX et alimente la FSM de réception.
            PumpRxFifo();
            
            // 2) Si une trame complète PID a été reçue, met à jour les gains.
            if (frameReady)          /* s?applique uniquement dans le menu */
            {
                KpXY = kpNew;
                KiXY = kiNew;
                KdXY = kdNew;
                
                menuForceRefresh = true; // Force refresh écran
                sendPIDXY();             // Accusé de réception
                frameReady = false;      // Prêt pour la suivante
                
                //affichage du mode remote Settings
                lcd_gotoxy(21,1);
                lcd_putc((uint8_t)'R');
            }
            
            // 3) Exécute la tâche menu (machine d?état du GUI).
            menuTache();
            if(menuActif())         // Le menu est-il affiché ?
            {  
                if(menuWasActive == 0)   // Tout juste entré dans le menu
                {
                    sendPIDXY();         // Envoie les gains courants
                    menuWasActive = 1;
                }
                
                // Centre les servos (plateau à plat) durant le réglage.
                PLIB_OC_PulseWidth16BitSet(OC_ID_1, MOT_CENTERED_PWM);
                PLIB_OC_PulseWidth16BitSet(OC_ID_2, MOT_CENTERED_PWM);
            }
            else  //Menu non actif : boucle de régulation               
            {            
                //dès que la bille quitte le plateau on arrête de régulé
                if (DebounceIsReleased(&DescrPenirq)){
                    
                    //clear des flags pour bille détectée et absente
                    DebounceClearPressed(&DescrPenirq);
                    DebounceClearReleased(&DescrPenirq);
                    
                    // Centre les servos (plateau à plat).
                    PLIB_OC_PulseWidth16BitSet(OC_ID_1, MOT_CENTERED_PWM);
                    PLIB_OC_PulseWidth16BitSet(OC_ID_2, MOT_CENTERED_PWM);
                    
                    //message pour indiquer que la bille est absente
                    lcd_gotoxy(1,1);
                    printf_lcd("Aucune bille presente");
                }
                
                //si 3 fois de suite bille détectée alors on régule
                if (DebounceIsPressed(&DescrPenirq)){
                    menuWasActive = 0;
                    // a) Lecture brute des positions tactiles X/Y.
                    appData.rawX = TSC2046_ReadRawX();
                    appData.rawY = TSC2046_ReadRawY();

                    // b) Conversion en pixels utilisables à l?écran.
                    appData.posPixelX = TSC2046_ReadPixel(appData.rawX, XMIN, XMAX);            
                    appData.posPixelY = 1599 - TSC2046_ReadPixel(appData.rawY, YMIN, YMAX);   

                    // c) Filtrage exponentiel (anti-bruit).
                    xfilt += IIR_BETA * ((float)appData.rawX - xfilt);
                    yfilt += IIR_BETA * ((float)appData.rawY - yfilt);

                    // d) Régulation PID sur chaque axe ? PWM
                    pwmX = PIDRegulation((uint16_t)xfilt, ZEROX, DIR_X,
                                         KpXY, KiXY, KdXY, Ts, 0); /* axe 0 */
                    pwmY = PIDRegulation((uint16_t)yfilt, ZEROY, DIR_Y,
                                         KpXY, KiXY, KdXY, Ts, 1); /* axe 1 */

                    // e) Application des rapports cycliques aux sorties PWM.
                    PLIB_OC_PulseWidth16BitSet(OC_ID_1, pwmY);
                    PLIB_OC_PulseWidth16BitSet(OC_ID_2, pwmX);

                    // f) Retour visuel + télémétrie.
                    lcdShowPos();   
                    sendPosition(); 
                }
            }
            APP_UpdateState (APP_STATE_WAIT); // Re-passe en attente
            break;
        }

        // --------------------------------------------------------------------
        //  État d?attente                                                     //
        // --------------------------------------------------------------------
        case APP_STATE_WAIT:
        {
            break;                  // Boucle vide (attente d?IT ou changement)
        }

        // --------------------------------------------------------------------
        //  État par défaut : erreur                                           //
        // --------------------------------------------------------------------
        default:
        {
            /* TODO : gérer l?erreur d?état inattendu. */
            break;
        }
    }
}

// -----------------------------------------------------------------------------
//  Met à jour l?état global (helper simple)                                    //
// -----------------------------------------------------------------------------
void APP_UpdateState ( APP_STATES NewState )
{
    appData.state = NewState;
}

// -----------------------------------------------------------------------------
//  Réception : décodage de la trame de gains PID envoyée par le PC             //
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//  PumpRxFifo : lit les octets de rxBuf et alimente ReceiveGainFSM            //
//  Exemple de trame : !kp=0080ki=0007kd=0030#
// -----------------------------------------------------------------------------
void PumpRxFifo(void)
{
    while (rxTail != rxHead)          /* tant qu?il reste des octets  */
    {
        char c = rxBuf[rxTail];
        rxTail = (rxTail + 1u) & (RX_SIZE-1);

        /* FSM existante pour la trame gains. */
        ReceiveGainFSM(c);
    }
}

// -----------------------------------------------------------------------------
//  FSM logicielle alternative pour les gains PID
// -----------------------------------------------------------------------------
void ReceiveGainFSM(char c)
{
    /* États de l'automate : attente du début '!' ou collecte jusqu'à '#' */
    typedef enum { WAIT_START, COLLECT } RSTATE;

    static RSTATE   st  = WAIT_START;   /* état courant */
    static char     buf[24];            /* tampon des caractères reçus   */
    static uint8_t  idx = 0;            /* index d'écriture dans le tampon */

    if (st == WAIT_START)
    {
        /* On surveille l'apparition du caractère de début '!' */
        if (c == '!')
        {
            idx = 0;        /* on réinitialise le tampon       */
            st  = COLLECT;  /* on passe à l'état de collecte   */
        }
    }
    else    /* st == COLLECT */
    {
        if (c == '#')   /* fin de trame : on traite le buffer */
        {
            buf[idx] = '\0';    /* termine la chaîne C */

            unsigned kp_i, ki_i, kd_i;
            if (sscanf(buf, "kp=%4uki=%4ukd=%4u", &kp_i, &ki_i, &kd_i) == 3)
            {
                kpNew = kp_i / 1000.f;   /* conversion milli ? unité */
                kiNew = ki_i / 1000.f;
                kdNew = kd_i / 1000.f;
                frameReady = true;       /* indique qu'une nouvelle trame est prête */
            }

            idx = 0;          /* prêt pour la trame suivante */
            st  = WAIT_START; /* retour en attente de '!' */
        }
        else if (idx < sizeof(buf) - 1)
        {
            buf[idx++] = c;   /* stocke le caractère et avance l?index */
        }
        else
        {
            /* Dépassement de tampon : on réinitialise l?automate      */
            st = WAIT_START;
        }
    }
}




// -----------------------------------------------------------------------------
//  TX_Enqueue : empile len octets dans le tampon TX (non bloquant)            //
//  - data : pointeur vers le tableau d?octets à transmettre                    //
//  - len  : nombre d?octets à empiler                                         //
//  Le buffer TX est circulaire : txHead pointe sur la prochaine case libre,   //
//  txTail sur la prochaine case à envoyer. Quand head rattrape tail,          //
//  le tampon est plein et la fonction sort sans bloquer.                      //
// -----------------------------------------------------------------------------
void TX_Enqueue(const char *data, uint8_t len)
{
    static uint8_t i = 0;                 /* Compteur de boucle (scope statique
                                             pour éviter une allocation pile
                                             récurrente, optimisation mineure) */

    for (i = 0; i < len; i++)             /* Parcourt tous les octets d?entrée */
    {
        /* Calcul de l?index suivant dans le buffer circulaire.
           L?opérateur & suppose que TX_SIZE est une puissance de 2 :
           (x & (TX_SIZE-1)) équivaut à (x % TX_SIZE) mais plus rapide. */
        uint16_t next = (uint16_t)((txHead + 1) & (TX_SIZE - 1));

        if (next == txTail)               /* Tampon plein : plus de place */
        {                                 /* On sort prématurément :       */
            break;                        /* les octets restants seront    */
        }                                 /* envoyés par un appel ultérieur */

        txBuf[txHead] = data[i];          /* Copie l?octet courant dans le buffer */
        txHead = next;                    /* Avance la tête vers la case libre
                                             suivante (écriture circulaire) */
    }

    /* Active l?interruption de transmission UART1.
       L?ISR (U1TX) se chargera d?envoyer les octets placés dans txBuf. */
    IEC1SET = _IEC1_U1TXIE_MASK;
}


// -----------------------------------------------------------------------------
//  Envoi de la position et des gains PID vers le PC                           //
// -----------------------------------------------------------------------------
void sendPosition(void)
{
    char f[24];
    uint8_t l = (uint8_t)sprintf(f, "!X=%04uY=%04u#\r\n", appData.posPixelX, appData.posPixelY);
    TX_Enqueue(f, l);  /* envoi non bloquant */
}

void sendPIDXY(void)
{
    char f[48];
    uint8_t l = (uint8_t)sprintf(f,
                 "!kp=%04uki=%04ukd=%04uX=%04uY=%04u#\r\n",
                 (uint16_t)(KpXY*1000+0.5f),
                 (uint16_t)(KiXY*1000+0.5f),
                 (uint16_t)(KdXY*1000+0.5f),
                 appData.posPixelX, appData.posPixelY);
    TX_Enqueue(f, l);
}

// -----------------------------------------------------------------------------
//  Callback
// -----------------------------------------------------------------------------
/*******************************************************************************
  Function:
    void IntCallBackDrvTmrInstance0(void)
  Description:
    callback de l'interuption du timer1 .
 Modificationss:
    01.04.26 : LFO : ajout de la détection de bille
 */
void IntCallBackDrvTmrInstance0(void){ 
    static uint32_t CntInit = 0;
    static uint8_t initMenu = 0;
    RC7Toggle();
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
        ScanBtn(BTN_UP_RAW(),BTN_DOWN_RAW(),BTN_LEFT_RAW(),BTN_RIGHT_RAW());
        DoDebounce(&DescrPenirq,PENIRQ_StateGet()); //détection de bille par anti-rebond
    }  
}

//// -----------------------------------------------------------------------------
////  Exemple de filtre moyenne glissante (commenté)                             //
//// -----------------------------------------------------------------------------
//void moyenneGlissante(uint16_t rawX, uint16_t rawY, uint16_t *avgX, uint16_t *avgY)
//{
//    static uint16_t bufX[NOMBRE_ECHANTILLONS] = {0};
//    static uint16_t bufY[NOMBRE_ECHANTILLONS] = {0};
//    static uint32_t sumX = 0, sumY = 0;
//    static uint16_t idxMA = 0; 
//    // Retire l?ancienne valeur
//    sumX -= bufX[idxMA];
//    sumY -= bufY[idxMA];
//
//    // Stocke la nouvelle
//    bufX[idxMA] = rawX;
//    bufY[idxMA] = rawY;
//
//    // Ajoute au total
//    sumX += rawX;
//    sumY += rawY;
//
//    // Calcule la moyenne
//    *avgX = (uint16_t)(sumX / NOMBRE_ECHANTILLONS);
//    *avgY = (uint16_t)(sumY / NOMBRE_ECHANTILLONS);
//
//    // Avance l?index circulaire
//    idxMA++;
//    if (idxMA >= NOMBRE_ECHANTILLONS)
//        idxMA = 0;
//}


// ============================================================================
//  Fin du fichier                                                             //
// ============================================================================
