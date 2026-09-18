/*--------------------------------------------------------*/
//      Mc32DriverLcd.h
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

#ifndef __MC32DRIVERLCD_H
#define __MC32DRIVERLCD_H

#include "system_config/default/system_definitions.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

#define LCD_RS_W      LATAbits.LATA4          // write 0/1
#define LCD_RS_R      PORTAbits.RA4           // read
#define LCD_RS_T      TRISAbits.TRISA4        // 1=input, 0=output

#define LCD_RW_W      LATAbits.LATA7
#define LCD_RW_R      PORTAbits.RA7
#define LCD_RW_T      TRISAbits.TRISA7

#define LCD_E_W       LATAbits.LATA8
#define LCD_E_R       PORTAbits.RA8
#define LCD_E_T       TRISAbits.TRISA8

#define LCD_BL_W      LATAbits.LATA9
#define LCD_BL_T      TRISAbits.TRISA9        // pas besoin de read

/* ------------- LCD data (4-bit) -------- */
#define LCD_DB4_W     LATCbits.LATC2
#define LCD_DB4_R     PORTCbits.RC2
#define LCD_DB4_T     TRISCbits.TRISC2

#define LCD_DB5_W     LATCbits.LATC3
#define LCD_DB5_R     PORTCbits.RC3
#define LCD_DB5_T     TRISCbits.TRISC3

#define LCD_DB6_W     LATCbits.LATC4
#define LCD_DB6_R     PORTCbits.RC4
#define LCD_DB6_T     TRISCbits.TRISC4

#define LCD_DB7_W     LATCbits.LATC5
#define LCD_DB7_R     PORTCbits.RC5
#define LCD_DB7_T     TRISCbits.TRISC5

/*--------------------------------------------------------*/
// Définition des fonctions prototypes
/*--------------------------------------------------------*/
void lcd_init( void );
uint8_t lcd_read_byte( void );
void lcd_send_nibble( uint8_t n );
void lcd_send_byte( uint8_t address, uint8_t n );
void lcd_gotoxy( uint8_t x, uint8_t y);
void lcd_putc( uint8_t c);
void lcd_put_string_ram( char *ptr_char );
//void lcd_put_string_rom( const char *ptr_char );
char lcd_getc( uint8_t x, uint8_t y);
void printf_lcd( const char *format,  ...);
void lcd_ClearLine( unsigned char lineNr );
void lcd_bl_on( void );
void lcd_bl_off( void );

void lcd_clear(void);          /* efface l?écran complet */

void lcd_ClignotementOff(void);
void lcd_ClignotementOn(void);



#endif /* __MC32DRIVERLCD_H */
