#ifndef APP_SUBGHZ_H
#define APP_SUBGHZ_H

#include <stdint.h>
#include "stm32wle5xx_hal.h"
#include "radio.h"
#include "subghz_phy.h"

/* SUBGHZ Configuration */
#define SUBGHZ_FREQ_868MHZ         868000000UL
#define SUBGHZ_FREQ_915MHZ         915000000UL
#define SUBGHZ_TX_POWER            14          /* 14 dBm */
#define SUBGHZ_BANDWIDTH           125000      /* 125 kHz */
#define SUBGHZ_DATARATE            4800        /* 4.8 kbps */
#define SUBGHZ_FDEV                4800        /* 4.8 kHz frequency deviation */

/* Default to 868 MHz (Europe) */
#ifndef SUBGHZ_FREQUENCY
#define SUBGHZ_FREQUENCY           SUBGHZ_FREQ_868MHZ
#endif

void SubGHz_Init(void);
void SubGHz_TX_Start_CW(void);
void SubGHz_TX_Stop(void);
void SubGHz_SetFrequency(uint32_t freq_hz);
void SubGHz_SetTxPower(int8_t power_dbm);

#endif
