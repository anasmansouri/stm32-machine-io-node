/*
 * machine_state.c
 *
 *  Created on: Jun 3, 2026
 *      Author: anas
 */

#include "machine_state.h"
#include <stddef.h>
#include "status_led.h"

/* Default load thresholds */
static int loadWarningThreshold = 75;
static int loadFaultThreshold = 90;

/* Default temperature thresholds */
static int tempWarningThreshold = 35;
static int tempFaultThreshold = 45;

/* Default vibration thresholds */
static int vibrationWarningThresholdMg = 1800;
static int vibrationFaultThresholdMg = 2500;

int Machine_SetLoadThresholds(int warningThreshold, int faultThreshold)
{
    if (warningThreshold < 0 || warningThreshold > 100 ||
        faultThreshold < 0 || faultThreshold > 100 ||
        warningThreshold >= faultThreshold)
    {
        return 0;
    }

    loadWarningThreshold = warningThreshold;
    loadFaultThreshold = faultThreshold;

    return 1;
}
void Machine_ApplyStateOutputs(MachineState state)
{
    switch (state)
    {
        case MACHINE_STATE_IDLE:
            StatusLed_AllOff();
            break;

        case MACHINE_STATE_RUNNING:
            StatusLed_Green();
            break;

        case MACHINE_STATE_WARNING:
            StatusLed_Yellow();
            break;

        case MACHINE_STATE_FAULT:
            StatusLed_Red();
            break;

        default:
            StatusLed_AllOff();
            break;
    }
}

int Machine_GetLoadWarningThreshold(void)
{
    return loadWarningThreshold;
}

int Machine_GetLoadFaultThreshold(void)
{
    return loadFaultThreshold;
}

int Machine_GetTempWarningThreshold(void)
{
    return tempWarningThreshold;
}

int Machine_GetTempFaultThreshold(void)
{
    return tempFaultThreshold;
}

int Machine_GetVibrationLevelFaultThreshold(void)
{
    return vibrationFaultThresholdMg;
}

int Machine_GetVibrationLevelWarningThreshold(void)
{
    return vibrationWarningThresholdMg;
}

void Machine_EvaluateRuntimeState(const TelemetryData *telemetry,
                                  MachineState *state,
                                  FaultCode_t *fault)
{

    if (telemetry == NULL || state == NULL || fault == NULL)
    {
        return;
    }
    MachineState previousState = *state;

    /*
     * IDLE means machine is stopped.
     * Do not automatically move from IDLE to WARNING/FAULT only because sensors change.
     * START_MACHINE is responsible for leaving IDLE.
     */
    if (*state == MACHINE_STATE_IDLE)
    {
        return;
    }

    /*
     * FAULT is latched.
     * Do not clear it automatically here.
     * RESET_FAULT is responsible for clearing FAULT.
     */
    if (*state == MACHINE_STATE_FAULT)
    {
        return;
    }

    /*
     * Runtime supervision only applies while machine is active.
     */
    if (*state == MACHINE_STATE_RUNNING || *state == MACHINE_STATE_WARNING)
    {
    	if(telemetry->emergency_button!=EMERGENCY_BUTTON_RELEASED){
    		*fault = FAULT_EMERGENCY_STOP;
    		*state = MACHINE_STATE_FAULT;
    	}

    	else if (telemetry->dhtStatus != DHT_OK)
        {
            *fault = FAULT_DHT_SENSOR_ERROR;
            *state = MACHINE_STATE_FAULT;
        }
        else if (telemetry->loadStatus != LOAD_OK)
        {
            *fault = FAULT_LOAD_SENSOR_ERROR;
            *state = MACHINE_STATE_FAULT;
        }
        else if (telemetry->load >= loadFaultThreshold)
        {
            *fault = FAULT_LOAD_TOO_HIGH;
            *state = MACHINE_STATE_FAULT;
        }
        else if (telemetry->temperature >= tempFaultThreshold)
        {
            *fault = FAULT_OVERTEMPERATURE;
            *state = MACHINE_STATE_FAULT;
        }
        else if(telemetry->vibration_level_mg>=vibrationFaultThresholdMg)
        {
        	*fault = FAULT_VIBRATION_HIGH;
            *state = MACHINE_STATE_FAULT;
        }
        else if (telemetry->load >= loadWarningThreshold ||
                 telemetry->temperature >= tempWarningThreshold||
				 telemetry->vibration_level_mg>=vibrationWarningThresholdMg)
        {
            *fault = FAULT_NONE;
            *state = MACHINE_STATE_WARNING;
        }
        else
        {
            *fault = FAULT_NONE;
            *state = MACHINE_STATE_RUNNING;
        }
    }
    if(previousState!=*state){
    	Machine_ApplyStateOutputs(*state);
    }
}
