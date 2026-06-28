/*
 * help_functions.c
 *
 *  Created on: Jun 3, 2026
 *      Author: anas
 */

#include "help_functions.h"

const char* fault_status_to_string(FaultCode_t status) {
    switch (status) {
        case FAULT_NONE:              return "FAULT_NONE";
        case FAULT_LOAD_TOO_HIGH: 		  return "FAULT_LOAD_TOO_HIGH";
        case FAULT_OVERTEMPERATURE: 		  return "FAULT_OVERTEMPERATURE";
        case FAULT_DHT_SENSOR_ERROR: 		  return "FAULT_DHT_SENSOR_ERROR";
        case FAULT_LOAD_SENSOR_ERROR: 		  return "FAULT_LOAD_SENSOR_ERROR";
        case FAULT_DOOR_OPEN: 		  return "FAULT_DOOR_OPEN";
        case FAULT_OBSTACLE_DETECTED: 		  return "FAULT_OBSTACLE_DETECTED";
        case FAULT_EMERGENCY_STOP:			return "FAULT_EMERGENCY_STOP";
        case FAULT_VIBRATION_HIGH: return "FAULT_VIBRATION_HIGH";
        default:                  return "FAULT_UNKNOWN_STATUS";
    }
}
const char* machine_status_to_string(MachineState status){
    switch (status) {
        case MACHINE_STATE_IDLE:              return "MACHINE_STATE_IDLE";
        case MACHINE_STATE_RUNNING: 		  return "MACHINE_STATE_RUNNING";
        case MACHINE_STATE_WARNING: 		  return "MACHINE_STATE_WARNING";
        case MACHINE_STATE_FAULT: 		  return "MACHINE_STATE_FAULT";
        default:                  return "MACHINE_UNKNOWN_STATUS";
    }

}
