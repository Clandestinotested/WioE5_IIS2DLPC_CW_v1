#include "app_cw.h"
#include "main.h"

static uint8_t cw_active = 0U;
static uint32_t cw_start_ms = 0U;
static uint32_t cw_phase_start_ms = 0U;
static uint32_t cw_phase = 0U;

void App_CW_Init(void)
{
    cw_active = 0U;
    cw_start_ms = 0U;
    cw_phase_start_ms = 0U;
    cw_phase = 0U;
}

void App_CW_Start(void)
{
    cw_start_ms = HAL_GetTick();
    cw_phase_start_ms = cw_start_ms;
    cw_phase = 0U;
    cw_active = 1U;
}

void App_CW_Stop(void)
{
    cw_active = 0U;
    cw_phase = 0U;
}

uint8_t App_CW_IsActive(void)
{
    return cw_active;
}

void App_CW_Task(void)
{
    uint32_t now = HAL_GetTick();
    uint32_t elapsed = 0U;
    uint8_t tx_on = 0U;

    if (cw_active == 0U) {
        return;
    }

    elapsed = now - cw_start_ms;
    if (elapsed >= CW_TOTAL_TIME_MS) {
        cw_active = 0U;
        return;
    }

    /* 600 ms ON / 1000 ms OFF pattern */
    uint32_t cycle = elapsed % (CW_ON_TIME_MS + CW_OFF_TIME_MS);
    tx_on = (cycle < CW_ON_TIME_MS) ? 1U : 0U;

    if (tx_on) {
        /* Replace with your actual SUBGHZ tx enable call here.
         * Example conceptually:
         * SubGHz_SetTxContinuousWave(...);
         */
    } else {
        /* Replace with your radio OFF call here.
         * Example conceptually:
         * SubGHz_StopTx();
         */
    }
}
