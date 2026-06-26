#ifndef MOVING_AVERAGE_H
#define MOVING_AVERAGE_H

#include <stdint.h>

#define MOVING_AVERAGE_SIZE 5

typedef struct
{
    int32_t buffer[MOVING_AVERAGE_SIZE];
    uint8_t index;
    int32_t sum;
} MovingAverageFilter;

void MovingAverage_Init(MovingAverageFilter *filter);

int32_t MovingAverage_Update(MovingAverageFilter *filter,
                             int32_t sample);

#endif
