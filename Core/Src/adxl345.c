#include "adxl345.h"
#define ADXL345_ADDRESS      (0x53 << 1)

#define ADXL345_DEVID        0x00
#define ADXL345_POWER_CTL    0x2D
#define ADXL345_DATA_FORMAT  0x31

#define ADXL345_DATAX0       0x32

static HAL_StatusTypeDef ADXL345_WriteRegister(
        I2C_HandleTypeDef *hi2c,
        uint8_t reg,
        uint8_t value)
{
    return HAL_I2C_Mem_Write(
            hi2c,
            ADXL345_ADDRESS,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            &value,
            1,
            HAL_MAX_DELAY);
}

static HAL_StatusTypeDef ADXL345_ReadRegister(
        I2C_HandleTypeDef *hi2c,
        uint8_t reg,
        uint8_t *value)
{
    return HAL_I2C_Mem_Read(
            hi2c,
            ADXL345_ADDRESS,
            reg,
            I2C_MEMADD_SIZE_8BIT,
            value,
            1,
            HAL_MAX_DELAY);
}


HAL_StatusTypeDef ADXL345_ReadID(I2C_HandleTypeDef *hi2c,
                                 uint8_t *deviceID)
{
    return ADXL345_ReadRegister(
            hi2c,
            ADXL345_DEVID,
            deviceID
            );
}



HAL_StatusTypeDef ADXL345_Init(I2C_HandleTypeDef *hi2c)
{
    uint8_t id;

    if (ADXL345_ReadID(hi2c, &id) != HAL_OK)
        return HAL_ERROR;

    if (id != 0xE5)
        return HAL_ERROR;

    return ADXL345_WriteRegister(
                hi2c,
                ADXL345_POWER_CTL,
                0x08);
}

HAL_StatusTypeDef ADXL345_ReadXYZ(I2C_HandleTypeDef *hi2c,
                                  int16_t *x,
                                  int16_t *y,
                                  int16_t *z)
{
    uint8_t data[6];

    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
            hi2c,
            ADXL345_ADDRESS,
            ADXL345_DATAX0,
            I2C_MEMADD_SIZE_8BIT,
            data,
            6,
            HAL_MAX_DELAY);

    if (status != HAL_OK)
    {
        return status;
    }

    *x = (int16_t)((data[1] << 8) | data[0]);
    *y = (int16_t)((data[3] << 8) | data[2]);
    *z = (int16_t)((data[5] << 8) | data[4]);

    return HAL_OK;
}
