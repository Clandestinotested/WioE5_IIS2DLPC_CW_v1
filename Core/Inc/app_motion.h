#ifndef APP_MOTION_H
#define APP_MOTION_H

#include <stdint.h>

void App_Motion_Init(void);
void App_Motion_Task(void);
void App_Motion_NotifyInterrupt(void);
uint8_t App_Motion_GetAxisMask(void);

#endif
