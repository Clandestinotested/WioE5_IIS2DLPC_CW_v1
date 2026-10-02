#include "app_cw.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* CW state machine */
static uint8_t cw_active = 0U;
static uint32_t cw_start_ms = 0U;
static uint32_t cw_elapsed_ms = 0U;

static void UART_PrintfDebug(const char *fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)strlen(buffer), HAL_MAX_DELAY);
}

/* Placeholder for actual SUBGHZ radio control.
 * These functions should be implemented with the actual STM32WLE5 SUBGHZ driver API. */

static void SubGHz_TX_Start_CW(void)
{
    /* TODO: Replace with actual SUBGHZ driver call.
     *
     * Example (pseudo-code):
     *   SubGHz_TX_SetFrequency(868000000);  // 868 MHz
     *   SubGHz_TX_SetPower(14);             // 14 dBm
     *   SubGHz_TX_StartContinuousWave();
     *
     * Actual implementation depends on your STM32CubeWL version.
     */
    UART_PrintfDebug("  [SUBGHZ] TX CW START\r\n");
}

static void SubGHz_TX_Stop_CW(void)
{
    /* TODO: Replace with actual SUBGHZ driver call.
     *
     * Example (pseudo-code):
     *   SubGHz_TX_Stop();
     *   SubGHz_SetMode(SUBGHZ_MODE_IDLE);
     */
    UART_PrintfDebug("  [SUBGHZ] TX CW STOP\r\n");
}

void App_CW_Init(void)
{
    cw_active = 0U;
    cw_start_ms = 0U;
    cw_elapsed_ms = 0U;
}

void App_CW_Start(void)
{
    if (cw_active == 0U) {
        cw_start_ms = HAL_GetTick();
        cw_active = 1U;
        UART_PrintfDebug("CW sequence started (15s total: 600ms ON / 1000ms OFF)\r\n");
        SubGHz_TX_Start_CW();
    }
}

void App_CW_Stop(void)
{
    if (cw_active == 1U) {
        cw_active = 0U;
        SubGHz_TX_Stop_CW();
        UART_PrintfDebug("CW sequence stopped\r\n");
    }
}

uint8_t App_CW_IsActive(void)
{
    return cw_active;
}

void App_CW_Task(void)
{
    uint32_t now_ms = HAL_GetTick();
    uint32_t elapsed_total = 0U;
    uint32_t cycle_pos = 0U;
    uint8_t should_tx = 0U;
    static uint8_t last_state = 0U;

    if (cw_active == 0U) {
        return;
    }

    /* Calculate total elapsed time */
    elapsed_total = now_ms - cw_start_ms;

    /* Check if total time exceeded */
    if (elapsed_total >= CW_TOTAL_TIME_MS) {
        UART_PrintfDebug("CW sequence completed (elapsed=%lums)\r\n", elapsed_total);
        App_CW_Stop();
        return;
    }

    /* Calculate position within the pulse cycle */
    cycle_pos = elapsed_total % (CW_ON_TIME_MS + CW_OFF_TIME_MS);

    /* Determine if we should transmit in this cycle */
    should_tx = (cycle_pos < CW_ON_TIME_MS) ? 1U : 0U;

    /* State change detection and action */
    if (should_tx != last_state) {
        if (should_tx == 1U) {
            SubGHz_TX_Start_CW();
            UART_PrintfDebug("  [%05lums] CW ON\r\n", elapsed_total);
        } else {
            SubGHz_TX_Stop_CW();
            UART_PrintfDebug("  [%05lums] CW OFF\r\n", elapsed_total);
        }
        last_state = should_tx;
    }
}
