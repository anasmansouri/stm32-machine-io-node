/*
 * dht11.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#include"dht11.h"
#include "FreeRTOS.h"
#include "task.h"
#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_PIN_6

static void DHT11_SetPinOutput(void);
static void DHT11_SetPinInput(void);
static int DHT11_WaitForState(GPIO_PinState state, uint32_t timeout_us);
static void delay_us(uint32_t us);
static void DWT_Delay_Init(void);
static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

static void DWT_Delay_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DHT11_Init(){
	DWT_Delay_Init();
}

static void delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000);

    while ((DWT->CYCCNT - start) < ticks)
    {
    }
}

static int DHT11_WaitForState(GPIO_PinState state, uint32_t timeout_us)
{
    while (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != state)
    {
        if (timeout_us-- == 0)
        {
            return 0;
        }

        delay_us(1);
    }

    return 1;
}

int DHT11_Read(int *temperature, int *humidity)
{
    uint8_t data[5] = {0};

    DHT11_SetPinInput();

    if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) != GPIO_PIN_SET)
    {
        return -10;  // line is not idle high
    }

    DHT11_SetPinOutput();

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);

    /*
     * Critical timing starts here.
     * no HAL_Delay() is allowed in this section.
     */
    taskENTER_CRITICAL();

    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET);
    delay_us(20);

    DHT11_SetPinInput();

    if (!DHT11_WaitForState(GPIO_PIN_RESET, 500))
    {
        taskEXIT_CRITICAL();
        return -1;
    }

    if (!DHT11_WaitForState(GPIO_PIN_SET, 500))
    {
        taskEXIT_CRITICAL();
        return -2;
    }

    if (!DHT11_WaitForState(GPIO_PIN_RESET, 500))
    {
        taskEXIT_CRITICAL();
        return -3;
    }

    for (int i = 0; i < 40; i++)
    {
        if (!DHT11_WaitForState(GPIO_PIN_SET, 300))
        {
            taskEXIT_CRITICAL();
            return -4;
        }

        delay_us(40);

        if (HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN) == GPIO_PIN_SET)
        {
            data[i / 8] |= (1 << (7 - (i % 8)));

            if (!DHT11_WaitForState(GPIO_PIN_RESET, 300))
            {
                taskEXIT_CRITICAL();
                return -5;
            }
        }
    }

    taskEXIT_CRITICAL();

    uint8_t checksum = data[0] + data[1] + data[2] + data[3];

    if (checksum != data[4])
    {
        return -6;
    }

    *humidity = data[0];
    *temperature = data[2];

    return 1;
}
