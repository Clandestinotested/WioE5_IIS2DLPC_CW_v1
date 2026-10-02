#include "main.h"
#include <stdio.h>

I2C_HandleTypeDef hi2c2;
UART_HandleTypeDef huart1;
volatile uint8_t motion_event_pending = 0U;
volatile uint8_t motion_axis = 0U;

static void MX_GPIO_Init(void);

static void PrintfUart(const char *msg)
{
    if (msg == NULL) {
        return;
    }

    HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

static uint8_t I2C_ReadReg8(uint8_t reg, uint8_t *value)
{
    return HAL_I2C_Mem_Read(&hi2c2, IIS2DLPC_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, value, 1, SENSOR_I2C_TIMEOUT_MS);
}

static uint8_t I2C_WriteReg8(uint8_t reg, uint8_t value)
{
    return HAL_I2C_Mem_Write(&hi2c2, IIS2DLPC_I2C_ADDR, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, SENSOR_I2C_TIMEOUT_MS);
}

static void App_I2C2_Config(void)
{
    MX_I2C2_Init();
}

static void App_Sensor_Init(void)
{
    uint8_t whoami = 0;

    if (I2C_ReadReg8(IIS2DLPC_WHO_AM_I, &whoami) != HAL_OK) {
        PrintfUart("IIS2DLPC: WHO_AM_I read failed\r\n");
        return;
    }

    if (whoami != 0x44U) {
        PrintfUart("IIS2DLPC: unexpected WHO_AM_I value\r\n");
        return;
    }

    /* 50Hz data rate, normal mode */
    I2C_WriteReg8(IIS2DLPC_CTRL1, ODR_50Hz);

    /* Wake-up threshold ~0.08g for 5 LSB example */
    I2C_WriteReg8(IIS2DLPC_WAKE_UP_THS, WAKE_UP_EN | 0x05U);

    /* Minimal wake duration */
    I2C_WriteReg8(IIS2DLPC_WAKE_UP_DUR, 0x01U);

    /* Route wake-up interrupt to INT1 */
    I2C_WriteReg8(IIS2DLPC_MD1_CFG, INT1_WU);

    PrintfUart("IIS2DLPC: any-motion wake-up configured\r\n");
}

static void App_Sensor_Process(void)
{
    uint8_t src = 0U;
    uint8_t axis = 0U;

    if (motion_event_pending == 0U) {
        return;
    }

    motion_event_pending = 0U;

    if (I2C_ReadReg8(IIS2DLPC_WAKE_UP_SRC, &src) != HAL_OK) {
        PrintfUart("IIS2DLPC: WAKE_UP_SRC read failed\r\n");
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

    char dbg[96];
    snprintf(dbg, sizeof(dbg), "Motion detected axis mask=%u (X=%d Y=%d Z=%d)\r\n",
             axis,
             (axis & MOTION_AXIS_X) ? 1 : 0,
             (axis & MOTION_AXIS_Y) ? 1 : 0,
             (axis & MOTION_AXIS_Z) ? 1 : 0);
    PrintfUart(dbg);
}

static void App_CW_Start(void)
{
    /* Placeholder: this is where direct SUBGHZ radio TX continuous wave control would be configured.
       Real implementation depends on the exact STM32WLE5 radio driver used in your project. */
    PrintfUart("CW pulse sequence start: 600ms ON / 1000ms OFF / total 15s\r\n");
}

static void App_CW_Stop(void)
{
    PrintfUart("CW pulse sequence stop\r\n");
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_9) {
        motion_event_pending = 1U;
    }
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    App_I2C2_Config();
    MX_USART1_UART_Init();

    PrintfUart("Wio-E5 mini startup\r\n");
    App_Sensor_Init();
    App_CW_Start();

    while (1) {
        App_Sensor_Process();
        HAL_Delay(10);
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio_init;

    gpio_init.Pin = GPIO_PIN_9;
    gpio_init.Mode = GPIO_MODE_IT_RISING;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = GPIO_PIN_15;
    gpio_init.Mode = GPIO_MODE_AF_OD;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = GPIO_PIN_15;
    gpio_init.Mode = GPIO_MODE_AF_OD;
    gpio_init.Pull = GPIO_PULLUP;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &gpio_init);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void MX_I2C2_Init(void)
{
    hi2c2.Instance = I2C2;
    hi2c2.Init.Timing = 0x00707CBBU;
    hi2c2.Init.OwnAddress1 = 0x00U;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.OwnAddress2 = 0x00U;
    hi2c2.Init.OwnAddress2Masks = 0x00U;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
    if (HAL_I2C_Init(&hi2c2) != HAL_OK) {
        Error_Handler();
    }
}

void MX_USART1_UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart1.Init.ClockPhase = UART_PHASE_1EDGE;
    huart1.Init.Clock polarity = UART_POLARITY_LOW;

    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    /* PB6 = TX, PB7 = RX */
    GPIO_InitTypeDef gpio_init;
    gpio_init.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_LOW;
    gpio_init.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &gpio_init);
}

void App_Init_Sensor(void)
{
    App_Sensor_Init();
}

void App_Process_Motion(void)
{
    App_Sensor_Process();
}

void App_Start_CW_Pulse_Sequence(void)
{
    App_CW_Start();
}

void App_Stop_CW(void)
{
    App_CW_Stop();
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1) {
    }
}

void MemManage_Handler(void)
{
    while (1) {
    }
}

void BusFault_Handler(void)
{
    while (1) {
    }
}

void UsageFault_Handler(void)
{
    while (1) {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}
