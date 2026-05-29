/*
 * load_sensor.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */
#include "load_sensor.h"
int LoadSensor_ReadPercent(ADC_HandleTypeDef *hadc){

	HAL_ADC_Start(hadc);
	int load=-1;
	if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
	{
		uint32_t adcRaw = HAL_ADC_GetValue(hadc);
	    load = (adcRaw * 100) / 4095;
	}
    HAL_ADC_Stop(hadc);
    return load;
}




