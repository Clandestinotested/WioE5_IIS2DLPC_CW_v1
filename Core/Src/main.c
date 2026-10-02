#include "main.h"
#include "app_motion.h"
#include "app_cw.h"
#include <stdio.h>
#include <string.h>

I2C_HandleTypeDef hi2c2;
UART_HandleTypeDef huart1;
volatile uint8_t motion_event_pending = 0U;
volatile uint8_t motion_axis = 0U;

static void PrintfUart(const char *msg)
{
    if (msg == NULL) {
        return;
    }
    HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)strlen(msg), HAL_MAX_DELAY);
}

void SystemClock_Config(void)
{
    /*
     * IMPORTANT:
     * This is a project skeleton. Use STM32CubeMX to generate the real clock tree for the exact board.
     * The example below keeps the code structurally valid and follows the same HAL flow as a generated project.
     */
    __HAL_RCC_HSE_CONFIG(RCC_HSE_ON);
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) == RESET) {
    }

    __HAL_RCC_PLL_ENABLE();
    while (__HAL_RCC_GET_FLAG(RCC_FLAG_PLLRDY) == RESET) {
    }

    __HAL_RCC_SYSCLK_CONFIG(RCC_SYSCLKSOURCE_PLLCLK);
}

void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void MX_I2C2_Init(void)
{
    __HAL_RCC_I2C2_CLK_ENABLE();

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

    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_15;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_15;
    gpio.Mode = GPIO_MODE_AF_OD;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOB, &gpio);
}

void MX_USART1_UART_Init(void)
{
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200U;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;

    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOB, &gpio);
}

void App_Init(void)
{
    App_Motion_Init();
    App_CW_Init();
    PrintfUart("Wio-E5 mini startup\r\n");
}

void App_Task(void)
{
    App_Motion_Task();
    App_CW_Task();
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_I2C2_Init();
    MX_USART1_UART_Init();
    App_Init();

    while (1) {
        App_Task();
        HAL_Delay(10U);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_9) {
        motion_event_pending = 1U;
        App_Motion_NotifyInterrupt();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void) { while (1) {} }
void MemManage_Handler(void) { while (1) {} }
void BusFault_Handler(void) { while (1) {} }
void UsageFault_Handler(void) { while (1) {} }
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
}
