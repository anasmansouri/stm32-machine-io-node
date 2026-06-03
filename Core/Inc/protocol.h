/*
 * protocol.h
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_
#include "shared_types.h"
#include "telemetry_data.h"
#include "help_functions.h"
#include <stdint.h>
#include <stdbool.h>
void Protocol_HandleCommand(const char *cmd,
                            const TelemetryData *telemetry,
							MachineState *state,
							FaultCode_t *fault,
                            char *response,
                            uint32_t responseSize);
#endif /* INC_PROTOCOL_H_ */
