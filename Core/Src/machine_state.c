/*
 * machine_state.c
 *
 *  Created on: Jun 3, 2026
 *      Author: anas
 */

#include "machine_state.h"
void Machine_EvaluateRuntimeState(const TelemetryData *telemetry,
                                  MachineState *state,
                                  FaultCode_t *fault)
{
    if (telemetry == NULL || state == NULL || fault == NULL)
    {
        return;
    }

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
        if (telemetry->dhtStatus != DHT_OK)
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
        else if (telemetry->load >= loadWarningThreshold ||
                 telemetry->temperature >= tempWarningThreshold)
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
}
