#ifndef APP_SUBGHZ_H
#define APP_SUBGHZ_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32wle5xx_hal.h"
#include "radio.h"

/* SUBGHZ Frequency Configuration */
#define SUBGHZ_FREQ_868MHZ         868000000UL    /* Europe */
#define SUBGHZ_FREQ_915MHZ         915000000UL    /* USA/Australia */
#define SUBGHZ_FREQ_DEFAULT        SUBGHZ_FREQ_868MHZ

/* SUBGHZ TX Parameters */
#define SUBGHZ_TX_POWER_DBM        14             /* 14 dBm */
#define SUBGHZ_FSK_BANDWIDTH       125000U        /* 125 kHz */
#define SUBGHZ_FSK_DATARATE        4800U          /* 4.8 kbps */
#define SUBGHZ_FSK_FDEV            4800U          /* 4.8 kHz frequency deviation */
#define SUBGHZ_FSK_PREAMBLE_LEN    8U             /* 8 bytes preamble */

void SubGHz_Init(void);
void SubGHz_TX_Start_CW(uint32_t freq_hz, int8_t power_dbm);
void SubGHz_TX_Stop(void);
void SubGHz_SetFrequency(uint32_t freq_hz);
void SubGHz_SetTxPower(int8_t power_dbm);
uint8_t SubGHz_IsInitialized(void);

#endif
