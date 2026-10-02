#ifndef APP_CW_H
#define APP_CW_H

#include <stdint.h>

#define CW_ON_TIME_MS     600U
#define CW_OFF_TIME_MS    1000U
#define CW_TOTAL_TIME_MS  15000U

void App_CW_Init(void);
void App_CW_Start(void);
void App_CW_Stop(void);
void App_CW_Task(void);
uint8_t App_CW_IsActive(void);

#endif
