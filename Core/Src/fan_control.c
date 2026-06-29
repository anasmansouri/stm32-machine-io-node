/*
 * fan_control.c
 *
 *  Created on: Jun 29, 2026
 *      Author: anas
 */

#include "fan_control.h"

static TIM_HandleTypeDef *fanTimer = NULL;
static uint32_t fanChannel = 0;

void Fan_Init(TIM_HandleTypeDef *htim, uint32_t channel){
	fanTimer =htim;
	fanChannel=channel;
}
void Fan_SetDutyPercent(uint8_t dutyPercent){

	if(fanTimer==NULL){

		return;
	}

	if(dutyPercent>100){

		dutyPercent=100;
	}

	 uint32_t compare = ((fanTimer->Init.Period + 1) * dutyPercent) / 100;
	 __HAL_TIM_SET_COMPARE(fanTimer, fanChannel, compare);
}
void Fan_Stop(void){
	Fan_SetDutyPercent(0);
}
