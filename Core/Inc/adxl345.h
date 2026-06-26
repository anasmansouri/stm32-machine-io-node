#ifndef ADXL345_H
#define ADXL345_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef ADXL345_ReadID(I2C_HandleTypeDef *hi2c,
                                 uint8_t *deviceID);

HAL_StatusTypeDef ADXL345_ReadXYZ(I2C_HandleTypeDef *hi2c,
                                  int16_t *x,
                                  int16_t *y,
                                  int16_t *z);

#endif
