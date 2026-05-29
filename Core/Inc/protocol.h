/*
 * protocol.h
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "main.h"
#include "telemetry_data.h"
#include <stdint.h>
#include "string.h"
#include "stdio.h"
void Protocol_HandleCommand(const char *cmd,
                            const TelemetryData *telemetry,
                            char *response,
                            uint32_t responseSize);

#endif /* INC_PROTOCOL_H_ */
