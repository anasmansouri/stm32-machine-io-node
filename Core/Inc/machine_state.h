/*
 * machine_state.h
 *
 *  Created on: Jun 3, 2026
 *      Author: anas
 */

#ifndef INC_MACHINE_STATE_H_
#define INC_MACHINE_STATE_H_
#include "telemetry_data.h"
#include "shared_types.h"


void Machine_EvaluateRuntimeState(const TelemetryData *telemetry,
                                  MachineState *state,
                                  FaultCode_t *fault);
void Machine_ApplyStateOutputs(MachineState state);
int Machine_SetLoadThresholds(int warningThreshold, int faultThreshold);
int Machine_GetLoadWarningThreshold(void);
int Machine_GetLoadFaultThreshold(void);
int Machine_GetTempWarningThreshold(void);
int Machine_GetTempFaultThreshold(void);
#endif /* INC_MACHINE_STATE_H_ */
