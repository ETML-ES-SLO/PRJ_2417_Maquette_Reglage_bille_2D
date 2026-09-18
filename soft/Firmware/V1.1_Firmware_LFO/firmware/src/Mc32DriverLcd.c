/*--------------------------------------------------------*/
//      Mc32DriverLcd.c
/*--------------------------------------------------------*/
//	Description :	Driver pour LCD PIC 32 sur starter-kit ES
//
//	Auteur 		: 	Gomes Andres
//	Compilateur	:	XC32 V1.33 & harmony 1.00
//
//  MODIFICATIONS :
//   CHR 15.05.2014   retouche lcd_putc
//   CHR 15.05.2014   ajout lcd_printf
//   CHR 02.12.2104   ajout lcd_ClearLine
//   CHR 09.09.2014   utilisation de bsp_cponfig.h
//   SCA 11.10.2016   utilisation des délais core timer
//   SCA 31.03.2022   Config des signaux ctrl en sortie
//                    MPLABX 5.45, xc32 2.50, Harmony 2.06
//   SCA 22.10.2024
//    Correction pour cohérence des types
//    Modifications pour compatibilité avec nouveau LCD depuis version 11020E
//     Adaptation delays lors de l'init,
//      et largeur pulse enable à l'écriture et lecture.
//     Modèle LCD v. A à D : BC2004ABNHEB / depuis v.E : NHD-0420AZ-FSW-GBW-33V3
//    MPLABX 5.50, xc32 2.50, Harmony 2.06
//
/*--------------------------------------------------------*/
#include "system_config/default/system_definitions.h"
#include "Mc32DriverLcd.h"
#include "GenericTypeDefs.h" //pout union UINT8_BITS
#include "Mc32Delays.h"

/*--------------------------------------------------------*/
// Définition du tableau pour l'adresse des lignes
/*--------------------------------------------------------*/
const uint8_t taddrLines[5] = { 0,    // ligne 0 pas utilisé
                             0,    // ligne 1 commence au caractère 0
                             64,    // ligne 2 commence au caractère 64 (0x40)
                             20,    // ligne 3 commence au caractère 20
                             84 };  // ligne 4 commence au caractère 84 (0x40+20)

/*--------------------------------------------------------*/
// Fonctions
/*--------------------------------------------------------*/

// Initialise signaux de comm et LCD.
// Resultat = ecran vide, pret pour affichage.
void lcd_init(void)
{
    //met les 4 signaux de controle en sortie (RE0-4 = RS, RW, E, BL)
    LCD_RS_T = 0;
    LCD_RW_T = 0;
    LCD_E_T = 0;
    LCD_BL_T = 0;

    // on va effectuer exactement ce que demande le ks0066
    // on repositionne LCD_E tout pour un démarrage correct
    LCD_E_W = 0;
    delay_usCt(1); // si LCD_E était à 1, on attend
    LCD_RS_W = 0;  // demandé pour une commande
    LCD_RW_W = 0;
    // suivant comment l'interfaçage avec le LCD s'est arrêté, il faut tout remettre à plat
    // selon les notes d'applications, il faut envoyer 3 fois un nibble 0x3
    // pour lui faire croire que nous sommes en interface8 bits
    // chaque envoi doit être séparé de 5ms!!

    delay_msCt(50); //LCD v. E :  Wait >40 ms after power is applied

    lcd_send_nibble(0x03); // correspond à 0x30, interface 8 bits
    delay_msCt(6); // attente > 5 ms
    lcd_send_nibble(0x03); // correspond à 0x30, interface 8 bits
    delay_msCt(6); // attente > 5 ms
    lcd_send_nibble(0x03); // correspond à 0x30, interface 8 bits
    delay_msCt(6); // attente > 5 ms

    // A partir d'ici, on peut lire BF a la place des délais

    //LCD_RS_W = 0;  // demandé pour une commande (on assure!)
    lcd_send_nibble(2);// 4 bits interface

    lcd_send_byte(0,0b00101000); //rs=0, 2 lines mode, display off
    delay_usCt(40); //ds0066 demande >39us

    lcd_send_byte(0,0b00001100); //rs=0, display on, cursor off, blink off
    delay_usCt(40); //ds0066 demande >39us

    lcd_send_byte(0,0b00000001); //rs=0, display clear
    delay_msCt(2); //ds0066 demande >1,53ms

    lcd_send_byte(0,0b00000110); //rs=0, increment mode, entire shift off
    delay_usCt(40); //ds0066 demande >39us
}

/*--------------------------------------------------------*/
// Lit un octet du LCD (registre avec busy flag et addresse)
uint8_t lcd_read_byte( void )
{
    UINT8_BITS lcd_read_byte;
    LCD_DB4_T = 1; // 1=input
    LCD_DB5_T = 1;
    LCD_DB6_T = 1;
    LCD_DB7_T = 1;
    LCD_RW_W = 1;
    delay500nsCt(); //ds0066 demande 0.5us
    LCD_E_W = 1;
    delay500nsCt(); //ds0066 demande 0.5us
    delay500nsCt(); //2e delai nécessaire pour LCD v. E (min 480 ns et delay500nsCt() est un peu juste)
    lcd_read_byte.bits.b7 = LCD_DB7_R;
    lcd_read_byte.bits.b6 = LCD_DB6_R;
    lcd_read_byte.bits.b5 = LCD_DB5_R;
    lcd_read_byte.bits.b4 = LCD_DB4_R;
    LCD_E_W = 0; // attention e pulse min = 500ns à 1 et autant à 0
    delay500nsCt();
    LCD_E_W = 1;
    delay500nsCt();
    delay500nsCt(); //2e delai nécessaire pour LCD v. E (min 480 ns et delay500nsCt() est un peu juste)
    lcd_read_byte.bits.b3 = LCD_DB7_R;
    lcd_read_byte.bits.b2 = LCD_DB6_R;
    lcd_read_byte.bits.b1 = LCD_DB5_R;
    lcd_read_byte.bits.b0 = LCD_DB4_R;
    LCD_E_W = 0;
    delay500nsCt();
    LCD_DB4_T = 0; // 0=output
    LCD_DB5_T = 0;
    LCD_DB6_T = 0;
    LCD_DB7_T = 0;
    return(lcd_read_byte.Val);
}

/*--------------------------------------------------------*/
// Ecrit un groupe de 4 bits a destination du LCD.
// Utilise car mode de comm. 4 bits, chaque octet doit
// etre transmis en 2x.
void lcd_send_nibble( uint8_t n )
{
    UINT8_BITS NibbleToWrite;
    NibbleToWrite.Val = n;
    LCD_DB7_W = NibbleToWrite.bits.b3;
    LCD_DB6_W = NibbleToWrite.bits.b2;
    LCD_DB5_W = NibbleToWrite.bits.b1;
    LCD_DB4_W = NibbleToWrite.bits.b0;
    delay500nsCt();
    LCD_E_W = 1;
    delay500nsCt(); // E pulse width min = 450ns pour le 1!
    delay500nsCt(); // 2e delai nécessaire pour LCD v. E (min 480 ns et delay500nsCt() est un peu juste)
    LCD_E_W = 0;
    delay500nsCt(); // E pulse width min = 450ns également pour le 0!
}

/*--------------------------------------------------------*/
// Ecrit un octet a destination du LCD.
void lcd_send_byte( uint8_t address, uint8_t n )
{
	LCD_RS_W = 0;
	while ( (lcd_read_byte() & 0x80) == 0x80 ) ;
    LCD_RS_W = address;
    LCD_RW_W = 0;
    //LCD_E déjà à 0
    lcd_send_nibble(n >> 4);
    lcd_send_nibble(n & 0xf);
}

/*--------------------------------------------------------*/
// Positionne curseur aux coordonnees x,y
// 1ere ligne = no 1, colonnes idem
void lcd_gotoxy( uint8_t x, uint8_t y)
{
   uint8_t address;
   address = taddrLines[y];
   address+=x-1;
   lcd_send_byte(0,0x80|address);
}

/*--------------------------------------------------------*/
// Ecrit un caractere sur LCD
// Attention, traitement des \f form feed, \n new line et \b backspage
// Modif du 15.05.2014 C. Huber
void lcd_putc( uint8_t c)
{
	switch (c)
	{
    	case '\f'   : 	lcd_send_byte(0,1);	break;  // modif du (1,1) en (0,1)
     	case '\n'   : 	lcd_gotoxy(1,2);        break;
     	case '\b'   : 	lcd_send_byte(0,0x10);	break;
     	default     : 	lcd_send_byte(1,c);		break;
   }
}

/*--------------------------------------------------------*/
// Affiche chaine passee en param. a l'endroit du curseur
void lcd_put_string_ram( char *ptr_char )
{
	while (*ptr_char != 0)
	{
    	lcd_putc(*ptr_char);
    	ptr_char++;
  	}
}

/*--------------------------------------------------------*/
//void lcd_put_string_rom( const char *ptr_char )
//{
//	while (*ptr_char != 0)
//	{
//    	lcd_putc(*ptr_char);
//    	ptr_char++;
//  	}
//}

/*--------------------------------------------------------*/
// Retourne le caractere affiche aux coordonnees x,y
char lcd_getc( uint8_t x, uint8_t y)
{
	uint8_t value;
    lcd_gotoxy(x,y);
    while ( lcd_read_byte() & 0x80 ); // wait until busy flag is low
    LCD_RS_W = 1;
    value = lcd_read_byte();
    LCD_RS_W = 0;
    return(value);
}

/*--------------------------------------------------------*/
// printf_lcd
// Meme format de parametres que printf standard
// Auteur C. Huber 15.05.2014
void printf_lcd( const char *format,  ...)
{
    char Buffer[21];
    va_list args;
    va_start(args, format);

    vsprintf(Buffer, format, args);
    lcd_put_string_ram(Buffer);

    va_end(args);
}

/*--------------------------------------------------------*/
// Rempli une ligne d'espaces
// Ajout a la demande des utilisateurs
// Auteur C. Huber 02.12.2014
void lcd_ClearLine( unsigned char lineNr )
{
    uint8_t i;
    if (lineNr >= 1 && lineNr <= 4)
    {
        lcd_gotoxy( 1, lineNr) ;
        for (i = 0 ; i < 20 ; i++)
        {
            lcd_send_byte(1,0x20);
        }
     }
}

/*--------------------------------------------------------*/
// Allumage backlight
void lcd_bl_on( void )
{
	LCD_BL_W = 1;
}

/*--------------------------------------------------------*/
// Extinction backlight
void lcd_bl_off( void )
{
	LCD_BL_W = 0;
}
void lcd_clear(void)
{
    lcd_send_byte(0, 0x01);     /* commande ?clear display? */
    /* datasheet : délai > 1,53 ms  ? un petit wait */
    delay_msCt(2);
}

/* -------------------------------------------------------------------------- */
/*  Active l?affichage, le curseur et son clignotement                        */
/* -------------------------------------------------------------------------- */
void lcd_ClignotementOn(void)
{
    /* 0x0F : display ON | cursor ON | blink ON                               */
    lcd_send_byte(0, 0x0F);
}

/* -------------------------------------------------------------------------- */
/*  Active l?affichage, mais désactive le curseur et son clignotement         */
/* -------------------------------------------------------------------------- */
void lcd_ClignotementOff(void)
{
    /* 0x0C : display ON | cursor OFF | blink OFF                             */
    lcd_send_byte(0, 0x0C);
}