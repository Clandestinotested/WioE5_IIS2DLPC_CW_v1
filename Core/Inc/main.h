#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "stm32wle5xx_hal.h"

/* I2C2 Configuration */
#define IIS2DLPC_I2C_ADDR            0x18U
#define IIS2DLPC_I2C_TIMEOUT_MS      50U

/* Motion Axis Flags */
#define MOTION_AXIS_X                (1U << 0)
#define MOTION_AXIS_Y                (1U << 1)
#define MOTION_AXIS_Z                (1U << 2)

/* IIS2DLPC Register Map */
#define IIS2DLPC_WHO_AM_I            0x0FU
#define IIS2DLPC_CTRL1               0x20U
#define IIS2DLPC_CTRL2               0x21U
#define IIS2DLPC_CTRL3               0x22U
#define IIS2DLPC_WAKE_UP_THS         0x34U
#define IIS2DLPC_WAKE_UP_DUR         0x35U
#define IIS2DLPC_MD1_CFG             0x5EU
#define IIS2DLPC_WAKE_UP_SRC         0x1BU

/* IIS2DLPC Wake-up Status Bits */
#define IIS2DLPC_X_WU                (1U << 0)
#define IIS2DLPC_Y_WU                (1U << 1)
#define IIS2DLPC_Z_WU                (1U << 2)
#define IIS2DLPC_WU_IA               (1U << 3)

/* IIS2DLPC Configuration Values */
#define ODR_50Hz                     0x10U
#define WAKE_UP_EN                   0x40U
#define INT1_WU                      0x20U

/* Global HAL Handles */
extern I2C_HandleTypeDef hi2c2;
extern UART_HandleTypeDef huart1;

/* Global Motion State */
extern volatile uint8_t motion_event_pending;
extern volatile uint8_t motion_axis;

/* Function Prototypes */
void Error_Handler(void);
void SystemClock_Config(void);
void HAL_MspInit(void);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);
void HAL_UART_MspInit(UART_HandleTypeDef *huart);
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif
