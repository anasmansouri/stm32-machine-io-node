/*
 * protocol.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#include "protocol.h"
#include <string.h>
#include <stdio.h>

#include <inttypes.h>

#include "machine_state.h"

static void Protocol_SetFaultAndNack(const char *command,
									 FaultCode_t detectedFault,
									 MachineState *state,
									 FaultCode_t *fault,
									 char *response,
									 uint32_t responseSize)
{
	*fault = detectedFault;
	*state = MACHINE_STATE_FAULT;

	snprintf(response,
			 responseSize,
			 "NACK:%s:%s\r\n",
			 command,
			 fault_status_to_string(*fault));
}

static FaultCode_t Protocol_GetActiveFault(const TelemetryData *telemetry)
{
	if (telemetry->emergency_button != EMERGENCY_BUTTON_RELEASED)
	{
		return FAULT_EMERGENCY_STOP;
	}

	if (telemetry->dhtStatus != DHT_OK)
	{
		return FAULT_DHT_SENSOR_ERROR;
	}

	if (telemetry->loadStatus != LOAD_OK)
	{
		return FAULT_LOAD_SENSOR_ERROR;
	}

	if (telemetry->load >= Machine_GetLoadFaultThreshold())
	{
		return FAULT_LOAD_TOO_HIGH;
	}

	if (telemetry->temperature >= Machine_GetTempFaultThreshold())
	{
		return FAULT_OVERTEMPERATURE;
	}

	if (telemetry->vibration_level_mg >= Machine_GetVibrationLevelFaultThreshold())
	{
		return FAULT_VIBRATION_HIGH;
	}

	return FAULT_NONE;
}
void Protocol_HandleCommand(const char *cmd,
							const TelemetryData *telemetry,
							MachineState *state,
							FaultCode_t *fault,
							char *response,
							uint32_t responseSize)
{
	if (cmd == NULL || telemetry == NULL || state == NULL ||
		fault == NULL || response == NULL || responseSize == 0)
	{
		return;
	}

	MachineState previousState = *state;

	if (strcmp(cmd, "PING") == 0)
	{
		snprintf(response, responseSize, "ACK:PING\r\n");
	}
	else if (strcmp(cmd, "START_MACHINE") == 0)
	{
		if (*state == MACHINE_STATE_FAULT)
		{
			snprintf(response, responseSize, "NACK:START_MACHINE:%s\r\n", fault_status_to_string(*fault));
		}
		else if (*state != MACHINE_STATE_IDLE)
		{
			snprintf(response, responseSize, "NACK:START_MACHINE:NOT_IDLE\r\n");
		}
		else
		{

			FaultCode_t active_fault = Protocol_GetActiveFault(telemetry);
			if (active_fault != FAULT_NONE)
			{
				Protocol_SetFaultAndNack(cmd,
										 active_fault,
										 state,
										 fault,
										 response,
										 responseSize);
			}
			else
			{
				*fault = FAULT_NONE;
				*state = MACHINE_STATE_RUNNING;
				Machine_EvaluateRuntimeState(telemetry, state, fault);
				snprintf(response, responseSize, "ACK:START_MACHINE\r\n");
			}
		}
	}
	else if (strcmp(cmd, "STOP_MACHINE") == 0)
	{
		/* STOP is always accepted, but it does not clear a latched fault. */
		if (*state != MACHINE_STATE_FAULT)
		{
			*state = MACHINE_STATE_IDLE;
			*fault = FAULT_NONE;
		}
		snprintf(response, responseSize, "ACK:STOP_MACHINE\r\n");
	}
	else if (strcmp(cmd, "RESET_FAULT") == 0)
	{
		if (*state != MACHINE_STATE_FAULT)
		{
			snprintf(response,
					 responseSize,
					 "NACK:RESET_FAULT:NO_ACTIVE_FAULT\r\n");
		}
		else
		{
			FaultCode_t active_fault = Protocol_GetActiveFault(telemetry);
			if (active_fault != FAULT_NONE)
			{
				Protocol_SetFaultAndNack(cmd,
										 active_fault,
										 state,
										 fault,
										 response,
										 responseSize);
			}
			else
			{
				*fault = FAULT_NONE;
				*state = MACHINE_STATE_IDLE;
				snprintf(response,
						 responseSize,
						 "ACK:RESET_FAULT\r\n");
			}
		}
	}
	else if (strncmp(cmd, "SET_LOAD_THRESHOLD:", strlen("SET_LOAD_THRESHOLD:")) == 0)
	{
		int warn = 0;
		int faultValue = 0;

		int parsed = sscanf(cmd,
							"SET_LOAD_THRESHOLD:WARN=%d;FAULT=%d",
							&warn,
							&faultValue);

		if (parsed != 2)
		{
			snprintf(response,
					 responseSize,
					 "NACK:SET_LOAD_THRESHOLD:INVALID_FORMAT\r\n");
		}
		else if (warn < 0 || warn > 100 ||
				 faultValue < 0 || faultValue > 100 ||
				 warn >= faultValue)
		{
			snprintf(response,
					 responseSize,
					 "NACK:SET_LOAD_THRESHOLD:INVALID_RANGE\r\n");
		}
		else
		{
			Machine_SetLoadThresholds(warn, faultValue);
			Machine_EvaluateRuntimeState(telemetry, state, fault);
			snprintf(response,
					 responseSize,
					 "ACK:SET_LOAD_THRESHOLD\r\n");
		}
	}
	else if (strcmp(cmd, "GET_STATUS") == 0)
	{
		snprintf(response,
				 responseSize,
				 "STATUS:TEMP=%d;HUM=%d;LOAD=%d;"
				 "VIB_X=%" PRId32 ";VIB_Y=%" PRId32 ";VIB_Z=%" PRId32 ";VIB_LEVEL=%" PRId32 ";"
				 "fanRPM=%" PRIu32 ";emergency_button=%d;"
				 "STATE=%s;FAULT=%s;OPERATING_MODE=AUTO_MODE;DHT_STATUS=%s;LOAD_STATUS=%s\r\n",
				 telemetry->temperature,
				 telemetry->humidity,
				 telemetry->load,
				 telemetry->vibrationX_mg,
				 telemetry->vibrationY_mg,
				 telemetry->vibrationZ_mg,
				 telemetry->vibration_level_mg,
				 telemetry->fan_rpm,
				 telemetry->emergency_button ? 1 : 0,
				 machine_status_to_string(*state),
				 fault_status_to_string(*fault),
				 dht_status_to_string(telemetry->dhtStatus),
				 load_status_to_string(telemetry->loadStatus));
	}
	else
	{
		snprintf(response, responseSize, "NACK:UNKNOWN_CMD\r\n");
	}

	if (*state != previousState)
	{
		Machine_ApplyStateOutputs(*state);
	}
}
