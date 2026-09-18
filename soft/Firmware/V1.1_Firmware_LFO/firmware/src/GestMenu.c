/* ************************************************************************** */
/** Descriptive File Name

  @Company
    Company Name

  @File Name
    filename.c

  @Summary
    Brief description of the file.

  @Description
    Describe the purpose of this file.
 */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */

/* This section lists the other files that are included in this file.
 */

/* TODO:  Include other files here if needed. */

#include <stdio.h>
#include <stdlib.h>
#include <math.h> 
#include "GestMenu.h"
#include "app.h"
#include "Mc32DriverLcd.h" 

#define BTN_UP_RAW()     (!SW0StateGet())   /* RB2  ? SW0  (?) */
#define BTN_DOWN_RAW()   (!SW2StateGet())   /* RB7  ? SW2  (?) */
#define BTN_LEFT_RAW()   (!SW1StateGet())   /* RB3  ? SW1  (?) */
#define BTN_RIGHT_RAW()  (!SW3StateGet())   /* RB9  ? SW3  (?) */


extern APP_DATA appData;       /* ? ajoute CETTE ligne */

volatile float KpXY = 0.075f;
volatile float KiXY = 0.006f;
volatile float KdXY = 0.020f;

volatile bool menuForceRefresh = false;

/*--------------------------------------------------------------
  Lecture instantanée des quatre boutons (haut-bas-gauche-droite)
  ? Chaque bit du résultat correspond à un bouton maintenu appuyé :
        bit 0 ? haut
        bit 1 ? bas
        bit 2 ? gauche
        bit 3 ? droite
--------------------------------------------------------------*/
uint8_t lectureBouton(void)
{
    /* Variable de retour : chaque bit est mis à 1
       si le bouton correspondant est appuyé                   */
    uint8_t etatBoutons  = 0;

    /* Bouton HAUT -------------------------------------------------------- */
    if (BTN_UP_RAW() != 0)          /* niveau logique actif ?              */
    {
        etatBoutons  |= 0x01;             /* bit 0 ? 1                           */
    }

    /* Bouton BAS --------------------------------------------------------- */
    if (BTN_DOWN_RAW() != 0)
    {
        etatBoutons  |= 0x02;             /* bit 1 ? 1                           */
    }

    /* Bouton GAUCHE ------------------------------------------------------ */
    if (BTN_LEFT_RAW() != 0)
    {
        etatBoutons  |= 0x04;             /* bit 2 ? 1                           */
    }

    /* Bouton DROITE ------------------------------------------------------ */
    if (BTN_RIGHT_RAW() != 0)
    {
        etatBoutons  |= 0x08;             /* bit 3 ? 1                           */
    }

    return etatBoutons ;
}


/*------------------------------------------------------------*/
/*  Détection de fronts descendants sur les boutons           */
/*      - etatActuel : états logiques actuels (1 = appuyé)    */
/*      - renvoie    : bits à 1 pour chaque bouton venant     */
/*                     juste de passer de 1 ? 0               */
/*------------------------------------------------------------*/
uint8_t boutonsDetecterFront(uint8_t etatActuel)
{
    static uint8_t etatPrecedent = 0;
    /* frontDesc : bit à 1 si le bouton vient de se relâcher   */
    uint8_t frontDescendant = etatActuel & (etatActuel ^ etatPrecedent);

    /* mémoriser l?état pour la prochaine comparaison          */
    etatPrecedent = etatActuel;

    return frontDescendant;
}

typedef enum
{
    MENU_OFF,       // 0 : le menu est complètement inactif
    MENU_BROWSE,    // 1 : on se déplace dans la liste des paramètres
    MENU_EDIT       // 2 : on édite la valeur du paramètre sélectionné
} MENU_STATE;

typedef enum 
{ 
    SEL_KP,     // 0 : le facteur proportionnel est sélectionné
    SEL_KI,     // 1 : l?intégrateur est sélectionné               
    SEL_KD      // 2 : le dérivateur est sélectionné  
} PID_PARAM;

typedef struct
{
    MENU_STATE state;   /* OFF / BROWSE / EDIT      */
    PID_PARAM  select;     /* ligne sélectionnée       */
    uint8_t    digit;   /* digit actif (0..4)       */
} MENU_DATA;


static MENU_DATA menu = { MENU_OFF, SEL_KP, 0 };


/******************************************************************************
 *  Affiche le menu « PID » sur l?écran LCD 20×4
 *
 *  ? Ligne 1 : position actuelle X / Y (4 chiffres chacun)
 *  ? Lignes 2-4 : gains Kp, Ki, Kd
 *        ? Un astérisque « * » est affiché devant la ligne actuellement
 *          sélectionnée (menu.select)
 *  ? Lorsque l?on édite un chiffre (MENU_EDIT) :
 *        ? Le curseur clignote sur le chiffre concerné.
 ******************************************************************************/
void menuAffichage(void)
{
    
    /* Buffer temporaire pour construire chaque ligne avant envoi */
    char buf[22];          /* 20 caractères + «\r\n» + ?\0?          */

    /* ---------------------------------------------------------------------
     * 1) Affichage de la position X / Y (ligne 1)
     * ------------------------------------------------------------------ */
    lcd_gotoxy(1, 1);      /* Colonne 1, ligne 1                     */
    sprintf(buf,
            "Pos : X=%04uY=%04u", 
                appData.posPixelX,   /* valeur X sur 4 chiffres    */
                appData.posPixelY);  /* valeur Y sur 4 chiffres    */
    printf_lcd(buf);       /* Envoi au LCD                           */

    /* ---------------------------------------------------------------------
     * 2) Sélection du marqueur ?*? pour chaque ligne PID
     *    ? Si la ligne est sélectionnée      ? marque = ?*?
     *    ? Sinon                            ? marque = ? ? (espace)
     * ------------------------------------------------------------------ */
    char marqueKP = ' ';
    char marqueKI = ' ';
    char marqueKD = ' ';

    if (menu.select == SEL_KP)
    {
        marqueKP = '*';
    }

    if (menu.select == SEL_KI)
    {
        marqueKI = '*';
    }

    if (menu.select == SEL_KD)
    {
        marqueKD = '*';
    }

    /* ---------------------------------------------------------------------
     * 3) Affichage des gains PID (lignes 2, 3, 4)
     * ------------------------------------------------------------------ */
    lcd_gotoxy(1, 2);      /* Ligne 2 : Kp                           */
    sprintf(buf, "%cKP : %1.3f", marqueKP, KpXY);
    printf_lcd(buf);

    lcd_gotoxy(1, 3);      /* Ligne 3 : Ki                           */
    sprintf(buf, "%cKI : %1.3f", marqueKI, KiXY);
    printf_lcd(buf);

    lcd_gotoxy(1, 4);      /* Ligne 4 : Kd                           */
    sprintf(buf, "%cKD : %1.3f", marqueKD, KdXY);
    printf_lcd(buf);

    /* ---------------------------------------------------------------------
     * 4) Curseur clignotant lorsqu'on édite un chiffre
     *    ? menu.state == MENU_EDIT : on active le clignotement
     *    ? Sinon : on le désactive
     * ------------------------------------------------------------------ */
    if (menu.state == MENU_EDIT)
    {
        /* Colonne du premier chiffre (« 0 » dans 0.000)                    */
        uint8_t colonneBase = 7;

        /* Colonne finale = base + indice du chiffre en cours                */
        uint8_t colonne = colonneBase + menu.digit;

        /* Si l?on édite un chiffre APRES le point décimal,
         * on saute la position du point (colonne +1).                       */
        if (menu.digit == 1)
        {
            colonne = colonne + 1;
        }

        /* Placement du curseur :
         *   ? Ligne = 2 + select (donc 2, 3 ou 4)
         *   ? Colonne = calculée ci-dessus                                 */
        lcd_gotoxy(colonne, 2 + menu.select);
        lcd_ClignotementOn();    /* Active le clignotement du LCD            */
    }
    else
    {
        lcd_ClignotementOff();   /* Aucun chiffre en édition ? pas de curseur */
    }
}

/* -------------------------------------------------------------------------- */
/*  Gère les quatre boutons (haut-bas-gauche-droite) et met à jour l?état du   */
/*  menu ainsi que l?affichage LCD.                                           */
/*                                                                            */
/*  Rappel des états :                                                        */
/*      MENU_OFF    : le menu n?est pas affiché                               */
/*      MENU_BROWSE : on se déplace ligne par ligne dans la liste des gains   */
/*      MENU_EDIT   : on modifie un chiffre du gain sélectionné               */
/*                                                                            */
/*  Chaque appel est non bloquant : on ne lit que ce qui se trouve déjà       */
/*  dans la FIFO RX des boutons.                                              */
/* -------------------------------------------------------------------------- */
void menuGestionBouton(void)
{
    /* 1) Lecture brute des boutons (instantané + fronts) ------------------ */
    uint8_t etatBoutons  = lectureBouton();              /* appuyé = 1        */
    uint8_t frontBoutons = boutonsDetecterFront(etatBoutons); /* front desc. */

    /* 2) Drapeaux ?front détecté? pour chaque touche ---------------------- */
    uint8_t frontHaut    = (frontBoutons & 0x01);   /* 1 si front sur HAUT    */
    uint8_t frontBas     = (frontBoutons & 0x02);   /* 1 si front sur BAS     */
    uint8_t frontGauche  = (frontBoutons & 0x04);   /* 1 si front sur GAUCHE  */
    uint8_t frontDroite  = (frontBoutons & 0x08);   /* 1 si front sur DROITE  */

    /* 3) Machine d?état du menu ------------------------------------------ */
    switch (menu.state)
    {
        /* ---------- 3.1  Menu éteint ------------------------------------ */
        case MENU_OFF:
        {
            if (frontDroite)                          /* bouton ?            */
            {
                menu.state = MENU_BROWSE;
                lcd_clear();
                menuAffichage();
            }
            break;
        }

        /* ---------- 3.2  Parcours des lignes ---------------------------- */
        case MENU_BROWSE:
        {
            /* --- HAUT --------------------------------------------------- */
            if (frontHaut)
            {
                if (menu.select == SEL_KP)
                {
                    menu.select = SEL_KD;             /* boucle vers le bas  */
                }
                else
                {
                    menu.select = (PID_PARAM)(menu.select - 1);
                }
                menuAffichage();
            }
            /* --- BAS ---------------------------------------------------- */
            else if (frontBas)
            {
                if (menu.select == SEL_KD)
                {
                    menu.select = SEL_KP;             /* boucle vers le haut */
                }
                else
                {
                    menu.select = (PID_PARAM)(menu.select + 1);
                }
                menuAffichage();
            }
            /* --- DROITE : passer en édition ----------------------------- */
            else if (frontDroite)
            {
                menu.state = MENU_EDIT;
                menu.digit = 0;
                menuAffichage();
            }
            /* --- GAUCHE : quitter le menu ------------------------------- */
            else if (frontGauche)
            {
                menu.state = MENU_OFF;
                lcd_clear();
            }
            break;
        }

        /* ---------- 3.3  Édition d?un gain ------------------------------ */
        case MENU_EDIT:
        {
            /* sélection du gain à ajuster -------------------------------- */
            volatile float *gain = &KpXY;             /* défaut */

            if (menu.select == SEL_KI)
            {
                gain = &KiXY;
            }
            else if (menu.select == SEL_KD)
            {
                gain = &KdXY;
            }

            /* représentation ASCII du gain courant ---------------------- */
            char txtGain[8];
            sprintf(txtGain, "%1.3f", *gain);        /* ex. ?0.075? */

            /* ----- HAUT / BAS : changer la valeur du chiffre ------------ */
            if (frontHaut || frontBas)
            {
                char car = txtGain[menu.digit];

                if (car != '.')
                {
                    int pas = 0;
                    if (frontHaut)  { pas =  1; }
                    if (frontBas)   { pas = -1; }

                    int nv = (car - '0') + pas;

                    if (nv < 0) { nv = 9; }
                    if (nv > 9) { nv = 0; }

                    txtGain[menu.digit] = (char)('0' + nv);
                    *gain = atof(txtGain);
                }
                menuAffichage();
                
            }
            /* ----- DROITE : curseur vers la droite ---------------------- */
            else if (frontDroite)
            {
                if (menu.digit == 0)      { menu.digit = 2; }
                else if (menu.digit == 2) { menu.digit = 3; }
                else if (menu.digit == 3) { menu.digit = 4; }
                else                      { menu.digit = 0; }

                menuAffichage();
            }
            /* ----- GAUCHE : retour / reculer le curseur ----------------- */
            else if (frontGauche)
            {
                if (menu.digit == 0)
                {
                    /* sortie de l?édition -------------------------------- */
                    menu.state = MENU_BROWSE;
                    menuAffichage();   /* (sauvegarde éventuelle ici) */
                }
                else
                {
                    if (menu.digit == 4)      { menu.digit = 3; }
                    else if (menu.digit == 3) { menu.digit = 2; }
                    else                      { menu.digit = 0; }

                    menuAffichage();
                }
            }

            break;
        }
    } /* fin switch */
}


void lcdShowPos(void)
{
    char buf[22];

    lcd_gotoxy(1, 1);          /* Ligne 1, colonne 1 */

    sprintf(buf, "X:%04u Y:%04u       ", appData.posPixelX, appData.posPixelY);

    printf_lcd(buf);           /* Pas de lcd_clear() pour éviter le clignotement */
}

void menuInit(void)
{
    // TODO : PID_LoadFromFlash();
    menuAffichage();
}

void menuTache(void)
{
    if(menuForceRefresh)            // flag levé par APP_Tasks()    
    {
        menuForceRefresh = false;    // on le consomme               
        lcd_clear();
        menuAffichage();             // redessine la page courante   
    } 
    menuGestionBouton();          // lecture + FSM          
}

uint8_t menuActif(void)
{
    return (menu.state != MENU_OFF);
}
/* *****************************************************************************
 End of File
 */
