#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include "stm32wle5xx_hal.h"

#define IIS2DLPC_I2C_ADDR            0x18U
#define IIS2DLPC_I2C_ADDR_WRITE      ((IIS2DLPC_I2C_ADDR << 1) | 0U)
#define IIS2DLPC_I2C_ADDR_READ       ((IIS2DLPC_I2C_ADDR << 1) | 1U)
#define SENSOR_I2C_TIMEOUT_MS         50U

#define MOTION_AXIS_X                 (1U << 0)
#define MOTION_AXIS_Y                 (1U << 1)
#define MOTION_AXIS_Z                 (1U << 2)

#define IIS2DLPC_WHO_AM_I            0x0FU
#define IIS2DLPC_CTRL1               0x20U
#define IIS2DLPC_CTRL2               0x21U
#define IIS2DLPC_CTRL3               0x22U
#define IIS2DLPC_WAKE_UP_THS         0x34U
#define IIS2DLPC_WAKE_UP_DUR         0x35U
#define IIS2DLPC_MD1_CFG             0x5EU
#define IIS2DLPC_WAKE_UP_SRC         0x1BU

#define IIS2DLPC_X_WU                (1U << 0)
#define IIS2DLPC_Y_WU                (1U << 1)
#define IIS2DLPC_Z_WU                (1U << 2)
#define IIS2DLPC_WU_IA               (1U << 3)

#define ODR_50Hz                     0x10U
#define WAKE_UP_EN                   0x40U
#define INT1_WU                      0x20U

extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;
extern volatile uint8_t motion_event_pending;
extern volatile uint8_t motion_axis;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_I2C2_Init(void);
void MX_USART1_UART_Init(void);
void App_Init(void);
void App_Task(void);
void App_CW_Start(void);
void App_CW_Stop(void);
void App_CW_Task(void);

#endif
