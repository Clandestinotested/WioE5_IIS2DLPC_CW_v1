#include "app_motion.h"
#include "main.h"
#include <stdio.h>

static uint8_t I2C_ReadReg8(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c2, ((IIS2DLPC_I2C_ADDR << 1) | 0U), reg, I2C_MEMADD_SIZE_8BIT, value, 1U, SENSOR_I2C_TIMEOUT_MS);
}

static uint8_t I2C_WriteReg8(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2, ((IIS2DLPC_I2C_ADDR << 1) | 0U), reg, I2C_MEMADD_SIZE_8BIT, &value, 1U, SENSOR_I2C_TIMEOUT_MS);
}

void App_Motion_Init(void)
{
    uint8_t whoami = 0U;

    if (I2C_ReadReg8(IIS2DLPC_WHO_AM_I, &whoami) != HAL_OK) {
        /* UART debug output can be added here if desired */
        return;
    }

    if (whoami != 0x44U) {
        return;
    }

    I2C_WriteReg8(IIS2DLPC_CTRL1, ODR_50Hz);
    I2C_WriteReg8(IIS2DLPC_WAKE_UP_THS, WAKE_UP_EN | 0x05U);
    I2C_WriteReg8(IIS2DLPC_WAKE_UP_DUR, 0x01U);
    I2C_WriteReg8(IIS2DLPC_MD1_CFG, INT1_WU);
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

    if (motion_event_pending == 0U) {
        return;
    }

    motion_event_pending = 0U;

    if (I2C_ReadReg8(IIS2DLPC_WAKE_UP_SRC, &src) != HAL_OK) {
        return;
    }

    if ((src & IIS2DLPC_WU_IA) == 0U) {
        return;
    }

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
}
