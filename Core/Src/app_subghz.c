#include "app_subghz.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* SUBGHZ Configuration State */
static uint32_t subghz_frequency = SUBGHZ_FREQUENCY;
static int8_t subghz_tx_power = SUBGHZ_TX_POWER;
static uint8_t subghz_initialized = 0U;

static void UART_PrintfDebug(const char *fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)strlen(buffer), HAL_MAX_DELAY);
}

/* RadioEvents callback structure (required by STM32CubeWL radio driver) */
static RadioEvents_t RadioEvents;

static void OnTxDone(void)
{
    /* TX transmission complete callback */
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    /* RX reception callback (not used in CW mode) */
}

static void OnTxTimeout(void)
{
    /* TX timeout callback */
}

static void OnRxTimeout(void)
{
    /* RX timeout callback (not used) */
}

static void OnRxError(void)
{
    /* RX error callback (not used) */
}

void SubGHz_Init(void)
{
    if (subghz_initialized == 1U) {
        return;
    }

    /* Initialize radio event callbacks */
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;

    /* Initialize the radio driver */
    Radio.Init(&RadioEvents);

    /* Set default frequency */
    Radio.SetChannel(subghz_frequency);

    /* Set TX power */
    Radio.SetTxConfig(MODEM_FSK, subghz_tx_power, SUBGHZ_FDEV, SUBGHZ_BANDWIDTH,
                      SUBGHZ_DATARATE, 0, 10, false, false, 0, 0, false, 1000);

    UART_PrintfDebug("SUBGHZ initialized (freq=%lu Hz, power=%d dBm)\r\n", subghz_frequency, subghz_tx_power);

    subghz_initialized = 1U;
}

void SubGHz_SetFrequency(uint32_t freq_hz)
{
    subghz_frequency = freq_hz;
    if (subghz_initialized == 1U) {
        Radio.SetChannel(freq_hz);
        UART_PrintfDebug("SUBGHZ frequency set to %lu Hz\r\n", freq_hz);
    }
}

void SubGHz_SetTxPower(int8_t power_dbm)
{
    subghz_tx_power = power_dbm;
    if (subghz_initialized == 1U) {
        Radio.SetTxConfig(MODEM_FSK, power_dbm, SUBGHZ_FDEV, SUBGHZ_BANDWIDTH,
                          SUBGHZ_DATARATE, 0, 10, false, false, 0, 0, false, 1000);
        UART_PrintfDebug("SUBGHZ TX power set to %d dBm\r\n", power_dbm);
    }
}

void SubGHz_TX_Start_CW(void)
{
    if (subghz_initialized == 0U) {
        SubGHz_Init();
    }

    /* Enter Transmit Continuous Wave mode */
    Radio.SetTxContinuousWave(subghz_frequency, subghz_tx_power, 0xFFFFFFFF);

    UART_PrintfDebug("[SUBGHZ] TX CW started (freq=%lu Hz, power=%d dBm)\r\n", subghz_frequency, subghz_tx_power);
}

void SubGHz_TX_Stop(void)
{
    if (subghz_initialized == 0U) {
        return;
    }

    /* Stop transmission and put radio in STDBY mode */
    Radio.IrqProcess();
    Radio.Standby();

    UART_PrintfDebug("[SUBGHZ] TX CW stopped (radio in STDBY)\r\n");
}
