#include "app_subghz.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* SUBGHZ State */
static uint32_t subghz_frequency = SUBGHZ_FREQ_DEFAULT;
static int8_t subghz_tx_power = SUBGHZ_TX_POWER_DBM;
static uint8_t subghz_initialized = 0U;

static void UART_PrintfDebug(const char *fmt, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)strlen(buffer), HAL_MAX_DELAY);
}

/* STM32CubeWL v1.5.0 Radio Event Callbacks */

static void OnTxDone(void)
{
    /* TX transmission complete */
}

static void OnTxTimeout(void)
{
    /* TX timeout occurred */
    UART_PrintfDebug("[RADIO] TX timeout\r\n");
}

static void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t LoraSnr_FskCfo)
{
    /* RX reception complete (not used in CW mode) */
}

static void OnRxTimeout(void)
{
    /* RX timeout occurred */
}

static void OnRxError(void)
{
    /* RX error occurred */
}

static void OnFhssChangeChannel(uint8_t currentChannel)
{
    /* FHSS channel change callback (not used) */
}

static void OnCadDone(bool channelActivityDetected)
{
    /* CAD done callback (not used) */
}

/* Initialize SUBGHZ Radio with STM32CubeWL v1.5.0 Driver */
void SubGHz_Init(void)
{
    if (subghz_initialized == 1U) {
        return;
    }

    UART_PrintfDebug("[RADIO] Initializing STM32CubeWL v1.5.0 radio driver...\r\n");

    /* Create RadioEvents structure and populate callbacks */
    RadioEvents_t RadioEvents;
    RadioEvents.TxDone = OnTxDone;
    RadioEvents.TxTimeout = OnTxTimeout;
    RadioEvents.RxDone = OnRxDone;
    RadioEvents.RxTimeout = OnRxTimeout;
    RadioEvents.RxError = OnRxError;
    RadioEvents.FhssChangeChannel = OnFhssChangeChannel;
    RadioEvents.CadDone = OnCadDone;

    /* Initialize radio driver with callbacks */
    Radio.Init(&RadioEvents);

    /* Set initial frequency */
    Radio.SetChannel(subghz_frequency);

    /* Configure FSK TX parameters:
     * - MODEM_FSK: FSK modulation
     * - subghz_tx_power: TX power in dBm
     * - SUBGHZ_FSK_FDEV: Frequency deviation (4.8 kHz)
     * - SUBGHZ_FSK_BANDWIDTH: Bandwidth (125 kHz)
     * - SUBGHZ_FSK_DATARATE: Data rate (4.8 kbps)
     * - 0: Coding rate (N/A for FSK)
     * - SUBGHZ_FSK_PREAMBLE_LEN: Preamble length (8 bytes)
     * - false: Variable length packets
     * - true: CRC enabled
     * - false: No frequency hopping
     * - 0: Hopping period (N/A)
     * - false: IQ not inverted
     * - 3000: TX timeout (3000 ms)
     */
    Radio.SetTxConfig(MODEM_FSK,
                      subghz_tx_power,
                      SUBGHZ_FSK_FDEV,
                      SUBGHZ_FSK_BANDWIDTH,
                      SUBGHZ_FSK_DATARATE,
                      0,
                      SUBGHZ_FSK_PREAMBLE_LEN,
                      false,
                      true,
                      false,
                      0,
                      false,
                      3000);

    UART_PrintfDebug("[RADIO] Radio initialized (freq=%lu Hz, power=%d dBm)\r\n",
                     subghz_frequency, subghz_tx_power);

    subghz_initialized = 1U;
}

/* Set Operating Frequency */
void SubGHz_SetFrequency(uint32_t freq_hz)
{
    subghz_frequency = freq_hz;
    if (subghz_initialized == 1U) {
        Radio.SetChannel(freq_hz);
        UART_PrintfDebug("[RADIO] Frequency set to %lu Hz\r\n", freq_hz);
    }
}

/* Set TX Power */
void SubGHz_SetTxPower(int8_t power_dbm)
{
    subghz_tx_power = power_dbm;
    if (subghz_initialized == 1U) {
        Radio.SetTxConfig(MODEM_FSK,
                          power_dbm,
                          SUBGHZ_FSK_FDEV,
                          SUBGHZ_FSK_BANDWIDTH,
                          SUBGHZ_FSK_DATARATE,
                          0,
                          SUBGHZ_FSK_PREAMBLE_LEN,
                          false,
                          true,
                          false,
                          0,
                          false,
                          3000);
        UART_PrintfDebug("[RADIO] TX power set to %d dBm\r\n", power_dbm);
    }
}

/* Start Continuous Wave Transmission */
void SubGHz_TX_Start_CW(uint32_t freq_hz, int8_t power_dbm)
{
    if (subghz_initialized == 0U) {
        SubGHz_Init();
    }

    /* Set frequency before CW */
    Radio.SetChannel(freq_hz);

    /* Start CW transmission with indefinite duration (0xFFFF seconds = ~18 hours)
     * The CW will be stopped by the application calling SubGHz_TX_Stop()
     */
    Radio.SetTxContinuousWave(freq_hz, power_dbm, 0xFFFFU);

    UART_PrintfDebug("[RADIO] CW TX started (freq=%lu Hz, power=%d dBm)\r\n", freq_hz, power_dbm);
}

/* Stop Transmission and Return to Standby */
void SubGHz_TX_Stop(void)
{
    if (subghz_initialized == 0U) {
        return;
    }

    /* Process any pending radio interrupts */
    Radio.IrqProcess();

    /* Put radio in standby mode (low power, no TX/RX) */
    Radio.Standby();

    UART_PrintfDebug("[RADIO] Radio in standby (CW stopped)\r\n");
}

/* Get Initialization State */
uint8_t SubGHz_IsInitialized(void)
{
    return subghz_initialized;
}
