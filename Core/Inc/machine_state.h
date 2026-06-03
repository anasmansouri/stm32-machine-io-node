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
#endif /* INC_MACHINE_STATE_H_ */
