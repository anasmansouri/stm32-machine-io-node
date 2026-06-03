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
/* Default load thresholds */
static int loadWarningThreshold = 75;
static int loadFaultThreshold = 90;

/* Default temperature thresholds */
static int tempWarningThreshold = 35;
static int tempFaultThreshold = 45;

void Machine_EvaluateRuntimeState(const TelemetryData *telemetry,
                                  MachineState *state,
                                  FaultCode_t *fault);
int Machine_SetLoadThresholds(int warningThreshold, int faultThreshold);
int Machine_GetLoadWarningThreshold(void);
int Machine_GetLoadFaultThreshold(void);
int Machine_GetTempWarningThreshold(void);
int Machine_GetTempFaultThreshold(void);
#endif /* INC_MACHINE_STATE_H_ */
