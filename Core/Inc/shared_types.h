/*
 * shared_types.h
 *
 *  Created on: Jun 3, 2026
 *      Author: anas
 */

#ifndef INC_SHARED_TYPES_H_
#define INC_SHARED_TYPES_H_

typedef struct
{
  char text[32];
} CommandMessage;

typedef enum{
	MACHINE_STATE_IDLE,
	MACHINE_STATE_RUNNING,
	MACHINE_STATE_WARNING,
	MACHINE_STATE_FAULT
}MachineState;

typedef enum
{
    FAULT_NONE,
    FAULT_LOAD_TOO_HIGH,
    FAULT_OVERTEMPERATURE,
    FAULT_DHT_SENSOR_ERROR,
	FAULT_LOAD_SENSOR_ERROR,
    FAULT_DOOR_OPEN,
    FAULT_OBSTACLE_DETECTED
} FaultCode_t;
#endif /* INC_SHARED_TYPES_H_ */
