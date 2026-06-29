/*
 * fan_control.h
 *
 *  Created on: Jun 29, 2026
 *      Author: anas
 */

#ifndef INC_FAN_CONTROL_H_
#define INC_FAN_CONTROL_H_
#include <stdint.h>
#include "stm32f4xx_hal.h"
void Fan_Init(TIM_HandleTypeDef *htim, uint32_t channel);
void Fan_SetDutyPercent(uint8_t dutyPercent);
void Fan_Stop(void);

#endif /* INC_FAN_CONTROL_H_ */
