#include "moving_average.h"

void MovingAverage_Init(MovingAverageFilter *filter)
{
    filter->index = 0;
    filter->sum = 0;
    filter->count = 0;

    for (uint8_t i = 0; i < MOVING_AVERAGE_SIZE; i++)
    {
        filter->buffer[i] = 0;
    }
}

int32_t MovingAverage_Update(MovingAverageFilter *filter,
                             int32_t sample)
{
    if (filter->count < MOVING_AVERAGE_SIZE)
    {
        filter->count++;
    }
    else
    {
        filter->sum -= filter->buffer[filter->index];
    }

    filter->buffer[filter->index] = sample;
    filter->sum += sample;

    filter->index++;

    if (filter->index >= MOVING_AVERAGE_SIZE)
    {
        filter->index = 0;
    }

    return filter->sum / filter->count;
}
