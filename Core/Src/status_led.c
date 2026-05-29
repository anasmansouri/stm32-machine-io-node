/*
 * status_led.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#include "status_led.h"

void StatusLed_AllOff(void)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);   // Yellow
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);   // Green
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);  // Red
}

void StatusLed_Green(void)
{
    StatusLed_AllOff();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);    	// Green
}

void StatusLed_Yellow(void)
{
    StatusLed_AllOff();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);   	// Yellow
}

void StatusLed_Red(void)
{
    StatusLed_AllOff();
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);   // Red
}
