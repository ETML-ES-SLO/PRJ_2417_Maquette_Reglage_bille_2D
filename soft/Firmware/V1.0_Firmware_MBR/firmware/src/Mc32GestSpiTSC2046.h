#ifndef Mc32GestSpiTSC2046_H
#define Mc32GestSpiTSC2046_H
/*--------------------------------------------------------*/
/*  Gestion SPI du contrôleur d’écran résistif TSC2046E   */
/*--------------------------------------------------------*/
/*  Auteur  :  Bucher M. Madapté pour TSC2046E, 05/2025)
 *  Version :  V1.0
 *  Cible   :  PIC32MX230F256D  (Harmony 1.06 / XC32 v1.40)
 *--------------------------------------------------------*/

#include <stdint.h>

/* --- Commandes de conversion (12 bits, mode différentiel, PD=0) --- */
//#define TSC_CMD_Y_POS 0x90    /* 1 0 0 1  0 0 0 0  => A2A1A0=001  Y+, Y‑  :contentReference[oaicite:0]{index=0} */
//#define TSC_CMD_X_POS 0xD0    /* 1 1 0 1  0 0 0 0  => A2A1A0=101  X+, X‑  :contentReference[oaicite:1]{index=1} */
#define TSC_CMD_Y_POS 0x93    /* 1 0 0 1  0 0 1 1  => A2A1A0=001  Y+, Y‑  :contentReference[oaicite:0]{index=0} */
#define TSC_CMD_X_POS 0xD3    /* 1 1 0 1  0 0 1 1  => A2A1A0=101  X+, X‑  :contentReference[oaicite:1]{index=1} */


/* Broche CS du TSC : à adapter si besoin ---------------------------- */
//#define CS_TSC LATBbits.LATB15

/* --- Prototypes ---------------------------------------------------- */
void SPI_InitTSC2046(void);
uint16_t TSC2046_ReadRaw(uint8_t cmd);
uint16_t TSC2046_ReadRawX(void);
uint16_t TSC2046_ReadRawY(void);
uint16_t TSC2046_ReadPixel(uint16_t raw, uint16_t rawMin, uint16_t rawMax);
//float rawEnPente(uint16_t raw, uint16_t rawMin, uint16_t rawMax);
uint16_t rawToPulse(uint16_t raw, uint16_t rawMin, uint16_t rawMax);


uint16_t PRegulation(uint16_t raw, uint16_t zero, float direction, float kp);
uint16_t IRegulation(uint16_t raw,  uint16_t zero, float direction, float ki, float Ts, int axe);      /* 0 = X, 1 = Y */
int16_t DRegulation(uint16_t raw, uint16_t zero, float direction, float kd, float Ts, int axe);
uint16_t PIRegulation(uint16_t raw,  uint16_t zero, float direction, float kp,float ki, float Ts, int axe);     /* 0 = X, 1 = Y */
uint16_t PDRegulation(uint16_t raw,  uint16_t zero, float direction,float kp, float kd, float Ts, int axe);
uint16_t PIDRegulation(uint16_t raw, uint16_t zero, float direction,float kp, float ki,float kd, float Ts, int axe);



#endif
