/*--------------------------------------------------------*/
/*  Mc32gestSpiTSC2046.c                                  */
/*--------------------------------------------------------*/
/*  Gestion par SPI du contrôleur TSC2046E                */
/*  Version entièrement commentée pour une compréhension  */
/*  optimale. Les lignes de code d'origine ne sont pas    */
/*  modifiées ; seuls des commentaires explicatifs ont    */
/*  été ajoutés.                                          */
/*--------------------------------------------------------*/

#include "app.h"                      /* Définitions globales de l'application */
#include "Mc32gestSpiTSC2046.h"       /* Fichier d'en-tête du module courant  */
#include "Mc32SpiUtil.h"              /* Fonctions utilitaires pour le bus SPI*/
#include "peripheral/SPI/plib_spi.h" /* Bibliothèque PLIB : accès registres SPI */

/* --- Le TSC est câblé sur SPI1 du kit ------------------ */
#define TSC_SPI1   (SPI_ID_1)          /* Identifiant matériel du contrôleur SPI employé */

/* Registres de contrôle (debug) ------------------------- */
//uint32_t  TSC_ConfigReg ;           /* Gardé en commentaire : sert au debug */
//uint32_t  TSC_BaudReg   ;
extern const float Ts;                 /* Période d'échantillonnage issue d'une horloge externe */

/*--------------------------------------------------------*/
/*  Configuration du bus SPI pour le TSC2046E             */
/*  Appelée une seule fois à l'initialisation.            */
/*--------------------------------------------------------*/
static void SPI_ConfigureTSC(void)
{
    /* Arrêt du module SPI avant reconfiguration */
    PLIB_SPI_Disable(TSC_SPI1);

    /* Remise à zéro du buffer et configuration */
    PLIB_SPI_BufferClear(TSC_SPI1);
    PLIB_SPI_StopInIdleDisable(TSC_SPI1);            /* Le module reste actif en Idle */
    PLIB_SPI_PinEnable(TSC_SPI1, SPI_PIN_DATA_OUT);  /* Activation de la broche SDO  */
    PLIB_SPI_CommunicationWidthSelect(TSC_SPI1, SPI_COMMUNICATION_WIDTH_8BITS); /* Trames de 8 bits */

    /* ----------------------------------------------------
       Vitesse du bus : 100 kHz (pour debug).               
       On peut monter à 2 MHz (valeur en commentaire).      
       La formule interne utilise la fréquence du bus
       périphérique 1 fournie par SYS_CLK_PeripheralFrequencyGet.
       ---------------------------------------------------- */
    PLIB_SPI_BaudRateSet(
        TSC_SPI1,
        SYS_CLK_PeripheralFrequencyGet(CLK_BUS_PERIPHERAL_1),
        100000UL); /* 100 kHz */

    /* Mode 0,0 : CPOL = 0, CPHA = 0.                       
       Le TSC décale les données sur le flanc descendant    */
    PLIB_SPI_InputSamplePhaseSelect (TSC_SPI1, SPI_INPUT_SAMPLING_PHASE_AT_END);
    PLIB_SPI_ClockPolaritySelect    (TSC_SPI1, SPI_CLOCK_POLARITY_IDLE_LOW);
    PLIB_SPI_OutputDataPhaseSelect  (TSC_SPI1, SPI_OUTPUT_DATA_PHASE_ON_ACTIVE_TO_IDLE_CLOCK);

    /* Passage en mode maître + FIFO activée */
    PLIB_SPI_MasterEnable(TSC_SPI1);
    PLIB_SPI_FramedCommunicationDisable(TSC_SPI1);  /* Pas de trame encadrée */
    PLIB_SPI_FIFOEnable(TSC_SPI1);

    /* Réactivation du module SPI avec la nouvelle config */
    PLIB_SPI_Enable(TSC_SPI1);

//    /* Pour contrôle/debug -------------------------------------- */
//    TSC_ConfigReg = SPI1CON;        /* Sauvegarde des registres de conf. */
//    TSC_BaudReg   = SPI1BRG;
}

/*--------------------------------------------------------*/
/*  Initialisation publique :                             */
/*  À appeler une fois au démarrage de l'application.     */
/*--------------------------------------------------------*/
void SPI_InitTSC2046(void)
{
    SPI_ConfigureTSC();  /* Configure le bus SPI selon la fonction ci-dessus */

    /* Aucun registre interne à écrire sur le TSC :        */
    /* on se contente de libérer la ligne CS (active bas). */
    SPI1_CSOn();
}

/*--------------------------------------------------------*/
/*  Lecture brute :                                       */
/*  - Envoie une commande (1 octet)                       */
/*  - Récupère 12 bits de résultat alignés sur le MSB     */
/*--------------------------------------------------------*/
uint16_t TSC2046_ReadRaw(uint8_t cmd)
{
    uint8_t msb;   /* Poids fort retourné par le TSC */
    uint8_t lsb;   /* Poids faible */
    uint16_t raw;  /* Valeur brute 12 bits reconstituée */

    SPI1_CSOff();  /* Abaisse CS pour débuter la communication */

    /* Envoi du byte de commande ; le retour immédiat est ignoré */
    spi_read1(cmd);

    /* Deux lectures "dummy" :                                   
       1) la première décale les 8 premiers bits utiles           
       2) la seconde complète pour obtenir 16 bits alignés        */
    msb = spi_read1(0x00); /* Lecture du MSB */
    lsb = spi_read1(0x00); /* Lecture du LSB */

    SPI1_CSOn();   /* Relâche CS : fin de transaction */

    /* Reconstruction sur 16 bits puis décalage à droite de 3     
       pour repasser sur 12 bits utiles                           */
    raw = ( ((uint16_t)msb << 8) | lsb );
    return (raw >> 3);
}

/*--------------------------------------------------------*/
/*  Conversion brute -> pixel (0..1599)                   */
/*  - raw     : valeur lue (12 bits)                      */
/*  - rawMin  : valeur correspondant au bord « 0 »        */
/*  - rawMax  : valeur correspondant au bord « 1599 »     */
/*--------------------------------------------------------*/
uint16_t TSC2046_ReadPixel(uint16_t raw, uint16_t rawMin, uint16_t rawMax)
{
    if(raw <= rawMin)                  /* Saturation basse */
    {
        return 0;
    }
    else if(raw >= rawMax)             /* Saturation haute */
    {
        return 1599;
    }
    else
    {
        /* Échelle linéaire sur la plage utile */
        return (uint32_t)(raw - rawMin) * 1599u / (rawMax - rawMin);
    }

}

/*--------------------------------------------------------*/
/*  rawToPulse : convertit la valeur capteur en largeur   */
/*               d'impulsion servo (en ticks µs)          */
/*--------------------------------------------------------*/
uint16_t rawToPulse(uint16_t raw, uint16_t rawMin, uint16_t rawMax)
{
    if (raw <= rawMin)          /* Extrême gauche : -90°  */
    {
        return 1800;             /* 1800 µs */
    }

    if (raw >= rawMax)          /* Extrême droite : +90° */
    {
        return 450;             /*  450 µs */
    }
    
    /* --------------------------------------------------------------
       Calcul linéaire :                                             
           pulse = 1800 µs - ((raw - rawMin) / (rawMax - rawMin)) × 1350 µs
       - 1350 = 1800 - 450 (plage totale)                           
       - Le signe négatif inverse la direction.                      
       -------------------------------------------------------------- */
    float pulse_f = 1800.0f - ((float)(raw - rawMin) / (float)(rawMax - rawMin)) * 1350.0f;
    
    return (uint16_t)pulse_f;    /* Conversion float -> uint16 */
}


/*--------------------------------------------------------*/
/*  PRegulation : Correcteur proportionnel pur            */
/*  - raw       : valeur capteur courante                 */
/*  - zero      : consigne (position de repos)            */
/*  - direction : +1 ou -1 suivant l'inversion souhaitée  */
/*  - kp        : gain proportionnel                      */
/*--------------------------------------------------------*/
uint16_t PRegulation(uint16_t raw, uint16_t zero, float direction, float kp)
{
    int16_t erreur;        /* Erreur instantanée (signée) */
    int16_t pulse;         /* Largeur d'impulsion finale */
    
    erreur = raw - zero;   /* écart entre mesure et zéro */
    pulse  = SERVO_PWM_NEUTRAL_TICK + direction * kp * erreur; /* Action P */
    
    /* Saturation pour rester dans la plage autorisée du servo */
    if(pulse > SERVO_PWM_MAX_TICK)
    {
        pulse = SERVO_PWM_MAX_TICK;
    }
    
    if(pulse < SERVO_PWM_MIN_TICK)
    {
        pulse = SERVO_PWM_MIN_TICK;
    }

    return (uint16_t)pulse;
}

/*--------------------------------------------------------*/
/*  IRegulation : Correcteur intégral                     */
/*  - axe 0 = X, 1 = Y pour disposer de deux accumulateurs*/
/*--------------------------------------------------------*/
uint16_t IRegulation(uint16_t raw,  uint16_t zero, float direction, float ki, float Ts, int axe)              /* 0 = X, 1 = Y */
{
    static float integ[2] = {0.0f, 0.0f}; /* Accumulateurs par axe */
    float err;        /* Erreur instantanée */
    float capPos;     /* Limite supérieure */
    float capNeg;     /* Limite inférieure */

    err = direction * ((float)raw - (float)zero);      /* Erreur signée */
    integ[axe] += ki * Ts * err;                       /* Intégration discrète */

    /* Saturation de l'intégrale pour éviter le wind-up */
    capPos =  SERVO_PWM_MAX_TICK - SERVO_PWM_NEUTRAL_TICK;
    capNeg = -(SERVO_PWM_NEUTRAL_TICK - SERVO_PWM_MIN_TICK);
    
    if (integ[axe] > capPos)
    {
        integ[axe] = capPos;
    }
    else if (integ[axe] < capNeg)
    {
        integ[axe] = capNeg;
    }

    return (int16_t)integ[axe]; /* *** Retourne une correction signée *** */
}

/*--------------------------------------------------------*/
/*  PIRegulation : Correcteur proportionnel + intégral    */
/*--------------------------------------------------------*/
uint16_t PIRegulation(uint16_t raw,  uint16_t zero, float direction, float kp, float ki, float Ts, int axe)           /* 0 = X, 1 = Y */
{
    int32_t pulseP  = PRegulation(raw, zero, direction, kp);     /* Terme P complet */
    int32_t correctionI   = IRegulation(raw, zero, direction, ki, Ts, axe);

    int32_t pulse   = pulseP + correctionI;                      /* Somme des actions */

    /* Saturation globale */
    if (pulse > SERVO_PWM_MAX_TICK)
    {
        pulse = SERVO_PWM_MAX_TICK;
    }
    else if (pulse < SERVO_PWM_MIN_TICK)
    {
        pulse = SERVO_PWM_MIN_TICK;
    }

    return (uint16_t)pulse;
}

/*--------------------------------------------------------*/
/*  DRegulation : Correcteur dérivatif                    */
/*--------------------------------------------------------*/
int16_t DRegulation(uint16_t raw, uint16_t zero, float direction, float kd, float Ts, int axe)
{
    static float erreurPrecedente[2] = {0.0f, 0.0f}; /* Mémoire par axe */
    float erreur;      /* Erreur instantanée */
    float derive;      /* Terme dérivé */
    float deriveMax;   /* Saturation + */
    float deriveMin;   /* Saturation - */
    
    
    erreur = direction * ((float)raw - (float)zero);
    derive = (kd * (erreur - erreurPrecedente[axe]) )/ Ts; /* (?e)/Ts */
    
    deriveMax =  SERVO_PWM_MAX_TICK - SERVO_PWM_NEUTRAL_TICK;
    deriveMin = -(SERVO_PWM_NEUTRAL_TICK - SERVO_PWM_MIN_TICK);
    

    erreurPrecedente[axe] = erreur; /* Mise à jour pour l'appel suivant */

    /* Saturation du terme D pour ne pas dépasser la mécanique */
    if (derive > deriveMax)
    {
        derive = deriveMax;
    }
    else if (derive < deriveMin)
    {
        derive = deriveMin;                                /* signe conservé */
    }

    return (int16_t)derive;
    
}

/*--------------------------------------------------------*/
/*  PDRegulation : Proportionnel + dérivé                 */
/*--------------------------------------------------------*/
uint16_t PDRegulation(uint16_t raw, uint16_t zero, float direction,float kp, float kd, float Ts, int axe)           /* 0 = X, 1 = Y */
{
    int32_t pulseP;          /* Terme P */
    int32_t correctionD;     /* Terme D */
    int32_t pulseRegule;     /* Somme régulée */

    pulseP       = PRegulation(raw, zero, direction, kp);
    correctionD  = DRegulation(raw, zero, direction, kd, Ts, axe);

    pulseRegule  = pulseP + correctionD ;

    if (pulseRegule > SERVO_PWM_MAX_TICK)
    {
        pulseRegule = SERVO_PWM_MAX_TICK;
    }
    else if (pulseRegule < SERVO_PWM_MIN_TICK)
    {
        pulseRegule = SERVO_PWM_MIN_TICK;
    }

    return (uint16_t)pulseRegule;
}

/*--------------------------------------------------------*/
/*  PIDRegulation : Proportionnel + Intégral + Dérivé      */
/*--------------------------------------------------------*/
uint16_t PIDRegulation(uint16_t raw, uint16_t zero, float direction,float kp, float ki,float kd, float Ts, int axe)
{
    int32_t pulseP;        /* Terme P */
    int32_t correctionI;   /* Terme I */
    int32_t correctionD;   /* Terme D */
    uint16_t pulsePIDReg;  /* Résultat combiné saturé */
    
    
    pulseP  = PRegulation(raw, zero, direction, kp);     /* Terme P complet */
    correctionI = IRegulation(raw, zero, direction, ki, Ts, axe);
    correctionD = DRegulation(raw, zero, direction, kd, Ts, axe);
    
    pulsePIDReg = pulseP + correctionI + correctionD;
    if (pulsePIDReg > SERVO_PWM_MAX_TICK)
    {
        pulsePIDReg = SERVO_PWM_MAX_TICK;
    }
    else if (pulsePIDReg < SERVO_PWM_MIN_TICK)
    {
        pulsePIDReg = SERVO_PWM_MIN_TICK;
    }       
    return (uint16_t)pulsePIDReg;
}



/*--------------------------------------------------------*/
/*  Wrappers simplifiés pour la lecture des axes X et Y   */
/*--------------------------------------------------------*/
uint16_t TSC2046_ReadRawX(void)
{
    return TSC2046_ReadRaw(TSC_CMD_X_POS); /* Commande pour l'axe X */
}
uint16_t TSC2046_ReadRawY(void)
{
    return TSC2046_ReadRaw(TSC_CMD_Y_POS); /* Commande pour l'axe Y */
}
