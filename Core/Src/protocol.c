/*
 * protocol.c
 *
 *  Created on: May 29, 2026
 *      Author: anas
 */

#include "protocol.h"
#include <string.h>
#include <stdio.h>


/* Default load thresholds */
static int loadWarningThreshold = 75;
static int loadFaultThreshold = 90;

/* Default temperature thresholds */
static int tempWarningThreshold = 35;
static int tempFaultThreshold = 45;

void Protocol_HandleCommand(const char *cmd,
                            const TelemetryData *telemetry,
							MachineState *state,
							FaultCode_t *fault,
                            char *response,
                            uint32_t responseSize){


	if (strcmp(cmd, "PING") == 0)
	    {
	      snprintf(response,responseSize,"ACK:PING\r\n");
	    }else if(strcmp(cmd,"START_MACHINE")==0){
	    	if(*state==MACHINE_STATE_FAULT){
	    		snprintf(response,responseSize,"NACK:START_MACHINE:%s\r\n",fault_status_to_string(*fault));
	    	}else if(*state!=MACHINE_STATE_IDLE){
	    		snprintf(response,responseSize,"NACK:START_MACHINE:NOT_IDLE\r\n");
	    	}else if(telemetry->dhtStatus!=DHT_OK){
	    		*fault=FAULT_DHT_SENSOR_ERROR;
	    		*state=MACHINE_STATE_FAULT;
	    		snprintf(response,responseSize,"NACK:START_MACHINE:%s\r\n",fault_status_to_string(*fault));
	    	}else if(telemetry->loadStatus!=LOAD_OK){
	    		*fault=FAULT_LOAD_SENSOR_ERROR;
	    		*state=MACHINE_STATE_FAULT;
	    		snprintf(response,responseSize,"NACK:START_MACHINE:%s\r\n",fault_status_to_string(*fault));
	    	}else if(telemetry->load>=loadFaultThreshold){
	    		*fault=FAULT_LOAD_TOO_HIGH;
	    		*state=MACHINE_STATE_FAULT;
	    		snprintf(response,responseSize,"NACK:START_MACHINE:%s\r\n",fault_status_to_string(*fault));
	    	}else if(telemetry->temperature>=tempFaultThreshold){
	    		*fault=FAULT_OVERTEMPERATURE;
	    		*state=MACHINE_STATE_FAULT;
	    		snprintf(response,responseSize,"NACK:START_MACHINE:%s\r\n",fault_status_to_string(*fault));
	    	}else{
	    		*fault=FAULT_NONE;
	    		if(telemetry->load>=loadWarningThreshold||telemetry->temperature>=tempWarningThreshold){
	    			*state=MACHINE_STATE_WARNING;
	    		}else{
	    			*state=MACHINE_STATE_RUNNING;
	    		}
	    		snprintf(response,responseSize,"ACK:START_MACHINE\r\n");
	    	}
	    }else if(strcmp(cmd,"STOP_MACHINE")==0){
	    	/* STOP is always accepted, but it does not clear a latched fault. */
	    	if(*state!=MACHINE_STATE_FAULT){
	    		*state = MACHINE_STATE_IDLE;
	    		*fault=FAULT_NONE;
	    	}
	    	snprintf(response,responseSize,"ACK:STOP_MACHINE\r\n");
	    }else if (strcmp(cmd, "RESET_FAULT") == 0)
	    {
	        if (*state != MACHINE_STATE_FAULT)
	        {
	            snprintf(response,
	                     responseSize,
	                     "NACK:RESET_FAULT:NO_ACTIVE_FAULT\r\n");
	        }
	        else if (telemetry->dhtStatus != DHT_OK)
	        {
	            *fault = FAULT_DHT_SENSOR_ERROR;

	            snprintf(response,
	                     responseSize,
	                     "NACK:RESET_FAULT:%s\r\n",
	                     fault_status_to_string(*fault));
	        }
	        else if (telemetry->loadStatus != LOAD_OK)
	        {
	            *fault = FAULT_LOAD_SENSOR_ERROR;

	            snprintf(response,
	                     responseSize,
	                     "NACK:RESET_FAULT:%s\r\n",
	                     fault_status_to_string(*fault));
	        }
	        else if (telemetry->load >= loadFaultThreshold)
	        {
	            *fault = FAULT_LOAD_TOO_HIGH;

	            snprintf(response,
	                     responseSize,
	                     "NACK:RESET_FAULT:%s\r\n",
	                     fault_status_to_string(*fault));
	        }
	        else if (telemetry->temperature >= tempFaultThreshold)
	        {
	            *fault = FAULT_OVERTEMPERATURE;

	            snprintf(response,
	                     responseSize,
	                     "NACK:RESET_FAULT:%s\r\n",
	                     fault_status_to_string(*fault));
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
	        	loadWarningThreshold = warn;
	        	loadFaultThreshold = faultValue;

	        	if (*state == MACHINE_STATE_RUNNING || *state == MACHINE_STATE_WARNING)
	        	{
	        	    if (telemetry->load >= loadFaultThreshold)
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
	            snprintf(response,
	                     responseSize,
	                     "ACK:SET_LOAD_THRESHOLD\r\n");
	        }
	    }
	    else if (strcmp(cmd, "GET_STATUS") == 0)
	    {
	    	snprintf(response,
	               responseSize,
	               "STATUS:TEMP=%d;HUM=%d;LOAD=%d;STATE=%s;FAULT=%s;OPERATING_MODE=AUTO_MODE;DHT_STATUS=%s;LOAD_STATUS=%s\r\n",
				   telemetry->temperature,
				   telemetry->humidity,
				   telemetry->load,
				   machine_status_to_string(*state),
				   fault_status_to_string(*fault),
				   dht_status_to_string(telemetry->dhtStatus),
				   load_status_to_string(telemetry->loadStatus)
				   );
	    }
	    else
	    {
	      snprintf(response,responseSize,"NACK:UNKNOWN_CMD\r\n");
	    }
}


