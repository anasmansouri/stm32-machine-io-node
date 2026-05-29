/*
 * protocol.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#include "protocol.h"
#include "status_led.h"
#include "string.h"
#include "stdio.h"
void Protocol_HandleCommand(const char *cmd,
                            const TelemetryData *telemetry,
                            char *response,
                            uint32_t responseSize){


	if (strcmp(cmd, "PING") == 0)
	    {
	      snprintf(response,responseSize,"ACK:PING\r\n");
	    }

	    else if (strcmp(cmd, "SET_LED:RED") == 0)
	    {
	      StatusLed_Red();
	      snprintf(response,responseSize,"ACK:SET_LED:RED\r\n");
	    }
	    else if (strcmp(cmd, "SET_LED:GREEN") == 0)
	    {
	      StatusLed_Green();
	      snprintf(response,responseSize,"ACK:SET_LED:GREEN\r\n");
	    }
	    else if (strcmp(cmd, "SET_LED:YELLOW") == 0)
	    {
	      StatusLed_Yellow();
	      snprintf(response,responseSize,"ACK:SET_LED:YELLOW\r\n");
	    }
	    else if (strcmp(cmd, "SET_LED:OFF") == 0)
	    {
	      StatusLed_AllOff();
	      snprintf(response,responseSize,"ACK:SET_LED:OFF\r\n");
	    }
	    else if (strcmp(cmd, "GET_STATUS") == 0)
	    {

	    	snprintf(response,
	               responseSize,
	               "STATUS:TEMP=%d;HUM=%d;LOAD=%d;DHT_STATUS=%s;LOAD_STATUS=%s;SYSTEM_STATUS=%s\r\n",
				   telemetry->temperature,
				   telemetry->humidity,
				   telemetry->load,
				   dht_status_to_string(telemetry->dhtStatus),
				   load_status_to_string(telemetry->loadStatus),
				   system_status_to_string(telemetry->systemStatus)
				   );
	    }
	    else
	    {
	      snprintf(response,responseSize,"NACK:UNKNOWN_CMD\r\n");
	    }
}
