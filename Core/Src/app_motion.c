#include "app_motion.h"
#include "app_subghz.h"
#include "main.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static uint8_t I2C_ReadReg8(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c2, ((IIS2DLPC_I2C_ADDR << 1) | 0U), reg, I2C_MEMADD_SIZE_8BIT, value, 1U, SENSOR_I2C_TIMEOUT_MS);
}

static uint8_t I2C_WriteReg8(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2, ((IIS2DLPC_I2C_ADDR << 1) | 0U), reg, I2C_MEMADD_SIZE_8BIT, &value, 1U, SENSOR_I2C_TIMEOUT_MS);
}

static void UART_PrintfDebug(const char *fmt, ...)
{
    char buffer[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, (uint16_t)strlen(buffer), HAL_MAX_DELAY);
}

void App_Motion_Init(void)
{
    uint8_t whoami = 0U;
    HAL_StatusTypeDef status;

    status = I2C_ReadReg8(IIS2DLPC_WHO_AM_I, &whoami);
    if (status != HAL_OK) {
        UART_PrintfDebug("ERROR: IIS2DLPC WHO_AM_I read failed (status=%d)\r\n", status);
        return;
    }

    if (whoami != 0x44U) {
        UART_PrintfDebug("ERROR: IIS2DLPC WHO_AM_I mismatch. Got 0x%02X, expected 0x44\r\n", whoami);
        return;
    }

    UART_PrintfDebug("IIS2DLPC detected (WHO_AM_I = 0x%02X)\r\n", whoami);

    /* CTRL1: 50 Hz ODR, Normal Mode */
    I2C_WriteReg8(IIS2DLPC_CTRL1, ODR_50Hz);
    UART_PrintfDebug("  CTRL1 set to ODR_50Hz\r\n");

    /* WAKE_UP_THS: Enable wake-up + set threshold to ~0.08g (5 LSB) */
    I2C_WriteReg8(IIS2DLPC_WAKE_UP_THS, WAKE_UP_EN | 0x05U);
    UART_PrintfDebug("  WAKE_UP_THS configured\r\n");

    /* WAKE_UP_DUR: Minimal wake duration */
    I2C_WriteReg8(IIS2DLPC_WAKE_UP_DUR, 0x01U);
    UART_PrintfDebug("  WAKE_UP_DUR configured\r\n");

    /* MD1_CFG: Route wake-up interrupt to INT1 pin */
    I2C_WriteReg8(IIS2DLPC_MD1_CFG, INT1_WU);
    UART_PrintfDebug("  MD1_CFG configured (INT1=wake-up)\r\n");

    UART_PrintfDebug("IIS2DLPC any-motion setup complete\r\n");
}

void App_Motion_NotifyInterrupt(void)
{
    motion_event_pending = 1U;
}

uint8_t App_Motion_GetAxisMask(void)
{
    return motion_axis;
}

void App_Motion_Task(void)
{
    uint8_t src = 0U;
    uint8_t axis = 0U;
    HAL_StatusTypeDef status;

    if (motion_event_pending == 0U) {
        return;
    }

    motion_event_pending = 0U;

    status = I2C_ReadReg8(IIS2DLPC_WAKE_UP_SRC, &src);
    if (status != HAL_OK) {
        UART_PrintfDebug("ERROR: WAKE_UP_SRC read failed\r\n");
        return;
    }

    /* Check if wake-up interrupt activity bit is set */
    if ((src & IIS2DLPC_WU_IA) == 0U) {
        return;
    }

    /* Decode which axis triggered */
    if (src & IIS2DLPC_X_WU) {
        axis |= MOTION_AXIS_X;
    }
    if (src & IIS2DLPC_Y_WU) {
        axis |= MOTION_AXIS_Y;
    }
    if (src & IIS2DLPC_Z_WU) {
        axis |= MOTION_AXIS_Z;
    }

    motion_axis = axis;

    /* Log detected motion and trigger CW sequence */
    UART_PrintfDebug("MOTION DETECTED: axis mask=0x%02X (X=%d Y=%d Z=%d)\r\n",
                     axis,
                     (axis & MOTION_AXIS_X) ? 1 : 0,
                     (axis & MOTION_AXIS_Y) ? 1 : 0,
                     (axis & MOTION_AXIS_Z) ? 1 : 0);

    /* Start CW pulse sequence */
    App_CW_Start();
}
